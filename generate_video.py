#!/usr/bin/env python3
"""
Updated generate_video.py: default codec set to libx265 (HEVC lossless), improved visuals (additional dynamics, contrast/gamma), retained plasma colormap, and added safe defaults. Falls back to grayscale if matplotlib missing.
"""

import argparse
import subprocess
import shutil
import sys
import numpy as np

try:
    from tqdm import tqdm
except Exception:
    tqdm = lambda x, **k: x

try:
    from matplotlib import cm
    _HAS_MATPLOTLIB = True
except Exception:
    _HAS_MATPLOTLIB = False

def parse_args():
    p = argparse.ArgumentParser(description="Generate an evolving grid video (visually lossless, HEVC default).")
    p.add_argument("--width", "-W", type=int, default=1000, help="frame width in pixels")
    p.add_argument("--height", "-H", type=int, default=1000, help="frame height in pixels")
    p.add_argument("--frames", "-n", type=int, default=10000, help="number of frames to generate")
    p.add_argument("--fps", type=int, default=30, help="frames per second for the video")
    p.add_argument("--seed", type=int, default=1337, help="random seed (reproducible)")
    p.add_argument("--noise", type=float, default=0.00, help="per-frame noise amplitude (0..1)")
    p.add_argument("--contrast", type=float, default=1.0, help="global contrast multiplier")
    p.add_argument("--gamma", type=float, default=1.0, help="gamma correction applied after colormap")
    p.add_argument("--output", "-o", type=str, default="out.mkv", help="output filename")
    p.add_argument("--codec", choices=["ffv1", "libx264", "libx265"], default="libx265",
                   help="lossless codec to use: ffv1, libx264 (H.264 lossless), or libx265 (HEVC lossless - default)")
    p.add_argument("--colormap", type=str, default="plasma",
                   help="colormap name to apply (matplotlib colormap). Use 'gray' for grayscale. Falls back to gray if matplotlib not available.")
    p.add_argument("--preview", action="store_true", help="generate only 300 frames for quick preview")
    return p.parse_args()

def build_ffmpeg_cmd(width, height, fps, output, codec):
    base = [
        "ffmpeg",
        "-y",
        "-f", "rawvideo",
        "-pixel_format", "rgb24",
        "-video_size", f"{width}x{height}",
        "-framerate", str(fps),
        "-i", "-",
    ]
    if codec == "ffv1":
        base += ["-c:v", "ffv1", "-pix_fmt", "rgb24", output]
    elif codec == "libx264":
        base += ["-c:v", "libx264", "-crf", "0", "-preset", "veryslow", "-pix_fmt", "yuv444p", output]
    else:  # libx265
        # Use x265 lossless mode to keep visually lossless HEVC
        base += ["-c:v", "libx265", "-preset", "veryslow", "-x265-params", "lossless=1", "-pix_fmt", "yuv444p", output]
    return base

def evolve_step(grid, noise_amp, t, global_mod):
    # Combination of diffusion, small advection and time-modulated perturbations
    center = 0.36 * grid
    ns = 0.11 * (np.roll(grid, 1, axis=0) + np.roll(grid, -1, axis=0))
    ew = 0.11 * (np.roll(grid, 1, axis=1) + np.roll(grid, -1, axis=1))
    diag = 0.055 * (
        np.roll(np.roll(grid, 1, axis=0), 1, axis=1) +
        np.roll(np.roll(grid, 1, axis=0), -1, axis=1) +
        np.roll(np.roll(grid, -1, axis=0), 1, axis=1) +
        np.roll(np.roll(grid, -1, axis=0), -1, axis=1)
    )
    new = center + ns + ew + diag

    # Add a small coherent sinusoidal modulation that moves over time for visual interest
    h, w = grid.shape
    ys = np.linspace(0, 2 * np.pi, h, endpoint=False)
    xs = np.linspace(0, 2 * np.pi, w, endpoint=False)
    X, Y = np.meshgrid(xs, ys)
    phase = t * 0.02
    wave = 0.02 * np.sin(3.0 * X + 2.0 * Y + phase)
    new += wave * global_mod

    # Add noise
    if noise_amp > 0:
        new += np.random.normal(loc=0.0, scale=noise_amp, size=grid.shape)

    # slight advection/shift to create flowing patterns
    shift_y = int((np.sin(t * 0.001) * 2.0))
    shift_x = int((np.cos(t * 0.0013) * 2.0))
    new = np.roll(new, shift_y, axis=0)
    new = np.roll(new, shift_x, axis=1)

    np.clip(new, 0.0, 1.0, out=new)
    return new

def apply_colormap_and_tone(rgb_f, contrast, gamma, cmap_name):
    # rgb_f expected float in 0..1 for each channel
    # apply contrast and gamma
    rgb = np.clip(rgb_f * contrast, 0.0, 1.0)
    if gamma != 1.0:
        rgb = np.power(rgb, 1.0 / gamma)
    rgb_u8 = (rgb * 255.0).astype(np.uint8)
    return rgb_u8

def map_grid_to_rgb(grid, cmap_name, contrast, gamma):
    if cmap_name is None:
        cmap_name = 'gray'
    if cmap_name.lower() == 'gray' or not _HAS_MATPLOTLIB:
        frame_u8 = (np.clip(grid, 0.0, 1.0) * 255.0).astype(np.uint8)
        rgb = np.stack([frame_u8, frame_u8, frame_u8], axis=2)
        return apply_colormap_and_tone(rgb.astype(np.float32) / 255.0, contrast, gamma, cmap_name)

    try:
        cmap = cm.get_cmap(cmap_name)
    except Exception:
        cmap = cm.get_cmap('plasma')

    rgba = cmap(grid)
    rgbf = rgba[:, :, :3]
    return apply_colormap_and_tone(rgbf, contrast, gamma, cmap_name)

def main():
    args = parse_args()

    width = args.width
    height = args.height
    frames = args.frames if not args.preview else min(args.frames, 300)
    fps = args.fps
    seed = args.seed
    noise_amp = args.noise
    output = args.output
    codec = args.codec
    cmap_name = args.colormap
    contrast = args.contrast
    gamma = args.gamma

    if shutil.which("ffmpeg") is None:
        print("Error: ffmpeg not found on PATH. Install ffmpeg and try again.", file=sys.stderr)
        sys.exit(1)

    if not _HAS_MATPLOTLIB and cmap_name.lower() != 'gray':
        print("Warning: matplotlib not installed, falling back to grayscale despite colormap request.")

    print(f"Generating {frames} frames of {width}x{height} at {fps} fps -> {output}")
    print(f"Codec: {codec}, seed: {seed}, noise_amp: {noise_amp}, colormap: {cmap_name}, contrast: {contrast}, gamma: {gamma}")
    estimated_raw_gb = (width * height * 3 * frames) / (1024**3)
    print(f"Estimated raw streamed data: ~{estimated_raw_gb:.2f} GB (final file will still be large)")

    ffmpeg_cmd = build_ffmpeg_cmd(width, height, fps, output, codec)
    print("Starting ffmpeg with command:")
    print(" ".join(ffmpeg_cmd))

    rng = np.random.default_rng(seed)

    grid = rng.random((height, width), dtype=np.float32)

    proc = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    try:
        for i in tqdm(range(frames), desc="frames"):
            grid = evolve_step(grid, noise_amp, i, global_mod=0.6)
            rgb = map_grid_to_rgb(grid, cmap_name, contrast, gamma)
            proc.stdin.write(rgb.tobytes())

        proc.stdin.close()
        ret = proc.wait()
        if ret != 0:
            print(f"ffmpeg exited with status {ret}", file=sys.stderr)
            sys.exit(ret)
        print("Finished successfully.")
    except KeyboardInterrupt:
        print("Interrupted by user, terminating ffmpeg.")
        proc.terminate()
        proc.wait()
        raise

if __name__ == "__main__":
    main()

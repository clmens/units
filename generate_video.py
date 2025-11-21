#!/usr/bin/env python3
"""
generate_video.py

Generate an evolving 2D grid (width x height) starting from random values,
simulate for N frames, and stream frames to ffmpeg to produce a visually
lossless video.

Usage examples:
  python generate_video.py --width 1000 --height 1000 --frames 10000 --fps 30 --output out.mkv

Requirements:
  - Python 3.8+
  - numpy
  - ffmpeg installed and on PATH
  - tqdm (optional, used if available)
  - matplotlib (optional, for color maps; falls back to grayscale if absent)

Notes:
  - Default codec is ffv1 (lossless) inside an MKV container.
  - Streaming avoids writing thousands of PNG files to disk.
  - Ensure you have plenty of disk space for the final encoded file (tens of GB).
"""

import argparse
import subprocess
import shutil
import sys
import numpy as np

try:
    from tqdm import tqdm
except Exception:
    tqdm = lambda x, **k: x  # fallback to identity iterator if tqdm not installed

# Optional matplotlib for colormaps
try:
    from matplotlib import cm
    _HAS_MATPLOTLIB = True
except Exception:
    _HAS_MATPLOTLIB = False

def parse_args():
    p = argparse.ArgumentParser(description="Generate an evolving grid video (visually lossless).")
    p.add_argument("--width", "-W", type=int, default=1000, help="frame width in pixels")
    p.add_argument("--height", "-H", type=int, default=1000, help="frame height in pixels")
    p.add_argument("--frames", "-n", type=int, default=10000, help="number of frames to generate")
    p.add_argument("--fps", type=int, default=30, help="frames per second for the video")
    p.add_argument("--seed", type=int, default=1337, help="random seed (reproducible)")
    p.add_argument("--noise", type=float, default=0.01, help="per-frame noise amplitude (0..1)")
    p.add_argument("--output", "-o", type=str, default="out.mkv", help="output filename")
    p.add_argument("--codec", choices=["ffv1", "libx264"], default="ffv1",
                   help="lossless codec to use: ffv1 (default) or libx264 (crf 0, yuv444p)")
    p.add_argument("--colormap", type=str, default="plasma",
                   help="colormap name to apply (matplotlib colormap). Use 'gray' for grayscale. Falls back to gray if matplotlib not available.")
    p.add_argument("--preview", action="store_true", help="generate only 300 frames for quick preview")
    return p.parse_args()

def build_ffmpeg_cmd(width, height, fps, output, codec):
    base = [
        "ffmpeg",
        "-y",  # overwrite
        "-f", "rawvideo",
        "-pixel_format", "rgb24",
        "-video_size", f"{width}x{height}",
        "-framerate", str(fps),
        "-i", "-",  # read from stdin
    ]
    if codec == "ffv1":
        # ffv1 is a widely supported lossless codec; keep rgb24 pixel format to avoid subsampling
        base += ["-c:v", "ffv1", "-pix_fmt", "rgb24", output]
    else:
        # libx264 lossless: use yuv444p (no chroma subsampling) and crf 0
        base += ["-c:v", "libx264", "-crf", "0", "-preset", "veryslow", "-pix_fmt", "yuv444p", output]
    return base

def evolve_step(grid, noise_amp):
    # Weighted smoothing:
    # center weight 0.4, N/S/E/W = 0.1, diagonals = 0.05 (sum = 1.0)
    center = 0.4 * grid
    ns = 0.1 * (np.roll(grid, 1, axis=0) + np.roll(grid, -1, axis=0))
    ew = 0.1 * (np.roll(grid, 1, axis=1) + np.roll(grid, -1, axis=1))
    diag = 0.05 * (np.roll(np.roll(grid, 1, axis=0), 1, axis=1) +
                   np.roll(np.roll(grid, 1, axis=0), -1, axis=1) +
                   np.roll(np.roll(grid, -1, axis=0), 1, axis=1) +
                   np.roll(np.roll(grid, -1, axis=0), -1, axis=1))
    new = center + ns + ew + diag
    if noise_amp > 0:
        new += np.random.normal(loc=0.0, scale=noise_amp, size=grid.shape)
    # clamp
    np.clip(new, 0.0, 1.0, out=new)
    return new

def apply_colormap(grid, cmap_name):
    """Map a float32 grid in [0,1] to uint8 RGB using matplotlib colormap (if available).
    Falls back to grayscale if matplotlib not present or cmap_name == 'gray'."""
    if cmap_name is None:
        cmap_name = 'gray'
    if cmap_name.lower() == 'gray' or not _HAS_MATPLOTLIB:
        # grayscale fallback
        frame_u8 = (np.clip(grid, 0.0, 1.0) * 255.0).astype(np.uint8)
        rgb = np.stack([frame_u8, frame_u8, frame_u8], axis=2)
        return rgb
    try:
        cmap = cm.get_cmap(cmap_name)
    except Exception:
        # fallback to plasma if invalid name
        cmap = cm.get_cmap('plasma')

    # cmap expects values in [0,1], returns NxMx4 RGBA floats
    rgba = cmap(grid)  # shape (H, W, 4)
    rgbf = rgba[:, :, :3]
    rgb = (np.clip(rgbf, 0.0, 1.0) * 255.0).astype(np.uint8)
    return rgb

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

    if shutil.which("ffmpeg") is None:
        print("Error: ffmpeg not found on PATH. Install ffmpeg and try again.", file=sys.stderr)
        sys.exit(1)

    if not _HAS_MATPLOTLIB and cmap_name.lower() != 'gray':
        print("Warning: matplotlib not installed, falling back to grayscale despite colormap request.")

    print(f"Generating {frames} frames of {width}x{height} at {fps} fps -> {output}")
    print(f"Codec: {codec}, seed: {seed}, noise_amp: {noise_amp}, colormap: {cmap_name}")
    estimated_raw_gb = (width * height * 3 * frames) / (1024**3)
    print(f"Estimated raw streamed data: ~{estimated_raw_gb:.2f} GB (final file will still be large)")

    ffmpeg_cmd = build_ffmpeg_cmd(width, height, fps, output, codec)
    print("Starting ffmpeg with command:")
    print(" ".join(ffmpeg_cmd))

    rng = np.random.default_rng(seed)

    # initialize grid with random values in [0,1)
    grid = rng.random((height, width), dtype=np.float32)

    # spawn ffmpeg process
    proc = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)

    try:
        for i in tqdm(range(frames), desc="frames"):
            # evolve grid
            grid = evolve_step(grid, noise_amp)

            # apply colormap / convert to uint8 RGB
            rgb = apply_colormap(grid, cmap_name)

            # write raw frame to ffmpeg stdin
            proc.stdin.write(rgb.tobytes())

        # close stdin to signal EOF to ffmpeg
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

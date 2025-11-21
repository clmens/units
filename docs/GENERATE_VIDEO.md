````markdown
```markdown
# Generate Video: evolving grid (visually lossless)

This document explains how to generate a visually-lossless video from an evolving 2D grid using the repository's helper script.

Overview
- Script: `generate_video.py`
- Default parameters create a 1000×1000 grid, 10,000 frames.
- Frames are streamed to `ffmpeg` (no intermediate PNGs).
- Default codec: ffv1 (lossless) in MKV container.
- Default colormap: plasma (matplotlib). Falls back to grayscale if matplotlib not installed.

Prerequisites
- Python 3.8+
- numpy (pip install numpy)
- ffmpeg (system package; must be on PATH)
- matplotlib (optional; pip install matplotlib) for color maps
- tqdm (optional for progress bar)

Quick command (Windows/Linux/macOS)
- Generate full video:
  python generate_video.py --width 1000 --height 1000 --frames 10000 --fps 30 --output out.mkv --colormap plasma
- For quick preview (300 frames):
  python generate_video.py --preview --width 1000 --height 1000 --frames 10000 --fps 30 --output preview.mkv --colormap plasma

Codec choices
- ffv1 (default): lossless, portable, preserves RGB.
- libx264 (lossless): `--codec libx264` (uses crf 0 and yuv444p). Slower and may produce larger files but compatible with many players.

Notes and warnings
- Estimated raw streamed data for default config: ~30 GB. Final encoded file will still be very large.
- Use `--preview` to test the pipeline before running the full job.
- Running 10,000 frames at 1000×1000 is CPU and IO intensive; run on a machine with sufficient RAM, CPU cores and disk space.
```
````
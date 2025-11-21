```markdown
# Generate Video: evolving grid (visually lossless)

Overview
- Script: `generate_video.py`
- Default parameters produce visually-interesting output using plasma colormap and HEVC lossless (libx265) by default.
- Default target used for CI preview: 512×512, 300 frames (uploaded as a preview artifact).

Prerequisites
- Python 3.8+
- numpy (pip install numpy)
- ffmpeg (system package; must be on PATH). On Windows CI we install ffmpeg via Chocolatey; CI will fall back to libx264 if libx265 is not available.
- matplotlib (optional; pip install matplotlib) for color maps
- tqdm (optional for progress bar)

Quick command (local)
- Preview (fast, smaller):
  python generate_video.py --preview --width 512 --height 512 --frames 300 --fps 30 --output preview-512x512.mkv --colormap plasma

- Full (be careful, this is large):
  python generate_video.py --width 1000 --height 1000 --frames 10000 --fps 30 --output out.mkv --colormap plasma

Codec choices
- libx265 (HEVC lossless) — default: visually lossless HEVC using x265 lossless mode.
- libx264 — available: lossless H.264 via crf 0 + yuv444p (used as fallback on CI if libx265 not available).
- ffv1 — alternative lossless codec (MKV/FFV1) remains supported.

Notes and warnings
- Preview (512×512, 300 frames): raw stream ~0.21 GB; encoded HEVC preview expected ~30–200 MB depending on content.
- Full job (1000×1000, 10k frames): raw stream ~30 GB; final file will still be very large — ensure >= 50 GB free and a machine with sufficient CPU.
```
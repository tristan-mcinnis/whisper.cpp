# Whisper Realtime macOS

A streamlined, macOS-only fork of [whisper.cpp](https://github.com/ggml-org/whisper.cpp) optimized for **realtime terminal-based speech transcription**. This version removes all cross-platform complexity to provide a clean, focused tool for voice-to-text on macOS.

## Features

- **Realtime transcription** from microphone with low latency
- **Voice Activity Detection (VAD)** with adjustable sensitivity  
- **Multi-language support** with translation to English
- **Clean signal handling** - Ctrl+C stops gracefully
- **macOS native** - uses Accelerate framework and Metal
- **85% smaller codebase** - removed cross-platform bloat

## Quick Start

```bash
# Install SDL2 (required for audio capture)
brew install sdl2

# Build everything
make

# Download a model
./download-ggml-model.sh base.en

# Start transcribing
./run-whisper --model models/ggml-base.en.bin
```

## Usage

```bash
./run-whisper [options]

Options:
  -h,  --help            show help message
  -m,  --model PATH      model file path
  -l,  --language CODE   language (en, es, fr, de, etc.)
  -t,  --threads N       number of threads
  -c,  --capture ID      capture device ID (-1 = default)
       --vad             enable voice activity detection
       --vad-threshold N VAD threshold (0.0-1.0)
       --translate       translate to English
       --gpu             enable GPU acceleration

Examples:
  ./run-whisper --model models/ggml-base.en.bin
  ./run-whisper --language es --translate
  ./run-whisper --vad --vad-threshold 0.7
```

## Available Models

Download models using the included script:

```bash
# English-only models (faster)
./download-ggml-model.sh tiny.en      # 39 MB
./download-ggml-model.sh base.en      # 142 MB
./download-ggml-model.sh small.en     # 466 MB
./download-ggml-model.sh medium.en    # 1.5 GB

# Multilingual models
./download-ggml-model.sh tiny         # 39 MB
./download-ggml-model.sh base         # 142 MB
./download-ggml-model.sh small        # 466 MB
./download-ggml-model.sh medium       # 1.5 GB
./download-ggml-model.sh large-v3-turbo  # 1.6 GB
```

## What's Different

This fork strips away everything not needed for macOS terminal transcription:

### Removed
- Cross-platform support (Windows, Linux, Android, iOS, WASM)
- Language bindings (Python, JavaScript, Go, Java, Ruby)
- 40+ example programs
- GPU backends (CUDA, Vulkan, OpenCL, SYCL)
- Docker configurations
- Complex build systems

### Kept
- Core whisper engine
- GGML with CPU/Metal/BLAS backends
- SDL2 audio capture
- Essential utilities

### Result
- **~50 files** instead of 300+
- **Simple `make` build** instead of complex CMake
- **One executable** instead of dozens
- **Focused on one thing**: realtime macOS transcription

## Building from Source

Requirements:
- macOS (Intel or Apple Silicon)
- Xcode Command Line Tools
- SDL2: `brew install sdl2`
- CMake: `brew install cmake`

```bash
# Clone the repository
git clone https://github.com/tristan-mcinnis/whisper.cpp.git
cd whisper.cpp
git checkout realtime-macos

# Build
make

# Run
./run-whisper --model models/ggml-base.en.bin
```

## Technical Details

- Based on whisper.cpp v1.7.6 (stable)
- C++17 standard
- Uses Apple's Accelerate framework
- SDL2 for cross-device audio capture
- Supports Metal acceleration on Apple Silicon
- Fixed buffer management (no segfaults)
- Proper SIGINT handling for Ctrl+C

## License

MIT License (same as original whisper.cpp)

## Credits

Original [whisper.cpp](https://github.com/ggml-org/whisper.cpp) by Georgi Gerganov and contributors.
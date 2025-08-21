# Whisper.cpp - Streamlined macOS Realtime Transcription

A minimalist, macOS-focused version of whisper.cpp for realtime speech transcription with Voice Activity Detection (VAD) support.

## Features

- **Realtime transcription** from microphone input
- **Voice Activity Detection (VAD)** for optimized processing  
- **Multiple language support** with translation to English
- **macOS optimized** - removed cross-platform bloat
- **Simple command-line interface** - clean and intuitive
- **Stable v1.7.6 base** - no segmentation faults

## Quick Start

```bash
# Build everything
make build

# Compile realtime transcription tool
c++ -std=c++17 -I./include -I./ggml/include -I./examples -O3 $(pkg-config --cflags sdl2) -c whisper-realtime.cpp -o whisper-realtime.o
c++ -std=c++17 -O3 whisper-realtime.o -L./build/src -L./build/ggml/src -L./build/examples -lwhisper -lggml -lcommon-sdl -lcommon $(pkg-config --libs sdl2) -framework Foundation -framework Accelerate -o whisper-realtime

# Run with default settings
./run-whisper

# Run with VAD enabled  
./run-whisper --vad --vad-threshold 0.7

# Transcribe Spanish and translate to English
./run-whisper --language es --translate
```

## Installation

1. **Prerequisites**: Ensure you have SDL2 installed:
   ```bash
   brew install sdl2
   ```

2. **Build dependencies**:
   ```bash
   make build
   ```

3. **Compile realtime tool** (or use the commands above):
   ```bash
   # See Quick Start section
   ```

## Usage

```bash
./run-whisper [options]

Options:
  -h,  --help            show this help message and exit
  -m,  --model PATH      model file path (default: models/ggml-large-v3-turbo.bin)
  -l,  --language CODE   language code (en, es, fr, de, etc.)
  -t,  --threads N       number of threads (default: 4)
  -c,  --capture ID      capture device ID (-1 = default)
       --vad             enable voice activity detection
       --vad-threshold N VAD threshold (0.0-1.0, default: 0.6)
       --translate       translate to English
       --gpu             enable GPU acceleration (experimental)

Examples:
  ./run-whisper --model models/ggml-base.en.bin
  ./run-whisper --language es --translate  
  ./run-whisper --vad --vad-threshold 0.7
```

## Models

Available models:
- `ggml-base.en.bin` - Fast, English only (~150MB) ✅
- `ggml-large-v3-turbo.bin` - High accuracy, multilingual (~1.6GB) ✅

Download additional models:
```bash
bash ./models/download-ggml-model.sh base.en
bash ./models/download-ggml-model.sh large-v3-turbo
```

## Streamlined Architecture

This minimal version contains only:

**Essential directories:**
- `src/` - Core whisper transcription engine  
- `ggml/` - Machine learning library (CPU, Metal, BLAS backends only)
- `include/` - Essential headers
- `examples/` - Only common libraries needed for audio capture
  - `common-sdl.*` - Audio capture via SDL2
  - `common.*` - Basic utilities  
  - `common-whisper.*` - Whisper-specific utilities
- `models/` - Model files and download scripts
- `build/` - CMake build output
- `cmake/` - Essential CMake configuration

**Key files:**
- `whisper-realtime.cpp` - Main streamlined application
- `run-whisper` - Convenience script with library paths
- `Makefile` - Standard build system
- `CMakeLists.txt` - Dependency build configuration

**Removed components:**
- Cross-platform code (Windows/Linux/WebAssembly)
- Language bindings (Python, Node.js, Go, Java, etc.)
- Multiple example programs (40+ removed)
- GPU backends (CUDA, Vulkan, OpenCL, etc.)
- Complex model conversion tools
- Test frameworks and documentation generators

## Technical Notes

- Built on stable whisper.cpp v1.7.6
- Uses Apple's Accelerate framework for CPU optimization
- SDL2 for audio capture
- C++17 standard
- Fixed buffer management (no segfaults)
- **~85% smaller codebase** (from 300+ files to ~50 essential files)

## License

Same as original whisper.cpp (MIT License)
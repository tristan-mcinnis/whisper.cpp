# Streamlined Makefile for macOS whisper-realtime
# Builds only the essential whisper-realtime executable

CXX = c++
CXXFLAGS = -std=c++17 -O3 -I./include -I./ggml/include -I./examples
LDFLAGS = -L./build/src -L./build/ggml/src -L./build/examples
LIBS = -lwhisper -lggml -lcommon-sdl -lcommon -framework Foundation -framework Accelerate
SDL_FLAGS = $(shell pkg-config --cflags --libs sdl2)

.PHONY: all clean build-deps whisper-realtime

all: whisper-realtime

# Build dependencies using CMake with SDL2 enabled
build-deps:
	@echo "Building whisper.cpp dependencies with SDL2..."
	cmake -B build -DWHISPER_SDL2=ON
	cmake --build build --config Release
	@echo "Dependencies built successfully"

# Build our streamlined whisper-realtime executable
whisper-realtime: whisper-realtime.cpp | build-deps
	@echo "Building whisper-realtime..."
	@if [ ! -d "build" ]; then echo "Error: build directory missing. Dependencies not built."; exit 1; fi
	$(CXX) $(CXXFLAGS) $(SDL_FLAGS) -c whisper-realtime.cpp -o whisper-realtime.o
	$(CXX) $(CXXFLAGS) whisper-realtime.o $(LDFLAGS) $(LIBS) $(SDL_FLAGS) -o whisper-realtime
	@rm -f whisper-realtime.o
	@echo "whisper-realtime built successfully!"
	@echo ""
	@echo "Usage:"
	@echo "  ./run-whisper --help"
	@echo "  ./run-whisper --model models/ggml-base.en.bin"
	@echo "  ./run-whisper --vad --language en"

# Clean build artifacts
clean:
	rm -f whisper-realtime *.o
	rm -rf build

# Legacy build target for compatibility
build: build-deps

.PHONY: help
help:
	@echo "Streamlined whisper.cpp for macOS realtime transcription"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build whisper-realtime (default)"
	@echo "  build-deps      - Build dependencies (CMake with SDL2)"
	@echo "  whisper-realtime- Build main executable"
	@echo "  clean           - Remove build artifacts"
	@echo "  help            - Show this help"
	@echo ""
	@echo "Usage:"
	@echo "  ./run-whisper --help"
	@echo "  ./run-whisper --model models/ggml-base.en.bin"
	@echo "  ./run-whisper --vad --language en"
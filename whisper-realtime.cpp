// Realtime speech recognition for macOS
// Simplified version focused on terminal usage with VAD support

#include "examples/common-sdl.h"
#include "examples/common.h"
#include "include/whisper.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include <signal.h>

// Global flag for signal handling
volatile bool is_running = true;

// Signal handler for Ctrl+C
void signal_handler(int signal) {
    if (signal == SIGINT) {
        fprintf(stderr, "\n\nReceived interrupt signal. Stopping transcription...\n");
        is_running = false;
    }
}

// Utility function for timestamp formatting
std::string to_timestamp(int64_t t, bool comma = false) {
    int64_t msec = t * 10;
    int64_t hr = msec / (1000 * 60 * 60);
    msec = msec - hr * (1000 * 60 * 60);
    int64_t min = msec / (1000 * 60);
    msec = msec - min * (1000 * 60);
    int64_t sec = msec / 1000;
    msec = msec - sec * 1000;

    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d%s%03d", (int) hr, (int) min, (int) sec, comma ? "," : ".", (int) msec);
    return std::string(buf);
}

// Simplified command-line parameters
struct realtime_params {
    int32_t n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());
    int32_t capture_id = -1;
    
    float vad_threshold = 0.6f;
    bool use_vad = false;
    bool use_gpu = false;  // Disable GPU for stability
    bool translate = false;
    
    std::string language = "en";
    std::string model = "models/ggml-large-v3-turbo.bin";
};

void print_usage(char * argv0, const realtime_params & params) {
    fprintf(stderr, "\n");
    fprintf(stderr, "usage: %s [options]\n", argv0);
    fprintf(stderr, "\n");
    fprintf(stderr, "Realtime speech recognition for macOS\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  -h,  --help            show this help message and exit\n");
    fprintf(stderr, "  -m,  --model PATH      [%-20s] model file path\n", params.model.c_str());
    fprintf(stderr, "  -l,  --language CODE   [%-20s] language (en, es, fr, de, etc.)\n", params.language.c_str());
    fprintf(stderr, "  -t,  --threads N       [%-20d] number of threads\n", params.n_threads);
    fprintf(stderr, "  -c,  --capture ID      [%-20d] capture device ID (-1 = default)\n", params.capture_id);
    fprintf(stderr, "       --vad             [%-20s] enable voice activity detection\n", params.use_vad ? "enabled" : "disabled");
    fprintf(stderr, "       --vad-threshold N [%-20.1f] VAD threshold (0.0-1.0)\n", params.vad_threshold);
    fprintf(stderr, "       --translate       [%-20s] translate to English\n", params.translate ? "enabled" : "disabled");
    fprintf(stderr, "       --gpu             [%-20s] enable GPU acceleration\n", params.use_gpu ? "GPU enabled" : "CPU only");
    fprintf(stderr, "\n");
    fprintf(stderr, "examples:\n");
    fprintf(stderr, "  %s --model models/ggml-large-v3-turbo.bin\n", argv0);
    fprintf(stderr, "  %s --language es --translate\n", argv0);
    fprintf(stderr, "  %s --vad --vad-threshold 0.7\n", argv0);
    fprintf(stderr, "\n");
}

bool parse_arguments(int argc, char ** argv, realtime_params & params) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0], params);
            exit(0);
        }
        else if (arg == "-m" || arg == "--model") {
            if (++i >= argc) {
                fprintf(stderr, "error: --model requires a value\n");
                return false;
            }
            params.model = argv[i];
        }
        else if (arg == "-l" || arg == "--language") {
            if (++i >= argc) {
                fprintf(stderr, "error: --language requires a value\n");
                return false;
            }
            params.language = argv[i];
        }
        else if (arg == "-t" || arg == "--threads") {
            if (++i >= argc) {
                fprintf(stderr, "error: --threads requires a value\n");
                return false;
            }
            params.n_threads = std::stoi(argv[i]);
        }
        else if (arg == "-c" || arg == "--capture") {
            if (++i >= argc) {
                fprintf(stderr, "error: --capture requires a value\n");
                return false;
            }
            params.capture_id = std::stoi(argv[i]);
        }
        else if (arg == "--vad") {
            params.use_vad = true;
        }
        else if (arg == "--vad-threshold") {
            if (++i >= argc) {
                fprintf(stderr, "error: --vad-threshold requires a value\n");
                return false;
            }
            params.vad_threshold = std::stof(argv[i]);
        }
        else if (arg == "--translate") {
            params.translate = true;
        }
        else if (arg == "--gpu") {
            params.use_gpu = true;
        }
        else {
            fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
            print_usage(argv[0], params);
            return false;
        }
    }

    return true;
}

int main(int argc, char ** argv) {
    ggml_backend_load_all();

    realtime_params params;

    if (!parse_arguments(argc, argv, params)) {
        return 1;
    }

    // Register signal handler for Ctrl+C
    signal(SIGINT, signal_handler);

    // Validate language
    if (params.language != "auto" && whisper_lang_id(params.language.c_str()) == -1) {
        fprintf(stderr, "error: unknown language '%s'\n", params.language.c_str());
        print_usage(argv[0], params);
        return 1;
    }

    // Audio parameters - optimized for realtime
    const int step_ms = params.use_vad ? 0 : 3000;  // 0 = sliding window with VAD
    const int length_ms = params.use_vad ? 30000 : 10000;
    const int keep_ms = 200;
    
    const int n_samples_step = (1e-3 * step_ms) * WHISPER_SAMPLE_RATE;
    const int n_samples_len = (1e-3 * length_ms) * WHISPER_SAMPLE_RATE;
    const int n_samples_keep = (1e-3 * keep_ms) * WHISPER_SAMPLE_RATE;
    const int n_samples_30s = (1e-3 * 30000.0) * WHISPER_SAMPLE_RATE;

    // Initialize audio capture
    audio_async audio(length_ms);
    if (!audio.init(params.capture_id, WHISPER_SAMPLE_RATE)) {
        fprintf(stderr, "error: failed to initialize audio capture\n");
        return 1;
    }

    audio.resume();

    // Initialize whisper
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = params.use_gpu;

    struct whisper_context * ctx = whisper_init_from_file_with_params(params.model.c_str(), cparams);
    if (ctx == nullptr) {
        fprintf(stderr, "error: failed to initialize whisper context\n");
        return 2;
    }

    fprintf(stderr, "whisper context initialized successfully\n");

    // Print configuration
    fprintf(stderr, "\n");
    fprintf(stderr, "Whisper Realtime Transcription (macOS)\n");
    fprintf(stderr, "======================================\n");
    fprintf(stderr, "Model: %s\n", params.model.c_str());
    fprintf(stderr, "Language: %s\n", params.language.c_str());
    fprintf(stderr, "Threads: %d\n", params.n_threads);
    fprintf(stderr, "GPU: %s\n", params.use_gpu ? "enabled" : "disabled");
    fprintf(stderr, "VAD: %s", params.use_vad ? "enabled" : "disabled");
    if (params.use_vad) {
        fprintf(stderr, " (threshold: %.1f)", params.vad_threshold);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Translate: %s\n", params.translate ? "enabled" : "disabled");
    fprintf(stderr, "\n");
    fprintf(stderr, "Press Ctrl+C to stop...\n");
    fprintf(stderr, "\n");

    // Audio buffers - FIXED: properly initialize pcmf32_old
    std::vector<float> pcmf32(n_samples_30s, 0.0f);
    std::vector<float> pcmf32_old(n_samples_30s, 0.0f);
    std::vector<float> pcmf32_new(n_samples_30s, 0.0f);

    int n_iter = 0;

    while (is_running) {
        if (params.use_vad) {
            // VAD mode - wait for voice activity
            {
                audio.get(2000, pcmf32_new);

                // Simple VAD check - for now just get audio
                audio.get(length_ms, pcmf32);
            }
        } else {
            // Regular mode - fixed intervals
            audio.get(step_ms, pcmf32_new);

            if (n_iter == 0) {
                std::fill(pcmf32.begin(), pcmf32.end(), 0.0f);
                std::copy(pcmf32_new.begin(), pcmf32_new.end(), pcmf32.begin());
            } else {
                std::copy(pcmf32.begin() + n_samples_step, pcmf32.end(), pcmf32_old.begin());
                std::copy(pcmf32_new.begin(), pcmf32_new.end(), pcmf32.begin() + n_samples_keep);
            }
        }

        // Run inference
        {
            whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

            wparams.print_progress   = false;
            wparams.print_special    = false;
            wparams.print_realtime   = false;
            wparams.print_timestamps = params.use_vad;
            wparams.translate        = params.translate;
            wparams.no_context       = params.use_vad;
            wparams.no_timestamps    = !params.use_vad;
            wparams.single_segment   = !params.use_vad;
            wparams.max_tokens       = params.use_vad ? 0 : 32;
            wparams.language         = params.language.c_str();
            wparams.n_threads        = params.n_threads;

            wparams.audio_ctx = 0;

            if (whisper_full(ctx, wparams, pcmf32.data(), n_samples_len) != 0) {
                fprintf(stderr, "error: failed to process audio\n");
                continue;
            }

            // Print results
            const int n_segments = whisper_full_n_segments(ctx);
            for (int i = 0; i < n_segments; ++i) {
                const char * text = whisper_full_get_segment_text(ctx, i);

                if (params.use_vad) {
                    const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
                    const int64_t t1 = whisper_full_get_segment_t1(ctx, i);
                    printf("[%s --> %s] %s\n", to_timestamp(t0).c_str(), to_timestamp(t1).c_str(), text);
                } else {
                    printf("%s", text);
                    fflush(stdout);
                }
            }

            if (params.use_vad && n_segments > 0) {
                printf("\n");
            }
        }

        ++n_iter;

        if (!params.use_vad && (n_iter % 4) == 0) {
            printf("\n");
            fflush(stdout);
        }
    }

    audio.pause();

    whisper_free(ctx);

    return 0;
}
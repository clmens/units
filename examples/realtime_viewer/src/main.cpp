#include "units_core.h"
#include <SDL2/SDL.h>
#ifdef USE_GPU_COLORMAP
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#endif
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

struct ViewerConfig {
    int width = 256;
    int height = 256;
    int scale = 2;
    int target_fps = 30;
    int scenario = 0; // 0=random, 1=center, 2=edges
};

ViewerConfig parse_args(int argc, char** argv) {
    ViewerConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--width" || arg == "-w") && i + 1 < argc) {
            cfg.width = std::stoi(argv[++i]);
        } else if ((arg == "--height" || arg == "-h") && i + 1 < argc) {
            cfg.height = std::stoi(argv[++i]);
        } else if (arg == "--scale" && i + 1 < argc) {
            cfg.scale = std::stoi(argv[++i]);
        } else if (arg == "--fps" && i + 1 < argc) {
            cfg.target_fps = std::stoi(argv[++i]);
        } else if (arg == "--scenario" && i + 1 < argc) {
            cfg.scenario = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: realtime_viewer [options]\n"
                      << "  --width <W>      Grid width (default: 256)\n"
                      << "  --height <H>     Grid height (default: 256)\n"
                      << "  --scale <S>      Pixel scale factor (default: 2)\n"
                      << "  --fps <F>        Target FPS (default: 30)\n"
                      << "  --scenario <N>   Initial scenario: 0=random, 1=center, 2=edges (default: 0)\n"
                      << "  --help           Show this help\n";
            std::exit(0);
        }
    }
    return cfg;
}

// Helper function to convert real_t values to RGBA pixels (CPU path)
// Reuses pixel buffer to avoid per-frame allocation
void convert_to_rgba(const std::vector<real_t>& values, std::vector<uint8_t>& pixels, int width, int height) {
    if (values.empty()) {
        pixels.clear();
        return;
    }
    
    // Find min/max for normalization
    real_t min_val = values[0];
    real_t max_val = values[0];
    for (const auto& v : values) {
        if (v < min_val) min_val = v;
        if (v > max_val) max_val = v;
    }
    
    real_t range = max_val - min_val;
    if (range < 1e-6) range = 1.0;
    
    // Reuse buffer: only resize if needed
    if (pixels.size() != static_cast<std::size_t>(width * height * 4)) {
        pixels.resize(width * height * 4);
    }
    
    for (std::size_t i = 0; i < values.size(); ++i) {
        real_t normalized = (values[i] - min_val) / range;
        // Clamp to [0, 1] to prevent overflow
        normalized = std::clamp(normalized, real_t(0.0), real_t(1.0));
        uint8_t intensity = static_cast<uint8_t>(normalized * real_t(255.0));
        
        pixels[i * 4 + 0] = intensity; // R
        pixels[i * 4 + 1] = intensity; // G
        pixels[i * 4 + 2] = intensity; // B
        pixels[i * 4 + 3] = 255;       // A
    }
}

#ifdef USE_GPU_COLORMAP
// GPU-accelerated colormap rendering using OpenGL
// Uploads float data as R32F texture and uses shader for normalization
class GPUColorMapper {
public:
    GPUColorMapper(int width, int height) 
        : m_width(width), m_height(height), m_texture_id(0), m_normalized_buffer(width * height)
    {
        // Create OpenGL texture for R32F upload
        glGenTextures(1, &m_texture_id);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Allocate texture storage
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    ~GPUColorMapper() {
        if (m_texture_id) {
            glDeleteTextures(1, &m_texture_id);
        }
    }
    
    void render(const std::vector<real_t>& values, int window_width, int window_height) {
        if (values.empty()) return;
        
        // Find min/max for normalization (done on CPU, but fast)
        real_t min_val = values[0];
        real_t max_val = values[0];
        for (const auto& v : values) {
            if (v < min_val) min_val = v;
            if (v > max_val) max_val = v;
        }
        
        real_t range = max_val - min_val;
        if (range < 1e-6f) range = 1.0f;
        
        // Normalize to [0,1] range for texture upload
        for (std::size_t i = 0; i < values.size(); ++i) {
            m_normalized_buffer[i] = (values[i] - min_val) / range;
        }
        
        // Upload to GPU as R32F texture
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, 
                        GL_RED, GL_FLOAT, m_normalized_buffer.data());
        
        // Render textured quad
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1, -1);
        glTexCoord2f(1, 0); glVertex2f( 1, -1);
        glTexCoord2f(1, 1); glVertex2f( 1,  1);
        glTexCoord2f(0, 1); glVertex2f(-1,  1);
        glEnd();
        
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
private:
    int m_width;
    int m_height;
    GLuint m_texture_id;
    std::vector<float> m_normalized_buffer; // Reused buffer for normalization
};
#endif

int main(int argc, char** argv) {
    ViewerConfig cfg = parse_args(argc, argv);
    
    if (cfg.width <= 0 || cfg.height <= 0 || cfg.scale <= 0) {
        std::cerr << "Error: width, height, and scale must be positive\n";
        return 1;
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }
    
#ifdef USE_GPU_COLORMAP
    // Request OpenGL context for GPU colormap
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif
    
    // Create window
    int window_width = cfg.width * cfg.scale;
    int window_height = cfg.height * cfg.scale;
    SDL_Window* window = SDL_CreateWindow(
        "Units Realtime Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
#ifdef USE_GPU_COLORMAP
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
#else
        SDL_WINDOW_SHOWN
#endif
    );
    
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    
#ifdef USE_GPU_COLORMAP
    // Create OpenGL context for GPU rendering
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize GPU color mapper
    GPUColorMapper gpu_mapper(cfg.width, cfg.height);
    
    std::cout << "Using GPU-accelerated colormap (OpenGL)\n";
#else
    // Create renderer for CPU path
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Create texture for visualization
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        cfg.width,
        cfg.height
    );
    
    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    std::cout << "Using CPU colormap (SDL2 texture)\n";
#endif
    
    // Create UnitsCore simulation
    UnitsCore core(cfg.width, cfg.height, 1.0, true);
    
    // Initialize based on scenario
    std::mt19937 rng(12345);
    std::uniform_real_distribution<real_t> dist(-1.0, 1.0);
    
    const std::size_t N = static_cast<std::size_t>(cfg.width) * static_cast<std::size_t>(cfg.height);
    
    if (cfg.scenario == 0) {
        // Random initialization
        for (std::size_t i = 0; i < N; ++i) {
            core.set_value_index(i, dist(rng));
        }
    } else if (cfg.scenario == 1) {
        // Center stimulus
        for (std::size_t i = 0; i < N; ++i) {
            core.set_value_index(i, 0.0);
        }
        int cx = cfg.width / 2;
        int cy = cfg.height / 2;
        core.set_value(cx, cy, 1.0);
    } else if (cfg.scenario == 2) {
        // Edge stimulus
        for (std::size_t i = 0; i < N; ++i) {
            core.set_value_index(i, 0.0);
        }
        for (int x = 0; x < cfg.width; ++x) {
            core.set_value(x, 0, 1.0);
            core.set_value(x, cfg.height - 1, 1.0);
        }
        for (int y = 0; y < cfg.height; ++y) {
            core.set_value(0, y, 1.0);
            core.set_value(cfg.width - 1, y, 1.0);
        }
    }
    
#ifndef USE_GPU_COLORMAP
    // Preallocate pixel buffer for CPU path (reused each frame)
    std::vector<uint8_t> pixels;
#endif
    
    bool running = true;
    SDL_Event event;
    
    const Uint32 frame_delay = 1000 / cfg.target_fps;
    
    std::cout << "Realtime viewer started. Press ESC or close window to exit.\n";
    std::cout << "Grid: " << cfg.width << "x" << cfg.height << ", Scale: " << cfg.scale << ", Target FPS: " << cfg.target_fps << "\n";
    
    while (running) {
        Uint32 frame_start = SDL_GetTicks();
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }
        
        // Step simulation
        core.step();
        
#ifdef USE_GPU_COLORMAP
        // GPU rendering path
        glClear(GL_COLOR_BUFFER_BIT);
        gpu_mapper.render(core.values(), window_width, window_height);
        SDL_GL_SwapWindow(window);
#else
        // CPU rendering path
        // Convert values to RGBA pixels (reuses buffer)
        convert_to_rgba(core.values(), pixels, cfg.width, cfg.height);
        
        // Update texture
        SDL_UpdateTexture(texture, nullptr, pixels.data(), cfg.width * 4);
        
        // Render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
#endif
        
        // Frame rate control
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time);
        }
    }
    
    // Cleanup
#ifdef USE_GPU_COLORMAP
    SDL_GL_DeleteContext(gl_context);
#else
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
#endif
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

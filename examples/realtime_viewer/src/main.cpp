#include "units_core.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

#ifdef USE_GPU_COLORMAP
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#endif

struct ViewerConfig {
    int width = 512;
    int height = 512;
    int scale = 1;
    int fps = 30;
    std::string scenario = "random";
};

ViewerConfig parse_args(int argc, char** argv) {
    ViewerConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--width" && i + 1 < argc) {
            cfg.width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            cfg.height = std::stoi(argv[++i]);
        } else if (arg == "--scale" && i + 1 < argc) {
            cfg.scale = std::stoi(argv[++i]);
        } else if (arg == "--fps" && i + 1 < argc) {
            cfg.fps = std::stoi(argv[++i]);
        } else if (arg == "--scenario" && i + 1 < argc) {
            cfg.scenario = argv[++i];
        }
    }
    return cfg;
}

#ifdef USE_GPU_COLORMAP

// GPU-accelerated color mapping using OpenGL shaders
class GPUColorMapper {
public:
    GPUColorMapper(int width, int height)
        : m_width(width), m_height(height), m_texture(0), m_pbo(0)
    {
        // Create R32F texture for float data
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Create PBO for asynchronous upload
        glGenBuffers(1, &m_pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * sizeof(float), nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // Compile shader program for color mapping
        const char* vertex_shader_src = R"(
#version 120
void main() {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";

        const char* fragment_shader_src = R"(
#version 120
uniform sampler2D tex;
void main() {
    float value = texture2D(tex, gl_TexCoord[0].xy).r;
    // Simple grayscale mapping: normalize to [0, 1]
    float normalized = (value + 1.0) * 0.5;
    normalized = clamp(normalized, 0.0, 1.0);
    gl_FragColor = vec4(normalized, normalized, normalized, 1.0);
}
)";

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertex_shader_src, nullptr);
        glCompileShader(vs);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragment_shader_src, nullptr);
        glCompileShader(fs);

        m_program = glCreateProgram();
        glAttachShader(m_program, vs);
        glAttachShader(m_program, fs);
        glLinkProgram(m_program);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    ~GPUColorMapper() {
        if (m_texture) glDeleteTextures(1, &m_texture);
        if (m_pbo) glDeleteBuffers(1, &m_pbo);
        if (m_program) glDeleteProgram(m_program);
    }

    void upload_and_render(const std::vector<value_t>& values) {
        // Upload float data to PBO
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
        void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr) {
            std::memcpy(ptr, values.data(), values.size() * sizeof(value_t));
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }

        // Upload from PBO to texture
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED, GL_FLOAT, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        // Render fullscreen quad with shader
        glUseProgram(m_program);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1, -1);
        glTexCoord2f(1, 0); glVertex2f( 1, -1);
        glTexCoord2f(1, 1); glVertex2f( 1,  1);
        glTexCoord2f(0, 1); glVertex2f(-1,  1);
        glEnd();

        glUseProgram(0);
    }

private:
    int m_width, m_height;
    GLuint m_texture;
    GLuint m_pbo;
    GLuint m_program;
};

#endif

int main(int argc, char** argv) {
    ViewerConfig cfg = parse_args(argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

#ifdef USE_GPU_COLORMAP
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#endif

    int window_width = cfg.width * cfg.scale;
    int window_height = cfg.height * cfg.scale;

#ifdef USE_GPU_COLORMAP
    SDL_Window* window = SDL_CreateWindow(
        "Units Realtime Viewer (GPU)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
#else
    SDL_Window* window = SDL_CreateWindow(
        "Units Realtime Viewer (CPU)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_SHOWN
    );
#endif

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

#ifdef USE_GPU_COLORMAP
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    GPUColorMapper mapper(cfg.width, cfg.height);
#else
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        cfg.width, cfg.height);

    if (!texture) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Pre-allocate u8 buffer for CPU conversion
    std::vector<uint32_t> pixels(cfg.width * cfg.height);
#endif

    // Create UnitsCore
    UnitsCore core(cfg.width, cfg.height);

    // Initialize with random values
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (std::size_t i = 0; i < core.size(); ++i) {
        core.set_value_index(i, static_cast<value_t>(dist(rng)));
    }

    bool running = true;
    SDL_Event event;
    Uint32 frame_delay = 1000 / cfg.fps;

    std::cout << "Realtime viewer running. Press ESC or close window to exit.\n";
    std::cout << "Grid: " << cfg.width << "x" << cfg.height << ", FPS: " << cfg.fps << "\n";
#ifdef USE_GPU_COLORMAP
    std::cout << "Using GPU color mapping (OpenGL shader)\n";
#else
    std::cout << "Using CPU color mapping\n";
#endif

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

        // Simulate one step
        core.step();

#ifdef USE_GPU_COLORMAP
        // GPU path: upload float buffer and render with shader
        mapper.upload_and_render(core.values());
        SDL_GL_SwapWindow(window);
#else
        // CPU path: convert to RGBA u8 and upload
        const auto& values = core.values();
        
        // Find max value for normalization
        value_t max_val = 0.0;
        for (value_t v : values) {
            value_t abs_v = std::abs(v);
            if (abs_v > max_val) max_val = abs_v;
        }
        if (max_val < 1e-6) max_val = 1.0;

        // Convert to grayscale pixels
        for (std::size_t i = 0; i < values.size(); ++i) {
            value_t v = values[i];
            value_t normalized = (v / max_val) * 0.5 + 0.5;
            normalized = std::min<value_t>(1.0, std::max<value_t>(0.0, normalized));
            uint8_t gray = static_cast<uint8_t>(normalized * 255.0);
            pixels[i] = (255U << 24) | (gray << 16) | (gray << 8) | gray; // RGBA
        }

        SDL_UpdateTexture(texture, nullptr, pixels.data(), cfg.width * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
#endif

        // Frame rate limiting
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < frame_delay) {
            SDL_Delay(frame_delay - frame_time);
        }
    }

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

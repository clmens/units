#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <SDL2/SDL.h>

#ifdef USE_GPU_COLORMAP
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#endif

#include "../../../src/units_core.h"

struct Config {
    int width = 512;
    int height = 512;
    int scale = 1;
    int fps = 30;
    std::string scenario = "random";
};

Config parse_args(int argc, char** argv) {
    Config cfg;
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

// Helper: HSV to RGB conversion
void hsv_to_rgb(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r_f = 0, g_f = 0, b_f = 0;
    
    if (h < 60) { r_f = c; g_f = x; b_f = 0; }
    else if (h < 120) { r_f = x; g_f = c; b_f = 0; }
    else if (h < 180) { r_f = 0; g_f = c; b_f = x; }
    else if (h < 240) { r_f = 0; g_f = x; b_f = c; }
    else if (h < 300) { r_f = x; g_f = 0; b_f = c; }
    else { r_f = c; g_f = 0; b_f = x; }
    
    r = static_cast<uint8_t>((r_f + m) * 255);
    g = static_cast<uint8_t>((g_f + m) * 255);
    b = static_cast<uint8_t>((b_f + m) * 255);
}

#ifdef USE_GPU_COLORMAP

// GPU color mapping shader sources
const char* vertex_shader_src = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragment_shader_src = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D valueTexture;

vec3 hsv_to_rgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
    float m = v - c;
    vec3 rgb;
    if (h < 60.0) rgb = vec3(c, x, 0.0);
    else if (h < 120.0) rgb = vec3(x, c, 0.0);
    else if (h < 180.0) rgb = vec3(0.0, c, x);
    else if (h < 240.0) rgb = vec3(0.0, x, c);
    else if (h < 300.0) rgb = vec3(x, 0.0, c);
    else rgb = vec3(c, 0.0, x);
    return rgb + m;
}

void main() {
    float value = texture(valueTexture, TexCoord).r;
    // Map value [0,1] to hue [240, 0] (blue to red)
    float hue = (1.0 - value) * 240.0;
    vec3 color = hsv_to_rgb(hue, 1.0, 1.0);
    FragColor = vec4(color, 1.0);
}
)";

GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compilation failed: " << info << std::endl;
    }
    return shader;
}

GLuint create_shader_program() {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program, 512, nullptr, info);
        std::cerr << "Shader linking failed: " << info << std::endl;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

#endif

int main(int argc, char** argv) {
    Config cfg = parse_args(argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

#ifdef USE_GPU_COLORMAP
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    SDL_Window* window = SDL_CreateWindow(
        "Units Realtime Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        cfg.width * cfg.scale, cfg.height * cfg.scale,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // VSync
    
    // Setup OpenGL for GPU color mapping
    GLuint shader_program = create_shader_program();
    
    // Fullscreen quad
    float vertices[] = {
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 0.0f
    };
    
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Create texture for float values
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate PBO for async upload
    GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, cfg.width * cfg.height * sizeof(float), nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    
#else
    SDL_Window* window = SDL_CreateWindow(
        "Units Realtime Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        cfg.width * cfg.scale, cfg.height * cfg.scale,
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        cfg.width, cfg.height
    );
    
    std::vector<uint8_t> rgb_buffer(cfg.width * cfg.height * 3);
#endif

    // Create UnitsCore
    UnitsCore core(cfg.width, cfg.height, 1.0, true);
    
    // Initialize with scenario
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    if (cfg.scenario == "random") {
        for (int y = 0; y < cfg.height; ++y) {
            for (int x = 0; x < cfg.width; ++x) {
                core.set_value(x, y, dist(rng));
            }
        }
    } else {
        // Center spike scenario
        core.set_value(cfg.width / 2, cfg.height / 2, 1.0);
    }

    bool running = true;
    SDL_Event event;
    auto last_frame = std::chrono::steady_clock::now();
    double frame_time = 1.0 / cfg.fps;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            }
        }

        // Simulation step
        core.step();

        // Render
        const std::vector<double>& values = core.values();

#ifdef USE_GPU_COLORMAP
        // GPU path: upload float values to texture and use shader for color mapping
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        float* mapped = (float*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (mapped) {
            for (size_t i = 0; i < values.size(); ++i) {
                mapped[i] = static_cast<float>(values[i]);
            }
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, cfg.width, cfg.height, 0, GL_RED, GL_FLOAT, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        SDL_GL_SwapWindow(window);
#else
        // CPU path: convert to RGB on CPU
        for (size_t i = 0; i < values.size(); ++i) {
            float v = static_cast<float>(values[i]);
            if (v < 0.0f) v = 0.0f;
            if (v > 1.0f) v = 1.0f;
            
            float hue = (1.0f - v) * 240.0f;
            hsv_to_rgb(hue, 1.0f, 1.0f, rgb_buffer[i * 3], rgb_buffer[i * 3 + 1], rgb_buffer[i * 3 + 2]);
        }
        
        SDL_UpdateTexture(texture, nullptr, rgb_buffer.data(), cfg.width * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
#endif

        // Frame rate limiting
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - last_frame;
        if (elapsed.count() < frame_time) {
            SDL_Delay(static_cast<Uint32>((frame_time - elapsed.count()) * 1000));
        }
        last_frame = std::chrono::steady_clock::now();
    }

#ifdef USE_GPU_COLORMAP
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &pbo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);
    SDL_GL_DeleteContext(gl_context);
#else
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
#endif

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

// ParticleSystem, Jackson Kruger, 2019
// Based on MazeGame, Jackson Kruger, 2018
// Credit to Stephen J. Guy, 2018 for the foundations

#define GLM_FORCE_RADIANS
#include <iomanip>
#include <sstream>
#include "Camera.h"
#include "Environment.h"
#include "GameObject.h"
#include "ParticleManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
const char* INSTRUCTIONS =
    "***************\n"
    "This is a particle system made by Jackson Kruger for CSCI 5611 at the University of Minnesota.\n"
    "\n"
    "Controls:\n"
    "WASD - Player movement\n"
    "Space - Player jump\n"
    "Left ctrl - Player crouch\n"
    "g - Drop key\n"
    "Esc - Quit\n"
    "F11 - Fullscreen\n"
    "***************\n";

const char* USAGE =
    "Usage:\n"
    "-w \'width\'x\'height\'\n"
    "   Example: -m 800x600\n"
    "-m map\n"
    "   This map must be in the root of the directory the game's being run from.\n"
    "   Example: -m map1.txt\n";

#include "glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include <cstdio>
#include <string>

#include "ModelManager.h"

using namespace std;

int screenWidth = 1536;
int screenHeight = 864;
float timePassed = 0;

bool fullscreen = false;

// srand(time(NULL));
float rand01() {
    return rand() / (float)RAND_MAX;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);  // Initialize Graphics (for OpenGL)

    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

    // Maximize the window if no size was specified
    SDL_SetWindowResizable(window, SDL_TRUE);                // Allow resizing
    SDL_MaximizeWindow(window);                              // Maximize
    SDL_GetWindowSize(window, &screenWidth, &screenHeight);  // Get the new size
    SDL_SetWindowResizable(window, SDL_FALSE);               // Disable future resizing

    // Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_SetRelativeMouseMode(SDL_TRUE);  // 'grab' the mouse

    // Load OpenGL extentions with GLAD
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        printf("\nOpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n\n", glGetString(GL_VERSION));
    } else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }

    SDL_GL_SetSwapInterval(1);

    Camera camera = Camera();

    Environment environment = Environment();

    ParticleManager particleManager = ParticleManager();

    // Load the textures
    TextureManager::InitTextures();

    // Build a Vertex Array Object (VAO) to store mapping of shader attributes to VBO
    GLuint vao;
    glGenVertexArrays(1, &vao);  // Create a VAO
    glBindVertexArray(vao);      // Bind the above created VAO to the current context

    ModelManager::InitVBO();

    ShaderManager::InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");

    TextureManager::InitTextures();

    glBindVertexArray(0);  // Unbind the VAO in case we want to create a new one

    glEnable(GL_DEPTH_TEST);

    printf("%s\n", INSTRUCTIONS);

    // Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool quit = false;
    float lastTickTime = 0;
    int frameCounter = 0;
    float lastFramesTimer = 0;
    float lastFramerate = 0;
    int framesPerSample = 20;
    string lastAverageFrameTime;
    while (!quit) {
        while (SDL_PollEvent(&windowEvent)) {  // inspect all events in the queue
            if (windowEvent.type == SDL_QUIT) quit = true;
            // List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            // Scancode refers to a keyboard position, keycode refers to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP) {  // Exit event loop
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (windowEvent.key.keysym.sym == SDLK_F11) {  // If F11 is pressed
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);  // Toggle fullscreen
                } else if (windowEvent.key.keysym.sym == SDLK_EQUALS || windowEvent.key.keysym.sym == SDLK_MINUS) {
                    float modAmount = 20;
                    if (windowEvent.key.keysym.mod & KMOD_SHIFT) {
                        modAmount = 100;
                    }

                    if (windowEvent.key.keysym.sym == SDLK_MINUS) modAmount *= -1;

                    particleManager.genRate += modAmount;
                }
            } else if (windowEvent.type == SDL_KEYDOWN) {
                if (windowEvent.key.keysym.sym == SDLK_SPACE) {
                    particleManager.SpawnParticle(camera.GetPosition() + camera.GetForward(), camera.GetForward() * 7.5f);
                }
            }

            if (windowEvent.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
                // printf("Mouse movement (xrel, yrel): (%i, %i)\n", windowEvent.motion.xrel, windowEvent.motion.yrel);
                camera.ProcessMouseInput(windowEvent.motion.xrel, windowEvent.motion.yrel);
            }

            switch (windowEvent.window.event) {
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    SDL_Log("Window focus lost");
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    SDL_Log("Window focus gained");
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    break;
            }
        }

        // Clear the screen to default color
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(ShaderManager::Textured_Shader);

        float deltaTime = (SDL_GetTicks() / 1000.0f) - lastTickTime;
        lastTickTime = SDL_GetTicks() / 1000.0f;

        frameCounter++;
        lastFramesTimer += deltaTime;
        if (frameCounter >= framesPerSample) {
            lastAverageFrameTime = std::to_string((lastFramesTimer * 1000.0f) / framesPerSample) + "ms";
            lastFramerate = 1.0f / (lastFramesTimer / framesPerSample);
            printf("Time for the last %i frames: %f ms, average time per frame: %s\n", framesPerSample, lastFramesTimer * 1000.0,
                   lastAverageFrameTime.c_str());

            frameCounter = 0;
            lastFramesTimer = 0;
        }

        stringstream debugText;
        debugText << fixed << setprecision(3) << particleManager.GetNumParticles() << " particles | genRate: " << particleManager.genRate
                  << " | " << lastAverageFrameTime << " per frame average | " << lastFramerate << " FPS average over " << framesPerSample
                  << " frames";
        SDL_SetWindowTitle(window, debugText.str().c_str());

        particleManager.SpawnParticles(deltaTime);
        particleManager.MoveParticles(deltaTime);
        camera.Update();

        glm::mat4 proj = glm::perspective(3.14f / 2, screenWidth / (float)screenHeight, 0.01f, 1000.0f);  // FOV, aspect, near, far
        glUniformMatrix4fv(ShaderManager::Attributes.projection, 1, GL_FALSE, glm::value_ptr(proj));

        TextureManager::Update();

        glBindVertexArray(vao);

        environment.UpdateAll();
        particleManager.RenderParticles();

        SDL_GL_SwapWindow(window);  // Double buffering
    }

    // Clean Up
    ShaderManager::Cleanup();
    ModelManager::Cleanup();
    glDeleteVertexArrays(1, &vao);

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}

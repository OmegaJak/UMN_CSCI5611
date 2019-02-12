// ParticleSystem, Jackson Kruger, 2019
// Based on MazeGame, Jackson Kruger, 2018
// Credit to Stephen J. Guy, 2018 for the foundations

#define GLM_FORCE_RADIANS
#include <SDL_image.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "Camera.h"
#include "Constants.h"
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

std::ostream& operator<<(std::ostream& out, glm::vec3 const& vec) {
    out << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return out;
}

// https://learnopengl.com/In-Practice/Debugging
void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                              const void* userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
    }
    std::cout << std::endl;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);  // Initialize Graphics (for OpenGL)

    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

    // Maximize the window if no size was specified
    SDL_SetWindowResizable(window, SDL_TRUE);                // Allow resizing
    SDL_MaximizeWindow(window);                              // Maximize
    SDL_GetWindowSize(window, &screenWidth, &screenHeight);  // Get the new size
    SDL_SetWindowResizable(window, SDL_FALSE);               // Disable future resizing

    // Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);

    // SDL_SetRelativeMouseMode(SDL_TRUE);  // 'grab' the mouse

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

    // OpenGL debug
    // https://learnopengl.com/In-Practice/Debugging
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    SDL_GL_SetSwapInterval(1);

    Camera camera = Camera();

    Environment environment = Environment();

    ParticleManager particleManager = ParticleManager();

    ModelManager::InitVBO();

    ShaderManager::InitShaders();

    TextureManager::InitTextures();

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_CULL_FACE);

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
    int mouseX = -1, mouseY = -1;
    float normalizedMouseX, normalizedMouseY;
    glm::vec3 lastMouseWorldCoord;
    float fullGravityAcceleration = 1.0f;
    float gravityCenterDistance = 10;
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
                    float modAmount = 0.1;
                    if (windowEvent.key.keysym.mod & KMOD_CTRL) modAmount *= 10;
                    if (windowEvent.key.keysym.mod & KMOD_ALT) modAmount *= 10;
                    if (windowEvent.key.keysym.mod & KMOD_SHIFT) modAmount *= 2;

                    if (windowEvent.key.keysym.sym == SDLK_MINUS) modAmount *= -1;

                    if (windowEvent.key.keysym.mod & KMOD_CTRL) {
                        fullGravityAcceleration += modAmount;
                        particleManager.particleParameters.gravityAccelerationFactor = fullGravityAcceleration;
                    } else if (windowEvent.key.keysym.mod & KMOD_ALT) {
                        gravityCenterDistance += modAmount;
                    } else {
                        particleManager.particleParameters.simulationSpeed += modAmount;
                    }
                }
            } else if (windowEvent.type == SDL_KEYDOWN) {
                if (windowEvent.key.keysym.sym == SDLK_SPACE) {
                    if (particleManager.particleParameters.simulationSpeed > 0) {
                        particleManager.particleParameters.simulationSpeed = 0;
                    } else {
                        particleManager.particleParameters.simulationSpeed = 1;
                    }
                }
            }

            if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {  // Right click is down
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }

            if (windowEvent.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
                // printf("Mouse movement (xrel, yrel): (%i, %i)\n", windowEvent.motion.xrel, windowEvent.motion.yrel);
                camera.ProcessMouseInput(windowEvent.motion.xrel, windowEvent.motion.yrel);
            } else if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
                SDL_GetMouseState(&mouseX, &mouseY);
                normalizedMouseX = (2 * (mouseX / (double)screenWidth)) - 1;
                normalizedMouseY = (2 * (mouseY / (double)screenHeight)) - 1;
            }

            switch (windowEvent.window.event) {
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    SDL_Log("Window focus lost");
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    SDL_Log("Window focus gained");
                    break;
            }
        }

        auto time = SDL_GetTicks() / 1000.0f;
        float deltaTime = time - lastTickTime;
        lastTickTime = time;

        particleManager.particleParameters.time = time;

        frameCounter++;
        lastFramesTimer += deltaTime;
        if (frameCounter >= framesPerSample) {
            lastAverageFrameTime = std::to_string((lastFramesTimer * 1000.0f) / framesPerSample) + "ms";
            lastFramerate = 1.0f / (lastFramesTimer / framesPerSample);

            frameCounter = 0;
            lastFramesTimer = 0;
        }

        particleManager.UpdateComputeParameters();

        // Particles compute shader //
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleManager.posSSbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleManager.velSSbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, particleManager.colSSbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, particleManager.paramSSbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleManager.lifeSSbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, particleManager.atomicsSSbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, particleManager.colModSSbo);

        glUseProgram(ShaderManager::ParticleComputeShader);
        glDispatchCompute(ParticleManager::NUM_PARTICLES / ParticleManager::WORK_GROUP_SIZE, 1, 1);  // Compute shader!!
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

        // Rendering //
        float gray = 0.6f;
        glClearColor(gray, gray, gray, 1.0f);  // Clear the screen to default color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Update();
        glm::mat4 proj = glm::perspective(3.14f / 2, screenWidth / (float)screenHeight, 0.4f, 10000.0f);  // FOV, aspect, near, far
        ShaderManager::ApplyToEachRenderShader(
            [proj](ShaderAttributes attributes) -> void { glUniformMatrix4fv(attributes.projection, 1, GL_FALSE, glm::value_ptr(proj)); },
            PROJ_SHADER_FUNCTION_ID);

        if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) & ~SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            lastMouseWorldCoord = camera.GetMousePosition(normalizedMouseX, normalizedMouseY, proj, gravityCenterDistance);
            environment.SetGravityCenterPosition(lastMouseWorldCoord);
            particleManager.particleParameters.centerX = lastMouseWorldCoord.x;
            particleManager.particleParameters.centerY = lastMouseWorldCoord.y;
            particleManager.particleParameters.centerZ = lastMouseWorldCoord.z;
            particleManager.particleParameters.gravityAccelerationFactor = fullGravityAcceleration;
        } else {
            particleManager.particleParameters.gravityAccelerationFactor = 0.0f;
        }

        stringstream debugText;
        debugText << fixed << setprecision(3) << particleManager.GetNumParticles() << " total " /*<< particleManager.numAlive << " alive "*/
                  << " | " << lastAverageFrameTime << " per frame (" << lastFramerate << "FPS) average over " << framesPerSample
                  << " frames "
                  << " | cameraPosition: " << camera.GetPosition() << " | CoG position: " << lastMouseWorldCoord
                  << " | simulationSpeed: " << particleManager.particleParameters.simulationSpeed
                  << " | gravityFactor: " << particleManager.particleParameters.gravityAccelerationFactor;
        SDL_SetWindowTitle(window, debugText.str().c_str());

        // Render the environment
        ShaderManager::ActivateShader(ShaderManager::EnvironmentShader);
        TextureManager::Update(ShaderManager::EnvironmentShader.Program);
        environment.UpdateAll();

        // Render particles!!
        ShaderManager::ActivateShader(ShaderManager::ParticleShader);
        TextureManager::Update(ShaderManager::ParticleShader.Program);
        glUniform2f(ShaderManager::ParticleShader.Attributes.screenSize, 10, 10);
        glUniform1f(ShaderManager::ParticleShader.Attributes.spriteSize, 30);
        particleManager.RenderParticles(deltaTime);

        SDL_GL_SwapWindow(window);  // Double buffering
    }

    // Clean Up
    ShaderManager::Cleanup();
    ModelManager::Cleanup();

    SDL_GL_DeleteContext(context);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

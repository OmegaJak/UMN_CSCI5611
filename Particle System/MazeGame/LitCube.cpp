//#include "glad.h"
//
//#if defined(__APPLE__) || defined(__linux__)
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
//#else
//#include <SDL.h>
//#include <SDL_opengl.h>
//#endif
//
//#define GLM_FORCE_RADIANS  // ensure we are using radians
//#include "glm.hpp"
//#include "gtc/matrix_transform.hpp"
//#include "gtc/type_ptr.hpp"
//
//#include <iostream>
//
//// Shader sources
// const GLchar *vertexSource =
//    "#version 150 core\n"
//    "in vec3 position;"
//    "in vec3 inColor;"
//    "in vec3 inNormal;"
//
//    "const vec3 inLightDir = normalize(vec3(1, 0, 0));"
//    "uniform mat4 model;"
//    "uniform mat4 view;"
//    "uniform mat4 proj;"
//
//    "out vec3 Color;"
//    "out vec3 normal;"
//    "out vec3 pos;"
//    "out vec3 eyePos;"
//    "out vec3 lightDir;"
//    "void main() {"
//    "   Color = inColor;"
//    "   vec4 pos4 = view * model * vec4(position, 1.0);"
//    "   pos = pos4.xyz/pos4.w;"  // Homogenous coordinate divide
//    "   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal, 0.0);"
//    "   normal = norm4.xyz;"
//    "   lightDir = (view * vec4(inLightDir, 0)).xyz;"  // Transform light into view space
//    "   gl_Position = proj * pos4;"
//    "}";
//
// const GLchar *fragmentSource =
//    "#version 150 core\n"
//    "in vec3 Color;"
//    "in vec3 normal;"
//    "in vec3 pos;"
//    "in vec3 eyePos;"
//    "in vec3 lightDir;"
//    "out vec4 outColor;"
//    "const float ambient = .3;"
//    "void main() {"
//    "   vec3 N = normalize(normal);"  // Re-normalized the interpolated normals
//    "   vec3 diffuseC = Color*max(dot(lightDir,N),0.0);"
//    "   vec3 ambC = Color*ambient;"
//    "   vec3 reflectDir = reflect(-lightDir,N);"
//    "   vec3 viewDir = normalize(-pos);"  // We know the eye is at 0,0
//    "   float spec = max(dot(reflectDir,viewDir),0.0);"
//    "   if (dot(lightDir,N) <= 0.0) spec = 0;"
//    "   vec3 specC = vec3(.8,.8,.8)*pow(spec,4);"
//    "   outColor = vec4(ambC+diffuseC+specC, 1.0);"
//    "}";
//
// bool fullscreen = false;
// int screenWidth = 800;
// int screenHeight = 600;
//
// void loadShader(GLuint shaderID, const GLchar *shaderSource) {
//    glShaderSource(shaderID, 1, &shaderSource, NULL);
//    glCompileShader(shaderID);
//
//    // Let's double check the shader compiled
//    GLint status;
//    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);  // Check for errors
//    if (!status) {
//        char buffer[512];
//        glGetShaderInfoLog(shaderID, 512, NULL, buffer);
//        printf("Shader Compile Failed. Info:\n\n%s\n", buffer);
//    }
//}
//
// int main2(int argc, char *argv[]) {
//    std::cout << "Hello World!\n";
//    SDL_Init(SDL_INIT_VIDEO);
//
//    SDL_version comp;
//    SDL_version linked;
//    SDL_VERSION(&comp);
//    SDL_GetVersion(&linked);
//    printf("\nCompiled against SDL version %d.%d.%d\n", comp.major, comp.minor, comp.patch);
//    printf("Linked SDL version %d.%d.%d.\n", linked.major, linked.minor, linked.patch);
//
//    // Ask SDL to get a recent version of OpenGL (3.2 or greater)
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//
//    // Create a window (offsetx, offsety, width, height, flags)
//    SDL_Window *window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
//
//    if (!window) {
//        printf("Could not create window: %s\n", SDL_GetError());
//        return EXIT_FAILURE;
//    }
//    float aspect = screenWidth / (float)screenHeight;  // aspect ratio needs update on resize
//
//    // Initialize OpenGL through GLAD
//    SDL_GLContext context = SDL_GL_CreateContext(window);  // Bind OpenGL to the window
//
//    if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
//        printf("OpenGL loaded\n");
//        printf("Vendor: %s\n", glGetString(GL_VENDOR));
//        printf("Renderer: %s\n", glGetString(GL_RENDERER));
//        printf("Version: %s\n", glGetString(GL_VERSION));
//    } else {
//        printf("ERROR: Failed to initialize OpenGL context.\n");
//        return -1;
//    }
//
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    loadShader(vertexShader, vertexSource);
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    loadShader(fragmentShader, fragmentSource);
//
//    // Join the vertex and fragment shaders together into one program
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glBindFragDataLocation(shaderProgram, 0, "outColor");  // set output
//    glLinkProgram(shaderProgram);                          // run the linker
//
//    GLfloat vertices[] = {// clang-format off
//      // X      Y      Z     R     G     B     U     V
//        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //Red face
//         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
//         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
//         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
//        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
//        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//
//        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  //Green face
//         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
//         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
//         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
//        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
//
//        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  //Yellow face
//        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
//        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//        -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
//        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
//
//         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  //Blue face
//         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
//         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
//         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
//         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
//         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
//
//        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  //Black face
//         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
//         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
//         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
//        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
//
//        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  //White face
//         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
//         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
//         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
//        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
//        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
//    };
//
//    float normals[] = { //Normals for 36 vertices
//       0.f,0.f,-1.f,  0.f,0.f,-1.f,  0.f,0.f,-1.f,  0.f,0.f,-1.f,  //1-4
//       0.f,0.f,-1.f,  0.f,0.f,-1.f,  0.f,0.f,1.f,   0.f,0.f,1.f,   //5-8
//       0.f,0.f,1.f,   0.f,0.f,1.f,   0.f,0.f,1.f,   0.f,0.f,1.f,   //9-12
//      -1.f,0.f,0.f,  -1.f,0.f,0.f,  -1.f,0.f,0.f,  -1.f,0.f,0.f,   //13-16
//      -1.f,0.f,0.f,  -1.f,0.f,0.f,   1.f,0.f,0.f,   1.f,0.f,0.f,   //17-20
//       1.f,0.f,0.f,   1.f,0.f,0.f,   1.f,0.f,0.f,   1.f,0.f,0.f,   //21-24
//       0.f,-1.f,0.f,  0.f,-1.f,0.f,  0.f,-1.f,0.f,  0.f,-1.f,0.f,  //25-28
//       0.f,-1.f,0.f,  0.f,-1.f,0.f,  0.f,1.f,0.f,   0.f,1.f,0.f,   //29-32
//       0.f,1.f,0.f,   0.f,1.f,0.f,   0.f,1.f,0.f,   0.f,1.f,0.f,   //33-36
//    };  // clang-format on
//
//    GLuint vbo[2];
//    glGenBuffers(2, vbo);                   // Create 1 buffer called vbo
//    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);  //(Only one buffer can be bound at a time)
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    GLuint vao;
//    glGenVertexArrays(1, &vao);  // Create a VAO (on the GPU)
//    glBindVertexArray(vao);      // Bind the above created VAO to the current context
//
//    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
//    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
//    //(above params: Attribute, vals/attrib., type, isNormalized, stride, offset)
//    glEnableVertexAttribArray(posAttrib);  // Mark the attribute's location as valid
//
//    GLint colorAttrib = glGetAttribLocation(shaderProgram, "inColor");
//    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
//    glEnableVertexAttribArray(colorAttrib);
//
//    // Set up a second buffer to hold the normals
//    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
//
//    // bind the normal attributes
//    GLint normalAttrib = glGetAttribLocation(shaderProgram, "inNormal");
//    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);  // No stride or offset necessary because it's all contiguous
//    glEnableVertexAttribArray(normalAttrib);
//
//    glBindVertexArray(0);  // Unbind the VAO so we don't accidentally modify it
//
//    glEnable(GL_DEPTH_TEST);
//
//    // Main loop
//    SDL_Event windowEvent;
//    bool quit = false;
//    while (!quit) {
//        while (SDL_PollEvent(&windowEvent)) {
//            if (windowEvent.type == SDL_QUIT) quit = true;                                                // Exit Game Loop
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE) quit = true;  // Exit Game Loop
//            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) {
//                fullscreen = !fullscreen;
//                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
//            }
//        }
//
//        glClearColor(.2f, 0.4f, 0.9f, 1.0f);  // Clear the screen to blue (background color)
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        float time = SDL_GetTicks() / 1000.0f;
//
//        // set up model matrix (3D obj coords -> 3d world coors)
//        glm::mat4 model;
//        model = glm::rotate(model, time * 3.14f / 2.f, glm::vec3(0.f, 1.f, 1.f));
//        model = glm::rotate(model, time * 3.14f / 4.f, glm::vec3(1.f, 0.f, 0.f));
//        GLint uniModel = glGetUniformLocation(shaderProgram, "model");
//        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
//
//        // set up the viewing (camera) matrix (3D world coords -> 3D camera coords)
//        glm::mat4 view = glm::lookAt(glm::vec3(3.f, 0.f, 0.f),   // Cam position
//                                     glm::vec3(0.f, 0.f, 0.f),   // Look at point
//                                     glm::vec3(0.f, 0.f, 1.f));  // Up
//        GLint uniView = glGetUniformLocation(shaderProgram, "view");
//        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
//
//        // set up the projection matrix (3D camera coords -> 2D screen coords)
//        glm::mat4 proj = glm::perspective(3.14f / 4.f, aspect, 1.f, 10.f);
//        GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
//        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
//
//        glUseProgram(shaderProgram);        // Set the active shader program
//        glBindVertexArray(vao);             // Bind the VAO for the shader(s) we are using
//        glDrawArrays(GL_TRIANGLES, 0, 36);  // Number of vertices
//
//        SDL_GL_SwapWindow(window);  // Double buffering
//    }
//
//    glDeleteProgram(shaderProgram);
//    glDeleteShader(fragmentShader);
//    glDeleteShader(vertexShader);
//    glDeleteBuffers(2, vbo);
//    glDeleteVertexArrays(1, &vao);
//    SDL_GL_DeleteContext(context);
//    SDL_Quit();
//
//    return 0;
//}

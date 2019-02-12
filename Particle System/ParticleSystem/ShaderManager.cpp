#include <fstream>
#include "Constants.h"
#include "ParticleManager.h"
#include "ShaderManager.h"

GLuint ShaderManager::ParticleComputeShader;
RenderShader ShaderManager::EnvironmentShader;
RenderShader ShaderManager::ParticleShader;

std::map<int, std::function<void(ShaderAttributes)>> ShaderManager::ShaderFunctions;

void ShaderManager::InitShaders() {
    EnvironmentShader.Program = CompileRenderShader("environment-Vertex.glsl", "environment-Fragment.glsl");
    ParticleShader.Program = CompileRenderShader("particle-Vertex.glsl", "particle-Fragment.glsl");
    ParticleComputeShader = CompileComputeShaderProgram("computeShader.glsl");

    InitEnvironmentShaderAttributes();
    InitParticleShaderAttributes();
}

void ShaderManager::Cleanup() {
    glDeleteProgram(EnvironmentShader.Program);
    glDeleteProgram(ParticleComputeShader);
    glDeleteProgram(ParticleShader.Program);

    glDeleteVertexArrays(1, &EnvironmentShader.VAO);
    glDeleteVertexArrays(1, &ParticleShader.VAO);
}

void ShaderManager::ActivateShader(RenderShader shader) {
    glUseProgram(shader.Program);
    glBindVertexArray(shader.VAO);

    for (const auto& func : ShaderFunctions) {
        func.second(shader.Attributes);  // absolutely ridiculous
    }
}

void ShaderManager::ApplyToEachRenderShader(std::function<void(ShaderAttributes)> Func, int shaderFunctionId) {
    ShaderFunctions[shaderFunctionId] = Func;  // This is absolute madness
}

// Tell OpenGL how to set fragment shader input
void ShaderManager::InitEnvironmentShaderAttributes() {
    // First build a Vertex Array Object (VAO) to store mapping of shader attributes to VBO
    glGenVertexArrays(1, &EnvironmentShader.VAO);  // Create a VAO
    glBindVertexArray(EnvironmentShader.VAO);      // Bind the above created VAO to the current context

    GLint posAttrib = glGetAttribLocation(EnvironmentShader.Program, "position");
    glVertexAttribPointer(posAttrib, VALUES_PER_POSITION, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float), POSITION_OFFSET);
    // Attribute, vals/attrib., type, isNormalized, stride, offset
    glEnableVertexAttribArray(posAttrib);

    GLint normAttrib = glGetAttribLocation(EnvironmentShader.Program, "inNormal");
    glVertexAttribPointer(normAttrib, VALUES_PER_NORMAL, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float),
                          (void*)(NORMAL_OFFSET * sizeof(float)));
    glEnableVertexAttribArray(normAttrib);

    GLint texAttrib = glGetAttribLocation(EnvironmentShader.Program, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, VALUES_PER_TEXCOORD, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float),
                          (void*)(TEXCOORD_OFFSET * sizeof(float)));

    // GLint colAttrib = glGetAttribLocation(EnvironmentShader.Program, "inColor");
    GLint uniColor = glGetUniformLocation(EnvironmentShader.Program, "inColor");
    GLint uniTexID = glGetUniformLocation(EnvironmentShader.Program, "texID");
    GLint uniView = glGetUniformLocation(EnvironmentShader.Program, "view");
    GLint uniProj = glGetUniformLocation(EnvironmentShader.Program, "proj");
    GLint uniModel = glGetUniformLocation(EnvironmentShader.Program, "model");
    GLint uniSpecFactor = glGetUniformLocation(EnvironmentShader.Program, "specFactor");

    EnvironmentShader.Attributes.position = posAttrib;
    EnvironmentShader.Attributes.normals = normAttrib;
    EnvironmentShader.Attributes.texCoord = texAttrib;
    EnvironmentShader.Attributes.color = uniColor;
    EnvironmentShader.Attributes.texID = uniTexID;
    EnvironmentShader.Attributes.view = uniView;
    EnvironmentShader.Attributes.projection = uniProj;
    EnvironmentShader.Attributes.model = uniModel;
    EnvironmentShader.Attributes.specFactor = uniSpecFactor;

    EnvironmentShader.Attributes.screenSize = EnvironmentShader.Attributes.spriteSize = -1;

    glBindVertexArray(0);  // Unbind the VAO in case we want to create a new one
}

void ShaderManager::InitParticleShaderAttributes() {
    glGenVertexArrays(1, &ParticleShader.VAO);
    glBindVertexArray(ParticleShader.VAO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ParticleManager::posSSbo);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleManager::posSSbo);
    GLint posAttrib = glGetAttribLocation(ParticleShader.Program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(position), (void*)0);
    glEnableVertexAttribArray(posAttrib);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ParticleManager::colSSbo);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleManager::colSSbo);
    GLint colAttrib = glGetAttribLocation(ParticleShader.Program, "inColor");
    glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(color), (void*)0);
    glEnableVertexAttribArray(colAttrib);

    GLint uniView = glGetUniformLocation(ParticleShader.Program, "view");
    GLint uniProj = glGetUniformLocation(ParticleShader.Program, "proj");
    GLint uniScreenSize = glGetUniformLocation(ParticleShader.Program, "screenSize");
    GLint uniSpriteSize = glGetUniformLocation(ParticleShader.Program, "spriteSize");

    ParticleShader.Attributes.position = posAttrib;
    ParticleShader.Attributes.color = colAttrib;
    ParticleShader.Attributes.view = uniView;
    ParticleShader.Attributes.projection = uniProj;
    ParticleShader.Attributes.screenSize = uniScreenSize;
    ParticleShader.Attributes.spriteSize = uniSpriteSize;

    // Make it obvious that these values aren't used
    ParticleShader.Attributes.normals = ParticleShader.Attributes.texCoord = ParticleShader.Attributes.texID =
        ParticleShader.Attributes.model = ParticleShader.Attributes.specFactor = -1;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint ShaderManager::CompileRenderShader(const std::string& vertex_shader_file, const std::string& fragment_shader_file) {
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    GLuint program;

    // check GLSL version
    printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    // Read source code from shader files
    vs_text = ReadShaderSource(vertex_shader_file.c_str());
    fs_text = ReadShaderSource(fragment_shader_file.c_str());

    // error check
    if (vs_text == NULL) {
        printf("Failed to read from vertex shader file %s\n", vertex_shader_file.c_str());
        exit(1);
    } else if (DEBUG_ON) {
        printf("Vertex Shader (%s):\n=====================\n", vertex_shader_file.c_str());
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }
    if (fs_text == NULL) {
        printf("Failed to read from fragment shader file %s\n", fragment_shader_file.c_str());
        exit(1);
    } else if (DEBUG_ON) {
        printf("\nFragment Shader (%s):\n=====================\n", fragment_shader_file.c_str());
        printf("%s\n", fs_text);
        printf("=====================\n\n");
    }

    // Load Vertex Shader
    const char* vv = vs_text;
    glShaderSource(vertex_shader, 1, &vv, NULL);  // Read source
    glCompileShader(vertex_shader);               // Compile shaders
    VerifyShaderCompiled(vertex_shader);          // Check for errors

    // Load Fragment Shader
    const char* ff = fs_text;
    glShaderSource(fragment_shader, 1, &ff, NULL);
    glCompileShader(fragment_shader);
    VerifyShaderCompiled(fragment_shader);

    // Create the program
    program = glCreateProgram();

    // Attach shaders to program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    // Link and set program to use
    glLinkProgram(program);

    glDetachShader(program, vertex_shader);
    glDeleteShader(vertex_shader);

    glDetachShader(program, fragment_shader);
    glDeleteShader(fragment_shader);

    // https://github.com/Crisspl/GPU-particle-system/blob/master/particles/main.cpp
    GLint success;
    GLchar infoLog[0x200];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 0x200, nullptr, infoLog);
        printf("CS LINK ERROR: %s\n", infoLog);
    }

    return program;
}

GLuint ShaderManager::CompileComputeShaderProgram(const std::string& compute_shader_file) {
    GLuint compute_shader;
    GLchar* vs_text;
    GLuint computeProgram;

    // check GLSL version
    printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Create shader handlers
    compute_shader = glCreateShader(GL_COMPUTE_SHADER);

    // Read source code from shader files
    vs_text = ReadShaderSource(compute_shader_file.c_str());

    // error check
    if (vs_text == NULL) {
        printf("Failed to read from compute shader file %s\n", compute_shader_file.c_str());
        exit(1);
    } else if (DEBUG_ON) {
        printf("Compute Shader (%s):\n=====================\n", compute_shader_file.c_str());
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }

    // Load Compute Shader
    const char* vv = vs_text;
    glShaderSource(compute_shader, 1, &vv, NULL);  // Read source
    glCompileShader(compute_shader);               // Compile shaders
    VerifyShaderCompiled(compute_shader);          // Check for errors

    // Create the program
    computeProgram = glCreateProgram();

    // Attach shaders to program
    glAttachShader(computeProgram, compute_shader);

    // Link and set program to use
    glLinkProgram(computeProgram);

    glDetachShader(computeProgram, compute_shader);
    glDeleteShader(compute_shader);

    // https://github.com/Crisspl/GPU-particle-system/blob/master/particles/main.cpp
    GLint success;
    GLchar infoLog[0x200];
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(computeProgram, 0x200, nullptr, infoLog);
        printf("CS LINK ERROR: %s\n", infoLog);
    }

    return computeProgram;
}

// Create a NULL-terminated string by reading the provided file
char* ShaderManager::ReadShaderSource(const char* shaderFile) {
    // FILE* fp;
    // long length;
    // char* buffer;

    //// open the file containing the text of the shader code
    // fopen_s(&fp, shaderFile, "r");

    //// check for errors in opening the file
    // if (fp == NULL) {
    //    printf("Can't open shader source file %s\n", shaderFile);
    //    return NULL;
    //}

    //// determine the file size
    // fseek(fp, 0, SEEK_END);  // move position indicator to the end of the file;
    // length = ftell(fp);      // return the value of the current position

    //// allocate a buffer with the indicated number of bytes, plus one
    // buffer = new char[length + 1];
    // for (int i = 0; i < length + 1; i++) {
    //    buffer[i] = '\0';
    //}

    //// read the appropriate number of bytes from the file
    // fseek(fp, 0, SEEK_SET);        // move position indicator to the start of the file
    // fread(buffer, 1, length, fp);  // read all of the bytes

    //// append a NULL character to indicate the end of the string
    // buffer[length] = '\0';

    //// close the file
    // fclose(fp);

    //// return the string
    // return buffer;
    // https://stackoverflow.com/questions/18816126/c-read-the-whole-file-in-buffer
    std::ifstream file(shaderFile, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size + 1];
    if (!file.read(buffer, size)) {
        printf("Failed to read file %s\n", shaderFile);
    }

    buffer[size] = '\0';

    return buffer;
}

void ShaderManager::VerifyShaderCompiled(GLuint shader) {
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printf("Vertex shader failed to compile:\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }
}

#include "Constants.h"
#include "ParticleManager.h"
#include "ShaderManager.h"

GLuint ShaderManager::Environment_Render_Shader;
GLuint ShaderManager::Particle_Compute_Shader;
GLuint ShaderManager::Particle_Render_Shader;

ShaderAttributes ShaderManager::EnvironmentAttributes;
ShaderAttributes ShaderManager::ParticleAttributes;
ShaderAttributes ShaderManager::RenderAttributes[NUM_RENDER_SHADERS];

GLuint ShaderManager::Environment_VAO;
GLuint ShaderManager::Particle_VAO;

std::map<int, std::function<void(ShaderAttributes)>> ShaderManager::ShaderFunctions;

void ShaderManager::InitShaders() {
    Environment_Render_Shader = CompileRenderShader("environment-Vertex.glsl", "environment-Fragment.glsl");
    Particle_Render_Shader = CompileRenderShader("particle-Vertex.glsl", "particle-Fragment.glsl");
    Particle_Compute_Shader = CompileComputerShaderProgram("computeShader.glsl");

    InitEnvironmentShaderAttributes();
    InitParticleShaderAttributes();

    RenderAttributes[0] = EnvironmentAttributes;
    RenderAttributes[1] = ParticleAttributes;
}

void ShaderManager::Cleanup() {
    glDeleteProgram(Environment_Render_Shader);
    glDeleteProgram(Particle_Render_Shader);
    glDeleteProgram(Particle_Compute_Shader);

    glDeleteVertexArrays(1, &Environment_VAO);
    glDeleteVertexArrays(1, &Particle_VAO);
}

void ShaderManager::ActivateShader(GLuint shader_program) {
    glUseProgram(shader_program);

    ShaderAttributes* attributes;
    GLuint vao;
    if (shader_program == Environment_Render_Shader) {
        attributes = &EnvironmentAttributes;
        vao = Environment_VAO;
    } else if (shader_program == Particle_Render_Shader) {
        attributes = &ParticleAttributes;
        vao = Particle_VAO;
    } else {
        printf("No corresponding attributes found for shader program that was activated");
        return;
    }

    glBindVertexArray(vao);

    for (const auto& func : ShaderFunctions) {
        func.second(*attributes);  // absolutely ridiculous
    }
}

void ShaderManager::ApplyToEachRenderShader(std::function<void(ShaderAttributes)> Func, int shaderFunctionId) {
    ShaderFunctions[shaderFunctionId] = Func;  // This is absolute madness
}

// Tell OpenGL how to set fragment shader input
void ShaderManager::InitEnvironmentShaderAttributes() {
    // First build a Vertex Array Object (VAO) to store mapping of shader attributes to VBO
    glGenVertexArrays(1, &Environment_VAO);  // Create a VAO
    glBindVertexArray(Environment_VAO);      // Bind the above created VAO to the current context

    GLint posAttrib = glGetAttribLocation(Environment_Render_Shader, "position");
    glVertexAttribPointer(posAttrib, VALUES_PER_POSITION, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float), POSITION_OFFSET);
    // Attribute, vals/attrib., type, isNormalized, stride, offset
    glEnableVertexAttribArray(posAttrib);

    GLint normAttrib = glGetAttribLocation(Environment_Render_Shader, "inNormal");
    glVertexAttribPointer(normAttrib, VALUES_PER_NORMAL, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float),
                          (void*)(NORMAL_OFFSET * sizeof(float)));
    glEnableVertexAttribArray(normAttrib);

    GLint texAttrib = glGetAttribLocation(Environment_Render_Shader, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, VALUES_PER_TEXCOORD, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float),
                          (void*)(TEXCOORD_OFFSET * sizeof(float)));

    // GLint colAttrib = glGetAttribLocation(Environment_Render_Shader, "inColor");
    GLint uniColor = glGetUniformLocation(Environment_Render_Shader, "inColor");
    GLint uniTexID = glGetUniformLocation(Environment_Render_Shader, "texID");
    GLint uniView = glGetUniformLocation(Environment_Render_Shader, "view");
    GLint uniProj = glGetUniformLocation(Environment_Render_Shader, "proj");
    GLint uniModel = glGetUniformLocation(Environment_Render_Shader, "model");

    EnvironmentAttributes.position = posAttrib;
    EnvironmentAttributes.normals = normAttrib;
    EnvironmentAttributes.texCoord = texAttrib;
    EnvironmentAttributes.color = uniColor;
    EnvironmentAttributes.texID = uniTexID;
    EnvironmentAttributes.view = uniView;
    EnvironmentAttributes.projection = uniProj;
    EnvironmentAttributes.model = uniModel;

    glBindVertexArray(0);  // Unbind the VAO in case we want to create a new one
}

void ShaderManager::InitParticleShaderAttributes() {
    glGenVertexArrays(1, &Particle_VAO);
    glBindVertexArray(Particle_VAO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ParticleManager::posSSbo);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleManager::posSSbo);
    GLint posAttrib = glGetAttribLocation(Particle_Render_Shader, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(position), (void*)0);
    glEnableVertexAttribArray(posAttrib);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ParticleManager::colSSbo);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleManager::colSSbo);
    GLint colAttrib = glGetAttribLocation(Particle_Render_Shader, "inColor");
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(color), (void*)0);
    glEnableVertexAttribArray(colAttrib);

    GLint uniView = glGetUniformLocation(Particle_Render_Shader, "view");
    GLint uniProj = glGetUniformLocation(Particle_Render_Shader, "proj");

    ParticleAttributes.position = posAttrib;
    ParticleAttributes.color = colAttrib;
    ParticleAttributes.view = uniView;
    ParticleAttributes.projection = uniProj;

    // Make it obvious that these values aren't used
    ParticleAttributes.normals = ParticleAttributes.texCoord = ParticleAttributes.texID = ParticleAttributes.model = -1;

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

GLuint ShaderManager::CompileComputerShaderProgram(const std::string& compute_shader_file) {
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
    FILE* fp;
    long length;
    char* buffer;

    // open the file containing the text of the shader code
    fopen_s(&fp, shaderFile, "r");

    // check for errors in opening the file
    if (fp == NULL) {
        printf("Can't open shader source file %s\n", shaderFile);
        return NULL;
    }

    // determine the file size
    fseek(fp, 0, SEEK_END);  // move position indicator to the end of the file;
    length = ftell(fp);      // return the value of the current position

    // allocate a buffer with the indicated number of bytes, plus one
    buffer = new char[length + 1];
    for (int i = 0; i < length + 1; i++) {
        buffer[i] = '\0';
    }

    // read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET);        // move position indicator to the start of the file
    fread(buffer, 1, length, fp);  // read all of the bytes

    // append a NULL character to indicate the end of the string
    buffer[length] = '\0';

    // close the file
    fclose(fp);

    // return the string
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

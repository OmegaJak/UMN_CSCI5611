#include "constants.h"
#include "shader_manager.h"

int ShaderManager::InitShader(const std::string& vertex_shader_file, const std::string& fragment_shader_file) {
    Textured_Shader = CompileShaderProgram(vertex_shader_file, fragment_shader_file);
    InitShaderAttributes();

    return Textured_Shader;
}

void ShaderManager::Cleanup() {
    glDeleteProgram(Textured_Shader);
}

// Tell OpenGL how to set fragment shader input
void ShaderManager::InitShaderAttributes() {
    GLint posAttrib = glGetAttribLocation(Textured_Shader, "position");
    glVertexAttribPointer(posAttrib, VALUES_PER_POSITION, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float), POSITION_OFFSET);
    // Attribute, vals/attrib., type, isNormalized, stride, offset
    glEnableVertexAttribArray(posAttrib);

    GLint normAttrib = glGetAttribLocation(Textured_Shader, "inNormal");
    glVertexAttribPointer(normAttrib, VALUES_PER_NORMAL, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float),
                          (void*)(NORMAL_OFFSET * sizeof(float)));
    glEnableVertexAttribArray(normAttrib);

    GLint texAttrib = glGetAttribLocation(Textured_Shader, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, VALUES_PER_TEXCOORD, GL_FLOAT, GL_FALSE, ATTRIBUTE_STRIDE * sizeof(float),
                          (void*)(TEXCOORD_OFFSET * sizeof(float)));

    GLint uniColor = glGetUniformLocation(Textured_Shader, "inColor");
    GLint uniTexID = glGetUniformLocation(Textured_Shader, "texID");
    GLint uniView = glGetUniformLocation(Textured_Shader, "view");
    GLint uniProj = glGetUniformLocation(Textured_Shader, "proj");
    GLint uniModel = glGetUniformLocation(Textured_Shader, "model");

    Attributes.position = posAttrib;
    Attributes.normals = normAttrib;
    Attributes.texCoord = texAttrib;
    Attributes.color = uniColor;
    Attributes.texID = uniTexID;
    Attributes.view = uniView;
    Attributes.projection = uniProj;
    Attributes.model = uniModel;
}

GLuint ShaderManager::CompileShaderProgram(const std::string& vertex_shader_file, const std::string& fragment_shader_file) {
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
        printf("Vertex Shader:\n=====================\n");
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }
    if (fs_text == NULL) {
        printf("Failed to read from fragment shader file %s\n", fragment_shader_file.c_str());
        exit(1);
    } else if (DEBUG_ON) {
        printf("\nFragment Shader:\n=====================\n");
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

    return program;
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

GLuint ShaderManager::Textured_Shader;

ShaderAttributes ShaderManager::Attributes;

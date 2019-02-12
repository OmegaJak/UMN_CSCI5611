#pragma once
#include <functional>
#include <map>
#include <string>
#include "glad.h"

typedef struct {
    GLint position;
    GLint normals;
    GLint texCoord;
    GLint view;
    GLint projection;
    GLint model;
    GLint color;
    GLint texID;
    GLint screenSize;
    GLint spriteSize;
    GLint specFactor;
} ShaderAttributes;

typedef struct {
    GLuint Program;
    ShaderAttributes Attributes;
    GLuint VAO;
} RenderShader;

class ShaderManager {
   public:
    static void InitShaders();
    static void Cleanup();
    static void ActivateShader(RenderShader shader);
    static void ApplyToEachRenderShader(std::function<void(ShaderAttributes)> Func, int shaderFunctionId);

    static RenderShader EnvironmentShader;
    static RenderShader ParticleShader;
    static GLuint ParticleComputeShader;

   private:
    static void InitEnvironmentShaderAttributes();
    static void InitParticleShaderAttributes();
    static GLuint CompileRenderShader(const std::string& vertex_shader_file, const std::string& fragment_shader_file);
    static GLuint CompileComputeShaderProgram(const std::string& compute_shader_file);
    static char* ReadShaderSource(const char* shaderFile);
    static void VerifyShaderCompiled(GLuint shader);

    static std::map<int, std::function<void(ShaderAttributes)>> ShaderFunctions;
};

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
} ShaderAttributes;

class ShaderManager {
   public:
    static void InitShaders();
    static void Cleanup();
    static void ActivateShader(GLuint shader_program);
    static void ApplyToEachRenderShader(std::function<void(ShaderAttributes)> Func, int shaderFunctionId);

    static GLuint Environment_Render_Shader;
    static GLuint Particle_Compute_Shader;
    static GLuint Particle_Render_Shader;

    static ShaderAttributes EnvironmentAttributes;
    static ShaderAttributes ParticleAttributes;
    static ShaderAttributes RenderAttributes[];

    static GLuint Environment_VAO;
    static GLuint Particle_VAO;

   private:
    static void InitEnvironmentShaderAttributes();
    static void InitParticleShaderAttributes();
    static GLuint CompileRenderShader(const std::string& vertex_shader_file, const std::string& fragment_shader_file);
    static GLuint CompileComputerShaderProgram(const std::string& compute_shader_file);
    static char* ReadShaderSource(const char* shaderFile);
    static void VerifyShaderCompiled(GLuint shader);

    static std::map<int, std::function<void(ShaderAttributes)>> ShaderFunctions;
};

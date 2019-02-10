#include <SDL.h>
#include <cstdio>
#include "ShaderManager.h"
#include "TextureManager.h"
#include "glad.h"

void TextureManager::InitTextures() {
    glBindVertexArray(ShaderManager::EnvironmentShader.VAO);

    // Allocate Texture 0
    InitTexture(&tex0, "stone_floor.bmp");

    glBindVertexArray(0);
}

void TextureManager::Update() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    glUniform1i(glGetUniformLocation(ShaderManager::EnvironmentShader.Program, "tex0"), 0);
}

void TextureManager::InitTexture(GLuint* tex_location, const char* file) {
    SDL_Surface* surface = SDL_LoadBMP(file);
    if (surface == NULL) {  // If it failed, print the error
        printf("Error: \"%s\"\n", SDL_GetError());
        exit(1);
    }
    glGenTextures(1, tex_location);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *tex_location);

    // What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);  // Mip maps the texture

    SDL_FreeSurface(surface);
}

GLuint TextureManager::tex0;

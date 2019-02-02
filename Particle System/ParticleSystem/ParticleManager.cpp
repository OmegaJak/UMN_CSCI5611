#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>
#include "ParticleManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "glad.h"

const float radius = 0.5;
const float bounceFactor = -0.8;

ParticleManager::ParticleManager() {
    Positions = std::vector<glm::vec3>();
    Velocities = std::vector<glm::vec3>();
    Lifetimes = std::vector<float>();

    _particleModel = new Model("models/sphere.txt");
}

void ParticleManager::MoveParticles(float dt) {
    for (int i = 0; i < Positions.size(); i++) {
        Positions[i] += Velocities[i] * dt;
        Velocities[i] += glm::vec3(0, 0, -9.86) * dt;
        Lifetimes[i] += dt;

        if (Positions[i].x > 10 - 2 * radius && Velocities[i].x > 0) {
            Positions[i].x = 10 - 2 * radius;
            Velocities[i].x *= bounceFactor;
        } else if (Positions[i].x < 0 && Velocities[i].x < 0) {
            Positions[i].x = 0;
            Velocities[i].x *= bounceFactor;
        }

        if (Positions[i].y > 10 - 2 * radius && Velocities[i].y > 0) {
            Positions[i].y = 10 - 2 * radius;
            Velocities[i].y *= bounceFactor;
        } else if (Positions[i].y < 0 && Velocities[i].y < 0) {
            Positions[i].y = 0;
            Velocities[i].y *= bounceFactor;
        }

        if (Positions[i].z < radius) {
            Positions[i].z = radius;
            Velocities[i].x *= 0.95;
            Velocities[i].y *= 0.95;
            Velocities[i].z *= bounceFactor;

            if (Velocities[i].z < 1.0f) {
                DeleteParticle(i);
                i--;
                continue;
            }
        }

        if (Lifetimes[i] > 15) {
            DeleteParticle(i);
            i--;
            continue;
        }
    }
}

void ParticleManager::SpawnParticle(const glm::vec3& position, const glm::vec3& velocity) {
    Positions.push_back(position);
    Velocities.push_back(velocity);
    Lifetimes.push_back(0);
}

void ParticleManager::RenderParticles() {
    auto mat = glm::mat4();
    auto color = glm::vec3(0.8f, 0.1f, 0.3f);
    for (int i = 0; i < Positions.size(); i++) {
        mat[3] = glm::vec4(Positions[i], 1);
        glUniformMatrix4fv(ShaderManager::Attributes.model, 1, GL_FALSE, glm::value_ptr(mat));  // pass model matrix to shader
        glUniform1i(ShaderManager::Attributes.texID, UNTEXTURED);                               // Set which texture to use
        glUniform3fv(ShaderManager::Attributes.color, 1, glm::value_ptr(color));                // Update the color

        glDrawArrays(GL_TRIANGLES, _particleModel->vbo_vertex_start_index_, _particleModel->NumVerts());  // Draw it!
    }
}

void ParticleManager::DeleteParticle(int index) {
    Positions.erase(Positions.begin() + index);
    Velocities.erase(Velocities.begin() + index);
    Lifetimes.erase(Lifetimes.begin() + index);
}

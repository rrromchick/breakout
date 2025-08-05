#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "texture.h"
#include "game_object.h"

struct Particle {
    glm::vec2 m_position, m_velocity;
    glm::vec4 m_color;
    float m_life;

    Particle() : m_position(0.0f), m_velocity(0.0f), m_color(1.0f), m_life(0.0f) {}
};

class ParticleGenerator {
public:
    ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);
    void update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
    void draw();
private:
    std::vector<Particle> m_particles;
    unsigned int m_amount;

    Shader m_shader;
    Texture2D m_texture;
    unsigned int m_VAO;

    void init();
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};

#endif
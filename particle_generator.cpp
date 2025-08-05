#include "particle_generator.h"

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount)
    : m_shader(shader), m_texture(texture), m_amount(amount)
{
    this->init();
}

void ParticleGenerator::update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset) {
    for (unsigned int i = 0; i < newParticles; ++i) {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->m_particles[unusedParticle], object, offset);
    }

    for (unsigned int i = 0; i < this->m_amount; ++i) {
        Particle& p = this->m_particles[i];
        p.m_life -= dt;
        if (p.m_life > 0.0f) {
            p.m_position -= p.m_velocity * dt;
            p.m_color.a -= dt * 2.5f;
        }
    }
}

void ParticleGenerator::draw() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    this->m_shader.use();
    for (Particle particle : this->m_particles) {
        if (particle.m_life > 0.0f) {
            this->m_shader.setVector2f("offset", particle.m_position);
            this->m_shader.setVector4f("color", particle.m_color);
            this->m_texture.bind();
            glBindVertexArray(this->m_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init() {
    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    glGenVertexArrays(1, &this->m_VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    for (unsigned int i = 0; i < this->m_amount; ++i) {
        this->m_particles.push_back(Particle());
    }
}

unsigned int lastUsedParticle = 0;
unsigned int ParticleGenerator::firstUnusedParticle() {
    for (unsigned int i = lastUsedParticle; i < this->m_amount; ++i) {
        if (this->m_particles[i].m_life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }

    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (this->m_particles[i].m_life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }

    lastUsedParticle = 0;
    return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset) {
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle.m_position = object.m_position + random + offset;
    particle.m_color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.m_life = 1.0f;
    particle.m_velocity = object.m_velocity * 0.1f;
}
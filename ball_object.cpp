#include "ball_object.h"

BallObject::BallObject()
    : GameObject(), m_radius(12.5f), m_stuck(true) {}

BallObject::BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite)
    : GameObject(pos, glm::vec2(radius * 2.0f, radius * 2.0f), sprite, glm::vec3(1.0), velocity), m_radius(radius), m_stuck(true)
{}

glm::vec2 BallObject::move(float dt, unsigned int window_width) {
    if (!this->m_stuck) {
        this->m_position += this->m_velocity * dt;

        if (this->m_position.x <= 0.0f) {
            this->m_velocity.x = -this->m_velocity.x;
            this->m_position.x = 0.0f;
        } else if (this->m_position.x + this->m_size.x >= window_width) {
            this->m_velocity.x = -this->m_velocity.x;
            this->m_position.x = window_width - this->m_size.x;
        } 
        if (this->m_position.y <= 0.0f) {
            this->m_velocity.y = -this->m_velocity.y;
            this->m_position.y = 0.0f;
        }
    }

    return this->m_position;
}

void BallObject::reset(glm::vec2 position, glm::vec2 velocity) {
    this->m_position = position;
    this->m_velocity = velocity;
    this->m_stuck = true;
}
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"

class GameObject {
public:
    glm::vec2 m_position, m_size, m_velocity;
    glm::vec3 m_color;
    float m_rotation;
    bool m_isSolid;
    bool m_destroyed;

    Texture2D m_sprite;

    GameObject();
    GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f), glm::vec2 velocity = glm::vec2(0.0f, 0.0f));

    virtual void draw(SpriteRenderer& renderer);
};

#endif
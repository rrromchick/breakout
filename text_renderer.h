#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "shader.h"

struct Character {
    unsigned int m_textureID;
    glm::ivec2 m_size;
    glm::ivec2 m_bearing;
    unsigned int m_advance;
};

class TextRenderer {
public:
    std::map<char, Character> m_characters;
    Shader m_textShader;

    TextRenderer(unsigned int width, unsigned int height);
    void load(std::string font, unsigned int fontSize);
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));
private:
    unsigned int VAO, VBO;
};

#endif
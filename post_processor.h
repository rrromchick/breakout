#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "sprite_renderer.h"
#include "shader.h"

class PostProcessor {
public:
    Shader m_postProcessingShader;
    Texture2D m_texture;
    unsigned int m_width, m_height;

    bool m_confuse, m_chaos, m_shake;

    PostProcessor(Shader shader, unsigned int width, unsigned int height);
    void beginRender();
    void endRender();
    void render(float time);
private:
    unsigned int MSFBO, FBO;
    unsigned int RBO;
    unsigned int VAO;

    void initRenderData();
};

#endif
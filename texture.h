#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture2D {
public:
    unsigned int ID;
    
    unsigned int m_width, m_height;

    unsigned int m_internalFormat;
    unsigned int m_imageFormat;

    unsigned int m_wrapS;
    unsigned int m_wrapT;
    unsigned int m_filterMin;
    unsigned int m_filterMax;

    Texture2D();

    void generate(unsigned int width, unsigned int height, unsigned char* data);

    void bind() const;
};

#endif
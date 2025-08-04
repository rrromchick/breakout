#include <iostream>

#include "texture.h"

Texture2D::Texture2D() 
    : m_width(0), m_height(0), m_internalFormat(GL_RGB), m_imageFormat(GL_RGB), m_wrapS(GL_REPEAT), m_wrapT(GL_REPEAT),
    m_filterMin(GL_LINEAR), m_filterMax(GL_LINEAR)
{
    glGenTextures(1, &this->ID);
}

void Texture2D::generate(unsigned int width, unsigned int height, unsigned char* data) {
    this->m_width = width;
    this->m_height = height;

    glBindTexture(GL_TEXTURE_2D, this->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, this->m_internalFormat, width, height, 0, this->m_imageFormat, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->m_wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->m_wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->m_filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->m_filterMax);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::bind() const {
    glBindTexture(GL_TEXTURE_2D, this->ID);
}
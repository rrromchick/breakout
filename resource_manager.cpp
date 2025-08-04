#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// instantiate static variables
std::map<std::string, Texture2D> ResourceManager::m_textures;
std::map<std::string, Shader> ResourceManager::m_shaders;

Shader ResourceManager::loadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name) {
    m_shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return m_shaders[name];
}

Shader& ResourceManager::getShader(std::string name) {
    return m_shaders[name];
}

Texture2D ResourceManager::loadTexture(const char* file, bool alpha, std::string name) {
    m_textures[name] = loadTextureFromFile(file, alpha);
    return m_textures[name];
}

Texture2D& ResourceManager::getTexture(std::string name) {
    return m_textures[name];
}

void ResourceManager::clear() {
    for (auto iter : m_shaders) {
        glDeleteProgram(iter.second.ID);
    }
    for (auto iter : m_textures) {
        glDeleteTextures(1, &iter.second.ID);
    }
}

Shader ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();

        vertexShaderFile.close();
        fragmentShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if (gShaderFile != nullptr) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::exception& e) {
        std::cout << "ERROR::SHADER: Failed to read shader files." << std::endl;
    } 
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();

    Shader shader;
    shader.compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha) {
    Texture2D texture;
    if (alpha) {
        texture.m_internalFormat = GL_RGBA;
        texture.m_imageFormat = GL_RGBA;
    }

    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

    texture.generate(width, height, data);

    stbi_image_free(data);
    return texture;
}
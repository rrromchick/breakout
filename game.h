#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game_level.h"

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
const float PLAYER_VELOCITY(500.0f);

class Game {
public:
    GameState m_state;
    bool m_keys[1024];
    unsigned int m_width, m_height;
    std::vector<GameLevel> m_levels;
    unsigned int m_level;

    Game(unsigned int width, unsigned int height);
    ~Game();

    void init();

    void processInput(float dt);
    void update(float dt);
    void render();
};

#endif
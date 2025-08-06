#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game_level.h"
#include "powerup.h"

#include <algorithm>

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

enum Direction {
    UP,
    RIGHT,
    DOWN, 
    LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
const float PLAYER_VELOCITY(500.0f);
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
const float BALL_RADIUS = 12.5f;

class Game {
public:
    GameState m_state;
    bool m_keys[1024];
    bool m_keysProcessed[1024];
    unsigned int m_width, m_height;
    std::vector<GameLevel> m_levels;
    std::vector<PowerUp> m_powerups;
    unsigned int m_level;
    unsigned int m_lives;

    Game(unsigned int width, unsigned int height);
    ~Game();

    void init();

    void processInput(float dt);
    void update(float dt);
    void render();
    void doCollisions();

    void resetLevel();
    void resetPlayer();

    void spawnPowerUps(GameObject& block);
    void updatePowerUps(float dt);
};

#endif
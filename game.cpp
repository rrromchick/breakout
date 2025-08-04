#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"

SpriteRenderer* renderer;
GameObject* player;

Game::Game(unsigned int width, unsigned int height) 
    : m_state(GAME_ACTIVE), m_keys(), m_width(width), m_height(height)
{}

Game::~Game() {
    delete renderer;
    delete player;
}

void Game::init() {
    FileSystem::chDir();
    FileSystem::chDir();

    ResourceManager::loadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->m_width),
        static_cast<float>(this->m_height), 0.0f, -1.0f, 1.0f);
    ResourceManager::getShader("sprite").use().setInteger("image", 0);
    ResourceManager::getShader("sprite").setMatrix4("projection", projection);

    renderer = new SpriteRenderer(ResourceManager::getShader("sprite"));

    ResourceManager::loadTexture("textures/background.jpg", false, "background");
    ResourceManager::loadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::loadTexture("textures/block.png", false, "block");
    ResourceManager::loadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::loadTexture("textures/paddle.png", true, "paddle");

    GameLevel one; one.load("levels/one.lvl", this->m_width, this->m_height / 2);
    GameLevel two; two.load("levels/two.lvl", this->m_width, this->m_height / 2);
    GameLevel three; three.load("levels/three.lvl", this->m_width, this->m_height / 2);
    GameLevel four; four.load("levels/four.lvl", this->m_width, this->m_height / 2);
    this->m_levels.push_back(one);
    this->m_levels.push_back(two);
    this->m_levels.push_back(three);
    this->m_levels.push_back(four);
    this->m_level = 0;
    
    glm::vec2 playerPos = glm::vec2(this->m_width / 2.0f - PLAYER_SIZE.x / 2.0f, this->m_height - PLAYER_SIZE.y);
    player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::getTexture("paddle"));
}

void Game::update(float dt) {}

void Game::processInput(float dt) {
    if (this->m_state == GAME_ACTIVE) {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->m_keys[GLFW_KEY_A]) {
            if (player->m_position.x >= 0.0f) {
                player->m_position.x -= velocity;
            }
        }
        if (this->m_keys[GLFW_KEY_D]) {
            if (player->m_position.x <= this->m_width - player->m_size.x) {
                player->m_position.x += velocity;
            }
        }
    }
}

void Game::render() {
    if (this->m_state == GAME_ACTIVE) {
        renderer->drawSprite(ResourceManager::getTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->m_width, this->m_height), 0.0f);
        this->m_levels[this->m_level].draw(*renderer);
        player->draw(*renderer);
    }
}
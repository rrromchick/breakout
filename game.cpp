#include <algorithm>
#include <sstream>
#include <iostream>

#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"
#include "post_processor.h"
#include "text_renderer.h"

SpriteRenderer* renderer;
GameObject* player;
BallObject* ball;
ParticleGenerator* particles;
PostProcessor* effects;
TextRenderer* text;

float shakeTime = 0.0f;

Game::Game(unsigned int width, unsigned int height) 
    : m_state(GAME_MENU), m_keys(), m_keysProcessed(), m_width(width), m_height(height), m_lives(3)
{}

Game::~Game() {
    delete renderer;
    delete player;
    delete ball;
    delete particles;
    delete effects;
    delete text;
}

void Game::init() {
    FileSystem::chDir();
    FileSystem::chDir();

    ResourceManager::loadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::loadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");
    ResourceManager::loadShader("shaders/post_processing.vs", "shaders/post_processing.fs", nullptr, "postprocessing");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->m_width),
        static_cast<float>(this->m_height), 0.0f, -1.0f, 1.0f);
    ResourceManager::getShader("sprite").use().setInteger("image", 0);
    ResourceManager::getShader("sprite").setMatrix4("projection", projection);
    ResourceManager::getShader("particle").use().setInteger("sprite", 0);
    ResourceManager::getShader("particle").setMatrix4("projection", projection);

    ResourceManager::loadTexture("textures/background.jpg", false, "background");
    ResourceManager::loadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::loadTexture("textures/block.png", false, "block");
    ResourceManager::loadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::loadTexture("textures/paddle.png", true, "paddle");
    ResourceManager::loadTexture("textures/particle.png", true, "particle");
    ResourceManager::loadTexture("textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::loadTexture("textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::loadTexture("textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::loadTexture("textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::loadTexture("textures/powerup_passthrough.png", true, "powerup_through");

    renderer = new SpriteRenderer(ResourceManager::getShader("sprite"));
    particles = new ParticleGenerator(ResourceManager::getShader("particle"), ResourceManager::getTexture("particle"), 500);
    effects = new PostProcessor(ResourceManager::getShader("postprocessing"), this->m_width, this->m_height);
    text = new TextRenderer(this->m_width, this->m_height);
    text->load(FileSystem::getPath("fonts/OCRAEXT.TTF").c_str(), 24);

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

    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::getTexture("face"));
}

void Game::update(float dt) {
    ball->move(dt, this->m_width);
    this->doCollisions();

    particles->update(dt, *ball, 2, glm::vec2(ball->m_radius / 2.0f));

    this->updatePowerUps(dt);

    if (shakeTime > 0.0f) {
        shakeTime -= dt;
        if (shakeTime <= 0.0f) {
            effects->m_shake = false;
        }
    }

    if (ball->m_position.y >= this->m_height) {
        --this->m_lives;

        if (this->m_lives == 0) {
            this->resetLevel();
            this->m_state = GAME_MENU;
        }
        this->resetPlayer();
    }

    if (this->m_state == GAME_ACTIVE && this->m_levels[this->m_level].isCompleted()) {
        this->resetLevel();
        this->resetPlayer();
        effects->m_chaos = true;
        this->m_state = GAME_WIN;
    }
}

void Game::processInput(float dt) {
    if (this->m_state == GAME_MENU) {
        if (this->m_keys[GLFW_KEY_ENTER] && !this->m_keysProcessed[GLFW_KEY_ENTER]) {
            this->m_state = GAME_ACTIVE;
            this->m_keysProcessed[GLFW_KEY_ENTER] = true;
        }
        if (this->m_keys[GLFW_KEY_UP] && !this->m_keysProcessed[GLFW_KEY_UP]) {
            this->m_level = (this->m_level + 1) % 4;
            this->m_keysProcessed[GLFW_KEY_UP] = true;
        }
        if (this->m_keys[GLFW_KEY_DOWN] && !this->m_keysProcessed[GLFW_KEY_DOWN]) {
            if (this->m_level > 0) {
                --this->m_level;
            } else {
                this->m_level = 3;
            }
            this->m_keysProcessed[GLFW_KEY_DOWN] = true;
        }
    }
    if (this->m_state == GAME_WIN) {
        if (this->m_keys[GLFW_KEY_ENTER]) {
            this->m_keysProcessed[GLFW_KEY_ENTER] = true;
            effects->m_chaos = false;
            this->m_state = GAME_MENU;
        }
    }
    if (this->m_state == GAME_ACTIVE) {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->m_keys[GLFW_KEY_LEFT]) {
            if (player->m_position.x >= 0.0f) {
                player->m_position.x -= velocity;
                if (ball->m_stuck) {
                    ball->m_position.x -= velocity;
                }
            }
        }
        if (this->m_keys[GLFW_KEY_RIGHT]) {
            if (player->m_position.x <= this->m_width - player->m_size.x) {
                player->m_position.x += velocity;
                if (ball->m_stuck) {
                    ball->m_position.x += velocity;
                }
            }
        }
        if (this->m_keys[GLFW_KEY_SPACE]) {
            ball->m_stuck = false;
        }
    }
}

void Game::render() {
    if (this->m_state == GAME_ACTIVE || this->m_state == GAME_MENU || this->m_state == GAME_WIN) {
        effects->beginRender();
        renderer->drawSprite(ResourceManager::getTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->m_width, this->m_height), 0.0f);
        this->m_levels[this->m_level].draw(*renderer);
        player->draw(*renderer);

        for (PowerUp& powerUp : this->m_powerups) {
            if (!powerUp.m_destroyed) {
                powerUp.draw(*renderer);
            }
        }
        particles->draw();
        ball->draw(*renderer);
        effects->endRender();
        effects->render(glfwGetTime());

        std::stringstream ss; ss << this->m_lives;
        text->renderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
    }
    if (this->m_state == GAME_MENU) {
        text->renderText("Press ENTER to start", 250.0f, this->m_height / 2.0f, 1.0f);
        text->renderText("Press W or S to select level", 245.0f, this->m_height / 2.0f + 20.0f, 0.75f);
    }
    if (this->m_state == GAME_WIN) {
        text->renderText("You WON!!!", 320.0f, this->m_height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        text->renderText("Press ENTER to retry or ESC to quit", 130.0f, this->m_height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }
}

void Game::resetLevel() {
    if (this->m_level == 0) {
        this->m_levels[0].load("levels/one.lvl", this->m_width, this->m_height / 2);
    } else if (this->m_level == 1) {
        this->m_levels[1].load("levels/two.lvl", this->m_width, this->m_height / 2); 
    } else if (this->m_level == 2) {
        this->m_levels[2].load("levels/three.lvl", this->m_width, this->m_height / 2);
    } else if (this->m_level == 3) {
        this->m_levels[3].load("levsl/four.lvl", this->m_width, this->m_height / 2);
    }

    this->m_lives = 3;
}

void Game::resetPlayer() {
    player->m_size = PLAYER_SIZE;
    player->m_position = glm::vec2(this->m_width / 2.0f - PLAYER_SIZE.x / 2.0f, this->m_height - PLAYER_SIZE.y);
    ball->reset(player->m_position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);

    effects->m_chaos = effects->m_confuse = false;
    ball->m_passThrough = ball->m_sticky = false;
    player->m_color = glm::vec3(1.0f);
    ball->m_color = glm::vec3(1.0f);
}

bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type);

void Game::updatePowerUps(float dt) {
    for (PowerUp& powerUp : this->m_powerups) {
        powerUp.m_position += powerUp.m_velocity * dt;
        if (powerUp.m_activated) {
            powerUp.m_duration -= dt;

            if (powerUp.m_duration <= 0.0f) {
                powerUp.m_activated = false;

                if (powerUp.m_type == "sticky") {
                    if (!isOtherPowerUpActive(this->m_powerups, "sticky")) {
                        ball->m_sticky = false;
                        player->m_color = glm::vec3(1.0f); 
                    }
                } else if (powerUp.m_type == "pass-through") {
                    if (!isOtherPowerUpActive(this->m_powerups, "pass-through")) {
                        ball->m_passThrough = false;
                        ball->m_color = glm::vec3(1.0f);
                    } 
                } else if (powerUp.m_type == "confuse") {
                    if (!isOtherPowerUpActive(this->m_powerups, "confuse")) {
                        effects->m_confuse = false;
                    }
                } else if (powerUp.m_type == "chaos") {
                    if (!isOtherPowerUpActive(this->m_powerups, "chaos")) {
                        effects->m_chaos = false;
                    }
                }
            }
        }
    }

    this->m_powerups.erase(std::remove_if(this->m_powerups.begin(), this->m_powerups.end(),
        [](const PowerUp& powerUp) { return powerUp.m_destroyed && !powerUp.m_activated; }), this->m_powerups.end());
}

bool shouldSpawn(unsigned int chance) {
    unsigned int random = rand() % chance;
    return random == 0;
}

void Game::spawnPowerUps(GameObject& block) {
    if (shouldSpawn(75)) {
        this->m_powerups.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.m_position, ResourceManager::getTexture("powerup_speed")));
    }
    if (shouldSpawn(75)) {
        this->m_powerups.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.m_position, ResourceManager::getTexture("powerup_sticky")));
    }
    if (shouldSpawn(75)) {
        this->m_powerups.push_back(PowerUp("pass-through", glm::vec3(0.5f, 0.5f, 1.0f), 10.0f, block.m_position, ResourceManager::getTexture("powerup_passthrough")));
    }
    if (shouldSpawn(75)) {
        this->m_powerups.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.m_position, ResourceManager::getTexture("powerup_increase")));
    }
    if (shouldSpawn(15)) {
        this->m_powerups.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.m_position, ResourceManager::getTexture("powerup_confuse")));
    }
    if (shouldSpawn(15)) {
        this->m_powerups.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.m_position, ResourceManager::getTexture("powerup_chaos")));
    }
}

void activatePowerUp(PowerUp& powerUp) {
    if (powerUp.m_type == "speed") {
        ball->m_velocity *= 1.2;
    } else if (powerUp.m_type == "sticky") {
        ball->m_sticky = true;
        player->m_color = glm::vec3(1.0f, 0.5f, 1.0f);
    } else if (powerUp.m_type == "pass-through") {
        ball->m_passThrough = true;
        ball->m_color = glm::vec3(1.0f, 0.5f, 0.5f);
    } else if (powerUp.m_type == "pad-size-increase") {
        player->m_size.x += 50;
    } else if (powerUp.m_type == "confuse") {
        if (!effects->m_chaos) {
            effects->m_confuse = true;
        }
    } else if (powerUp.m_type == "chaos") {
        if (!effects->m_confuse) {
            effects->m_chaos = true;
        }
    }
}

bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type) {
    for (const PowerUp& powerUp : powerUps) {
        if (powerUp.m_activated) {
            if (powerUp.m_type == type) {
                return true;
            }
        }
    }
    return false;
}

bool checkCollision(GameObject& one, GameObject& two);
Collision checkCollision(BallObject& one, GameObject& two);
Direction vectorDirection(glm::vec2 closest);

void Game::doCollisions() {
    for (GameObject& box : this->m_levels[this->m_level].m_bricks) {
        if (!box.m_destroyed) {
            Collision collision = checkCollision(*ball, box);
            if (std::get<0>(collision)) {
                if (!box.m_isSolid) {
                    box.m_destroyed = true;
                    this->spawnPowerUps(box);
                } else {
                    shakeTime = 0.05f;
                    effects->m_shake = true;
                }

                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (!(ball->m_passThrough && !box.m_isSolid)) { // don't do collision resolution on non-solid bricks if passthrough is activated
                    if (dir == LEFT || dir == RIGHT) {
                        ball->m_velocity.x = -ball->m_velocity.x;

                        float penetration = ball->m_radius - std::abs(diff_vector.x);
                        if (dir == LEFT) {
                            ball->m_position.x += penetration;
                        } else {
                            ball->m_position.x -= penetration;
                        }
                    } else {
                        ball->m_velocity.y = -ball->m_velocity.y;

                        float penetration = ball->m_radius - std::abs(diff_vector.y);
                        if (dir == UP) {
                            ball->m_position.y -= penetration;
                        } else {
                            ball->m_position.y += penetration;
                        }
                    }
                }
            }
        }
    }

    for (PowerUp& powerUp : this->m_powerups) {
        if (!powerUp.m_destroyed) {
            if (powerUp.m_position.y >= this->m_height) {
                powerUp.m_destroyed = true;
            } 
            if (checkCollision(*player, powerUp)) {
                activatePowerUp(powerUp);
                powerUp.m_destroyed = true;
                powerUp.m_activated = true;
            }
        }
    }
    Collision result = checkCollision(*ball, *player);
    if (!ball->m_stuck && std::get<0>(result)) {
        float centerBoard = player->m_position.x + player->m_size.x / 2.0f;
        float distance = (ball->m_position.x + ball->m_radius) - centerBoard;
        float percentage = distance / (player->m_size.x / 2.0f);

        float strength = 2.0f;
        glm::vec2 oldVelocity = ball->m_velocity;
        ball->m_velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        ball->m_velocity = glm::normalize(ball->m_velocity) * glm::length(oldVelocity);
        ball->m_velocity.y = -1.0f * abs(ball->m_velocity.y);

        ball->m_stuck = ball->m_sticky;
    }
}

bool checkCollision(GameObject& one, GameObject& two) {
    bool collisionX = one.m_position.x + one.m_size.x >= two.m_position.x &&
        two.m_position.x + two.m_size.x >= one.m_position.x;

    bool collisionY = one.m_position.y + one.m_size.y >= two.m_position.y &&
        two.m_position.y + two.m_size.y >= one.m_position.y;

    return collisionX && collisionY;
}

Collision checkCollision(BallObject& one, GameObject& two) {
    glm::vec2 center(one.m_position + one.m_radius);

    glm::vec2 aabb_half_extents(two.m_size.x / 2.0f, two.m_size.y / 2.0f);
    glm::vec2 aabb_center(two.m_position.x + aabb_half_extents.x, two.m_position.y + aabb_half_extents.y);

    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

    glm::vec2 closest = aabb_center + clamped;
    difference = closest - center;

    if (glm::length(difference) < one.m_radius) {
        return std::make_tuple(true, vectorDirection(difference), difference);
    } else {
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
    }
}

Direction vectorDirection(glm::vec2 target) {
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f)  // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; ++i) {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max) {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include "particle_generator.h"

SpriteRenderer* renderer;
GameObject* player;
BallObject* ball;
ParticleGenerator* particles;

Game::Game(unsigned int width, unsigned int height) 
    : m_state(GAME_ACTIVE), m_keys(), m_width(width), m_height(height)
{}

Game::~Game() {
    delete renderer;
    delete player;
    delete ball;
    delete particles;
}

void Game::init() {
    FileSystem::chDir();
    FileSystem::chDir();

    ResourceManager::loadShader("shaders/sprite.vs", "shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::loadShader("shaders/particle.vs", "shaders/particle.fs", nullptr, "particle");

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

    renderer = new SpriteRenderer(ResourceManager::getShader("sprite"));
    particles = new ParticleGenerator(ResourceManager::getShader("particle"), ResourceManager::getTexture("particle"), 500);

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

    if (ball->m_position.y >= this->m_height) {
        this->resetLevel();
        this->resetPlayer();
    }
}

void Game::processInput(float dt) {
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
    if (this->m_state == GAME_ACTIVE) {
        renderer->drawSprite(ResourceManager::getTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->m_width, this->m_height), 0.0f);
        this->m_levels[this->m_level].draw(*renderer);
        player->draw(*renderer);
        particles->draw();
        ball->draw(*renderer);
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
}

void Game::resetPlayer() {
    player->m_size = PLAYER_SIZE;
    player->m_position = glm::vec2(this->m_width / 2.0f - PLAYER_SIZE.x / 2.0f, this->m_height - PLAYER_SIZE.y);
    ball->reset(player->m_position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
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
                }

                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
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
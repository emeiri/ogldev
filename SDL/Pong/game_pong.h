#pragma once

#include <assert.h>

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};


class Paddle {

public:

    Paddle() = default;

    void Init(const Vec2& size, const Vec2& pos, float speed, const Vec2& WindowSize) {
        m_speed = speed; 
        m_pos = pos; 
        m_size = size; 
        m_halfSize = { size.x / 2.0f, size.y / 2.0f };
        m_windowSize = WindowSize;
    }

    void HandleUpKey(float deltaTime);

    void HandleDownKey(float deltaTime);

    const Vec2& GetPosition() const
    {
        return m_pos;
    }

private:

    Vec2 m_pos;
    Vec2 m_size;
    Vec2 m_halfSize;
    Vec2 m_windowSize;
    float m_speed = 0.0f;
};


class Ball {

public:

    Ball() = default;

    void Init(float Size, const Vec2& pos, const Vec2& velocity, const Vec2& WindowSize) { 
        assert(Size > 0.0f);
        m_size = Size; 
        m_halfSize = Size / 2.0f;
        m_pos = pos; 
        m_velocity = velocity; 
        m_windowSize = WindowSize;
    }

    void Update(float deltaTime);

    const Vec2& GetPosition() const
    {
        return m_pos;
    }

    const Vec2& GetVelocity() const
    {
        return m_velocity;
    }

    void SetVelocity(const Vec2& velocity)
    {
        m_velocity = velocity;
    }

    void SetPosition(const Vec2& pos)
    {
        m_pos = pos;
    }

    float GetHalfSize() const
    {
        return m_halfSize;
    }

private:

    float m_size = 0.0f;
    float m_halfSize = 0.0f;
    Vec2 m_pos;
    Vec2 m_velocity = { -200.0f, 235.0f };
    Vec2 m_windowSize;
};


struct GameConfig {
    Vec2 WindowSize = { 1920.0f, 1080.0f };
    float BallSize = 20.0f;
    float PaddleSpeed = 600.0f; // Pixels per second
    float PaddleWidth = 30.0f;
    float PaddleHeight = 300.0f;
    float PaddleOffset = 25.0f; // Distance from the edge of the window
};

class Pong {

public:
    Pong() = default;
    
    ~Pong() = default;
    
    void Init(const GameConfig& config);

    void Update(bool PaddleLUp, bool PaddleLDown, bool PaddleRUp, bool PaddleRDown, float DeltaTime);

    Vec2 GetBallPosition() const { return m_ball.GetPosition(); }

    Vec2 GetPaddleLPosition() const { return m_paddleL.GetPosition(); }

    Vec2 GetPaddleRPosition() const { return m_paddleR.GetPosition(); }

private:

    void ResolvePaddleBallCollision();

    // Add private members here, such as the ball, paddles, and game state
    GameConfig m_config;
    Ball m_ball;
    Paddle m_paddleL;
    Paddle m_paddleR;
};

#pragma once

#include <assert.h>

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};


struct Rect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};


class BaseObject {
    
public:

    void Init(const Vec2& size, const Vec2& pos, float speed, const Vec2& WindowSize) {
        m_pos = pos; 
        m_size = size; 
        m_halfSize = { size.x / 2.0f, size.y / 2.0f };
        m_windowSize = WindowSize;
    }

    const Vec2& GetPosition() const
    {
        return m_pos;
    }

    void GetRect(Rect& rect) const
    {
        rect.x = m_pos.x - m_halfSize.x;
        rect.y = m_pos.y - m_halfSize.y;
        rect.w = m_size.x;
        rect.h = m_size.y;
    }

protected:

    Vec2 m_pos;
    Vec2 m_size;
    Vec2 m_halfSize;
    Vec2 m_windowSize;
};


class Paddle : public BaseObject {

public:

    Paddle() = default;

    void Init(const Vec2& size, const Vec2& pos, float speed, const Vec2& WindowSize) {
        BaseObject::Init(size, pos, speed, WindowSize);
        m_speed = speed; 
    }

    void HandleUpKey(float deltaTime);

    void HandleDownKey(float deltaTime);

private:

    float m_speed = 0.0f;
};


class Ball : public BaseObject {

public:

    Ball() = default;

    void Init(float Size, const Vec2& pos, const Vec2& velocity, const Vec2& WindowSize) { 
        assert(Size > 0.0f);
        BaseObject::Init({ Size, Size }, pos, 0.0f, WindowSize);
        m_velocity = velocity; 
        m_halfSize = Size / 2.0f;
        m_halfWindowSize = { WindowSize.x / 2.0f, WindowSize.y / 2.0f };
    }

    void Update(float deltaTime);

    const Vec2& GetVelocity() const
    {
        return m_velocity;
    }

    void SetVelocity(const Vec2& velocity)
    {
        m_velocity = velocity;
    }

    float GetHalfSize() const
    {
        return m_halfSize;
    }

private:

    Vec2 m_velocity = { -200.0f, 235.0f };
    Vec2 m_halfWindowSize = { 0.0f, 0.0f };
    float m_halfSize = 0.0f;
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

    void GetRects(Rect& BallRect, Rect& PaddleLRect, Rect& PaddleRRect) const;

private:

    void ResolvePaddleBallCollision();

    GameConfig m_config;
    Ball m_ball;
    Paddle m_paddleL;
    Paddle m_paddleR;
};

/*

        Copyright 2026  Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include <cassert>

#include "ogldev_math_3d.h"

struct Rect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};


class BaseObject {
    
public:

    void Init(const Vector2f& size, const Vector2f& pos, const Vector2f& WindowSize) {
        m_pos = pos; 
        m_size = size; 
        m_halfSize = { size.x / 2.0f, size.y / 2.0f };
        m_windowSize = WindowSize;
    }

    const Vector2f& GetPosition() const
    {
        return m_pos;
    }

    Rect GetRect() const {
        return {
            m_pos.x - m_halfSize.x,
            m_pos.y - m_halfSize.y,
            m_size.x,
            m_size.y
        };
    }

    Vector2f GetHalfSize() const
    {
        return m_halfSize;
    }

protected:

    Vector2f m_pos;
    Vector2f m_size;
    Vector2f m_halfSize;
    Vector2f m_windowSize;
};


class Paddle : public BaseObject {

public:

    Paddle() = default;

    void Init(const Vector2f& size, const Vector2f& pos, float speed, const Vector2f& WindowSize) {
        BaseObject::Init(size, pos, WindowSize);
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

    void Init(float Size, const Vector2f& pos, const Vector2f& velocity, const Vector2f& WindowSize) { 
        assert(Size > 0.0f);
        BaseObject::Init({ Size, Size }, pos, WindowSize);
        m_velocity = velocity; 
        m_halfWindowSize = { WindowSize.x / 2.0f, WindowSize.y / 2.0f };
    }

    void Update(float deltaTime);

    const Vector2f& GetVelocity() const
    {
        return m_velocity;
    }

    void SetVelocity(const Vector2f& velocity)
    {
        m_velocity = velocity;
    }

private:

    Vector2f m_velocity = { -200.0f, 235.0f };
    Vector2f m_halfWindowSize = { 0.0f, 0.0f };
};


struct GameConfig {
    Vector2f WindowSize = { 1920.0f, 1080.0f };
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

    void ResolvePaddleBallCollision(Paddle& paddle);

    GameConfig m_config;
    Ball m_ball;
    Paddle m_paddleL;
    Paddle m_paddleR;
    Vector2f m_halfWindowSize;
};

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

#include <glm/glm.hpp>

#include "ogldev_accelorobject.h"

struct Rect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};


class BaseObject {
    
public:

    void Init(const glm::vec2& BaseOffset, const glm::vec2& size, 
              const glm::vec2& pos, const glm::vec2& WindowSize) 
    {
        m_baseOffset = BaseOffset;
        m_pos = pos; 
        m_size = size; 
        m_halfSize = { size.x / 2.0f, size.y / 2.0f };
        m_windowSize = WindowSize;
    }

    const glm::vec2& GetPosition() const
    {
        return m_pos;
    }


    void SetPosition(const glm::vec2& pos)
    {
        m_pos = pos;
    }

    Rect GetRect() const {
        return {
            m_pos.x - m_halfSize.x,
            m_pos.y - m_halfSize.y,
            m_size.x,
            m_size.y
        };
    }

    glm::vec2 GetHalfSize() const
    {
        return m_halfSize;
    }

protected:

    glm::vec2 m_baseOffset = { 0.0f, 0.0f };
    glm::vec2 m_pos = { 0.0f, 0.0f };
    glm::vec2 m_size = { 0.0f, 0.0f };
    glm::vec2 m_halfSize = { 0.0f, 0.0f };
    glm::vec2 m_windowSize = { 0.0f, 0.0f };
};


class Paddle : public BaseObject {

public:

    Paddle() = default;

    void Init(const glm::vec2& BaseOffset, const glm::vec2& size, const glm::vec2& pos, 
              float MaxSpeed, float Acceleration, float Deceleration, const glm::vec2& WindowSize)
    {
        BaseObject::Init(BaseOffset, size, pos, WindowSize);
        m_accelObj.Init(MaxSpeed, Acceleration, Deceleration);
    }

    void Update(float deltaTime, bool IsUpKeyPressed, bool IsDownKeyPressed);

private:

    AccelorObject m_accelObj;
};


class Ball : public BaseObject {

public:

    Ball() = default;

    void Init(const glm::vec2& BaseOffset, float Size, const glm::vec2& pos, 
              const glm::vec2& velocity, const glm::vec2& WindowSize) 
    { 
        assert(Size > 0.0f);
        BaseObject::Init(BaseOffset, { Size, Size }, pos, WindowSize);
        m_velocity = velocity; 
        m_halfWindowSize = { WindowSize.x / 2.0f, WindowSize.y / 2.0f };
    }

    void Update(float deltaTime);

    const glm::vec2& GetVelocity() const
    {
        return m_velocity;
    }

    void SetVelocity(const glm::vec2& velocity)
    {
        m_velocity = velocity;
    }

private:

    glm::vec2 m_velocity = { -200.0f, 235.0f };
    glm::vec2 m_halfWindowSize = { 0.0f, 0.0f };
};


struct GameConfig {
    glm::vec2 WindowSize = { 1920.0f, 1080.0f };
    glm::vec2 BaseWindowPosition = { 0.0f, 0.0f };
    float BallSpeed = 400.0f; // Pixels per second
    float BallSize = 20.0f;
    float PaddleMaxSpeed = 600.0f; // Pixels per second
    float PaddleAcceleration = 2000.0f;
    float PaddleDeceleration = 1000.0f;
    float PaddleWidth = 30.0f;
    float PaddleHeight = 300.0f;
    float PaddleOffset = 25.0f; // Distance from the edge of the window
};


struct PongInputState {
    bool PaddleLUp = false;
    bool PaddleLDown = false;
    bool PaddleRUp = false;
    bool PaddleRDown = false;
};


class Pong {

public:
    Pong() = default;
    
    ~Pong() = default;
    
    void Init(const GameConfig& config);

    void Update(float DeltaTime, bool PaddleLUp, bool PaddleLDown, bool PaddleRUp, bool PaddleRDown);

    void GetRects(Rect& BallRect, Rect& PaddleLRect, Rect& PaddleRRect) const;
    
    void GetPositions(glm::vec2& BallPos, glm::vec2& PaddleLPos, glm::vec2& PaddleRPos) const;

private:

    void ResolvePaddleBallCollision(Paddle& paddle);

    GameConfig m_config;
    Ball m_ball;
    Paddle m_paddleL;
    Paddle m_paddleR;
    glm::vec2 m_halfWindowSize;
};

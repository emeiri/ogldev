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


#include <cmath>
#include <cstdlib>
#include <ctime>

#include "pong.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static glm::vec2 CreateRandomNormalizedVector()
{
    // 1. Generate a random offset between -45 and +45 degrees (-PI/4 to +PI/4 radians)
    float randomSpread = ((float)rand() / RAND_MAX) * (float)(M_PI / 2.0) - (float)(M_PI / 4.0);

    // 2. Randomly decide to point Left (PI radians base) or Right (0 radians base)
    float baseAngle = (rand() % 2 == 0) ? 0.0f : (float)M_PI;

    // 3. Combine the base direction and the offset spread
    float angle = baseAngle + randomSpread;

    glm::vec2 v;
    v.x = cos(angle);
    v.y = sin(angle);

    return v;
}



void Paddle::Update(float deltaTime, bool IsUpKeyPressed, bool IsDownKeyPressed)
{
    m_accelObj.Update(deltaTime, IsUpKeyPressed, IsDownKeyPressed);

    m_pos.y += m_accelObj.GetCurrentSpeed() * deltaTime;

    //printf("Paddle Position: x=%f, y=%f\n", m_pos.x, m_pos.y);
    if (m_pos.y - m_halfSize.y < m_baseOffset.y) {
        m_pos.y = m_baseOffset.y + m_halfSize.y;
        // printf("Paddle hit the top wall. New position: x=%f, y=%f\n", m_pos.x, m_pos.y);
    }   if (m_pos.y + m_halfSize.y > m_baseOffset.y + m_windowSize.y) {
        m_pos.y = m_baseOffset.y + m_windowSize.y - m_halfSize.y;
        //  printf("Paddle hit the wall. New position: x=%f, y=%f\n", m_pos.x, m_pos.y);
    }
}


void Ball::Update(float deltaTime)
{
    m_pos.x += m_velocity.x * deltaTime;
    m_pos.y += m_velocity.y * deltaTime;

    //printf("dt %f Ball Position: x=%f, y=%f velocity: x=%f, y=%f\n", deltaTime, m_pos.x, m_pos.y, m_velocity.x, m_velocity.y);

    bool BallHitsBottom = m_pos.y + m_halfSize.y >= m_baseOffset.y + m_windowSize.y;
    bool BallHitsTop = m_pos.y - m_halfSize.y <= m_baseOffset.y;

    if (BallHitsTop && m_velocity.y < 0.0f) {
        m_pos.y = m_baseOffset.y + m_halfSize.y; // Positional correction to prevent wall trapping
        m_velocity.y = -m_velocity.y;
     //   printf("Ball hit the top wall. New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
     //   exit(0); 
    } else if (BallHitsBottom && m_velocity.y > 0.0f) {
        m_pos.y = m_baseOffset.y + m_windowSize.y - m_halfSize.y; // Positional correction
        m_velocity.y = -m_velocity.y;
      //  printf("Ball hit the bottom wall. New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
      //  exit(0);
    }

    // --- Scoring & Reset States ---
    // Left Wall Out-of-Bounds (Right Player Scores)
    if (m_pos.x + m_halfSize.x < m_baseOffset.x) {
        m_pos = m_baseOffset + glm::vec2(m_halfWindowSize.x, m_halfWindowSize.y);
        float Speed = glm::length(m_velocity);
        glm::vec2 BallVelocity = CreateRandomNormalizedVector() * Speed;
        //printf("Ball out of bounds on the left side. Right player scores! New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
        m_velocity.x = abs(BallVelocity.x); // Serve towards player 2 this time
        m_velocity.y = BallVelocity.y;
      //  printf("Left player missed the ball. Right player scores! New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
//        exit(0);
    }
    // Right Wall Out-of-Bounds (Left Player Scores) - FIXED from bouncing wall
    else if (m_pos.x - m_halfSize.x > m_baseOffset.x + m_windowSize.x) {
        m_pos = m_baseOffset + glm::vec2(m_halfWindowSize.x, m_halfWindowSize.y);
        float Speed = glm::length(m_velocity);
        glm::vec2 BallVelocity = CreateRandomNormalizedVector() * Speed;
        //printf("Ball out of bounds on the right side. Left player scores! New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
        m_velocity.x = -abs(BallVelocity.x); // Serve towards player 2 this time
        m_velocity.y = BallVelocity.y;
     //   printf("Right player missed the ball. Left player scores! New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
       // exit(0);
    }
}


void Pong::Init(const GameConfig& config)
{
    m_config = config;

    m_halfWindowSize = { m_config.WindowSize.x / 2.0f, m_config.WindowSize.y / 2.0f };

    glm::vec2 pos = config.BaseWindowPosition + glm::vec2(m_halfWindowSize.x, m_halfWindowSize.y);

    srand((unsigned int)time(NULL));
    glm::vec2 BallVelocity = CreateRandomNormalizedVector() * m_config.BallSpeed;

    m_ball.Init(config.BaseWindowPosition, m_config.BallSize, pos, 
                BallVelocity, m_config.WindowSize);

    pos = config.BaseWindowPosition + glm::vec2(m_config.PaddleOffset, m_halfWindowSize.y);

    m_paddleL.Init(config.BaseWindowPosition, { m_config.PaddleWidth, m_config.PaddleHeight }, pos, 
                   m_config.PaddleMaxSpeed, m_config.PaddleAcceleration, m_config.PaddleDeceleration, m_config.WindowSize);

    pos = config.BaseWindowPosition + glm::vec2(m_config.WindowSize.x - m_config.PaddleOffset, m_halfWindowSize.y);

    m_paddleR.Init(config.BaseWindowPosition, { m_config.PaddleWidth, m_config.PaddleHeight }, pos, 
                   m_config.PaddleMaxSpeed, m_config.PaddleAcceleration, m_config.PaddleDeceleration, m_config.WindowSize);
}


void Pong::Update(bool PaddleLUp, bool PaddleLDown, bool PaddleRUp, bool PaddleRDown, float DeltaTime)
{
    //printf("DeltaTime: %f\n", DeltaTime);
    m_paddleL.Update(DeltaTime, PaddleLUp, PaddleLDown);

    m_paddleR.Update(DeltaTime, PaddleRUp, PaddleRDown);
    
    m_ball.Update(DeltaTime);

    ResolvePaddleBallCollision(m_paddleL);

    ResolvePaddleBallCollision(m_paddleR);
}


void Pong::ResolvePaddleBallCollision(Paddle& paddle)
{
    float BallLeftSide = m_ball.GetPosition().x - m_ball.GetHalfSize().x;
    float BallRightSide = m_ball.GetPosition().x + m_ball.GetHalfSize().x;
    float BallTopSide = m_ball.GetPosition().y + m_ball.GetHalfSize().y;
    float BallBottomSide = m_ball.GetPosition().y - m_ball.GetHalfSize().y;
    float PaddleLeftSide = paddle.GetPosition().x - m_config.PaddleWidth / 2.0f;
    float PaddleRightSide = paddle.GetPosition().x + m_config.PaddleWidth / 2.0f;
    float PaddleTopSide = paddle.GetPosition().y + m_config.PaddleHeight / 2.0f;
    float PaddleBottomSide = paddle.GetPosition().y - m_config.PaddleHeight / 2.0f;

   // printf("Ball half size: x=%f, y=%f\n", m_ball.GetHalfSize().x, m_ball.GetHalfSize().y);

    bool CollideWithPaddle = (BallLeftSide <= PaddleRightSide) && (BallRightSide >= PaddleLeftSide) &&
                             (BallTopSide >= PaddleBottomSide) && (BallBottomSide <= PaddleTopSide);

  //  printf("Ball Position: x=%f, y=%f\n", m_ball.GetPosition().x, m_ball.GetPosition().y);
  //  printf("Ball left: %f, right: %f, top: %f, bottom: %f\n", BallLeftSide, BallRightSide, BallBottomSide, BallTopSide);
  //  printf("Paddle Position: x=%f, y=%f\n", paddle.GetPosition().x, paddle.GetPosition().y);
  //  printf("Paddle left: %f, right: %f, top: %f, bottom: %f\n", PaddleLeftSide, PaddleRightSide, PaddleBottomSide, PaddleTopSide);

    if (CollideWithPaddle) {
       // printf("!!!!!!!!!!!!!!!!!!!!!!!!! COLLISION DETECTED !!!!!!!!!!!!!!!!!!!!!!!!!\n");
        glm::vec2 NewVelocity = m_ball.GetVelocity() * 1.05f;
        glm::vec2 NewPosition = m_ball.GetPosition(); // Get current position

        // Bounce ball outward and correct position to prevent clipping
        glm::vec2 WorldCenter = m_config.BaseWindowPosition + m_halfWindowSize; 
        if (m_ball.GetPosition().x < WorldCenter.x && NewVelocity.x < 0.0f) {
            NewVelocity.x = std::abs(NewVelocity.x);

            // Snap the ball's left edge to the paddle's right edge
            NewPosition.x = PaddleRightSide + m_ball.GetHalfSize().x;

        } else if (m_ball.GetPosition().x > WorldCenter.x && NewVelocity.x > 0.0f) {
            NewVelocity.x = -std::abs(NewVelocity.x);

            // Snap the ball's right edge to the paddle's left edge
            NewPosition.x = PaddleLeftSide - m_ball.GetHalfSize().x;
        }

        m_ball.SetVelocity(NewVelocity);
        m_ball.SetPosition(NewPosition); // Apply the position correction immediately!
    }
}


void Pong::GetRects(Rect& BallRect, Rect& PaddleLRect, Rect& PaddleRRect) const
{
    BallRect = m_ball.GetRect();
    PaddleLRect = m_paddleL.GetRect();
    PaddleRRect = m_paddleR.GetRect();
}

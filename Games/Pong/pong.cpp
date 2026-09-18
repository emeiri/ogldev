#include "pong.h"
#include <cmath> // For std::abs

void Paddle::HandleUpKey(float deltaTime)
{
    m_pos.y -= m_speed * deltaTime;

    if (m_pos.y - m_halfSize.y < m_baseOffset.y) {
        m_pos.y = m_baseOffset.y + m_halfSize.y;
    }
}


void Paddle::HandleDownKey(float deltaTime)
{
    m_pos.y += m_speed * deltaTime;

    if (m_pos.y + m_halfSize.y > m_baseOffset.y + m_windowSize.y) {
        m_pos.y = m_baseOffset.y + m_windowSize.y - m_halfSize.y;
    }
}


void Ball::Update(float deltaTime)
{
    m_pos.x += m_velocity.x * deltaTime;
    m_pos.y += m_velocity.y * deltaTime;

 //   printf("dt %f Ball Position: x=%f, y=%f velocity: x=%f, y=%f\n", deltaTime, m_pos.x, m_pos.y, m_velocity.x, m_velocity.y);

    bool BallHitsBottom = m_pos.y + m_halfSize.y >= m_windowSize.y;
    bool BallHitsTop = m_pos.y - m_halfSize.y <= 0.0f;

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
        m_velocity = -m_velocity; // Serve towards player 2 this time
      //  printf("Left player missed the ball. Right player scores! New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
//        exit(0);
    }
    // Right Wall Out-of-Bounds (Left Player Scores) - FIXED from bouncing wall
    else if (m_pos.x - m_halfSize.x > m_baseOffset.x + m_windowSize.x) {
        m_pos = m_baseOffset + glm::vec2(m_halfWindowSize.x, m_halfWindowSize.y);
        m_velocity = -m_velocity; // Serve towards player 1
     //   printf("Right player missed the ball. Left player scores! New velocity: x=%f, y=%f\n", m_velocity.x, m_velocity.y);
       // exit(0);
    }
}


void Pong::Init(const GameConfig& config)
{
    m_config = config;

    m_halfWindowSize = { m_config.WindowSize.x / 2.0f, m_config.WindowSize.y / 2.0f };

    glm::vec2 pos = config.BaseWindowPosition + glm::vec2(m_halfWindowSize.x, m_halfWindowSize.y);

    m_ball.Init(config.BaseWindowPosition, m_config.BallSize, pos, 
                m_config.BallInitialVelocity, m_config.WindowSize);

    pos = config.BaseWindowPosition + glm::vec2(m_config.PaddleOffset, m_halfWindowSize.y);

    m_paddleL.Init(config.BaseWindowPosition, { m_config.PaddleWidth, m_config.PaddleHeight }, pos, 
                   m_config.PaddleSpeed, m_config.WindowSize);

    pos = config.BaseWindowPosition + glm::vec2(m_config.WindowSize.x - m_config.PaddleOffset, m_halfWindowSize.y);

    m_paddleR.Init(config.BaseWindowPosition, { m_config.PaddleWidth, m_config.PaddleHeight }, pos, 
                   m_config.PaddleSpeed, m_config.WindowSize);
}


void Pong::Update(bool PaddleLUp, bool PaddleLDown, bool PaddleRUp, bool PaddleRDown, float DeltaTime)
{
    //printf("DeltaTime: %f\n", DeltaTime);
    if (PaddleLUp) {
        m_paddleL.HandleUpKey(DeltaTime);
    }

    if (PaddleLDown) {
        m_paddleL.HandleDownKey(DeltaTime);
    }

    if (PaddleRUp) {
        m_paddleR.HandleUpKey(DeltaTime);
    }

    if (PaddleRDown) {
        m_paddleR.HandleDownKey(DeltaTime);
    }
    
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
        glm::vec2 NewVelocity = m_ball.GetVelocity();
        glm::vec2 NewPosition = m_ball.GetPosition(); // Get current position

        // Bounce ball outward and correct position to prevent clipping
        glm::vec2 WorldCenter = m_config.BaseWindowPosition + m_halfWindowSize; 
        if (m_ball.GetPosition().x < WorldCenter.x && NewVelocity.x < 0.0f) {
            NewVelocity.x = std::abs(NewVelocity.x) * 1.05f;
            NewVelocity.y *= 1.05f;

            // Snap the ball's left edge to the paddle's right edge
            NewPosition.x = PaddleRightSide + m_ball.GetHalfSize().x;

        } else if (m_ball.GetPosition().x > WorldCenter.x && NewVelocity.x > 0.0f) {
            NewVelocity.x = -std::abs(NewVelocity.x) * 1.05f;
            NewVelocity.y *= 1.05f;

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

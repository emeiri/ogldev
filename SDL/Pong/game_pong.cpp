#include "game_pong.h"

float BallSize = 20.0f;
float HalfBallSize = BallSize / 2.0f;
float PaddleSpeed = 600.0f; // Pixels per second
float PaddleWidth = 30.0f;
float HalfPaddleWidth = PaddleWidth / 2.0f;
float PaddleHeight = 300.0f;
float HalfPaddleHeight = PaddleHeight / 2.0f;


void Paddle::HandleUpKey(float deltaTime)
{
    float CurrentPaddleVelocity = 0.0f;

    CurrentPaddleVelocity += PaddleSpeed;

    m_pos.y -= CurrentPaddleVelocity * deltaTime;

    if (m_pos.y - HalfPaddleHeight < 0.0f) {
        m_pos.y = HalfPaddleHeight;
    }
}

void Paddle::HandleDownKey(float deltaTime)
{
    float CurrentPaddleVelocity = 0.0f;

    CurrentPaddleVelocity += PaddleSpeed;

    m_pos.y += CurrentPaddleVelocity * deltaTime;

    if (m_pos.y + HalfPaddleHeight > WINDOW_HEIGHT) {
        m_pos.y = WINDOW_HEIGHT - HalfPaddleHeight;
    }
}


void Ball::Update(float deltaTime)
{
    m_pos.x += m_velocity.x * deltaTime;
    m_pos.y += m_velocity.y * deltaTime;

    bool BallHitsBottom = m_pos.y + HalfBallSize >= WINDOW_HEIGHT;
    bool BallHitsTop = m_pos.y - HalfBallSize <= 0.0f;
    if ((BallHitsTop && (m_velocity.y < 0.0f)) ||
        (BallHitsBottom && (m_velocity.y > 0.0f))) {
        m_velocity.y = -m_velocity.y;
    }

    if (m_pos.x < 0.0f) {
        m_pos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
        m_velocity = { -200.0f, 235.0f }; // Reset speed
    }

    if (m_pos.x + HalfBallSize >= WINDOW_WIDTH && m_velocity.x > 0.0f) {
        m_velocity.x = -m_velocity.x;
    }
}


void Pong::Init(const GameConfig& config)
{
    m_config = config;
}


void Pong::Update(bool PaddleLUp, bool PaddleLDown, bool PaddleRUp, bool PaddleRDown, float DeltaTime)
{
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

    ResolvePaddleBallCollision();
}


void Pong::ResolvePaddleBallCollision()
{
    bool CollideWithPaddle =
        (m_ball.GetPosition().x - HalfBallSize <= m_paddleL.GetPosition().x + HalfPaddleWidth) &&
        (m_ball.GetPosition().x + HalfBallSize >= m_paddleL.GetPosition().x - HalfPaddleWidth) &&
        (m_ball.GetPosition().y + HalfBallSize >= m_paddleL.GetPosition().y - HalfPaddleHeight) &&
        (m_ball.GetPosition().y - HalfBallSize <= m_paddleL.GetPosition().y + HalfPaddleHeight);

    if (CollideWithPaddle && m_ball.GetVelocity().x < 0.0f) {
        Vec2 NewVelocity = m_ball.GetVelocity();
        NewVelocity.x = -NewVelocity.x;
        // Optional: Slightly boost speed upon impact to increase difficulty
        NewVelocity.x *= 1.05f;
        NewVelocity.y *= 1.05f;
        m_ball.SetVelocity(NewVelocity);
    }
}

#include "game_pong.h"


void Paddle::HandleUpKey(float deltaTime)
{
    float CurrentPaddleVelocity = 0.0f;

    CurrentPaddleVelocity += m_speed;

    m_pos.y -= CurrentPaddleVelocity * deltaTime;

    if (m_pos.y - m_size.y / 2.0f < 0.0f) {
        m_pos.y = m_size.y / 2.0f;
    }
}


void Paddle::HandleDownKey(float deltaTime)
{
    float CurrentPaddleVelocity = 0.0f;

    CurrentPaddleVelocity += m_speed;

    m_pos.y += CurrentPaddleVelocity * deltaTime;

    if (m_pos.y + m_size.y / 2.0f > WINDOW_HEIGHT) {
        m_pos.y = WINDOW_HEIGHT - m_size.y / 2.0f;
    }
}


void Ball::Update(float deltaTime)
{
    m_pos.x += m_velocity.x * deltaTime;
    m_pos.y += m_velocity.y * deltaTime;

    bool BallHitsBottom = m_pos.y + m_halfSize >= WINDOW_HEIGHT;
    bool BallHitsTop = m_pos.y - m_halfSize <= 0.0f;

    if ((BallHitsTop && (m_velocity.y < 0.0f)) ||
        (BallHitsBottom && (m_velocity.y > 0.0f))) {
        m_velocity.y = -m_velocity.y;
    }

    if (m_pos.x < 0.0f) {
        m_pos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
        m_velocity = { -200.0f, 235.0f }; // Reset speed
    }

    if (m_pos.x + m_halfSize >= WINDOW_WIDTH && m_velocity.x > 0.0f) {
        m_velocity.x = -m_velocity.x;
    }
}


void Pong::Init(const GameConfig& config)
{
    m_config = config;

    m_ball.Init(m_config.BallSize, 
                { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f }, { -200.0f, 235.0f });

    m_paddleL.Init(m_config.PaddleSpeed, 
                   { m_config.PaddleOffset, WINDOW_HEIGHT / 2.0f }, 
                   { m_config.PaddleWidth, m_config.PaddleHeight });

    m_paddleR.Init(m_config.PaddleSpeed, 
                   { WINDOW_WIDTH - m_config.PaddleOffset, WINDOW_HEIGHT / 2.0f }, 
                   { m_config.PaddleWidth, m_config.PaddleHeight });
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
        (m_ball.GetPosition().x - m_ball.GetHalfSize() <= m_paddleL.GetPosition().x + m_config.PaddleWidth / 2.0f) &&
        (m_ball.GetPosition().x + m_ball.GetHalfSize() >= m_paddleL.GetPosition().x - m_config.PaddleWidth / 2.0f) &&
        (m_ball.GetPosition().y + m_ball.GetHalfSize() >= m_paddleL.GetPosition().y - m_config.PaddleHeight / 2.0f) &&
        (m_ball.GetPosition().y - m_ball.GetHalfSize() <= m_paddleL.GetPosition().y + m_config.PaddleHeight / 2.0f);

    if (CollideWithPaddle && m_ball.GetVelocity().x < 0.0f) {
        Vec2 NewVelocity = m_ball.GetVelocity();
        NewVelocity.x = -NewVelocity.x;
        // Optional: Slightly boost speed upon impact to increase difficulty
        NewVelocity.x *= 1.05f;
        NewVelocity.y *= 1.05f;
        m_ball.SetVelocity(NewVelocity);
    }
}

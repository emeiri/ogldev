#include <SFML/Graphics.hpp>
#include <iostream>
#include "pong.h"

class SFML_Clock {

public:

    SFML_Clock() = default;

    void Init()
    {
        m_clock.restart();
    }

    float GetDeltaTime()
    {
        float DeltaTime = m_clock.restart().asSeconds();

        // Clamp to 100ms max to prevent massive simulation jumps during lag spikes
        if (DeltaTime > 0.1f) {
            DeltaTime = 0.1f;
        }

        return DeltaTime;
    }

private:
    sf::Clock m_clock;
};


class SFMLPong {

public:

    SFMLPong() = default;

    bool Init()
    {
        sf::VideoMode VideoMode(sf::Vector2u((unsigned int)(m_config.WindowSize.x), 
                                             (unsigned int)(m_config.WindowSize.y)));

        m_window.create(VideoMode, "Pong", sf::Style::Default);

        if (!m_window.isOpen()) {
            std::cerr << "Failed to create SFML window." << std::endl;
            return false;
        }

        std::cout << "SFML Window initialized successfully." << std::endl;

        m_window.setVerticalSyncEnabled(true);
        //UpdateLetterboxView(m_window, m_config.WindowSize.x, m_config.WindowSize.y);

        m_gameClock.Init();
        m_game.Init(m_config);

        return true;
    }


    void GameLoop()
    {
        while (m_window.isOpen()) {
            float DeltaTime = m_gameClock.GetDeltaTime();

            ProcessEvents();

            m_game.Update(DeltaTime, m_input.PaddleLUp, m_input.PaddleLDown, m_input.PaddleRUp, m_input.PaddleRDown);

            RenderFrame();
        }
    }

private:

    void ProcessEvents()
    {
        while (const std::optional<sf::Event> event = m_window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                m_window.close();
            } else if (const sf::Event::KeyPressed* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressed->code) {
                case sf::Keyboard::Key::Escape:
                    std::cout << "Escape key pressed, quitting" << std::endl;
                    m_window.close();
                    break;
                case sf::Keyboard::Key::W:
                    m_input.PaddleLUp = true;
                    break;
                case sf::Keyboard::Key::S:
                    m_input.PaddleLDown = true;
                    break;
                case sf::Keyboard::Key::O:
                    m_input.PaddleRUp = true;
                    break;
                case sf::Keyboard::Key::L:
                    m_input.PaddleRDown = true;
                    break;
                default: break;
                }
            } else if (const sf::Event::KeyReleased* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                switch (keyReleased->code) {
                case sf::Keyboard::Key::W:
                    m_input.PaddleLUp = false;
                    break;
                case sf::Keyboard::Key::S:
                    m_input.PaddleLDown = false;
                    break;
                case sf::Keyboard::Key::O:
                    m_input.PaddleRUp = false;
                    break;
                case sf::Keyboard::Key::L:
                    m_input.PaddleRDown = false;
                    break;
                default: break;
                }
            }
        }
    }


    void RenderFrame()
    {
        // Clear backbuffer with specific grey color matching (16,16,16)
        m_window.clear(sf::Color(16, 16, 16));

        Rect PaddleLRect, PaddleRRect, BallRect;
        m_game.GetRects(BallRect, PaddleLRect, PaddleRRect);

        // Instantiate simple primitive rectangle layouts matching your Pong structures
        sf::RectangleShape sfPaddleL(sf::Vector2f(PaddleLRect.w, PaddleLRect.h));
        sfPaddleL.setPosition({ PaddleLRect.x, PaddleLRect.y });
        sfPaddleL.setFillColor(sf::Color::White);

        sf::RectangleShape sfPaddleR(sf::Vector2f(PaddleRRect.w, PaddleRRect.h));
        sfPaddleR.setPosition({ PaddleRRect.x, PaddleRRect.y });
        sfPaddleR.setFillColor(sf::Color::White);

        sf::RectangleShape sfBall(sf::Vector2f(BallRect.w, BallRect.h));
        sfBall.setPosition({ BallRect.x, BallRect.y });
        sfBall.setFillColor(sf::Color::White);

        // Issue sequential composition instructions to the open view context
        m_window.draw(sfPaddleL);
        m_window.draw(sfPaddleR);
        m_window.draw(sfBall);

        // Swaps buffers to display the rendered frame onto the screen
        m_window.display();
    }

    sf::RenderWindow m_window;
    SFML_Clock m_gameClock;
    GameConfig m_config;
    Pong m_game;
    PongInputState m_input;
};


SFMLPong g_sfmlPong;


// Utility function to calculate letterbox view ratios dynamically on resize
void UpdateLetterboxView(sf::RenderWindow& window, float TargetWidth, float TargetHeight)
{
    float WindowWidth = (float)(window.getSize().x);
    float WindowHeight = (float)(window.getSize().y);

    float WindowRatio = WindowWidth / WindowHeight;
    float TargetRatio = TargetWidth / TargetHeight;

    sf::View view;
    view.setSize({ TargetWidth, TargetHeight });
    view.setCenter({ TargetWidth / 2.0f, TargetHeight / 2.0f }); // Centers the camera on your game canvas

    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (WindowRatio >= TargetRatio) {
        sizeX = TargetRatio / WindowRatio;
        posX = (1.0f - sizeX) / 2.0f;
    } else {
        sizeY = WindowRatio / TargetRatio;
        posY = (1.0f - sizeY) / 2.0f;
    }

    view.setViewport(sf::FloatRect(sf::Vector2f{ posX, posY }, sf::Vector2f{ sizeX, sizeY }));
    window.setView(view);
}


int main(int argc, char* argv[])
{
    if (!g_sfmlPong.Init()) {
        return -1;
    }

    g_sfmlPong.GameLoop();

    return 0;
}

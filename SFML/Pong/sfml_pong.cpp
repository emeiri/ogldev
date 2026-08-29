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

// Input State Struct to track paddle keys cleanly
struct InputState {
    bool PaddleLUp = false;
    bool PaddleLDown = false;
    bool PaddleRUp = false;
    bool PaddleRDown = false;
};

static sf::RenderWindow Window;
static SFML_Clock GameClock;
static GameConfig Config;
static Pong Game;
static InputState Input;

// Utility function to calculate letterbox view ratios dynamically on resize
void UpdateLetterboxView(sf::RenderWindow& window, float targetWidth, float targetHeight)
{
    float windowWidth = (float)(window.getSize().x);
    float windowHeight = (float)(window.getSize().y);

    float windowRatio = windowWidth / windowHeight;
    float targetRatio = targetWidth / targetHeight;

    sf::View view;
    view.setSize({ targetWidth, targetHeight });
    view.setCenter({ targetWidth / 2.0f, targetHeight / 2.0f }); // Centers the camera on your game canvas

    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (windowRatio >= targetRatio) {
        sizeX = targetRatio / windowRatio;
        posX = (1.0f - sizeX) / 2.0f;
    } else {
        sizeY = windowRatio / targetRatio;
        posY = (1.0f - sizeY) / 2.0f;
    }

    view.setViewport(sf::FloatRect(sf::Vector2f{ posX, posY }, sf::Vector2f{ sizeX, sizeY }));
    window.setView(view);
}


bool AppInit()
{
    sf::VideoMode videoMode(sf::Vector2u((unsigned int)(Config.WindowSize.x), (unsigned int)(Config.WindowSize.y)));

    // SFML 3 target window initializer 
    Window.create(videoMode, "Pong", sf::Style::Default);

    if (!Window.isOpen()) {
        std::cerr << "Failed to create SFML window." << std::endl;
        return false;
    }

    std::cout << "SFML Window initialized successfully." << std::endl;

    Window.setVerticalSyncEnabled(true);
    UpdateLetterboxView(Window, Config.WindowSize.x, Config.WindowSize.y);

    GameClock.Init();
    Game.Init(Config);

    return true;
}


void ProcessEvents()
{
    // SFML 3 uses modern type-safe optional events instead of uninitialized structs
    while (const std::optional<sf::Event> event = Window.pollEvent())
    {
        // 1. Check for window closing triggers
        if (event->is<sf::Event::Closed>()) {
            Window.close();
        }

        // 2. Map rendering rules cleanly when the user scales the platform window
        else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            UpdateLetterboxView(Window, Config.WindowSize.x, Config.WindowSize.y);
        }

        // 3. Process key down allocations
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->code) {
            case sf::Keyboard::Key::Escape:
                std::cout << "Escape key pressed, quitting" << std::endl;
                Window.close();
                break;
            case sf::Keyboard::Key::W: Input.PaddleLUp = true; break;
            case sf::Keyboard::Key::S: Input.PaddleLDown = true; break;
            case sf::Keyboard::Key::O: Input.PaddleRUp = true; break;
            case sf::Keyboard::Key::L: Input.PaddleRDown = true; break;
            default: break;
            }
        }

        // 4. Process key up cancellations
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            switch (keyReleased->code) {
            case sf::Keyboard::Key::W: Input.PaddleLUp = false; break;
            case sf::Keyboard::Key::S: Input.PaddleLDown = false; break;
            case sf::Keyboard::Key::O: Input.PaddleRUp = false; break;
            case sf::Keyboard::Key::L: Input.PaddleRDown = false; break;
            default: break;
            }
        }
    }
}

/* This replaces static void RenderGame() */
static void RenderGame()
{
    // Clear backbuffer with specific grey color matching (16,16,16)
    Window.clear(sf::Color(16, 16, 16));

    Rect PaddleLRect, PaddleRRect, BallRect;
    Game.GetRects(BallRect, PaddleLRect, PaddleRRect);

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
    Window.draw(sfPaddleL);
    Window.draw(sfPaddleR);
    Window.draw(sfBall);

    // Swaps buffers to display the rendered frame onto the screen
    Window.display();
}

/* Synthesizes SDL_AppIterate and your application framework loop context */
void AppIterate()
{
    float DeltaTime = GameClock.GetDeltaTime();

    // Game updates utilizing the Event-Driven clean input state flags
    Game.Update(Input.PaddleLUp, Input.PaddleLDown, Input.PaddleRUp, Input.PaddleRDown, DeltaTime);

    RenderGame();
}

/* Replaces SDL_AppQuit wrapper behaviors cleanly upon structural scope termination */
void AppQuit()
{
    // SFML RAII takes care of destroying window configurations automatically
    std::cout << "Application terminated successfully." << std::endl;
}

// Global Main Application Runner to orchestrate loop flow cleanly without callbacks
int main(int argc, char* argv[])
{
    if (!AppInit()) {
        return -1;
    }

    // Standard structural loop replacement context execution loop
    while (Window.isOpen()) {
        ProcessEvents();
        AppIterate();
    }

    AppQuit();
    return 0;
}

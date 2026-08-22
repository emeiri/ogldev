#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    // Create a 800x600 window with a title
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML 3 Hello World");

    // Set the frame rate limit to 60 FPS
    window.setFramerateLimit(60);

    // Load a font (Ensure you place a valid font file in your execution folder!)
   // sf::Font font;
  //  if (!font.openFromFile("arial.ttf"))
  //  {
  //      std::cerr << "Error loading font! Continuing with a blank screen.\n";
  //  }

    // Setup the "Hello World" text object
  //  sf::Text text(font, "Hello, SFML 3!", 50);
   // text.setFillColor(sf::Color::Green);

    // Center the text on the screen
   // sf::FloatRect textBounds = text.getLocalBounds();
   // text.setOrigin({ textBounds.size.x / 2.0f, textBounds.size.y / 2.0f });
   // text.setPosition({ 400.0f, 300.0f });

    // Main game loop
    while (window.isOpen())
    {
        // SFML 3 Event Handling: Uses optional return values instead of pollEvent(event)
        while (const std::optional event = window.pollEvent())
        {
            // Check if the user clicked the window's close button
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            // Check if the user pressed the Escape key
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
            }
        }

        // Clear the screen with a dark blue color
        window.clear(sf::Color(10, 15, 30));

        // Draw our text object
      //  window.draw(text);

        // Display everything rendered to the screen
        window.display();
    }

    return 0;
}

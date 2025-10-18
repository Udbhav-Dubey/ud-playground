#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Mouse Controlled Donut");

    float outerRadius = 100.f;
    float innerRadius = 60.f;

    sf::CircleShape outer(outerRadius);
    outer.setFillColor(sf::Color::Blue);
    outer.setOrigin(outerRadius, outerRadius);

    sf::CircleShape inner(innerRadius);
    inner.setFillColor(sf::Color::Black);
    inner.setOrigin(innerRadius, innerRadius);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Get mouse position
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        outer.setPosition(mousePos.x, mousePos.y);
        inner.setPosition(mousePos.x, mousePos.y);

        window.clear(sf::Color::Black);
        window.draw(outer);
        window.draw(inner);
        window.display();
    }

    return 0;
}


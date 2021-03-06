#include "Settings.h"

#include <game/Game.h>

#include <entities/units/Marine.h>
#include <entities/enemies/EnemyGrunt.h>
#include <entities/enemies/EnemySiegeTank.h>

#include <time.h>
#include <iostream>
#include <random>

sf::View view;
int windowSquareSize;

sf::Vector2f getMousePos(const sf::Vector2f &pos, const sf::Sprite &gameSprite) {
    sf::Vector2f offsetPos = pos - gameSprite.getPosition();
    return sf::Vector2f(offsetPos.x / gameSprite.getScale().x * static_cast<float>(view.getSize().x) / windowSquareSize, offsetPos.y / gameSprite.getScale().y * static_cast<float>(view.getSize().y) / windowSquareSize) + sf::Vector2f(128.0f, 128.0f);
}

int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(1920, 1200), "Roguelike", sf::Style::Fullscreen, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	// ---------------------------- Game Loop -----------------------------

    sf::RenderTexture gameTarget;

    windowSquareSize = std::min(window.getSize().x, window.getSize().y);

    gameTarget.create(windowSquareSize, windowSquareSize);

	view = gameTarget.getDefaultView();

	view.setSize(256.0f, 256.0f);
	view.setCenter(sf::Vector2f(view.getSize().x * 0.5f, view.getSize().y * 0.5f));

	bool quit = false;

	sf::Clock clock;

	float dt = 0.017f;

	Game g;

	g.create(window.getSize(), "resources/desc.txt");

	std::vector<std::shared_ptr<Marine>> marines;

	for (int i = 0; i < 4; i++) {
		std::shared_ptr<Marine> marine = std::make_shared<Marine>();

		g.getCurrentLevel()->getCurrentRoom().add(marine);

		marine->create();

		std::uniform_real_distribution<float> marineDist(128.0f - 60.0f, 128.0 + 60.0f);

		marine->setPosition(sf::Vector2f(marineDist(generator), marineDist(generator)));

		marine->stop();

		marines.push_back(marine);
	}

	for (int x = 0; x < g.getCurrentLevel()->getWidth(); x++)
		for (int y = 0; y < g.getCurrentLevel()->getHeight(); y++) {
			if (x == g.getCurrentLevel()->getCurrentCellX() && y == g.getCurrentLevel()->getCurrentCellY())
				continue;

			if (g.getCurrentLevel()->getCell(x, y)._room != nullptr) {
				for (int i = 0; i < 8; i++) {			
					std::shared_ptr<EnemyGrunt> grunt = std::make_shared<EnemyGrunt>();

					g.getCurrentLevel()->getCell(x, y)._room->add(grunt);

                    grunt->create();

					std::uniform_real_distribution<float> gruntDist(128.0f - 60.0f, 128.0 + 60.0f);

                    grunt->setPosition(sf::Vector2f(gruntDist(generator), gruntDist(generator)));

                    grunt->stop();
				}

				/*for (int i = 0; i < 2; i++) {
					std::shared_ptr<EnemySiegeTank> tank = std::make_shared<EnemySiegeTank>();

					g.getCurrentLevel()->getCell(x, y)._room->add(tank);

					tank->create();

					std::uniform_real_distribution<float> marineDist(128.0f - 40.0f, 128.0 + 40.0f);

					tank->setPosition(sf::Vector2f(marineDist(generator), marineDist(generator)));

					tank->stop();
				}*/
			}
		}

	bool prevLMBDown = false;
	bool prevRMBDown = false;
	bool prevADown = false;
	bool attack = false;
	bool prevTDown = false;
	bool transit = false;
	sf::Vector2f selectionStart(0.0f, 0.0f);
	bool selecting = false;
	bool orderGiven = false;
	bool orderGivenPrev = false;
	
	do {
		clock.restart();

		// ----------------------------- Input -----------------------------

		sf::Event windowEvent;

		while (window.pollEvent(windowEvent))
		{
			switch (windowEvent.type)
			{
			case sf::Event::Closed:
				quit = true;
				break;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			quit = true;

        gameTarget.clear();

        gameTarget.setView(view);

		g.update(0.017f);
		g.render(gameTarget);

        gameTarget.display();

        window.clear();

        sf::Sprite gameSprite;
        gameSprite.setTexture(gameTarget.getTexture());

        gameSprite.setOrigin(gameTarget.getSize().x * 0.5f, gameTarget.getSize().y * 0.5f);

        gameSprite.setPosition(window.getSize().x * 0.5f, window.getSize().y * 0.5f);

        gameSprite.setScale(static_cast<float>(windowSquareSize) / gameTarget.getSize().x, static_cast<float>(windowSquareSize) / gameTarget.getSize().y);

        window.draw(gameSprite);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
				if ((*it)->_type == 1) {
					static_cast<Friendly*>(*it)->split();
				}

			attack = false;
			orderGiven = true;
		}
		//else if (!sf::Mouse::isButtonPressed(sf::Mouse::Right) && prevRMBDown)
		//	orderGiven = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            attack = true;
            orderGiven = true;
        }

		//else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::A) && prevADown)
		//	orderGiven = false;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
            transit = true;
            orderGiven = true;
        }

		//else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::T) && prevTDown)
		//	orderGiven = false;

		if (!orderGiven) {
			if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && prevLMBDown) {
				if (selecting) {
					// Make selection
                    sf::Vector2f selectionEnd = window.mapPixelToCoords(sf::Mouse::getPosition(window));

					g.select(ltbl::rectFromBounds(getMousePos(selectionStart, gameSprite) - sf::Vector2f(0.01f, 0.01f), getMousePos(selectionEnd, gameSprite) + sf::Vector2f(0.01f, 0.01f)));

					selecting = false;
				}
			}
			else if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !prevLMBDown) {
                selectionStart = window.mapPixelToCoords(sf::Mouse::getPosition(window));

				selecting = true;
			}
		}

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !prevLMBDown) {
            if (attack) {
                for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
                    if ((*it)->_type == 1) {
                        static_cast<Friendly*>(*it)->attackMove(getMousePos(window.mapPixelToCoords(sf::Mouse::getPosition(window)), gameSprite));
                    }

                attack = false;
            }
            else if (transit) {
                for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
                    if ((*it)->_type == 1) {
                        static_cast<Friendly*>(*it)->transitMove(getMousePos(window.mapPixelToCoords(sf::Mouse::getPosition(window)), gameSprite));
                    }

                transit = false;
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !prevLMBDown)
            orderGiven = false;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !prevRMBDown)
            orderGiven = false;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !prevRMBDown) {
            for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
                if ((*it)->_type == 1) {
                    static_cast<Friendly*>(*it)->move(getMousePos(window.mapPixelToCoords(sf::Mouse::getPosition(window)), gameSprite));
                }

            attack = false;
            orderGiven = false;
        }
		
        orderGivenPrev = orderGiven;

		if (selecting) {
			sf::Vector2f selectionCurrent = window.mapPixelToCoords(sf::Mouse::getPosition(window));

			sf::RectangleShape selectionShape;
			
			sf::Vector2f lower, upper;

			lower.x = std::min(selectionStart.x, selectionCurrent.x);
			lower.y = std::min(selectionStart.y, selectionCurrent.y);
			upper.x = std::max(selectionStart.x, selectionCurrent.x);
			upper.y = std::max(selectionStart.y, selectionCurrent.y);

			selectionShape.setPosition(lower);
			selectionShape.setSize(upper - lower);

			selectionShape.setFillColor(sf::Color(0, 210, 0, 127));
			selectionShape.setOutlineColor(sf::Color(0, 210, 0, 255));

			selectionShape.setOutlineThickness(1);

			window.draw(selectionShape);
		}

		prevADown = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
		prevLMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
		prevRMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Right);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			g.getCurrentLevel()->transition(2);
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			g.getCurrentLevel()->transition(0);
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			g.getCurrentLevel()->transition(3);
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			g.getCurrentLevel()->transition(1);

		window.display();
	} while (!quit);

	return 0;
}
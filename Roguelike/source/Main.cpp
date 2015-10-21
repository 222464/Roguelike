#include "Settings.h"

#include <game/Game.h>

#include <entities/units/Marine.h>
#include <entities/enemies/EnemyMarine.h>

#include <time.h>
#include <iostream>
#include <random>

int main() {
	sf::RenderWindow window;

	sf::ContextSettings glContextSettings;
	glContextSettings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 800), "Roguelike", sf::Style::Default, glContextSettings);

	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	std::mt19937 generator(time(nullptr));

	// ---------------------------- Game Loop -----------------------------

	sf::View view = window.getDefaultView();

	view.setSize(sf::Vector2f(256.0f, 256.0f));
	view.setCenter(sf::Vector2f(view.getSize().x * 0.5f, view.getSize().y * 0.5f));

	bool quit = false;

	sf::Clock clock;

	float dt = 0.017f;

	Game g;

	g.create(window.getSize(), "resources/desc.txt");

	std::vector<std::shared_ptr<Marine>> marines;

	for (int i = 0; i < 32; i++) {
		std::shared_ptr<Marine> marine = std::make_shared<Marine>();

		g.getCurrentLevel()->getCurrentRoom().add(marine);

		marine->create();

		std::uniform_real_distribution<float> marineDist(16.0f, 86.0f);

		marine->setPosition(sf::Vector2f(marineDist(generator), marineDist(generator)));

		marine->stop();

		marines.push_back(marine);
	}

	for (int i = 0; i < 0; i++) {
		std::shared_ptr<EnemyMarine> marine = std::make_shared<EnemyMarine>();

		g.getCurrentLevel()->getCurrentRoom().add(marine);

		marine->create();

		std::uniform_real_distribution<float> marineDist(160.0f, 240.0f);

		marine->setPosition(sf::Vector2f(marineDist(generator), marineDist(generator)));

		marine->stop();
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

		window.clear();

		window.setView(view);

		g.update(0.017f);
		g.render(window);

		orderGivenPrev = orderGiven;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
				if ((*it)->_type == 1) {
					static_cast<Friendly*>(*it)->split();
				}

			attack = false;
			orderGiven = false; // Instant order
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !prevRMBDown) {
			for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
				if ((*it)->_type == 1) {
					static_cast<Friendly*>(*it)->move(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
				}

			attack = false;
			orderGiven = true;
		}
		else if (!sf::Mouse::isButtonPressed(sf::Mouse::Right) && prevRMBDown)
			orderGiven = false;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && !prevADown) {
			attack = true;
			orderGiven = true;
		}
		else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::A) && prevADown)
			orderGiven = false;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T) && !prevTDown) {
			transit = true;
			orderGiven = true;
		}
		else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::T) && prevTDown)
			orderGiven = false;
		
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !prevLMBDown) {		
			if (attack) {
				for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
					if ((*it)->_type == 1) {
						static_cast<Friendly*>(*it)->attackMove(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
					}

				attack = false;
				orderGiven = true;
			}
			else if (transit) {
				for (std::unordered_set<Entity*>::iterator it = g._selection.begin(); it != g._selection.end(); it++)
					if ((*it)->_type == 1) {
						static_cast<Friendly*>(*it)->transitMove(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
					}

				transit = false;
				orderGiven = true;
			}
		}
		else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && prevLMBDown)
			orderGiven = false;

		if (!orderGiven && !orderGivenPrev) {
			if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
				if (prevLMBDown && selecting) {
					// Make selection
					sf::Vector2f selectionEnd = window.mapPixelToCoords(sf::Mouse::getPosition(window));

					g.select(ltbl::rectFromBounds(selectionStart - sf::Vector2f(0.01f, 0.01f), selectionEnd + sf::Vector2f(0.01f, 0.01f)));

					selecting = false;
				}
			}
			else {
				if (!prevLMBDown && !selecting) {
					selectionStart = window.mapPixelToCoords(sf::Mouse::getPosition(window));

					selecting = true;
				}
			}
		}
		
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
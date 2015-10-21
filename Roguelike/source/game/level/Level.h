#pragma once

#include "room/Room.h"
#include <random>

class Game;

class Level {
public:
	struct Cell {
		std::unique_ptr<Room> _room;
	};

private:
	std::vector<Cell> _cells;

	int _width, _height;

	std::vector<std::shared_ptr<sf::Texture>> _backgroundTextures;
	std::vector<std::shared_ptr<sf::Texture>> _doorTextures;
	std::vector<std::shared_ptr<sf::Texture>> _roofTextures;

	int _currentCellX, _currentCellY;

	float _transitionTimer;
	int _transitionDir;

	Game* _pGame;

public:
	float _transitionTime;

	float _maxAllowedTransitTime;

	Level();

	void create(Game* pGame, int width, int height, int drunkSteps, int maxStepDist,
		const std::vector<std::shared_ptr<sf::Texture>> &backgroundTextures,
		const std::vector<std::shared_ptr<sf::Texture>> &doorTextures,
		const std::vector<std::shared_ptr<sf::Texture>> &roofTextures,
		std::mt19937 &generator);

	void update(float dt);
	
	void render(sf::RenderTarget &rt);

	void transition(int dir);

	bool inTransition() const {
		return _transitionTimer > 0.0f;
	}

	Cell &getCell(int x, int y) {
		return _cells[x + y * _width];
	}

	const Cell &getCell(int x, int y) const {
		return _cells[x + y * _width];
	}

	Room &getCurrentRoom() {
		return *_cells[_currentCellX + _currentCellY * _width]._room;
	}

	const Room &getCurrentRoom() const {
		return *_cells[_currentCellX + _currentCellY * _width]._room;
	}

	int getWidth() const {
		return _width;
	}

	int getHeight() const {
		return _height;
	}

	Game* getGame() const;
};
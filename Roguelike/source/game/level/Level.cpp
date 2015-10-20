#include "Level.h"

#include "../Game.h"

#include <assert.h>
#include <iostream>

Game* Level::getGame() const {
	return _pGame;
}

void Level::create(Game* pGame, int width, int height, int drunkSteps, int maxStepDist, const std::vector<std::shared_ptr<sf::Texture>> &backgroundTextures, std::mt19937 &generator) {
	_width = width;
	_height = height;

	_pGame = pGame;

	_backgroundTextures = backgroundTextures;

	_cells.resize(_width * _height);

	int drunkX = _width / 2;
	int drunkY = _height / 2;

	_currentCellX = drunkX;
	_currentCellY = drunkY;

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
	std::uniform_int_distribution<int> backgroundDist(0, backgroundTextures.size() - 1);

	// First room
	{
		int cIndex = drunkX + drunkY * _width;

		Cell &c = _cells[cIndex];

		if (c._room == nullptr) {
			c._room = std::make_unique<Room>();

			c._room->create(this, std::vector<int>(1, cIndex), backgroundDist(generator));
		}
	}

	for (int i = 0; i < drunkSteps; i++) {
		int dir = static_cast<int>(dist01(generator) * 4.0f) % 4;

		for (int l = 0; l < maxStepDist; l++) {
			int prevDrunkX = drunkX;
			int prevDrunkY = drunkY;

			switch (dir) {
			case 0:
				drunkX++;

				break;

			case 1:
				drunkY++;

				break;

			case 2:
				drunkX--;

				break;

			case 3:
				drunkY--;

				break;
			}
			
			if (drunkX < 0 || drunkY < 0 || drunkX >= _width || drunkY >= _height) {
				prevDrunkX = drunkX;
				prevDrunkY = drunkY;

				break;
			}

			int cIndex = drunkX + drunkY * _width;

			Cell &c = _cells[cIndex];

			if (c._room == nullptr) {
				c._room = std::make_unique<Room>();

				c._room->create(this, std::vector<int>(1, cIndex), backgroundDist(generator));
			}
		}
	}
}

void Level::update(float dt) {
	if (_transitionTimer > 0.0f) {
		// Update transition

		_transitionTimer -= dt;
	}
	else
		getCurrentRoom().update(dt);
}

void Level::render(sf::RenderTarget &rt) {
	if (_transitionTimer > 0.0f) {
		float interp = _transitionTimer / _transitionTime;

		// Render transition
		sf::Sprite oldRoomSprite;
		
		oldRoomSprite.setTexture(getGame()->_transitionTexture0.getTexture());

		oldRoomSprite.setColor(sf::Color(interp * 255.0f, interp * 255.0f, interp * 255.0f));
		
		sf::Sprite newRoomSprite;

		newRoomSprite.setTexture(getGame()->_transitionTexture1.getTexture());

		sf::Vector2f sizef(getGame()->_transitionTexture0.getSize().x, getGame()->_transitionTexture0.getSize().y);

		newRoomSprite.setColor(sf::Color((1.0f - interp) * 255.0f, (1.0f - interp) * 255.0f, (1.0f - interp) * 255.0f));

		switch (_transitionDir) {
		case 0:
			oldRoomSprite.setPosition((interp - 1.0f) * sizef.x, 0.0f);
			newRoomSprite.setPosition(interp * sizef.x, 0.0f);
			break;
		case 1:
			oldRoomSprite.setPosition(0.0f, (interp - 1.0f) * sizef.y);
			newRoomSprite.setPosition(0.0f, interp * sizef.y);
			break;
		case 2:
			oldRoomSprite.setPosition(-(interp - 1.0f) * sizef.x, 0.0f);
			newRoomSprite.setPosition(-interp * sizef.x, 0.0f);
			break;
		case 3:
			oldRoomSprite.setPosition(0.0f, -(interp - 1.0f) * sizef.y);
			newRoomSprite.setPosition(0.0f, -interp * sizef.y);
			break;
		}

		rt.draw(oldRoomSprite);
		rt.draw(newRoomSprite);
	}
	else
		getCurrentRoom().render(rt, _backgroundTextures);
}

void Level::transition(int dir) {
	if (_transitionTimer > 0.0f)
		return;

	_transitionTimer = _transitionTime;

	_transitionDir = dir;

	Room &oldRoom = getCurrentRoom();

	int oldCellX = _currentCellX;
	int oldCellY = _currentCellY;

	switch (_transitionDir) {
	case 0:
		_currentCellX++;

		break;

	case 1:
		_currentCellY++;

		break;

	case 2:
		_currentCellX--;

		break;

	case 3:
		_currentCellY--;

		break;
	}

	if (getCell(_currentCellX, _currentCellY)._room == nullptr) {
		std::cerr << "ERROR: Cannot transition: No room in direction: " << dir << std::endl;

		_currentCellX = oldCellX;
		_currentCellY = oldCellY;

		return;
	}

	// Render old room to temporary rt
	getGame()->_transitionTexture0.clear();
	getGame()->_transitionTexture1.clear();

	oldRoom.render(getGame()->_transitionTexture0, _backgroundTextures);
	getCurrentRoom().render(getGame()->_transitionTexture1, _backgroundTextures);

	getGame()->_transitionTexture0.display();
	getGame()->_transitionTexture1.display();
}
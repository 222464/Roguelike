#include "Game.h"

#include <fstream>
#include <iostream>

bool Game::create(const sf::Vector2u &lsImageSize, const std::string &descName) {
	_generator.seed(time(nullptr));

	_penumbraTexture.loadFromFile("resources/lighting/penumbraTexture.png");
	_unshadowShader.loadFromFile("resources/lighting/unshadowShader.vert", "resources/lighting/unshadowShader.frag");
	_lightOverShapeShader.loadFromFile("resources/lighting/lightOverShapeShader.vert", "resources/lighting/lightOverShapeShader.frag");

	_lightSystem.create(sf::FloatRect(0.0f, 0.0f, lsImageSize.x, lsImageSize.y), lsImageSize, _penumbraTexture, _unshadowShader, _lightOverShapeShader);

	std::ifstream fromDesc(descName);

	if (!fromDesc.is_open()) {
		std::cerr << "ERROR: Could not find desc: " << descName << std::endl;

		return false;
	}

	std::string line;
	std::string param;

	int numLevels;

	fromDesc >> param;
	fromDesc >> numLevels;

	_levelDescs.resize(numLevels);

	for (int l = 0; l < numLevels; l++) {
		fromDesc >> param;
		fromDesc >> _levelDescs[l]._name;

		fromDesc >> param;
		fromDesc >> _levelDescs[l]._width >> _levelDescs[l]._height;

		fromDesc >> param;
		fromDesc >> _levelDescs[l]._maxStepDist;

		fromDesc >> param;
		fromDesc >> _levelDescs[l]._drunkSteps;

		int numBackgrounds;

		fromDesc >> param;
		fromDesc >> numBackgrounds;

		_levelDescs[l]._backgroundTextures.resize(numBackgrounds);
		_levelDescs[l]._doorTextures.resize(numBackgrounds);
		_levelDescs[l]._roofTextures.resize(numBackgrounds);

		for (int b = 0; b < numBackgrounds; b++) {
			fromDesc >> param;

			std::shared_ptr<sf::Texture> background = std::make_shared<sf::Texture>();

			background->loadFromFile(param);

			_levelDescs[l]._backgroundTextures[b] = background;

			fromDesc >> param;

			std::shared_ptr<sf::Texture> door = std::make_shared<sf::Texture>();

			door->loadFromFile(param);

			_levelDescs[l]._doorTextures[b] = door;

			fromDesc >> param;

			std::shared_ptr<sf::Texture> roof = std::make_shared<sf::Texture>();

			roof->loadFromFile(param);

			_levelDescs[l]._roofTextures[b] = roof;
		}
	}

	// First level
	nextLevel();

	_transitionTexture0.create(_levelDescs.front()._backgroundTextures.front()->getSize().x, _levelDescs.front()._backgroundTextures.front()->getSize().y);
	_transitionTexture1.create(_levelDescs.front()._backgroundTextures.front()->getSize().x, _levelDescs.front()._backgroundTextures.front()->getSize().y);
}

void Game::nextLevel() {
	_currentLevelIndex++;

	// Generate first level
	_currentLevel = std::make_unique<Level>();

	_currentLevel->create(this, _levelDescs[_currentLevelIndex]._width, _levelDescs[_currentLevelIndex]._height, _levelDescs[_currentLevelIndex]._drunkSteps, _levelDescs[_currentLevelIndex]._maxStepDist, _levelDescs[_currentLevelIndex]._backgroundTextures, _levelDescs[_currentLevelIndex]._doorTextures, _levelDescs[_currentLevelIndex]._roofTextures, _generator);
}

void Game::update(float dt) {
	if (_transitionTimer > 0.0f) {
		// Update transition

		_transitionTimer -= dt;
	}
	else
		_currentLevel->update(dt);
}

void Game::render(sf::RenderTarget &rt) {
	if (_transitionTimer > 0.0f) {
		// Render transition

	}
	else
		_currentLevel->render(rt);
}

void Game::transition() {
	if (_transitionTimer > 0.0f)
		return;

	_transitionTimer = _transitionTime;

	nextLevel();
}

void Game::select(const sf::FloatRect &rect) {
	if (_transitionTimer > 0.0f)
		return;

	_selection.clear();

	// Search for enemy in range
	std::vector<ltbl::QuadtreeOccupant*> occupants;

	getCurrentLevel()->getCurrentRoom().getQuadtree().queryRegion(occupants, rect);

	for (int i = 0; i < occupants.size(); i++) {
		Entity* pEntity = static_cast<Entity*>(occupants[i]);

		if (pEntity->_type == 1) {
			_selection.insert(pEntity);
		}
	}
}
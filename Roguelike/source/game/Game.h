#pragma once

#include "level/Level.h"

#include <Table.h>

class Game {
public:
	struct LevelDesc {
		int _width, _height;

		int _maxStepDist;

		int _drunkSteps;

		std::string _name;

		std::vector<std::shared_ptr<sf::Texture>> _backgroundTextures;
		std::vector<std::shared_ptr<sf::Texture>> _doorTextures;
		std::vector<std::shared_ptr<sf::Texture>> _roofTextures;
	};

private:
	std::vector<LevelDesc> _levelDescs;
	std::unique_ptr<Level> _currentLevel;
	int _currentLevelIndex = -1;

	void nextLevel();

	float _transitionTimer;

	sf::Texture _penumbraTexture;
	sf::Shader _unshadowShader;
	sf::Shader _lightOverShapeShader;

public:
	Table _resources;

	ltbl::LightSystem _lightSystem;

	std::unordered_set<Entity*> _selection;

	float _transitionTime;

	std::mt19937 _generator;

	sf::RenderTexture _transitionTexture0;
	sf::RenderTexture _transitionTexture1;

	Game()
		: _transitionTime(5.0f)
	{}

	bool create(const sf::Vector2u &lsImageSize, const std::string &descName);

	void update(float dt);

	void render(sf::RenderTarget &rt);

	// Proceeds a level
	void transition();

	void select(const sf::FloatRect &rect);

	void select(const sf::Vector2f &position) {
		select(sf::FloatRect(position, sf::Vector2f(0.001f, 0.001f)));
	}

	bool inTransition() const {
		return _transitionTimer > 0.0f;
	}

	const std::vector<LevelDesc> &getLevelDescs() const {
		return _levelDescs;
	}

	Level* getCurrentLevel() const {
		return _currentLevel.get();
	}
};
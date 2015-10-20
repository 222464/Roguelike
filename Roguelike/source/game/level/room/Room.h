#pragma once

#include "entities/Entity.h"
#include <ltbl/quadtree/StaticQuadtree.h>

class Level;
class Game;

class Room {
private:
	std::vector<std::shared_ptr<Entity>> _entities;
	std::vector<std::shared_ptr<Entity>> _newEntities;

	ltbl::DynamicQuadtree _quadtree;

	Level* _pLevel;

	int _background;

public:
	int _numSubSteps;

	Room()
		: _pLevel(nullptr), _numSubSteps(8)
	{}

	void create(Level* pLevel, const std::vector<int> &cellIndices, int background);

	void update(float dt);

	void render(sf::RenderTarget &rt, const std::vector<std::shared_ptr<sf::Texture>> &backgrounds);

	void add(const std::shared_ptr<Entity> &entity, bool addToQuadtree = true);

	ltbl::DynamicQuadtree &getQuadtree() {
		return _quadtree;
	}

	const std::vector<std::shared_ptr<Entity>> &getEntities() const {
		return _entities;
	}

	Level* getLevel() const;

	Game* getGame() const;

	friend class Level;
};
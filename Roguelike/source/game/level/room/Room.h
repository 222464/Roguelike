#pragma once

#include "entities/Entity.h"
#include <ltbl/quadtree/StaticQuadtree.h>

class Level;
class Game;

class Room {
private:
	std::vector<std::shared_ptr<Entity>> _entities;
	std::vector<std::shared_ptr<Entity>> _addedEntitiesBuffer;
	std::vector<bool> _addedEntitiesQuadtreeStatusBuffer;
	std::vector<std::shared_ptr<Entity>> _transitLimbo;
	std::vector<bool> _transitLimboEntitiesQuadtreeStatusBuffer;

	ltbl::DynamicQuadtree _quadtree;

	Level* _pLevel;

	int _background;

	int _cellX, _cellY;

	int _width, _height;

	std::vector<sf::FloatRect> _walls;

public:
	int _numSubSteps;

	float _portalSize;

	float _transitDistanceRange;

	float _wallRange;

	Room();

	void create(Level* pLevel, int cellX, int cellY, int width, int height, const std::vector<int> &cellIndices, int background);

	void update(float dt);

	void render(sf::RenderTarget &rt,
		const std::vector<std::shared_ptr<sf::Texture>> &backgrounds,
		const std::vector<std::shared_ptr<sf::Texture>> &doors,
		const std::vector<std::shared_ptr<sf::Texture>> &roofs);

	void add(const std::shared_ptr<Entity> &entity, bool addToQuadtree = true);
	void addToTransitLimbo(const std::shared_ptr<Entity> &entity, bool addToQuadtree = true);

	ltbl::DynamicQuadtree &getQuadtree() {
		return _quadtree;
	}

	const std::vector<std::shared_ptr<Entity>> &getEntities() const {
		return _entities;
	}

	// Returns direction if on a portal, otherwise -1
	int getPortal(const sf::FloatRect &aabb);
	int getPortalContains(const sf::FloatRect &aabb);

	bool wallCollision(sf::FloatRect &aabb);

	Level* getLevel() const;

	Game* getGame() const;

	int getWidth() const {
		return _width;
	}

	int getHeight() const {
		return _height;
	}

	int getCellX() const {
		return _cellX;
	}

	int getCellY() const {
		return _cellY;
	}

	friend class Level;
};
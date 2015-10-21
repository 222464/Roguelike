#pragma once

#include <SFML/Graphics.hpp>

#include <ltbl/quadtree/QuadtreeOccupant.h>
#include <ltbl/lighting/LightSystem.h>
#include <ltbl/Math.h>

#include <memory>

class Room;
class Level;
class Game;

class Entity : public ltbl::QuadtreeOccupant {
private:
	Room* _pCurrentRoom;

	bool _remove;

public:
	int _transit;

	float _timeSinceTransit;

	std::string _name;
	int _type;

	float _layer;

	Entity()
		: _pCurrentRoom(nullptr), _name("_"), _layer(0.0f), _remove(false), _type(0), _transit(-1), _timeSinceTransit(0.0f)
	{}

	virtual ~Entity() {}

	virtual void update(float dt) {}

	virtual void subUpdate(float dt, int subStep, int numSubSteps) {}

	virtual void preRender(sf::RenderTarget &rt) {}
	virtual void render(sf::RenderTarget &rt) {}

	virtual sf::FloatRect getAABB() const = 0;

	virtual void removeDeadReferences() {}

	void remove() {
		_remove = true;
	}

	bool removed() const {
		return _remove;
	}

	static bool compareLayers(const std::shared_ptr<Entity> &lhs, const std::shared_ptr<Entity> &rhs)
	{
		return lhs->_layer < rhs->_layer;
	}

	bool inTransit() const {
		return _transit != -1;
	}

	int getTransit() const {
		return _transit;
	}

	void transit(int dir);

	Room* getRoom() const;

	Level* getLevel() const;

	Game* getGame() const;

	friend class Room;
};
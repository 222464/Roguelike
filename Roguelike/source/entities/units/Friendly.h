#pragma once

#include <game/level/room/entities/Entity.h>

class Friendly : public Entity {
private:

public:
	float _hp;
    float _maxhp;

	Friendly()
	{
		_name = "friendly";
		_type = 1;
        _maxhp = _hp = 100.0f;
	}

	virtual void move(sf::Vector2f &position) {}
	virtual void attackMove(const sf::Vector2f &position) {}
	virtual void transitMove(sf::Vector2f &position) {}

	virtual void split() {}

	virtual void hold() {}
	virtual void stop() {}

	virtual void setPosition(const sf::Vector2f &position) {}

	virtual const sf::Vector2f &getPosition() const {
		return sf::Vector2f(0.0f, 0.0f);
	}

	void setRotation(float rotation) {}

	virtual float getRotation() const {
		return 0.0f;
	}

	// Inherited from Entity
	virtual void update(float dt) {}

	virtual void render(sf::RenderTarget &rt) {}

	virtual sf::FloatRect getAABB() const = 0;
};

#pragma once

#include <game/level/room/entities/Entity.h>

class Enemy : public Entity {
private:
	sf::Vector2f _position;

	sf::Texture _testTexture;

public:
	Enemy()
	{
		_name = "enemy";
		_type = 2;
	}

	void create();

	void setPosition(const sf::Vector2f &position);

	const sf::Vector2f &getPosition() const {
		return _position;
	}

	// Inherited from Entity
	void update(float dt);

	void render(sf::RenderTarget &rt);

	sf::FloatRect getAABB() const;
};

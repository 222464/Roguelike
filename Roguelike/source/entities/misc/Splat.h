#pragma once

#include <game/level/room/entities/Entity.h>

class Splat : public Entity {
private:
	sf::Vector2f _position;

	float _rotation;
	float _lifespan;
	float _lifeTimer;

	std::shared_ptr<sf::Texture> _splatTexture;

public:
	Splat()
		: _lifeTimer(0.0f)
	{
		_name = "splat";
		_layer = -0.1f;
	}

	void create(const sf::Vector2f &initPosition, float initRotation, float lifespan);

	const sf::Vector2f &getPosition() const {
		return _position;
	}

	// Inherited from Entity
	void update(float dt);

	void render(sf::RenderTarget &rt);

	sf::FloatRect getAABB() const;
};

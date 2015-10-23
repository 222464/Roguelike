#pragma once

#include <game/level/room/entities/Entity.h>

class Explosion : public Entity {
private:
	sf::Vector2f _position;

	float _rotation;
	float _lifespan;
	float _lifeTimer;

	std::shared_ptr<sf::Texture> _explosionTexture;

public:
	Explosion()
		: _lifeTimer(0.0f)
	{
		_name = "explosion";
		_layer = -0.05f;
	}

	void create(const sf::Vector2f &initPosition, float initRotation);

	const sf::Vector2f &getPosition() const {
		return _position;
	}

	// Inherited from Entity
	void update(float dt);

	void render(sf::RenderTarget &rt);

	sf::FloatRect getAABB() const;
};

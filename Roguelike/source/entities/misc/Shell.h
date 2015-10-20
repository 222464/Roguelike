#pragma once

#include <game/level/room/entities/Entity.h>

class Shell : public Entity {
private:
	sf::Vector2f _position;
	sf::Vector2f _velocity;
	
	float _rotation;
	float _rotationVelocity;
	float _velocityDecay;
	float _lifespan;
	float _lifeTimer;

	std::shared_ptr<sf::Texture> _shellTexture;

public:
	Shell()
		: _lifeTimer(0.0f)
	{
		_name = "shell";
		_layer = -0.05f;
	}

	void create(const sf::Vector2f &initVelocity, const sf::Vector2f &initPosition, float initRotationVelocity, float initRotation, float lifespan, float velocityDecay);

	const sf::Vector2f &getPosition() const {
		return _position;
	}

	// Inherited from Entity
	void update(float dt);

	void render(sf::RenderTarget &rt);

	sf::FloatRect getAABB() const;
};

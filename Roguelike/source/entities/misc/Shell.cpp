#include "Shell.h"

#include <game/Game.h>

void Shell::create(const sf::Vector2f &initVelocity, const sf::Vector2f &initPosition, float initRotation, float initRotationVelocity, float lifespan, float velocityDecay) {
	if (!getGame()->_resources.exists("shell1")) {
		Ptr<sf::Texture> shellTexture = std::make_shared<sf::Texture>();

		shellTexture->loadFromFile("resources/misc/shell1.png");

		_shellTexture = shellTexture;

		getGame()->_resources.insertPtr("shell1", shellTexture);
	}
	else
		_shellTexture = getGame()->_resources.getPtr<sf::Texture>("shell1");
	
	_position = initPosition;
	_velocity = initVelocity;

	_rotation = initRotation;
	_rotationVelocity = initRotationVelocity;

	_lifespan = lifespan;
	_velocityDecay = velocityDecay;
}

// Inherited from Entity
void Shell::update(float dt) {
	_velocity += -_velocityDecay * _velocity * dt;

	_rotationVelocity += -_velocityDecay * _rotationVelocity * dt;

	_position += _velocity * dt;

	_rotation += _rotationVelocity * dt;

	_lifeTimer += dt;

	if (_lifeTimer > _lifespan)
		remove();
}

void Shell::render(sf::RenderTarget &rt) {
	sf::Sprite s;

	s.setTexture(*_shellTexture);

	s.setOrigin(_shellTexture->getSize().x * 0.5f, _shellTexture->getSize().y * 0.5f);

	s.setPosition(_position);

	s.setRotation(_rotation);

	rt.draw(s);
}

sf::FloatRect Shell::getAABB() const {
	const float radius = 0.5f;

	return ltbl::rectFromBounds(_position - sf::Vector2f(radius, radius), _position + sf::Vector2f(radius, radius));
}
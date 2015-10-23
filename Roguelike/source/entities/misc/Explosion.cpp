#include "Explosion.h"

#include <game/Game.h>

void Explosion::create(const sf::Vector2f &initPosition, float initRotation) {
	if (!getGame()->_resources.exists("explosion1")) {
		Ptr<sf::Texture> explosionTexture = std::make_shared<sf::Texture>();

		explosionTexture->loadFromFile("resources/misc/explosion1.png");

		_explosionTexture = explosionTexture;

		getGame()->_resources.insertPtr("explosion1", explosionTexture);
	}
	else
		_explosionTexture = getGame()->_resources.getPtr<sf::Texture>("explosion1");

	_position = initPosition;

	_rotation = initRotation;

	_lifespan = 1.0f;
}

// Inherited from Entity
void Explosion::update(float dt) {
	_lifeTimer += dt;

	if (_lifeTimer > _lifespan)
		remove();
}

void Explosion::render(sf::RenderTarget &rt) {
	const float sizeInv = 0.25f;

	int frame = std::floor(_lifeTimer / _lifespan * 14.0f);

	int x = frame % 4;
	int y = frame / 4;

	sf::Sprite s;

	s.setTexture(*_explosionTexture);

	s.setTextureRect(sf::IntRect(_explosionTexture->getSize().x * sizeInv * x, _explosionTexture->getSize().x * sizeInv * y, _explosionTexture->getSize().x * sizeInv, _explosionTexture->getSize().x * sizeInv));

	s.setOrigin(_explosionTexture->getSize().x * sizeInv * 0.5f, _explosionTexture->getSize().y * sizeInv * 0.5f);

	s.setColor(sf::Color(255, 255, 255, 255));

	s.setPosition(_position);

	s.setRotation(_rotation);

	s.setScale(1.0f, 1.0f);

	rt.draw(s);
}

sf::FloatRect Explosion::getAABB() const {
	const float radius = 0.5f;

	return ltbl::rectFromBounds(_position - sf::Vector2f(radius, radius), _position + sf::Vector2f(radius, radius));
}
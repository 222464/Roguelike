#include "Splat.h"

#include <game/Game.h>

void Splat::create(const sf::Vector2f &initPosition, float initRotation, float lifespan) {
	if (!getGame()->_resources.exists("splat1")) {
		Ptr<sf::Texture> SplatTexture = std::make_shared<sf::Texture>();

		SplatTexture->loadFromFile("resources/misc/bloodSplat1.png");

		_splatTexture = SplatTexture;

		getGame()->_resources.insertPtr("splat1", SplatTexture);
	}
	else
		_splatTexture = getGame()->_resources.getPtr<sf::Texture>("splat1");

	_position = initPosition;

	_rotation = initRotation;

	_lifespan = lifespan;
}

// Inherited from Entity
void Splat::update(float dt) {
	_lifeTimer += dt;

	if (_lifeTimer > _lifespan)
		remove();
}

void Splat::render(sf::RenderTarget &rt) {
	sf::Sprite s;

	s.setTexture(*_splatTexture);

	s.setOrigin(_splatTexture->getSize().x * 0.5f, _splatTexture->getSize().y * 0.5f);

	const float fadeStart = 0.75f * _lifespan;

	float alpha = 1.0f - std::min(1.0f, std::max(0.0f, _lifeTimer - fadeStart) / (_lifespan - fadeStart));

	s.setColor(sf::Color(255, 255, 255, 255.0f * alpha));

	s.setPosition(_position);

	s.setRotation(_rotation);

	rt.draw(s);
}

sf::FloatRect Splat::getAABB() const {
	const float radius = 0.5f;

	return ltbl::rectFromBounds(_position - sf::Vector2f(radius, radius), _position + sf::Vector2f(radius, radius));
}
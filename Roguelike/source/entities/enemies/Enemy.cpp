#include "Enemy.h"

void Enemy::create() {
	_testTexture.loadFromFile("resources/enemies/targetDummy.png");
}

void Enemy::setPosition(const sf::Vector2f &position) {
	_position = position;
}

// Inherited from Entity
void Enemy::update(float dt) {

}

void Enemy::render(sf::RenderTarget &rt) {
	sf::Sprite s;

	s.setTexture(_testTexture);

	s.setOrigin(_testTexture.getSize().x * 0.5f, _testTexture.getSize().y * 0.5f);

	s.setPosition(_position);

	rt.draw(s);
}

sf::FloatRect Enemy::getAABB() const {
	const float radius = 2.0f;

	return ltbl::rectFromBounds(_position - sf::Vector2f(radius, radius), _position + sf::Vector2f(radius, radius));
}
#include "Room.h"

#include "../Level.h"
#include "../../Game.h"

Room::Room()
	: _pLevel(nullptr), _numSubSteps(8), _portalSize(32.0f), _transitDistanceRange(16.0f), _wallRange(16.0f)
{}

Level* Room::getLevel() const {
	return _pLevel;
}

Game* Room::getGame() const {
	return getLevel()->getGame();
}

void Room::create(Level* pLevel, int cellX, int cellY, int width, int height, const std::vector<int> &cellIndices, int background) {
	_pLevel = pLevel;
	_background = background;

	_cellX = cellX;
	_cellY = cellY;

	_quadtree.create(sf::FloatRect(0.0f, 0.0f, _width, _height));

	_width = width;
	_height = height;
}

void Room::update(float dt) {
	std::vector<std::shared_ptr<Entity>> died;
	std::vector<std::shared_ptr<Entity>> newEntities;
	std::vector<std::shared_ptr<Entity>> transit;

	for (int i = 0; i < _entities.size(); i++) {
		_entities[i]->removeDeadReferences();
		_entities[i]->update(dt);

		if (!_entities[i]->inTransit())
			_entities[i]->_timeSinceTransit += dt;
	}

	// Substeps
	for (int s = 0; s < _numSubSteps; s++)
		for (int i = 0; i < _entities.size(); i++) {
			if (!_entities[i]->_remove && !_entities[i]->inTransit())
				_entities[i]->subUpdate(dt, s, _numSubSteps);
		}

	for (int i = 0; i < _entities.size(); i++) {
		if (!_entities[i]->_remove && !_entities[i]->inTransit())
			newEntities.push_back(_entities[i]);
		else if (_entities[i]->inTransit()) {
			_entities[i]->removeDeadReferences();

			transit.push_back(_entities[i]);

			_entities[i]->quadtreeRemove();
		}
		else {
			_entities[i]->removeDeadReferences();

			died.push_back(_entities[i]);
		}
	}

	for (int i = 0; i < died.size(); i++)
		died[i]->quadtreeRemove();

	std::sort(newEntities.begin(), newEntities.end(), Entity::compareLayers);

	_entities = newEntities;

	// Add transit to other room
	for (int i = 0; i < transit.size(); i++) {
		switch (transit[i]->getTransit()) {
		case 0:
			getLevel()->getCell(getCellX() + 1, getCellY())._room->addToTransitLimbo(transit[i]);
			break;
		case 1:
			getLevel()->getCell(getCellX(), getCellY() + 1)._room->addToTransitLimbo(transit[i]);
			break;
		case 2:
			getLevel()->getCell(getCellX() - 1, getCellY())._room->addToTransitLimbo(transit[i]);
			break;
		case 3:
			getLevel()->getCell(getCellX(), getCellY() - 1)._room->addToTransitLimbo(transit[i]);
			break;
		}
	}

	// Add new entities
	for (int i = 0; i < _addedEntitiesBuffer.size(); i++) {
		_entities.push_back(_addedEntitiesBuffer[i]);

		if (_addedEntitiesQuadtreeStatusBuffer[i])
			_quadtree.add(_entities[i].get());
	}

	_addedEntitiesBuffer.clear();
	_addedEntitiesQuadtreeStatusBuffer.clear();

	for (int i = 0; i < transit.size(); i++)
		transit[i]->_transit = -1;

	// Add limbo entries
	for (int i = 0; i < _transitLimbo.size();) {
		_transitLimbo[i]->_timeSinceTransit += dt;

		if (_transitLimbo[i]->_timeSinceTransit >= 0.0f) {
			_entities.push_back(_transitLimbo[i]);

			if (_transitLimboEntitiesQuadtreeStatusBuffer[i])
				_quadtree.add(_transitLimbo[i].get());

			// Erase
			_transitLimbo.erase(_transitLimbo.begin() + i);
			_transitLimboEntitiesQuadtreeStatusBuffer.erase(_transitLimboEntitiesQuadtreeStatusBuffer.begin() + i);
		}
		else
			i++;
	}

	// Remove dead selections
	for (std::unordered_set<Entity*>::iterator it = getGame()->_selection.begin(); it != getGame()->_selection.end();) {
		if ((*it)->removed() || (*it)->inTransit())
			it = getGame()->_selection.erase(it);
		else
			it++;
	}
}

void Room::render(sf::RenderTarget &rt,
	const std::vector<std::shared_ptr<sf::Texture>> &backgrounds,
	const std::vector<std::shared_ptr<sf::Texture>> &doors,
	const std::vector<std::shared_ptr<sf::Texture>> &roofs)
{
	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(*backgrounds[_background]);

	rt.draw(backgroundSprite);

	if (getCellX() > 0)
		if (getLevel()->getCell(getCellX() - 1, getCellY())._room != nullptr) {
			sf::Sprite doorSprite;

			doorSprite.setTexture(*doors[_background]);

			doorSprite.setOrigin(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setPosition(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setRotation(0.0f);

			rt.draw(doorSprite);
		}

	if (getCellX() < getLevel()->getWidth() - 1)
		if (getLevel()->getCell(getCellX() + 1, getCellY())._room != nullptr) {
			sf::Sprite doorSprite;

			doorSprite.setTexture(*doors[_background]);

			doorSprite.setOrigin(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setPosition(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setRotation(180.0f);

			rt.draw(doorSprite);
		}

	if (getCellY() > 0)
		if (getLevel()->getCell(getCellX(), getCellY() - 1)._room != nullptr) {
			sf::Sprite doorSprite;

			doorSprite.setTexture(*doors[_background]);

			doorSprite.setOrigin(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setPosition(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setRotation(90.0f);

			rt.draw(doorSprite);
		}

	if (getCellY() < getLevel()->getHeight() - 1)
		if (getLevel()->getCell(getCellX(), getCellY() + 1)._room != nullptr) {
			sf::Sprite doorSprite;

			doorSprite.setTexture(*doors[_background]);

			doorSprite.setOrigin(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setPosition(backgrounds[_background]->getSize().x * 0.5f, backgrounds[_background]->getSize().y * 0.5f);
			doorSprite.setRotation(270.0f);

			rt.draw(doorSprite);
		}

	for (int i = 0; i < _entities.size(); i++)
		_entities[i]->preRender(rt);

	for (int i = 0; i < _entities.size(); i++)
		_entities[i]->render(rt);

	// Roof
	sf::Sprite roofSprite;
	roofSprite.setTexture(*roofs[_background]);

	rt.draw(roofSprite);
}

void Room::add(const std::shared_ptr<Entity> &entity, bool addToQuadtree) {
	_addedEntitiesBuffer.push_back(entity);

	_addedEntitiesQuadtreeStatusBuffer.push_back(addToQuadtree);

	entity->_pCurrentRoom = this;
}

void Room::addToTransitLimbo(const std::shared_ptr<Entity> &entity, bool addToQuadtree) {
	_transitLimbo.push_back(entity);

	_transitLimboEntitiesQuadtreeStatusBuffer.push_back(addToQuadtree);

	entity->_pCurrentRoom = this;
}

int Room::getPortal(const sf::FloatRect &aabb) {
	sf::Vector2f center(_width * 0.5f, _height * 0.5f);

	sf::FloatRect horizontal(0.0f, 0.0f, _portalSize * 2.0f, _portalSize);
	sf::FloatRect vertical(0.0f, 0.0f, _portalSize, _portalSize * 2.0f);

	if (getCellX() > 0 && getLevel()->getCell(getCellX() - 1, getCellY())._room != nullptr)
		if (ltbl::rectIntersects(aabb, ltbl::rectRecenter(horizontal, sf::Vector2f(0.0f, center.y))))
			return 2;

	if (getCellX() < getLevel()->getWidth() - 1 && getLevel()->getCell(getCellX() + 1, getCellY())._room != nullptr)
		if (ltbl::rectIntersects(aabb, ltbl::rectRecenter(horizontal, sf::Vector2f(_width, center.y))))
			return 0;

	if (getCellY() > 0 && getLevel()->getCell(getCellX(), getCellY() - 1)._room != nullptr)
		if (ltbl::rectIntersects(aabb, ltbl::rectRecenter(vertical, sf::Vector2f(center.x, 0.0f))))
			return 3;

	if (getCellY() < getLevel()->getHeight() - 1 && getLevel()->getCell(getCellX(), getCellY() + 1)._room != nullptr)
		if (ltbl::rectIntersects(aabb, ltbl::rectRecenter(vertical, sf::Vector2f(center.x, _height))))
			return 1;

	return -1;
}

bool Room::wallCollision(sf::FloatRect &aabb) {
	// Ignore collision on portals
	if (getPortal(aabb) != -1)
		return false;

	if (aabb.left < _wallRange) {
		aabb.left = _wallRange;

		return true;
	}

	if (aabb.top < _wallRange) {
		aabb.top = _wallRange;

		return true;
	}

	if (aabb.left + aabb.width > getWidth() - _wallRange) {
		aabb.left = getWidth() - _wallRange - aabb.width;

		return true;
	}

	if (aabb.top + aabb.height > getHeight() - _wallRange) {
		aabb.top = getHeight() - _wallRange - aabb.height;

		return true;
	}

	return false;
}
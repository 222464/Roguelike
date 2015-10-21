#include "Room.h"

#include "../Level.h"
#include "../../Game.h"

Level* Room::getLevel() const {
	return _pLevel;
}

Game* Room::getGame() const {
	return getLevel()->getGame();
}

void Room::create(Level* pLevel, const std::vector<int> &cellIndices, int background) {
	_pLevel = pLevel;
	_background = background;

	_quadtree.create(sf::FloatRect(0.0f, 0.0f, 512.0f, 512.0f));
}

void Room::update(float dt) {
	std::vector<std::shared_ptr<Entity>> died;
	std::vector<std::shared_ptr<Entity>> newEntities;

	for (int i = 0; i < _entities.size(); i++) {
		_entities[i]->removeDeadReferences();
		_entities[i]->update(dt);
	}

	// Substeps
	for (int s = 0; s < _numSubSteps; s++)
		for (int i = 0; i < _entities.size(); i++) {
			if (!_entities[i]->_remove)
				_entities[i]->subUpdate(dt, s, _numSubSteps);
		}

	for (int i = 0; i < _entities.size(); i++) {
		if (!_entities[i]->_remove)
			newEntities.push_back(_entities[i]);
		else
			died.push_back(_entities[i]);
	}

	for (int i = 0; i < died.size(); i++)
		died[i]->quadtreeRemove();

	std::sort(newEntities.begin(), newEntities.end(), Entity::compareLayers);

	_entities = newEntities;

	// Add new entities
	for (int i = 0; i < _addedEntitiesBuffer.size(); i++) {
		_entities.push_back(_addedEntitiesBuffer[i]);

		if (_addedEntitiesQuadtreeStatusBuffer[i])
			_quadtree.add(_entities[i].get());
	}

	_addedEntitiesBuffer.clear();
	_addedEntitiesQuadtreeStatusBuffer.clear();

	// Remove dead selections
	for (std::unordered_set<Entity*>::iterator it = getGame()->_selection.begin(); it != getGame()->_selection.end();) {
		if ((*it)->removed())
			it = getGame()->_selection.erase(it);
		else
			it++;
	}
}

void Room::render(sf::RenderTarget &rt, const std::vector<std::shared_ptr<sf::Texture>> &backgrounds) {
	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(*backgrounds[_background]);

	rt.draw(backgroundSprite);

	for (int i = 0; i < _entities.size(); i++)
		_entities[i]->preRender(rt);

	for (int i = 0; i < _entities.size(); i++)
		_entities[i]->render(rt);
}

void Room::add(const std::shared_ptr<Entity> &entity, bool addToQuadtree) {
	_addedEntitiesBuffer.push_back(entity);

	_addedEntitiesQuadtreeStatusBuffer.push_back(addToQuadtree);

	entity->_pCurrentRoom = this;
}
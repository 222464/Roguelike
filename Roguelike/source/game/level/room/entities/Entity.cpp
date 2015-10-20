#include "Entity.h"

#include "../Room.h"
#include "../../Level.h"
#include "../../../Game.h"

Room* Entity::getRoom() const {
	return _pCurrentRoom;
}

Level* Entity::getLevel() const {
	return getRoom()->getLevel();
}

Game* Entity::getGame() const {
	return getRoom()->getLevel()->getGame();
}
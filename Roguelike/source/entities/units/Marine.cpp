#include "Marine.h"

#include "../misc/Shell.h"
#include "../misc/Splat.h"

#include "../enemies/Enemy.h"

#include <game/Game.h>

const float Marine::_radius = 4.0f;

Marine::Stats::Stats()
	: _spinRate(720.0f), _idleSpinRate(60.0f),
	_walkRate(30.0f),
	_range(64.0f),
	_splitRadius(32.0f),
	_damage(20.0f)
{}

void Marine::Assets::load() {
	_marineBodyWalk.loadFromFile("resources/units/marine/marine_body_walk.png");
	_marineBodyNoFire.loadFromFile("resources/units/marine/marine_body_nofire.png");

	_marineBodyFlashes.resize(3);

	_marineBodyFlashes[0].loadFromFile("resources/units/marine/marine_body_fire_flash1.png");
	_marineBodyFlashes[1].loadFromFile("resources/units/marine/marine_body_fire_flash2.png");
	_marineBodyFlashes[2].loadFromFile("resources/units/marine/marine_body_fire_flash3.png");

	_marineFoot.loadFromFile("resources/units/marine/marine_foot.png");

	_marineShadow.loadFromFile("resources/units/marine/marine_shadow.png");

	_marineSelected.loadFromFile("resources/units/marine/marine_selected.png");

	_shootSound.loadFromFile("resources/sounds/weapons/mg.wav");
}

Marine::Marine()
	: _prevPosition(0.0f, 0.0f), _position(0.0f, 0.0f), _reactedPosition(0.0f, 0.0f), _pushedOverVel(false), _target(0.0f, 0.0f), _pTarget(nullptr), _hold(false),
	_rotation(0.0f), _attack(false), _footCycle(0.0f), _footCycleRate(0.09f),
	_firingCycle(0.0f), _firingCycleRate(10.0f), _currentFlash(0), _fireSoundTimer(0.0f), _maxFireSoundTime(0.05f),
	_isSelected(false), _idleFaceDirection(0.0f), _idleFaceTime(5.0f), _lastFacedDirection(0.0f), _wantsTransit(false),
	_hit(false), _hitWall(false), _stuckTimer(0.0f), _stuckTime(0.5f), _stuckTime_transit(8.0f), _stuckVelocity(0.125f)
{
	_name = "marine";
	_type = 1;

	_idleFaceTimer = _idleFaceTime;
}

void Marine::create() {
	if (!getGame()->_resources.exists("marine_assets")) {
		Ptr<Assets> assets = std::make_shared<Assets>();

		assets->load();

		_assets = assets;

		getGame()->_resources.insertPtr("marine_assets", assets);
	}
	else
		_assets = getGame()->_resources.getPtr<Assets>("marine_assets");

	if (!getGame()->_resources.exists("marine_stats")) {
		Ptr<Stats> stats = std::make_shared<Stats>();

		_stats = stats;

		getGame()->_resources.insertPtr("marine_stats", stats);
	}
	else
		_stats = getGame()->_resources.getPtr<Stats>("marine_stats");

	_firingSound.setBuffer(_assets->_shootSound);

	_firingSound.setVolume(10);

	// Give it a slight layer offset
	std::uniform_real_distribution<float> layerDist(0.01f, 0.015f);

	_layer = layerDist(getGame()->_generator);
}

void Marine::setPosition(const sf::Vector2f &position) {
	_position = position;
	_reactedPosition = position;
}

void Marine::setRotation(float rotation) {
	_rotation = rotation;
}

void Marine::move(sf::Vector2f &position) {
	_attack = false;
	_target = position;
	_pTarget = nullptr;
	_hold = false;
	_wantsTransit = false;
	_stuckTimer = 0.0f;
}

void Marine::attackMove(const sf::Vector2f &position) {
	_attack = true;
	_target = position;
	_hold = false;
	_wantsTransit = false;
	_stuckTimer = 0.0f;
}

void Marine::transitMove(sf::Vector2f &position) {
	_attack = false;
	_target = position;
	_pTarget = nullptr;
	_hold = false;
	_wantsTransit = true;
	_stuckTimer = 0.0f;
}

void Marine::split() {
	_attack = false;
	_target = _position;
	_stuckTimer = 0.0f;

	// Accumulate pushes
	std::vector<QuadtreeOccupant*> occupants;

	sf::FloatRect rangeRect = ltbl::rectFromBounds(_position - sf::Vector2f(_stats->_splitRadius, _stats->_splitRadius), _position + sf::Vector2f(_stats->_splitRadius, _stats->_splitRadius));

	getRoom()->getQuadtree().queryRegion(occupants, rangeRect);

	sf::Vector2f moveDir(0.0f, 0.0f);

	for (int i = 0; i < occupants.size(); i++) {
		Entity* pEntity = static_cast<Entity*>(occupants[i]);

		if (pEntity != this && (pEntity->_type == 1 || pEntity->_type == 2)) {
			sf::Vector2f dir = _position - ltbl::rectCenter(pEntity->getAABB());

			float dist = ltbl::vectorMagnitude(dir);

			if (dist < _stats->_splitRadius) {
				moveDir += ltbl::vectorNormalize(dir) * (_stats->_splitRadius - dist) / _stats->_splitRadius;
			}
		}
	}

	if (moveDir != sf::Vector2f(0.0f, 0.0f))
		_target += ltbl::vectorNormalize(moveDir) * _stats->_walkRate * 0.017f * 1.5f;

	_pTarget = nullptr;
	_hold = false;
}

void Marine::face(float &angle, float rate, float dt) {
	if (angle < 0.0f)
		angle += 360.0f;

	float roundabout = std::min(std::abs((360.0f - _rotation) + angle), std::abs((360.0f - angle) + _rotation));
	float direct = std::abs(angle - _rotation);

	float directDir = (angle - _rotation) > 0.0f ? 1.0f : -1.0f;
	float rotateDir = direct < roundabout ? directDir : -directDir;

	float minAngle = std::min(roundabout, direct);

	if (minAngle <= rate * dt) {
		_rotation = angle;
	}
	else {
		if (rotateDir < 0.0f)
			_rotation += -rate * dt;
		else
			_rotation += rate * dt;

		if (_rotation < 0.0f)
			_rotation += 360.0f;
	}

	angle = std::fmod(angle, 360.0f);
	_rotation = std::fmod(_rotation, 360.0f);

	_lastFacedDirection = _rotation;
}

void Marine::update(float dt) {
	_isSelected = getGame()->_selection.find(this) != getGame()->_selection.end();

	int portal = getRoom()->getPortal(getAABB());

	// Get collision detected (reacted) position
	_prevPosition = _reactedPosition;
	_reactedPosition = _position;
	
	if (ltbl::vectorMagnitude(_target - _reactedPosition) < _stats->_walkRate * dt) {
		_attack = true;
		_target = _reactedPosition;
		_stuckTimer = 0.0f;

		// Settle feet
		_footCycle += (0.0f - _footCycle) * 1.2f * dt;
	}
	else if (!_hold && _pTarget == nullptr) {
		sf::Vector2f dir = ltbl::vectorNormalize(_target - _position);

		_position += dir * _stats->_walkRate * dt;

		// Rotate to target rotation
		float angle = std::atan2(dir.y, dir.x) * 180.0f / ltbl::_pi + 90.0f;
		
		face(angle, dt, _stats->_spinRate);

		_footCycle = std::fmod(_footCycle + _stats->_walkRate * _footCycleRate * dt, 1.0f);

		// If should transit and in transit zone, walk off screen
		if (_wantsTransit) {
			_attack = false;

			// If not contained by portal
			int portalContains = getRoom()->getPortalContains(getAABB());

			if (portalContains == -1 || ltbl::vectorMagnitude(_reactedPosition - _prevPosition) < 0.5f * _stats->_walkRate * dt || _hitWall) {
				// Guide to portal, since we are stuck but on a portal tile
				switch (portalContains) {
				case 0:
					_target = sf::Vector2f(getRoom()->getWidth() - getRoom()->_wallRange, getRoom()->getHeight() * 0.5f);
					break;

				case 1:
					_target = sf::Vector2f(getRoom()->getWidth() * 0.5f, getRoom()->getHeight() - getRoom()->_wallRange);
					break;

				case 2:
					_target = sf::Vector2f(getRoom()->_wallRange, getRoom()->getHeight() * 0.5f);
					break;

				case 3:
					_target = sf::Vector2f(getRoom()->getWidth() * 0.5f, getRoom()->_wallRange);
					break;
				}
			}
			else { // If not stuck, head down portal
				if (portal != -1) {
					switch (portal) {
					case 0:
						_target = _position + sf::Vector2f(getRoom()->getWidth() * 0.5f, 0.0f);
						break;
					case 1:
						_target = _position + sf::Vector2f(0.0f, getRoom()->getHeight() * 0.5f);
						break;
					case 2:
						_target = _position + sf::Vector2f(-getRoom()->getWidth() * 0.5f, 0.0f);
						break;
					case 3:
						_target = _position + sf::Vector2f(0.0f, -getRoom()->getHeight() * 0.5f);
						break;
					}
				}
			}

			// If off screen enough, perform transit
			if (_position.x < -_radius * 2.0f ||
				_position.x > getRoom()->getWidth() + _radius * 2.0f ||
				_position.y < -_radius * 2.0f ||
				_position.y > getRoom()->getHeight() + _radius * 2.0f)
			{
				//float offset;

				switch (portal) {
				case 0:
					//offset = _position.x - getRoom()->getWidth();

					setPosition(sf::Vector2f(-_radius * 2.0f, _position.y));
					
					break;
				case 3:
					//offset = _position.y - getRoom()->getHeight();

					setPosition(sf::Vector2f(_position.x, getRoom()->getHeight() + _radius * 2.0f));

					break;
				case 2:
					//offset = -_position.x;

					setPosition(sf::Vector2f(getRoom()->getWidth() + _radius * 2.0f, _position.y));

					break;
				case 1:
					//offset = -_position.y;

					setPosition(sf::Vector2f(_position.x, -_radius * 2.0f));

					break;
				}

				_pTarget = nullptr;

				_wantsTransit = false;

				// Remove self from selection if in it
				std::unordered_set<Entity*>::iterator it = getGame()->_selection.find(this);

				if (it != getGame()->_selection.end())
					getGame()->_selection.erase(it);

				transit(portal);
			}
		}
		else {
			// Walk on screen if offscreen
			if (_position.x < _radius * 2.0f)
				_target = _position + sf::Vector2f(getRoom()->_wallRange + getRoom()->_portalSize, 0.0f);
			if (_position.y < _radius * 2.0f)
				_target = _position + sf::Vector2f(0.0f, getRoom()->_wallRange + getRoom()->_portalSize);
			if (_position.x > getRoom()->getWidth() - _radius * 2.0f)
				_target = _position + sf::Vector2f(-getRoom()->_wallRange - getRoom()->_portalSize, 0.0f);
			if (_position.y > getRoom()->getHeight() - _radius * 2.0f)
				_target = _position + sf::Vector2f(0.0f, -getRoom()->_wallRange - getRoom()->_portalSize);
		}
	}

	if (_attack && _pTarget == nullptr) {
		// Search for enemy in range
		std::vector<QuadtreeOccupant*> occupants;

		sf::FloatRect rangeRect = ltbl::rectFromBounds(_position - sf::Vector2f(_stats->_range, _stats->_range), _position + sf::Vector2f(_stats->_range, _stats->_range));

		getRoom()->getQuadtree().queryRegion(occupants, rangeRect);

		float minDist = 999999.0f;

		for (int i = 0; i < occupants.size(); i++) {
			Entity* pEntity = static_cast<Entity*>(occupants[i]);

			if (pEntity->_type == 2) {
				Enemy* pEnemy = static_cast<Enemy*>(occupants[i]);

				float dist = ltbl::vectorMagnitude(pEnemy->getPosition() - _position);

				if (dist < minDist) {
					minDist = dist;

					_pTarget = pEnemy;
				}
			}
		}

		if (minDist > _stats->_range)
			_pTarget = nullptr;
	}

	if (_pTarget != nullptr) {
		float dist = ltbl::vectorMagnitude(_pTarget->getPosition() - _position);

		if (dist > _stats->_range)
			_pTarget = nullptr;
		else {
			sf::Vector2f dir = ltbl::vectorNormalize(_pTarget->getPosition() - _position);

			float angle = std::atan2(dir.y, dir.x) * 180.0f / ltbl::_pi + 90.0f;

			face(angle, dt, _stats->_spinRate);

			// Play firing animation
			if (_firingCycle >= 1.0f) {
				_firingCycle = 0.0f;

				// Spawn shell
				std::shared_ptr<Shell> shell = std::make_shared<Shell>();

				sf::Vector2f perpendicular(std::cos(_rotation * ltbl::_pi / 180.0f), std::sin(_rotation * ltbl::_pi / 180.0f));
				sf::Vector2f forward(perpendicular.y, -perpendicular.x);

				std::uniform_real_distribution<float> positionNoise(-15.0f, 15.0f);
				std::uniform_real_distribution<float> rotationNoise(-10.0f, 10.0f);
				
				getRoom()->add(shell, false);

				shell->create(perpendicular * 20.0f + sf::Vector2f(positionNoise(getGame()->_generator), positionNoise(getGame()->_generator)), _position + perpendicular * 2.0f, rotationNoise(getGame()->_generator), _rotation, 1.0f, 2.0f);

				_currentFlash = (_currentFlash + 1) % _assets->_marineBodyFlashes.size();
			}
			else
				_firingCycle += _firingCycleRate * dt;

			// Play firing sound
			if (_firingSound.getStatus() != sf::Sound::Playing) {
				if (_fireSoundTimer <= 0.0f) {	
					// Randomize pitch and volume a bit
					std::uniform_real_distribution<float> pitchDist(0.8f, 1.0f);
					std::uniform_real_distribution<float> volumeDist(10.0f, 20.0f);

					_firingSound.setPitch(pitchDist(getGame()->_generator));
					_firingSound.setVolume(volumeDist(getGame()->_generator));

					_firingSound.play();
				}
				else
					_fireSoundTimer -= dt;
			}
			else {
				std::uniform_real_distribution<float> fireSoundDist(0.0f, _maxFireSoundTime);

				_fireSoundTimer = fireSoundDist(getGame()->_generator);
			}

			// Damage target
			_pTarget->_hp -= _stats->_damage * dt;
		}
	}
	else {
		_firingCycle = 0.0f;
	}

	float vel = ltbl::vectorMagnitude(_reactedPosition - _prevPosition);

	if (!_hitWall && (!_wantsTransit || portal != -1) && (_pushedOverVel || vel < _stuckVelocity * _stats->_walkRate * dt)) {
		if (_stuckTimer < (_wantsTransit ? _stuckTime_transit : _stuckTime)) {
			_stuckTimer += dt;
		}
		else {
			_attack = true;
			_target = _reactedPosition;
			_stuckTimer = 0.0f;
		}

		// Settle feet
		_footCycle += (0.0f - _footCycle) * 1.2f * dt;
	}

	// If not walking
	if (std::abs(_footCycle) < 0.02f) {
		if (_idleFaceTimer < 0.0f) {
			std::uniform_real_distribution<float> newTimeDist(_idleFaceTime * 0.5f, _idleFaceTime);

			_idleFaceTimer = newTimeDist(getGame()->_generator);

			std::uniform_real_distribution<float> idleFaceDist(-30.0f, 30.0f);

			_idleFaceDirection = _lastFacedDirection + idleFaceDist(getGame()->_generator);
		}
		else
			_idleFaceTimer -= dt;

		face(_idleFaceDirection, dt, _stats->_idleSpinRate);
	}
	else {
		_idleFaceTimer = _idleFaceTime;

		_idleFaceDirection = _rotation;
	}

	// Death
	if (_hp <= 0.0f) {
		remove();

		// Spawn splat
		std::shared_ptr<Splat> splat = std::make_shared<Splat>();

		std::uniform_real_distribution<float> rotationNoise(0.0f, 360.0f);

		getRoom()->add(splat, false);

		splat->create(_position, _rotation + rotationNoise(getGame()->_generator), 30.0f);
	}

	_hit = false;
	_hitWall = false;
	_pushedOverVel = false;

	quadtreeUpdate();
}

void Marine::subUpdate(float dt, int subStep, int numSubSteps) {
	float numSubstepsInv = 1.0f / numSubSteps;

	// Walls
	sf::FloatRect newAABB = getAABB();

	if (getRoom()->wallCollision(newAABB)) {
		setPosition(ltbl::rectCenter(newAABB));
		_hitWall = true;
	}

	// Accumulate pushes
	std::vector<QuadtreeOccupant*> occupants;

	sf::FloatRect rangeRect = ltbl::rectFromBounds(_position - sf::Vector2f(_radius, _radius), _position + sf::Vector2f(_radius, _radius));

	getRoom()->getQuadtree().queryRegion(occupants, rangeRect);

	sf::Vector2f moveDir(0.0f, 0.0f);

	for (int i = 0; i < occupants.size(); i++) {
		Entity* pEntity = static_cast<Entity*>(occupants[i]);

		if (pEntity != this && (pEntity->_type == 1 || pEntity->_type == 2)) {
			float entityRadius = std::max(pEntity->getAABB().width, pEntity->getAABB().height);

			sf::Vector2f dir = _position - ltbl::rectCenter(pEntity->getAABB());

			float dist = ltbl::vectorMagnitude(dir);

			if (dist < entityRadius + _radius) {
				moveDir += ltbl::vectorNormalize(dir) * numSubstepsInv;

				_hit = true;
			}
		}
	}

	_position += moveDir;

	_pushedOverVel = ltbl::vectorMagnitude(_reactedPosition - _position) > (1.0f + (1.0f - _stuckVelocity)) * ltbl::vectorMagnitude(_reactedPosition - _prevPosition);
}

void Marine::preRender(sf::RenderTarget &rt) {
	if (_isSelected) {
		sf::Sprite selectedSprite;
		selectedSprite.setTexture(_assets->_marineSelected);

		selectedSprite.setOrigin(_assets->_marineSelected.getSize().x * 0.5f, _assets->_marineSelected.getSize().y * 0.5f);

		selectedSprite.setPosition(_position);

		rt.draw(selectedSprite);
	}
}

void Marine::render(sf::RenderTarget &rt) {
	sf::Sprite shadowSprite;
	shadowSprite.setTexture(_assets->_marineShadow);

	shadowSprite.setOrigin(_assets->_marineShadow.getSize().x * 0.5f, _assets->_marineShadow.getSize().y * 0.5f);

	shadowSprite.setPosition(_position);

	rt.draw(shadowSprite);

	sf::Sprite footSprite;
	footSprite.setTexture(_assets->_marineFoot);

	footSprite.setOrigin(2.0f, 2.5f);

	sf::Vector2f perpendicular(std::cos(_rotation * ltbl::_pi / 180.0f), std::sin(_rotation * ltbl::_pi / 180.0f));
	sf::Vector2f forward(perpendicular.y, -perpendicular.x);

	const float feetSpread = 1.5f;
	const float feetStepDistance = 2.5f;

	float leftFootOffset = std::sin(2.0f * ltbl::_pi * _footCycle);

	footSprite.setRotation(_rotation);

	footSprite.setPosition(_position - perpendicular * feetSpread + forward * feetStepDistance * leftFootOffset - forward * 1.0f);

	rt.draw(footSprite);

	footSprite.setPosition(_position + perpendicular * feetSpread - forward * feetStepDistance * leftFootOffset - forward * 1.0f);

	rt.draw(footSprite);

	sf::Sprite bodySprite;

	if (_pTarget == nullptr) {
		bodySprite.setTexture(_assets->_marineBodyWalk);

		bodySprite.setPosition(_position);
	}
	else {
		std::uniform_real_distribution<float> noiseDist(-0.5f, 0.5f);

		// Show flash for only a bit of the flash cycle time
		if (_firingCycle < 0.3f) {
			bodySprite.setPosition(_position + sf::Vector2f(noiseDist(getGame()->_generator), noiseDist(getGame()->_generator)));
			bodySprite.setTexture(_assets->_marineBodyFlashes[_currentFlash]);
		}
		else {
			bodySprite.setPosition(_position);
			bodySprite.setTexture(_assets->_marineBodyNoFire);
		}
	}

	bodySprite.setOrigin(sf::Vector2f(_assets->_marineBodyWalk.getSize().x * 0.5f, _assets->_marineBodyWalk.getSize().y * 0.5f));

	bodySprite.setRotation(_rotation + leftFootOffset * 8.0f); // Sway a bit

	rt.draw(bodySprite);
}

sf::FloatRect Marine::getAABB() const {
	return ltbl::rectFromBounds(_position - sf::Vector2f(_radius, _radius), _position + sf::Vector2f(_radius, _radius));
}

void Marine::removeDeadReferences() {
	if (_pTarget != nullptr) {
		if (_pTarget->removed())
			_pTarget = nullptr;
	}
}
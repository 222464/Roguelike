#include "EnemySiegeTank.h"

#include "../misc/Shell.h"
#include "../misc/Splat.h"
#include "../misc/Explosion.h"

#include "../units/Friendly.h"

#include <game/Game.h>

const float EnemySiegeTank::_radius = 10.0f;

EnemySiegeTank::Stats::Stats()
	: _spinRate(720.0f), _idleSpinRate(60.0f),
	_walkRate(30.0f),
	_range(128.0f),
	_splitRadius(32.0f),
	_damage(60.0f),
	_splashRadius(48.0f),
	_siegeToggleRate(0.6f)
{}

void EnemySiegeTank::Assets::load() {
	_siegeTankNoSiege.loadFromFile("resources/enemies/siegetank/siegetank_nosiege.png");
	_siegeTankHalfSiege.loadFromFile("resources/enemies/siegetank/siegetank_halfsiege.png");
	_siegeTankSiege.loadFromFile("resources/enemies/siegetank/siegetank_siege.png");

	_siegeTankCannonNoFire.loadFromFile("resources/enemies/siegetank/siegetank_cannon_nofire.png");

	_siegeTankCannons.resize(3);

	_siegeTankCannons[0].loadFromFile("resources/enemies/siegetank/siegetank_cannon1.png");
	_siegeTankCannons[1].loadFromFile("resources/enemies/siegetank/siegetank_cannon2.png");
	_siegeTankCannons[2].loadFromFile("resources/enemies/siegetank/siegetank_cannon3.png");

	_siegeTankSelected.loadFromFile("resources/enemies/siegetank/siegetank_selected.png");
	_siegeTankShadow.loadFromFile("resources/enemies/siegetank/siegetank_shadow.png");

	if (!_shootSound.loadFromFile("resources/sounds/weapons/cannon1.wav"))
		abort();

	if (!_siegeSound.loadFromFile("resources/enemies/siegetank/siegesound.wav"))
		abort();

	if (!_unsiegeSound.loadFromFile("resources/enemies/siegetank/unsiegesound.wav"))
		abort();
}

EnemySiegeTank::EnemySiegeTank()
	: _prevPosition(0.0f, 0.0f), _position(0.0f, 0.0f), _target(0.0f, 0.0f), _pTarget(nullptr), _hold(false),
	_rotation(0.0f), _cannonRotation(0.0f), _attack(false), _footCycle(0.0f), _footCycleRate(0.09f),
	_firingCycle(0.0f), _firingCycleRate(0.5f), _currentFlash(0), _fireSoundTimer(0.0f), _maxFireSoundTime(0.05f),
	_isSelected(false), _idleFaceDirection(0.0f), _idleFaceTime(5.0f), _lastFacedDirection(0.0f), _hitWall(false),
	_siegeDelta(0), _siege(0.0f)
{
	_name = "enemy_siegetank";
	_type = 2;

    _maxhp = _hp = 400;

	_idleFaceTimer = _idleFaceTime;
}

void EnemySiegeTank::create() {
	if (!getGame()->_resources.exists("enemy_siegetank_assets")) {
		Ptr<Assets> assets = std::make_shared<Assets>();

		assets->load();

		_assets = assets;

		getGame()->_resources.insertPtr("enemy_siegetank_assets", assets);
	}
	else
		_assets = getGame()->_resources.getPtr<Assets>("enemy_siegetank_assets");

	if (!getGame()->_resources.exists("enemy_siegetank_stats")) {
		Ptr<Stats> stats = std::make_shared<Stats>();

		_stats = stats;

		getGame()->_resources.insertPtr("enemy_siegetank_stats", stats);
	}
	else
		_stats = getGame()->_resources.getPtr<Stats>("enemy_siegetank_stats");

	_firingSound.setBuffer(_assets->_shootSound);

	_firingSound.setVolume(100);

	_siegeSound.setBuffer(_assets->_siegeSound);

	_siegeSound.setVolume(25);

	_unsiegeSound.setBuffer(_assets->_unsiegeSound);

	_unsiegeSound.setVolume(25);

	// Give it a slight layer offset
	std::uniform_real_distribution<float> layerDist(0.01f, 0.015f);

	_layer = layerDist(getGame()->_generator);
}

void EnemySiegeTank::setPosition(const sf::Vector2f &position) {
	_position = position;
}

void EnemySiegeTank::setRotation(float rotation) {
	_rotation = rotation;
	_cannonRotation = rotation;
}

void EnemySiegeTank::move(sf::Vector2f &position) {
	_attack = false;
	_target = position;
	_pTarget = nullptr;
	_hold = false;
}

void EnemySiegeTank::attackMove(const sf::Vector2f &position) {
	_attack = true;
	_target = position;
	_hold = false;
}

void EnemySiegeTank::split() {
	_attack = false;
	_target = _position;

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

void EnemySiegeTank::face(float &angle, float rate, float dt) {
	if (_siege < 0.5f) {
		angle = std::fmod(angle, 360.0f);
		_rotation = std::fmod(_rotation, 360.0f);

		if (angle < 0.0f)
			angle += 360.0f;

		if (_rotation < 0.0f)
			_rotation += 360.0f;

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
		
		_lastFacedDirection = _rotation;
	}
	else {
		angle = std::fmod(angle, 360.0f);
		_cannonRotation = std::fmod(_cannonRotation, 360.0f);

		if (angle < 0.0f)
			angle += 360.0f;

		if (_cannonRotation < 0.0f)
			_cannonRotation += 360.0f;

		float roundabout = std::min(std::abs((360.0f - _cannonRotation) + angle), std::abs((360.0f - angle) + _cannonRotation));
		float direct = std::abs(angle - _cannonRotation);

		float directDir = (angle - _cannonRotation) > 0.0f ? 1.0f : -1.0f;
		float rotateDir = direct < roundabout ? directDir : -directDir;

		float minAngle = std::min(roundabout, direct);

		if (minAngle <= rate * dt) {
			_cannonRotation = angle;
		}
		else {
			if (rotateDir < 0.0f)
				_cannonRotation += -rate * dt;
			else
				_cannonRotation += rate * dt;

			if (_cannonRotation < 0.0f)
				_cannonRotation += 360.0f;
		}

		_lastFacedDirection = _cannonRotation;
	}
}

void EnemySiegeTank::update(float dt) {
	// Temporary, auto-siege (REMOVE)
	siegeMode(true);

	if (_siege == 1.0f && _siegeDelta == -1) {
		_unsiegeSound.play();
	}
	else if (_siege == 0.0f && _siegeDelta == 1) {
		_siegeSound.play();
	}

	_siege = std::min(1.0f, std::max(0.0f, _siege + _siegeDelta * _stats->_siegeToggleRate * dt));

	_isSelected = getGame()->_selection.find(this) != getGame()->_selection.end();

	if (!_hitWall && ltbl::vectorMagnitude(_target - _position) < 0.5f * _stats->_walkRate * dt) {
		_attack = true;
		_target = _position;

		// Settle feet
		_footCycle += (0.0f - _footCycle) * 1.2f * dt;
	}
	else if (_siege == 0.0f && !_hold && _pTarget == nullptr) {
		sf::Vector2f dir = ltbl::vectorNormalize(_target - _position);

		_position += dir * _stats->_walkRate * dt;

		// Rotate to target rotation
		float angle = std::atan2(dir.y, dir.x) * 180.0f / ltbl::_pi + 90.0f;

		face(angle, dt, _stats->_spinRate);

		_footCycle = std::fmod(_footCycle + _stats->_walkRate * _footCycleRate * dt, 1.0f);

		quadtreeUpdate();
	}

	if (_attack && _pTarget == nullptr && _siege == 1.0f) {
		// Search for enemy in range
		std::vector<QuadtreeOccupant*> occupants;

		sf::FloatRect rangeRect = ltbl::rectFromBounds(_position - sf::Vector2f(_stats->_range, _stats->_range), _position + sf::Vector2f(_stats->_range, _stats->_range));

		getRoom()->getQuadtree().queryRegion(occupants, rangeRect);

		float minDist = 999999.0f;

		for (int i = 0; i < occupants.size(); i++) {
			Entity* pEntity = static_cast<Entity*>(occupants[i]);

			if (pEntity->_type == 1) {
				Friendly* pEnemy = static_cast<Friendly*>(occupants[i]);

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

	if (_siege == 1.0f && _pTarget != nullptr) {
		float dist = ltbl::vectorMagnitude(_pTarget->getPosition() - _position);

		if (dist > _stats->_range)
			_pTarget = nullptr;
		else {
			sf::Vector2f dir = ltbl::vectorNormalize(_pTarget->getPosition() - _position);

			float angle = std::atan2(dir.y, dir.x) * 180.0f / ltbl::_pi + 90.0f;

			face(angle, dt, _stats->_spinRate);

			// Play firing animation
			bool fired = false;

			if (_firingCycle >= 1.0f) {
				_firingCycle = 0.0f;

				fired = true;

				// Randomize pitch and volume a bit
				std::uniform_real_distribution<float> pitchDist(0.8f, 1.0f);
				std::uniform_real_distribution<float> volumeDist(10.0f, 20.0f);

				_firingSound.setPitch(pitchDist(getGame()->_generator));
				_firingSound.setVolume(volumeDist(getGame()->_generator));

				_firingSound.play();

				// Spawn shell
				std::shared_ptr<Shell> shell = std::make_shared<Shell>();

				sf::Vector2f perpendicular(std::cos(_rotation * ltbl::_pi / 180.0f), std::sin(_rotation * ltbl::_pi / 180.0f));
				sf::Vector2f forward(perpendicular.y, -perpendicular.x);

				std::uniform_real_distribution<float> positionNoise(-15.0f, 15.0f);
				std::uniform_real_distribution<float> rotationNoise(-10.0f, 10.0f);

				getRoom()->add(shell, false);

				shell->create(perpendicular * 20.0f + sf::Vector2f(positionNoise(getGame()->_generator), positionNoise(getGame()->_generator)), _position + perpendicular * 2.0f, rotationNoise(getGame()->_generator), _rotation, 1.0f, 2.0f);

				_currentFlash = (_currentFlash + 1) % _assets->_siegeTankCannons.size();
			}
			else
				_firingCycle += _firingCycleRate * dt;

			// Play firing sound
			/*if (_firingSound.getStatus() != sf::Sound::Playing) {
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
			else if (fired) {
				std::uniform_real_distribution<float> fireSoundDist(0.0f, _maxFireSoundTime);

				_fireSoundTimer = fireSoundDist(getGame()->_generator);
			}*/

			if (fired) {
				// Damage targets in radius
				std::vector<QuadtreeOccupant*> occupants;

				sf::FloatRect splashRect = ltbl::rectFromBounds(_pTarget->getPosition() - sf::Vector2f(_stats->_splashRadius, _stats->_splashRadius), _pTarget->getPosition() + sf::Vector2f(_stats->_splashRadius, _stats->_splashRadius));

				getRoom()->getQuadtree().queryRegion(occupants, splashRect);

				for (int i = 0; i < occupants.size(); i++) {
					Entity* pEntity = static_cast<Entity*>(occupants[i]);

					if (pEntity->_type == 1) {
						Friendly* pEnemy = static_cast<Friendly*>(occupants[i]);

						float dist = ltbl::vectorMagnitude(pEnemy->getPosition() - _pTarget->getPosition());

						pEnemy->_hp -= _stats->_damage * std::max(0.0f, (_stats->_splashRadius - dist) / _stats->_splashRadius);
					}
				}

				// Spawn explosion
				std::shared_ptr<Explosion> explosion = std::make_shared<Explosion>();

				getRoom()->add(explosion);

				std::uniform_real_distribution<float> rotationDist(0.0f, 360.0f);

				explosion->create(_pTarget->getPosition(), rotationDist(getGame()->_generator));
			}
		}
	}
	else {
		_firingCycle = 0.0f;
	}

	if (!_hitWall && ltbl::vectorMagnitude(_position - _prevPosition) < 0.25f * _stats->_walkRate * dt) {
		// Stop walking, stuck
		_target = _position;
	}

	if (_pTarget == nullptr)
		_lastFacedDirection = _cannonRotation;

	// If not walking
	if (std::abs(_footCycle) < 0.02f && _pTarget == nullptr) {
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

		if (_siege == 1.0f)
			_idleFaceDirection = _cannonRotation;
		else
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

	_hitWall = false;

	_prevPosition = _position;
}

void EnemySiegeTank::subUpdate(float dt, int subStep, int numSubSteps) {
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

			if (dist < entityRadius + _radius)
				moveDir += ltbl::vectorNormalize(dir) * numSubstepsInv;
		}
	}

	_position += moveDir;
}

void EnemySiegeTank::preRender(sf::RenderTarget &rt) {
	if (_isSelected) {
		sf::Sprite selectedSprite;
		selectedSprite.setTexture(_assets->_siegeTankSelected);

		selectedSprite.setOrigin(_assets->_siegeTankSelected.getSize().x * 0.5f, _assets->_siegeTankSelected.getSize().y * 0.5f);

		selectedSprite.setPosition(_position);

		rt.draw(selectedSprite);
	}
}

void EnemySiegeTank::render(sf::RenderTarget &rt) {
	sf::Sprite shadowSprite;
	shadowSprite.setTexture(_assets->_siegeTankShadow);

	shadowSprite.setOrigin(_assets->_siegeTankShadow.getSize().x * 0.5f, _assets->_siegeTankShadow.getSize().y * 0.5f);

	shadowSprite.setPosition(_position);

	rt.draw(shadowSprite);

	sf::Sprite bodySprite;

	if (_siege < 0.333f)
		bodySprite.setTexture(_assets->_siegeTankNoSiege);
	else if (_siege >= 0.666f)
		bodySprite.setTexture(_assets->_siegeTankSiege);
	else
		bodySprite.setTexture(_assets->_siegeTankHalfSiege);

	bodySprite.setOrigin(bodySprite.getTexture()->getSize().x * 0.5f, bodySprite.getTexture()->getSize().y * 0.5f);

	bodySprite.setPosition(_position);
	bodySprite.setRotation(_rotation);

	rt.draw(bodySprite);

	sf::Sprite cannonSprite;

	// Show flash for only a bit of the flash cycle time
	if (_pTarget != nullptr && _firingCycle < 0.06f) {
		sf::Vector2f perpendicular(std::cos(_cannonRotation * ltbl::_pi / 180.0f), std::sin(_cannonRotation * ltbl::_pi / 180.0f));
		sf::Vector2f forward(perpendicular.y, -perpendicular.x);

		std::uniform_real_distribution<float> noiseDist(-0.5f, 0.5f);

		cannonSprite.setPosition(_position - forward * 2.0f + sf::Vector2f(noiseDist(getGame()->_generator), noiseDist(getGame()->_generator)));
		cannonSprite.setTexture(_assets->_siegeTankCannons[_currentFlash]);
	}
	else {
		cannonSprite.setPosition(_position);
		cannonSprite.setTexture(_assets->_siegeTankCannonNoFire);
	}

	cannonSprite.setOrigin(cannonSprite.getTexture()->getSize().x * 0.5f, cannonSprite.getTexture()->getSize().y * 0.75f);
	cannonSprite.setRotation(_cannonRotation);

	rt.draw(cannonSprite);
}

sf::FloatRect EnemySiegeTank::getAABB() const {

	return ltbl::rectFromBounds(_position - sf::Vector2f(_radius, _radius), _position + sf::Vector2f(_radius, _radius));
}

void EnemySiegeTank::removeDeadReferences() {
	if (_pTarget != nullptr) {
		if (_pTarget->removed())
			_pTarget = nullptr;
	}
}
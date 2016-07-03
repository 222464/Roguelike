#pragma once

#include <game/level/room/entities/Entity.h>
#include "Enemy.h"
#include <SFML/Audio.hpp>


class EnemyGrunt : public Enemy {
public:
	struct Stats {
		float _spinRate;
		float _idleSpinRate;
		float _walkRate;

		float _range;

		float _splitRadius;

		float _damage;

		Stats();
	};

	struct Assets {
		sf::Texture _gruntBodyWalk;
		sf::Texture _gruntBodyNoFire;
		std::vector<sf::Texture> _gruntBodyFlashes;
		sf::Texture _gruntFoot;
		sf::Texture _gruntShadow;
		sf::Texture _gruntSelected;

		sf::SoundBuffer _shootSound;

		void load();
	};

private:
	std::shared_ptr<Assets> _assets;
	std::shared_ptr<Stats> _stats;

	sf::Vector2f _prevPosition;
	sf::Vector2f _position;
	float _rotation;

	sf::Vector2f _target;
	bool _attack;
	bool _hold;

	class Friendly* _pTarget;

	float _footCycle;
	float _footCycleRate;

	float _firingCycle;
	float _firingCycleRate;

	int _currentFlash;

	void face(float &angle, float rate, float dt);

	sf::Sound _firingSound;

	float _fireSoundTimer;
	float _maxFireSoundTime;

	bool _isSelected;

	float _idleFaceDirection;
	float _idleFaceTimer;
	float _idleFaceTime;
	float _lastFacedDirection;

	bool _hitWall;

public:
	static const float _radius;

	EnemyGrunt();

	void create();

	void move(sf::Vector2f &position);

	void stop() {
		_target = _position;
	}

	void attackMove(const sf::Vector2f &position);
	void split();

	void hold() {
		_hold = true;
	}

	void setPosition(const sf::Vector2f &position);

	const sf::Vector2f &getPosition() const {
		return _position;
	}

	void setRotation(float rotation);

	float getRotation() const {
		return _rotation;
	}

	// Inherited from Entity
	void update(float dt);

	void subUpdate(float dt, int subStep, int numSubSteps);

	void render(sf::RenderTarget &rt);
	void preRender(sf::RenderTarget &rt);

	sf::FloatRect getAABB() const;

	void removeDeadReferences();
};
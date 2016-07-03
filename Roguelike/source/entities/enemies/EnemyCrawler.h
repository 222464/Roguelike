#pragma once

#include <game/level/room/entities/Entity.h>
#include "Enemy.h"
#include <SFML/Audio.hpp>


class EnemyCrawler : public Enemy {
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
        sf::Texture _crawlerBodyWalk;
        sf::Texture _crawlerBodyNoFire;
        std::vector<sf::Texture> _crawlerBodyFlashes;
        sf::Texture _crawlerFoot;
        sf::Texture _crawlerShadow;
        sf::Texture _crawlerSelected;

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

    EnemyCrawler();

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
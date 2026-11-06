#pragma once

#include "Config.h"
#include "Entity.h"

#include <array>
#include <functional>
#include <random>

class WaveSpawner {
public:
    void reset();
    void update(float dt, int enemiesAliveCount,
                const std::function<void(EnemyKind, int)>& spawnFn,
                SimEvents& ev);
    int waveIndex() const { return mWave; }
    bool allWavesCleared() const { return mAllWavesCleared; }
    int spawnedThisWave() const { return mSpawnedThisWave; }

private:
    void prepareWave();
    int mWave = 0;
    int mSpawnedThisWave = 0;
    float mSpawnTimer = 0.8f;
    bool mAllWavesCleared = false;
    std::array<EnemyKind, Config::WAVE_COUNTS[2]> mRoster{};
    std::mt19937 mRandom{std::random_device{}()};
};

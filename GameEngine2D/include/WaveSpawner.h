#pragma once

#include "Config.h"
#include "Entity.h"

#include <functional>

class WaveSpawner {
public:
    void reset() {
        mWave = 0;
        mSpawnedThisWave = 0;
        mSpawnTimer = 0.8f;
        mAllWavesCleared = false;
    }
    void update(float dt, int enemiesAliveCount,
                const std::function<void(EnemyKind, int)>& spawnFn,
                SimEvents& ev);
    int waveIndex() const { return mWave; }
    bool allWavesCleared() const { return mAllWavesCleared; }
    int spawnedThisWave() const { return mSpawnedThisWave; }

private:
    EnemyKind rollKind() const;
    int mWave = 0;
    int mSpawnedThisWave = 0;
    float mSpawnTimer = 0.8f;
    bool mAllWavesCleared = false;
    bool mWaveStarted[3] = { false, false, false };
};

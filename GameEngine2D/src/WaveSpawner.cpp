#include "WaveSpawner.h"

EnemyKind WaveSpawner::rollKind() const {
    if (mWave == 0) return EnemyKind::Easy;
    if (mWave == 1) return (rand() % 2) == 0 ? EnemyKind::Easy : EnemyKind::Medium;
    int roll = rand() % 3;
    if (roll == 0) return EnemyKind::Easy;
    if (roll == 1) return EnemyKind::Medium;
    return EnemyKind::Hard;
}

void WaveSpawner::update(float dt, int enemiesAliveCount,
                         const std::function<void(EnemyKind, int)>& spawnFn,
                         SimEvents& ev) {
    if (mAllWavesCleared || !spawnFn) return;

    if (enemiesAliveCount == 0 && mSpawnedThisWave >= Config::WAVE_COUNTS[mWave]) {
        if (mWave >= 2) {
            mAllWavesCleared = true;
            ev.emitSound(Sfx::Win);
            return;
        }
        ++mWave;
        mSpawnedThisWave = 0;
        mSpawnTimer = 0.6f;
        mWaveStarted[mWave] = false;
    }

    bool spawnPhaseDone = mSpawnedThisWave >= Config::WAVE_COUNTS[mWave];
    if (spawnPhaseDone) return;

    if (!mWaveStarted[mWave]) {
        mWaveStarted[mWave] = true;
        ev.emitSound(Sfx::Wave);
    }

    mSpawnTimer -= dt;
    if (mSpawnTimer <= 0.0f) {
        int lane = rand() % Config::LANE_COUNT;
        EnemyKind k = rollKind();
        spawnFn(k, lane);
        ++mSpawnedThisWave;
        mSpawnTimer = Config::WAVE_INTERVALS[mWave];
    }
}

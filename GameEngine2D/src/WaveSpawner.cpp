#include "WaveSpawner.h"
#include <algorithm>

void WaveSpawner::reset() {
    mWave = 0;
    mSpawnedThisWave = 0;
    mSpawnTimer = 0.8f;
    mAllWavesCleared = false;
    prepareWave();
}

void WaveSpawner::prepareWave() {
    const int count = Config::WAVE_COUNTS[mWave];
    std::uniform_int_distribution<int> roll(0, mWave);
    // Reserve one slot per required type, fill the rest, then shuffle the order.
    for (int i = 0; i < count; ++i) {
        mRoster[i] = static_cast<EnemyKind>(i <= mWave ? i : roll(mRandom));
    }
    std::shuffle(mRoster.begin(), mRoster.begin() + count, mRandom);
}

void WaveSpawner::update(float dt, int enemiesAliveCount,
                         const std::function<void(EnemyKind, int)>& spawnFn,
                         SimEvents& ev) {
    if (mAllWavesCleared || !spawnFn) return;

    if (enemiesAliveCount == 0 && mSpawnedThisWave >= Config::WAVE_COUNTS[mWave]) {
        if (mWave >= 2) {
            mAllWavesCleared = true;
            return;
        }
        ++mWave;
        mSpawnedThisWave = 0;
        mSpawnTimer = 0.6f;
        prepareWave();
    }

    bool spawnPhaseDone = mSpawnedThisWave >= Config::WAVE_COUNTS[mWave];
    if (spawnPhaseDone) return;

    mSpawnTimer -= dt;
    if (mSpawnTimer <= 0.0f) {
        if (mSpawnedThisWave == 0) ev.emitSound(Sfx::Wave);
        std::uniform_int_distribution<int> lane(0, Config::LANE_COUNT - 1);
        spawnFn(mRoster[mSpawnedThisWave], lane(mRandom));
        ++mSpawnedThisWave;
        mSpawnTimer = Config::WAVE_INTERVALS[mWave];
    }
}

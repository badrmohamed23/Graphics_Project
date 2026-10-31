#pragma once

#include "Entity.h"

class AudioEngine {
public:
    bool init();
    void shutdown();
    void play(Sfx s);
    void playBgm();
    void stopBgm();
    void setVolume(float v);
    float volume() const { return mVolume; }
    void stepVolume(float delta);
    void toggleMute();
    bool isMuted() const { return mMuted; }
    bool ready() const { return mReady; }

private:
    float mVolume = 0.6f;
    bool mMuted = false;
    bool mReady = false;
    void* mEngine = nullptr;
    void* mBgmSound = nullptr;
};

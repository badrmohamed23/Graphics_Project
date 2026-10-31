#include "AudioEngine.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static const unsigned SAMPLE_RATE = 22050;

static std::vector<short> synthAddTone(std::vector<short> acc, float startAt, float dur, float f0, float f1, float amp, int waveType) {
    unsigned total = (unsigned)acc.size();
    unsigned n = (unsigned)(dur * SAMPLE_RATE);
    unsigned start = (unsigned)(startAt * SAMPLE_RATE);
    if (start + n + 2400 > total) {
        acc.resize(start + n + 2400, 0);
    }
    float phase = 0.0f;
    for (unsigned i = 0; i < n; ++i) {
        float t = (float)i / SAMPLE_RATE;
        float k = t / dur;
        float freq = f0 + (f1 - f0) * k;
        phase += 6.2831853f * freq / SAMPLE_RATE;
        float v = 0.0f;
        if (waveType == 0) v = sinf(phase);
        else if (waveType == 1) v = sinf(phase) >= 0.0f ? 1.0f : -1.0f;
        else v = ((rand() % 2000) - 1000) / 1000.0f;
        float env = 1.0f - k;
        float s = v * amp * env * 0.9f;
        int idx = start + i;
        if (idx >= 0 && idx < (int)acc.size()) {
            int mixed = acc[idx] + (int)(s * 32767.0f);
            if (mixed > 32767) mixed = 32767;
            if (mixed < -32768) mixed = -32768;
            acc[idx] = (short)mixed;
        }
    }
    return acc;
}

static std::vector<short> synthBgm() {
    const float notes[8] = { 130.8f, 155.6f, 174.6f, 155.6f, 196.0f, 174.6f, 155.6f, 130.8f };
    std::vector<short> out;
    out.reserve(SAMPLE_RATE * 8);
    for (int bar = 0; bar < 8; ++bar) {
        float f = notes[bar];
        for (unsigned i = 0; i < SAMPLE_RATE; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float beat = t - (float)(int)t;
            float v = sinf(6.2831853f * f * t) * 0.35f;
            for (int harm = 2; harm <= 8; harm += 2) v += sinf(6.2831853f * f * harm * t) * 0.05f;
            v *= 0.8f + 0.2f * sinf(6.2831853f * 1.5f * t);
            v *= (1.0f - 0.55f * beat) * 0.55f;
            out.push_back((short)(v * 22000.0f));
        }
    }
    return out;
}

static bool writeWav(const std::string& path, const std::vector<short>& samples) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    unsigned numSamples = (unsigned)samples.size();
    short mono = 1, bits = 16;
    unsigned dataLen = numSamples * 2;
    struct Hdr {
        char riff[4]; unsigned chunkSize; char waveTag[4];
        char fmtTag[4]; unsigned fmtSize; short waveFmt; short ch; unsigned rate; unsigned byteRate; short blockAlign; short bitsPerSample;
        char dataTag[4]; unsigned dataLenOut;
    } h;
    memcpy(h.riff, "RIFF", 4);
    h.chunkSize = 36 + dataLen;
    memcpy(h.waveTag, "WAVE", 4);
    memcpy(h.fmtTag, "fmt ", 4);
    h.fmtSize = 16;
    h.waveFmt = 1;
    h.ch = mono;
    h.rate = SAMPLE_RATE;
    h.byteRate = SAMPLE_RATE * 2;
    h.blockAlign = mono * 2;
    h.bitsPerSample = bits;
    memcpy(h.dataTag, "data", 4);
    h.dataLenOut = dataLen;
    fwrite(&h, sizeof(Hdr), 1, f);
    fwrite(samples.data(), sizeof(short), numSamples, f);
    fclose(f);
    return true;
}

struct SoundEntry { Sfx id; std::string file; std::vector<short> samples; };
static std::vector<SoundEntry> gSounds;
static std::string gDir;

static void ensureSounds() {
    if (!gSounds.empty()) return;
    SoundEntry e;
    e = { Sfx::Attack, "sAttack.wav", synthAddTone({}, 0.0f, 0.14f, 950.0f, 500.0f, 0.5f, 1) };
    gSounds.push_back(e);
    e = { Sfx::Hit, "sHit.wav", synthAddTone({}, 0.0f, 0.12f, 220.0f, 90.0f, 0.9f, 2) };
    gSounds.push_back(e);
    e = { Sfx::Coin, "sCoin.wav", synthAddTone(synthAddTone({}, 0.0f, 0.07f, 880.0f, 880.0f, 0.4f, 1), 0.07f, 0.1f, 1318.0f, 1318.0f, 0.4f, 1) };
    gSounds.push_back(e);
    e = { Sfx::Death, "sDeath.wav", synthAddTone({}, 0.0f, 0.35f, 420.0f, 70.0f, 0.6f, 0) };
    gSounds.push_back(e);
    e = { Sfx::Place, "sPlace.wav", synthAddTone({}, 0.0f, 0.15f, 320.0f, 160.0f, 0.7f, 1) };
    gSounds.push_back(e);
    e = { Sfx::Wave, "sWave.wav", synthAddTone(synthAddTone(synthAddTone({}, 0.0f, 0.16f, 523.0f, 523.0f, 0.45f, 0), 0.16f, 0.16f, 659.0f, 659.0f, 0.45f, 0), 0.32f, 0.2f, 784.0f, 784.0f, 0.45f, 0) };
    gSounds.push_back(e);
    e = { Sfx::Win, "sWin.wav", synthAddTone(synthAddTone(synthAddTone({}, 0.0f, 0.25f, 392.0f, 392.0f, 0.5f, 0), 0.25f, 0.25f, 523.0f, 523.0f, 0.5f, 0), 0.5f, 0.4f, 659.0f, 659.0f, 0.5f, 0) };
    gSounds.push_back(e);
    e = { Sfx::Lose, "sLose.wav", synthAddTone({}, 0.0f, 0.8f, 300.0f, 60.0f, 0.6f, 0) };
    gSounds.push_back(e);
}

bool AudioEngine::init() {
    if (mReady) return true;
    ensureSounds();
    const char* tmp = getenv("TEMP");
    std::string base = tmp ? std::string(tmp) : std::string(".");
#if defined(_WIN32)
    base += "\\GameEngine2DAudio";
#else
    base += "/GameEngine2DAudio";
#endif
    system((std::string("mkdir \"") + base + std::string("\" >NUL 2>NUL")).c_str());
#if defined(_WIN32)
    gDir = base + "\\";
#else
    gDir = base + "/";
#endif
    for (SoundEntry& e : gSounds) {
        if (!writeWav(gDir + e.file, e.samples)) return false;
    }
    std::string bgmPath = gDir + "bgm.wav";
    if (!writeWav(bgmPath, synthBgm())) return false;

    ma_engine* eng = new ma_engine();
    if (ma_engine_init(nullptr, eng) != MA_SUCCESS) {
        delete eng;
        return false;
    }
    mEngine = eng;
    ma_engine_set_volume(eng, mVolume);

    ma_sound* bgm = new ma_sound();
    if (ma_sound_init_from_file(eng, bgmPath.c_str(), MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, nullptr, bgm) != MA_SUCCESS) {
        delete bgm;
        ma_engine_uninit(eng);
        delete eng;
        mEngine = nullptr;
        return false;
    }
    ma_sound_set_looping(bgm, MA_TRUE);
    mBgmSound = bgm;
    mReady = true;
    return true;
}

void AudioEngine::shutdown() {
    if (!mReady) return;
    ma_sound* bgm = static_cast<ma_sound*>(mBgmSound);
    ma_engine* eng = static_cast<ma_engine*>(mEngine);
    if (bgm) {
        ma_sound_uninit(bgm);
        delete bgm;
    }
    if (eng) {
        ma_engine_uninit(eng);
        delete eng;
    }
    mBgmSound = nullptr;
    mEngine = nullptr;
    mReady = false;
}

void AudioEngine::playBgm() {
    if (mReady) ma_sound_start(static_cast<ma_sound*>(mBgmSound));
}

void AudioEngine::stopBgm() {
    if (mReady) ma_sound_stop(static_cast<ma_sound*>(mBgmSound));
}

void AudioEngine::play(Sfx s) {
    if (!mReady || s == Sfx::None) return;
    for (SoundEntry& e : gSounds) {
        if (e.id == s) {
            ma_engine_play_sound(static_cast<ma_engine*>(mEngine), (gDir + e.file).c_str(), nullptr);
            return;
        }
    }
}

void AudioEngine::setVolume(float v) {
    mVolume = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    if (mReady) ma_engine_set_volume(static_cast<ma_engine*>(mEngine), mMuted ? 0.0f : mVolume);
}

void AudioEngine::stepVolume(float delta) {
    setVolume(mVolume + delta);
}

void AudioEngine::toggleMute() {
    mMuted = !mMuted;
    if (mReady) ma_engine_set_volume(static_cast<ma_engine*>(mEngine), mMuted ? 0.0f : mVolume);
}


#pragma once

#include "Config.h"
#include "Entity.h"

class WalletManager {
public:
    void reset() {
        mBalance = Config::START_COINS;
        for (int i = 0; i < 3; ++i) mPreviouslyUnlocked[i] = false;
        refreshUnlockState();
    }
    int balance() const { return mBalance; }
    void spend(int amount) { mBalance -= amount; if (mBalance < 0) mBalance = 0; refreshUnlockState(); }
    void earn(int amount) { mBalance += amount; refreshUnlockState(); }

    bool isUnlocked(PlayerKind k) const {
        int cost = cardCost(k);
        return mBalance >= cost;
    }
    void recentlyUnlocked(PlayerKind k) { setRecently(k, true); }
    bool consumeUnlockFlag(PlayerKind k) {
        bool v = mRecentlyUnlocked[idx(k)];
        if (v) setRecently(k, false);
        return v;
    }
    static int cardCost(PlayerKind k) {
        switch (k) {
            case PlayerKind::Melee: return Config::COST_MELEE;
            case PlayerKind::Ranged: return Config::COST_RANGED;
            default: return Config::COST_DEFENSE;
        }
    }
    void refreshUnlockState() {
        for (int i = 0; i < 3; ++i) {
            bool now = mBalance >= cardCost((PlayerKind)i);
            if (now && !mPreviouslyUnlocked[i]) setRecently((PlayerKind)i, true);
            mPreviouslyUnlocked[i] = now;
        }
    }

private:
    static int idx(PlayerKind k) { return (k == PlayerKind::Melee) ? 0 : (k == PlayerKind::Ranged ? 1 : 2); }
    void setRecently(PlayerKind k, bool v) { mRecentlyUnlocked[idx(k)] = v; }
    int mBalance = Config::START_COINS;
    bool mPreviouslyUnlocked[3] = { false, false, false };
    bool mRecentlyUnlocked[3] = { false, false, false };
};

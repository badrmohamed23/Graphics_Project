#pragma once

#include "Config.h"
#include "Entity.h"

class WalletManager {
public:
    void reset() { mBalance = Config::START_COINS; }
    int balance() const { return mBalance; }
    void spend(int amount) { mBalance -= amount; if (mBalance < 0) mBalance = 0; }
    void earn(int amount) { mBalance += amount; }

    bool isUnlocked(PlayerKind k) const {
        return mBalance >= cardCost(k);
    }
    static int cardCost(PlayerKind k) {
        switch (k) {
            case PlayerKind::Melee: return Config::COST_MELEE;
            case PlayerKind::Ranged: return Config::COST_RANGED;
            default: return Config::COST_DEFENSE;
        }
    }

private:
    int mBalance = Config::START_COINS;
};

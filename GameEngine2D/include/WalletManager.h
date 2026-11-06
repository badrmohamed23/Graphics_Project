#pragma once

#include "Config.h"
#include "Entity.h"

enum class CardLock { None, CollectCoins, AffordCost, NeedAttacker };

class WalletManager {
public:
    void reset() {
        mBalance = Config::START_COINS;
        mCollected = 0;
    }
    int balance() const { return mBalance; }
    int collected() const { return mCollected; }
    void spend(int amount) { mBalance -= amount; if (mBalance < 0) mBalance = 0; }
    void earn(int amount) {
        mBalance += amount;
        mCollected += amount;
    }

    CardLock lockReason(PlayerKind k, bool hasAttacker) const {
        const int cost = cardCost(k);
        if (k != PlayerKind::Melee && mCollected < cost) return CardLock::CollectCoins;
        if (mBalance < cost) return CardLock::AffordCost;
        if (k == PlayerKind::Defense && !hasAttacker) return CardLock::NeedAttacker;
        return CardLock::None;
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
    int mCollected = 0;
};

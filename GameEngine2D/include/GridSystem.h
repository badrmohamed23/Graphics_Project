#pragma once

#include "Config.h"
#include "Renderer2D.h"

#include <cstdint>

class GridSystem {
public:
    void reset() {
        mHoveredLane = -1;
        mSelectedMask = 0;
    }
    void updateHover(float worldX, float worldY);
    bool toggleAt(float worldX, float worldY);
    bool isSelected(int lane) const { return lane >= 0 && lane < Config::LANE_COUNT && (mSelectedMask & (1u << lane)) != 0; }
    void setSelected(int lane, bool selected) {
        if (lane >= 0 && lane < Config::LANE_COUNT) {
            if (selected) mSelectedMask |= (1u << lane);
            else mSelectedMask &= ~(1u << lane);
        }
    }
    int hoveredLane() const { return mHoveredLane; }
    bool hovered() const { return mHoveredLane >= 0; }
    bool inBattleground(float x, float y) const {
        return x >= 0.0f && x <= Config::WORLD_W && y >= Config::LANE_Y_MIN[0] && y <= Config::LANE_Y_MIN[Config::LANE_COUNT - 1] + Config::LANE_HEIGHT;
    }
    void draw(Renderer2D& r, float time) const;

private:
    int mHoveredLane = -1;
    uint32_t mSelectedMask = 0;
};

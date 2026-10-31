#include "GameEngine.h"

int main() {
    GameEngine engine;
    if (!engine.init()) return 1;
    engine.run();
    engine.shutdown();
    return 0;
}

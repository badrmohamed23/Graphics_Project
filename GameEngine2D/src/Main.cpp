#include "GameEngine.h"
#include <cstdio>

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    GameEngine engine;
    if (!engine.init()) return 1;
    engine.run();
    engine.shutdown();
    return 0;
}

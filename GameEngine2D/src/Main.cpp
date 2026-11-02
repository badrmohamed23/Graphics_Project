#include "GameEngine.h"
#include <cstdio>
#include <cstdlib>

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (getenv("NEON_DEBUG_LOG")) freopen("neon_debug.log", "w", stderr);
    GameEngine engine;
    if (!engine.init()) return 1;
    engine.run();
    engine.shutdown();
    return 0;
}

#include "engine.h"

int main(int argc, char** argv){
    engine_startUp(argc, argv);
    engine_mainLoop();
    engine_cleanUp();

    return 0;
}

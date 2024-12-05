#include "ludo_game.hpp"
#include "ludo_game.h"

int main() {
    try {
        LudoGame game;
        game.runGame();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
#include "ludo_game.hpp"

int main() {
    try {
        LudoGame game;
        game.simulateGameplay();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
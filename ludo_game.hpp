#ifndef LUDO_GAME_HPP
#define LUDO_GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <algorithm>

class LudoGame {
private:
    static constexpr int GRID_SIZE = 15;
    static constexpr int MAX_TOKENS_PER_PLAYER = 4;
    static constexpr int NUM_PLAYERS = 4;
    static constexpr int TILE_SIZE = 50;
    static constexpr int SAFE_ZONES = 8;

    std::vector<std::vector<sf::Vector2i>> playerTokens;
    std::vector<sf::Color> playerColors;
    std::vector<sf::Vector2i> playerStartPositions;
    std::vector<sf::Vector2i> playerHomeColumns;
    std::vector<sf::Vector2i> ludoPath;
    std::vector<sf::Vector2i> safeZones;

    sf::RenderWindow window;
    sf::Font defaultFont;
    sf::Text infoText;

    std::mt19937 randomGenerator;
    int currentPlayer;
    int diceValue;
    bool diceRolled;

    std::mutex gameMutex;

    void initializeGame();
    int rollDice();
    sf::Vector2i moveTokenOnBoard(sf::Vector2i token, int player);
    void moveToken(int player, int tokenIndex);
    bool isTokenInYard(const sf::Vector2i& token, int player);
    bool isSafeZone(const sf::Vector2i& position);
    void renderGame();
    void handleMouseClick(int x, int y);

public:
    LudoGame();
    void runGame();
};

#endif // LUDO_GAME_HPP
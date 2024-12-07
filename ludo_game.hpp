#ifndef LUDO_GAME_HPP
#define LUDO_GAME_HPP

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <mutex>
#include <iostream>
#include <pthread.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include <map>

using namespace std;

class LudoGame {
public:
    LudoGame();
    void runGame();
    void simulateGameplay();

private:
    static const int GRID_SIZE = 15;
    static const int TILE_SIZE = 60;
    static const int MAX_TOKENS_PER_PLAYER = 4;
    static const int MAX_PLAYERS = 4;

    sf::RenderWindow window;
    sf::Font defaultFont;
    sf::Text infoText;

    vector<sf::Color> playerColors;
    vector<vector<sf::Vector2i>> playerTokens;
    vector<vector<bool>> finishedPlayerTokens;
    vector<sf::Vector2i> playerStartPositions;
    vector<sf::Vector2i> ludoPath;
    vector<sf::Vector2i> safeZones;
    vector<vector<sf::Vector2i>> killersPath;
    vector<bool> killers;
    vector<int> finishingOrder;

    bool teamMode;
    int numPlayers;
    int currentPlayer;
    int diceValue;
    bool diceRolled;
    bool simulationMode;

    sem_t semaphore;
    condition_variable cv;
    vector<int> consecutiveTurnsWithoutProgress;

    mt19937 randomGenerator;
    mutex gameMutex;

    pthread_t playerThreads[MAX_PLAYERS];
    pthread_t rowColumnThreads[GRID_SIZE * 2];
    pthread_t masterThreadHandle;

    void initializeGame();
    int rollDice();
    sf::Vector2i moveTokenOnBoard(sf::Vector2i token, int player, int tokenIndex);
    void moveToken(int player, int tokenIndex);
    bool isTokenInYard(const sf::Vector2i& token, int player);
    bool isSafeZone(const sf::Vector2i& position);
    bool shouldSkipTurn(int player);
    bool allPlayersFinished();
    void renderGame();
    void handleMouseClick(int x, int y);
    void drawBoard();
    void displayFinishingOrder();
    void askNumberOfPlayers(sf::RenderWindow& gameWindow);
    void initializeThreads();
    
    
    static void* playerThread(void* arg);
    static void* rowColumnThread(void* arg);
    static void* masterThread(void* arg);
    static void* gameThread(void* arg);

    bool playerMadeProgress(int player);
    bool allTokensHome(int player);
    void finishPlayer(int player);
    void removePlayer(int player);
    bool gameIsOver();
    void checkForHits(int player, int token);
    bool areTeammates(int player1, int player2);
    void discardTurn(int player);
};

#endif // LUDO_GAME_HPP
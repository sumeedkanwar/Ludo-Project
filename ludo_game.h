#include "ludo_game.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <unistd.h>

using namespace std;

struct ThreadParams {
    int player;
    int row;
    int column;
    int hit_record;
    LudoGame* game;
};

void* LudoGame::playerThread(void* arg) {
    ThreadParams* params = static_cast<ThreadParams*>(arg);
    LudoGame* game = params->game;

    while (true) {
        if (game->shouldSkipTurn(params->player)) {
            continue;
        }

        if (!game->diceRolled) {
            continue;
        }

        game->moveToken(params->player, params->hit_record);

        if (game->gameIsOver()) {
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }

    return nullptr;
}

void* LudoGame::rowColumnThread(void* arg) {
    ThreadParams* params = static_cast<ThreadParams*>(arg);
    LudoGame* game = params->game;

    while (true) {
        for (int player = 0; player < game->numPlayers; ++player) {
            for (int token = 0; token < MAX_TOKENS_PER_PLAYER; ++token) {
                game->checkForHits(player, token);
            }
        }

        if (game->gameIsOver()) {
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }

    return nullptr;
}

void* LudoGame::masterThread(void* arg) {
    LudoGame* game = static_cast<LudoGame*>(arg);
    vector<int> consecutiveTurnsWithoutProgress(game->numPlayers, 0);

    while (true) {
        // Lock the game state
        lock_guard<mutex> lock(game->gameMutex);

        for (int player = 0; player < game->numPlayers; ++player) {
            if (game->playerMadeProgress(player)) {
                consecutiveTurnsWithoutProgress[player] = 0;
            } else {
                consecutiveTurnsWithoutProgress[player]++;
                if (consecutiveTurnsWithoutProgress[player] >= 20) {
                    // Cancel this player's thread
                    pthread_cancel(game->playerThreads[player]);
                    game->removePlayer(player);
                }
            }

            if (game->allTokensHome(player)) {
                // Player has finished
                game->finishPlayer(player);
                pthread_cancel(game->playerThreads[player]);
            }
        }

        if (game->gameIsOver()) {
            break;
        }

        // Sleep for a short duration before next check
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    return nullptr;
}

void LudoGame::initializeThreads() {
    for (int i = 0; i < numPlayers; ++i) {
        ThreadParams* params = new ThreadParams{i, 0, 0, 0, this};
        pthread_create(&playerThreads[i], nullptr, LudoGame::playerThread, params); // Use LudoGame::playerThread
    }

    for (int i = 0; i < GRID_SIZE; ++i) {
        ThreadParams* rowParams = new ThreadParams{0, i, 0, 0, this};
        pthread_create(&rowColumnThreads[i], nullptr, LudoGame::rowColumnThread, rowParams); // Use LudoGame::rowColumnThread
        
        ThreadParams* colParams = new ThreadParams{0, 0, i, 0, this};
        pthread_create(&rowColumnThreads[GRID_SIZE + i], nullptr, LudoGame::rowColumnThread, colParams); // Use LudoGame::rowColumnThread
    }

    pthread_create(&masterThreadHandle, nullptr, LudoGame::masterThread, this); // Use LudoGame::masterThread
}

LudoGame::LudoGame()
    : window(sf::VideoMode(GRID_SIZE * TILE_SIZE, GRID_SIZE * TILE_SIZE), "Ludo Game"),
      currentPlayer(0),
      diceValue(0),
      diceRolled(false)
{
    askNumberOfPlayers();
    initializeGame();
}

void LudoGame::askNumberOfPlayers()
{
    sf::RenderWindow inputWindow(sf::VideoMode(400, 200), "Enter Number of Players");
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        throw runtime_error("Font not loaded. Adjust font path.");
    }

    sf::Text prompt("Enter the number of players (2-4) or Enter 0 for Team Mode:", font, 20);
    prompt.setPosition(20, 20);
    prompt.setFillColor(sf::Color::Black);

    sf::Text input("", font, 20);
    input.setPosition(20, 80);
    input.setFillColor(sf::Color::Black);

    string inputStr;
    while (inputWindow.isOpen())
    {
        sf::Event event;
        while (inputWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                inputWindow.close();
            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode >= '0' && event.text.unicode <= '9')
                {
                    inputStr += static_cast<char>(event.text.unicode);
                    input.setString(inputStr);
                }
                else if (event.text.unicode == '\b' && !inputStr.empty())
                {
                    inputStr.pop_back();
                    input.setString(inputStr);
                }
                else if (event.text.unicode == '\r')
                {
                    int players = stoi(inputStr);
                    if (players >= 2 && players <= 4)
                    {
                        numPlayers = players;
                        inputWindow.close();
                    }else if (inputStr == "0") { //team mode
                        numPlayers = 4;
                        teamMode = true;
                        inputWindow.close();
                    }
                    else
                    {
                        inputStr.clear();
                        input.setString(inputStr);
                    }

                }
            }
        }

        inputWindow.clear(sf::Color::White);
        inputWindow.draw(prompt);
        inputWindow.draw(input);
        inputWindow.display();
    }
}

void LudoGame::initializeGame()
{

    teamMode = false;

    playerColors = {
        sf::Color::Red,
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color::Yellow
    };

    killers.resize(numPlayers, false);

    playerStartPositions = {
        {1, 1}, {1, 2}, {2, 1}, {2, 2}, // Red
        {1, 12}, {1, 13}, {2, 12}, {2, 13}, // Green
        {12, 12}, {12, 13}, {13, 12}, {13, 13}, // Yellow
        {12, 1}, {12, 2}, {13, 1}, {13, 2} // Blue
    };

    ludoPath = {
        {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}
    };

    vector<sf::Vector2i> killerRedLudoPath = {
        {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}
    };

    vector<sf::Vector2i> killerGreenLudoPath = {
        {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}
    };

    vector<sf::Vector2i> killerBlueLudoPath = {
        {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {7, 13}, {7, 12}, {7, 11}, {7, 10}, {7, 9}, {7, 8}
    };

    vector<sf::Vector2i> killerYellowLudoPath = {
        {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7}, {8, 7}
    };

    killersPath = {killerRedLudoPath, killerGreenLudoPath, killerBlueLudoPath, killerYellowLudoPath};

    safeZones = {
        {2, 6}, {6, 1}, {8, 2}, {13, 6}, {12, 8}, {8, 13}, {6, 12}, {1, 8}
    };

    playerTokens.resize(numPlayers);
    for (int player = 0; player < numPlayers; ++player) {
        for (int token = 0; token < MAX_TOKENS_PER_PLAYER; ++token) {
            playerTokens[player].push_back(playerStartPositions[player * MAX_TOKENS_PER_PLAYER + token]);
        }
    }

    finishedPlayerTokens.resize(numPlayers);
    for (int player = 0; player < numPlayers; ++player) {
        finishedPlayerTokens[player].resize(MAX_TOKENS_PER_PLAYER, false);
    }

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(
        sf::VideoMode(GRID_SIZE * TILE_SIZE, GRID_SIZE * TILE_SIZE),
        "Ludo Game",
        sf::Style::Default,
        settings);
    window.setFramerateLimit(30);

    if (!defaultFont.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        throw runtime_error("Font not loaded. Adjust font path.");
    }

    infoText.setFont(defaultFont);
    infoText.setCharacterSize(10);
    infoText.setFillColor(sf::Color::Black);
    infoText.setPosition(10, GRID_SIZE * TILE_SIZE - 30);
}

int LudoGame::rollDice()
{
    uniform_int_distribution<> dis(1, 6);
    return dis(randomGenerator);
}

bool LudoGame::playerMadeProgress(int player) {
    // Check if the player rolled a 6 or hit an opponent
    return (diceValue == 6 || killers[player]);
}

void LudoGame::removePlayer(int player) {
    // Remove the player from the game
    // This could involve setting a flag, removing their tokens, etc.
    cout << "Player " << player + 1 << " has been eliminated." << endl;
}

bool LudoGame::allTokensHome(int player) {
    return all_of(finishedPlayerTokens[player].begin(), 
                  finishedPlayerTokens[player].end(), 
                  [](bool finished) { return finished; });
}

void LudoGame::finishPlayer(int player) {
    finishingOrder.push_back(player);
}

bool LudoGame::gameIsOver() {
    return finishingOrder.size() >= numPlayers - 1;
}

void LudoGame::checkForHits(int player, int tokenIndex) {
    sf::Vector2i tokenPosition = playerTokens[player][tokenIndex];

    for (int otherPlayer = 0; otherPlayer < numPlayers; ++otherPlayer) {
        if (otherPlayer == player) continue;

        for (int otherTokenIndex = 0; otherTokenIndex < MAX_TOKENS_PER_PLAYER; ++otherTokenIndex) {
            sf::Vector2i otherTokenPosition = playerTokens[otherPlayer][otherTokenIndex];

            if (tokenPosition == otherTokenPosition && !isSafeZone(tokenPosition)) {
                // Hit detected, move the hit token back to its yard
                playerTokens[otherPlayer][otherTokenIndex] = playerStartPositions[otherPlayer * MAX_TOKENS_PER_PLAYER + otherTokenIndex];
                cout << "Player " << player + 1 << " hit Player " << otherPlayer + 1 << "'s token!" << endl;
            }
        }
    }
}

bool LudoGame::areTeammates(int player1, int player2) {
    if (!teamMode) return false;
    return (player1 % 2 == player2 % 2);
}

sf::Vector2i LudoGame::moveTokenOnBoard(sf::Vector2i token, int player, int tokenIndex)
{
    const vector<sf::Vector2i>& path = killers[player] ? killersPath[player] : ludoPath;
    
    auto it = find(path.begin(), path.end(), token);
    if (it == path.end()) {
        it = min_element(path.begin(), path.end(), 
            [&token](const sf::Vector2i& a, const sf::Vector2i& b) {
                return (abs(a.x - token.x) + abs(a.y - token.y)) < 
                       (abs(b.x - token.x) + abs(b.y - token.y));
            });
    }

    size_t currentIndex = distance(path.begin(), it);
    size_t newIndex = currentIndex + diceValue;

    if (newIndex >= path.size()) {
        if (killers[player]) {
            finishedPlayerTokens[player][tokenIndex] = true;
            return token;
        } else {
            newIndex = path.size() - 1;
        }
    }

    while (newIndex < path.size()) {
        sf::Vector2i newPosition = path[newIndex];

        int sameColorTokenCount = count(playerTokens[player].begin(), 
                                             playerTokens[player].end(), 
                                             newPosition);
        
        int teamBlockCount = 0;
        int opposingBlockCount = 0;
        for (int otherPlayer = 0; otherPlayer < numPlayers; ++otherPlayer) {
            if (otherPlayer == player) continue;

            int tokenCount = count(playerTokens[otherPlayer].begin(), 
                                   playerTokens[otherPlayer].end(), 
                                   newPosition);

            if (areTeammates(player, otherPlayer)) {
                teamBlockCount += tokenCount;
            } else if (tokenCount >= 2) {
                opposingBlockCount++;
            }
        }

        if ((sameColorTokenCount + teamBlockCount <= 1 || isSafeZone(newPosition)) && 
            (opposingBlockCount == 0 || isSafeZone(newPosition))) {
            return newPosition;
        }

        newIndex++;
    }

    return token;
}

void LudoGame::moveToken(int player, int tokenIndex)
{
    lock_guard<mutex> lock(gameMutex);

    auto& token = playerTokens[player][tokenIndex];
    bool tokenCaptured = false;

    if (isTokenInYard(token, player)) {
        if (diceValue == 6) {
            token = ludoPath[player * 13];
        }
    } else {
        sf::Vector2i newPosition = moveTokenOnBoard(token, player, tokenIndex);
        token = newPosition;
    }

    for (int otherPlayer = 0; otherPlayer < numPlayers; ++otherPlayer) {
        if (otherPlayer != player && !areTeammates(player, otherPlayer)) {
            for (auto& otherToken : playerTokens[otherPlayer]) {
                if (otherToken == token && !isSafeZone(token) && token != ludoPath.back()) {
                    killers[player] = true;
                    tokenCaptured = true;
                    for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i) {
                        sf::Vector2i yardPosition = playerStartPositions[otherPlayer * MAX_TOKENS_PER_PLAYER + i];
                        if (find(playerTokens[otherPlayer].begin(), playerTokens[otherPlayer].end(), yardPosition) == playerTokens[otherPlayer].end()) {
                            otherToken = yardPosition;
                            break;
                        }
                    }
                }
            }
        }
    }

    diceRolled = false;
    if (diceValue != 6 && !tokenCaptured) {
        if (allTokensHome(player)) {
            // Pass the dice roll to a teammate if the current player has finished all their tokens
            for (int teammate = 0; teammate < numPlayers; ++teammate) {
                if (areTeammates(player, teammate) && !allTokensHome(teammate)) {
                    currentPlayer = teammate;
                    std::cout << "Player " << player + 1 << " has finished all their tokens. Passing the turn to Player " << currentPlayer + 1 << std::endl;
                    return;
                }
            }
        }
        currentPlayer = (currentPlayer + 1) % numPlayers;
    }
}

bool LudoGame::isTokenInYard(const sf::Vector2i& token, int player)
{
    for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i) {
        if (token == playerStartPositions[player * MAX_TOKENS_PER_PLAYER + i]) {
            return true;
        }
    }
    return false;
}

bool LudoGame::isSafeZone(const sf::Vector2i& position)
{
    return find(safeZones.begin(), safeZones.end(), position) != safeZones.end();
}

void LudoGame::renderGame()
{
    window.clear(sf::Color::White);

    drawBoard();

    sf::CircleShape token(TILE_SIZE / 3.f);
    token.setOutlineThickness(1);
    token.setOutlineColor(sf::Color::Black);

    sf::ConvexShape star;
    star.setPointCount(10);
    float radius = TILE_SIZE / 6.f;
    float innerRadius = radius / 2.5f;
    for (int i = 0; i < 10; ++i) {
        float angle = i * 36 * 3.14159f / 180;
        float r = (i % 2 == 0) ? radius : innerRadius;
        star.setPoint(i, sf::Vector2f(r * cos(angle), r * sin(angle)));
    }
    star.setFillColor(sf::Color::Black);

    for (int player = 0; player < numPlayers; ++player) {
        for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i) {
            const auto& tokenPos = playerTokens[player][i];
            if (finishedPlayerTokens[player][i]) continue;

            int tokenCount = count(playerTokens[player].begin(), playerTokens[player].end(), tokenPos);

            token.setRadius(tokenCount > 1 ? TILE_SIZE / 4.f : TILE_SIZE / 3.f);
            star.setScale(tokenCount > 1 ? 0.5f : 1.f, tokenCount > 1 ? 0.5f : 1.f);

            token.setFillColor(playerColors[player]);

            float offset = (tokenCount > 1) ? TILE_SIZE / 8.f : 0.f;
            token.setPosition(tokenPos.y * TILE_SIZE + TILE_SIZE / 6.f + (i * offset),
                              tokenPos.x * TILE_SIZE + TILE_SIZE / 6.f + (i * offset));
            star.setPosition(token.getPosition() + sf::Vector2f(token.getRadius() - radius / 4.f, token.getRadius() - radius / 4.f));
            
            window.draw(token);
            window.draw(star);
        }
    }

    infoText.setString("Player " + to_string(currentPlayer + 1) +
                       " | Dice: " + to_string(diceValue) +
                       (diceRolled ? " | Click to move" : " | Click to roll"));
    window.draw(infoText);

    window.display();
}

bool LudoGame::shouldSkipTurn(int player) {
    if (find(finishingOrder.begin(), finishingOrder.end(), player) != finishingOrder.end()) {
        return true;
    }

    for (int token = 0; token < MAX_TOKENS_PER_PLAYER; ++token) {
        if (!finishedPlayerTokens[player][token]) {
            return false;
        }
    }

    finishingOrder.push_back(player);
    return true;
}

bool LudoGame::allPlayersFinished() {
    int finishedPlayersCount = 0;
    for (int player = 0; player < numPlayers; ++player) {
        if (shouldSkipTurn(player)) {
            finishedPlayersCount++;
        }
    }
    return finishedPlayersCount >= numPlayers - 1;
}

void LudoGame::runGame()
{
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed)
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
        }

        window.clear(sf::Color::White);
        drawBoard();
        renderGame();
        window.display();
    }
}

void LudoGame::handleMouseClick(int x, int y)
{
    if (!diceRolled) {
        diceValue = rollDice();
        diceRolled = true;
    } else {
        int clickedRow = y / TILE_SIZE;
        int clickedCol = x / TILE_SIZE;

        for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i) {
            if (playerTokens[currentPlayer][i].x == clickedRow &&
                playerTokens[currentPlayer][i].y == clickedCol) {
                moveToken(currentPlayer, i);
                break;
            }
        }
    }
}

void LudoGame::drawBoard()
{
    sf::RectangleShape cell(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    cell.setOutlineThickness(1);
    cell.setOutlineColor(sf::Color::Black);

    sf::Color red(sf::Color::Red);
    sf::Color green(sf::Color::Green);
    sf::Color blue(sf::Color::Blue);
    sf::Color yellow(sf::Color::Yellow);
    sf::Color white(sf::Color::White);
    sf::Color black(sf::Color::Black);
    sf::Color grey(sf::Color(128, 128, 128));

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            cell.setPosition(j * TILE_SIZE, i * TILE_SIZE);

            if (i < 6 && j < 6)
                cell.setFillColor(red);
            else if (i > 8 && j < 6)
                cell.setFillColor(yellow);
            else if (i < 6 && j > 8)
                cell.setFillColor(green);
            else if (i > 8 && j > 8)
                cell.setFillColor(blue);
            else if (i == 7 && j >= 1 && j <= 6)
                cell.setFillColor(red);
            else if (j == 7 && i >= 1 && i <= 6)
                cell.setFillColor(green);
            else if (i == 7 && j >= 8 && j <= 13)
                cell.setFillColor(blue);
            else if (j == 7 && i >= 8 && i <= 13)
                cell.setFillColor(yellow);
            else if ((i == 7 && j == 7) || (i == 6 && j == 6) || (i == 6 && j == 8) || (i == 8 && j == 6) || (i == 8 && j == 8))
                cell.setFillColor(black);
            else if ((i == 2 && j == 6) || (i == 6 && j == 1) || (i == 13 && j == 6) || (i == 8 && j == 13) || (i == 6 && j == 12) || (i == 12 && j == 8) || (i == 8 && j == 2) || (i == 1 && j == 8))
                cell.setFillColor(grey);
            else
                cell.setFillColor(white);

            window.draw(cell);
        }
    }
}

void LudoGame::displayFinishingOrder() {
    window.clear(sf::Color::White);

    // Create a title for the finishing order
    sf::Text titleText;
    titleText.setFont(defaultFont);
    titleText.setCharacterSize(30);
    titleText.setFillColor(sf::Color::Blue);
    titleText.setStyle(sf::Text::Bold);
    titleText.setString("Game Over!");
    titleText.setPosition(
        (window.getSize().x - titleText.getGlobalBounds().width) / 2, 20);

    // Display the finishing order
    sf::Text finishingOrderText;
    finishingOrderText.setFont(defaultFont);
    finishingOrderText.setCharacterSize(20);
    finishingOrderText.setFillColor(sf::Color::Black);

    std::string orderText = "Finishing Order:\n";
    const std::vector<std::string> placeSuffix = {"1st Place (Winner)", "2nd Place", "3rd Place"};

    for (size_t i = 0; i < finishingOrder.size(); ++i) {
        orderText += placeSuffix[i] + ": Player " + std::to_string(finishingOrder[i] + 1) + "\n";
    }

    finishingOrderText.setString(orderText);
    finishingOrderText.setPosition(
        (window.getSize().x - finishingOrderText.getGlobalBounds().width) / 2, 80);

    // Draw everything on the window
    window.draw(titleText);
    window.draw(finishingOrderText);
    window.display();

    // Wait for user input to close the window
    sf::Event event;
    while (window.waitEvent(event)) {
        if (event.type == sf::Event::Closed || 
            (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
            window.close();
            break;
        }
    }
}


void LudoGame::simulateGameplay()
{
    while (window.isOpen()) {
        if (allPlayersFinished()) {
            displayFinishingOrder();
            break;
        }

        if (shouldSkipTurn(currentPlayer)) {
            cout << "Player " << currentPlayer + 1 << " has no tokens left to move." << endl;
            currentPlayer = (currentPlayer + 1) % numPlayers;
            continue;
        }

        if (!diceRolled) {
            diceValue = rollDice();
            diceRolled = true;
        } else {
            int tokenIndex;
            do {
                tokenIndex = uniform_int_distribution<>(0, MAX_TOKENS_PER_PLAYER - 1)(randomGenerator);
            } while (finishedPlayerTokens[currentPlayer][tokenIndex]);
            
            moveToken(currentPlayer, tokenIndex);
            currentPlayer = (currentPlayer + 1) % numPlayers;
            diceRolled = false;
        }

        renderGame();
        this_thread::sleep_for(chrono::milliseconds(1));
    }

   
    sleep(2);
    renderGame();
    window.close();
}


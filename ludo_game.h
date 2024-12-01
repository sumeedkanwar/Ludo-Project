#include "ludo_game.hpp"

using namespace std;

LudoGame::LudoGame()
    : window(sf::VideoMode(800, 800), "Ludo Game"),
      currentPlayer(0),
      diceValue(0),
      diceRolled(false),
      randomGenerator(std::random_device{}())
{
    initializeGame();
}

void LudoGame::initializeGame()
{
    playerColors = {
        sf::Color::Red,
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color::Yellow};

    for (int i = 0; i < 4; ++i) {
    killers[i] = false;
    }

    playerStartPositions = {
        {1, 1}, {1, 2}, {2, 1}, {2, 2}, // Red
        {1, 12}, {1, 13}, {2, 12}, {2, 13}, // Green
        {12, 12}, {12, 13}, {13, 12}, {13, 13}, // Yellow
        {12, 1}, {12, 2}, {13, 1}, {13, 2} // Blue
    };

    // Define paths for each player
    ludoPath = {
        {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}
    };
    std::vector<sf::Vector2i> killerRedLudoPath = {// part of ludo path starting from red spawn position to 7,0 then in to home column
        {6, 1},{6, 2},{6, 3},{6, 4},{6, 5},{5, 6},{4, 6},{3, 6},{2, 6},{1, 6},{0, 6},{0, 7},{0, 8},{1, 8},{2, 8},{3, 8},{4, 8},{5, 8},{6, 9},{6, 10},{6, 11},{6, 12},{6, 13},{6, 14},{7, 14},{8, 14},{8, 13},{8, 12},{8, 11},{8, 10},{8, 9},{9, 8},{10, 8},{11, 8},{12, 8},{13, 8},{14, 8},{14, 7},{14, 6},{13, 6},{12, 6},{11, 6},{10, 6},{9, 6},{8, 5},{8, 4},{8, 3},{8, 2},{8, 1},{8, 0},{7, 0},{7, 1},{7, 2},{7, 3},{7, 4},{7, 5}, {7, 6}};

    std::vector<sf::Vector2i> killerGreenLudoPath = {// part of ludo path starting from green spawn position to 0,7 then in to home column of green
        {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 7}};

    std::vector<sf::Vector2i> killerBlueLudoPath = {// part of ludo path starting from blue spawn position to 7,14 then in to home column of blue
        {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {7, 13}, {7, 12}, {7, 11}, {7, 10}, {7, 9}, {7, 8}};

    std::vector<sf::Vector2i> killerYellowLudoPath = {// part of ludo path starting from yellow spawn position to 14,7 then in to home column of yellow
        {13, 6}, {12, 6}, {11, 6}, {10, 6}, {9, 6}, {8, 5}, {8, 4}, {8, 3}, {8, 2}, {8, 1}, {8, 0}, {7, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {5, 6}, {4, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 6}, {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 9}, {6, 10}, {6, 11}, {6, 12}, {6, 13}, {6, 14}, {7, 14}, {8, 14}, {8, 13}, {8, 12}, {8, 11}, {8, 10}, {8, 9}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {14, 7}, {13, 7}, {12, 7}, {11, 7}, {10, 7}, {9, 7}, {8, 7}};

    killersPath[0] = killerRedLudoPath;
    killersPath[1] = killerGreenLudoPath;
    killersPath[2] = killerBlueLudoPath;
    killersPath[3] = killerYellowLudoPath;

    safeZones = {
        {2, 6}, {6, 1}, {8, 2}, {13, 6}, {12, 8}, {8, 13}, {6, 12}, {1, 8}};

    playerTokens.resize(NUM_PLAYERS);

    // Initialize tokens at the starting positions
    for (int player = 0; player < NUM_PLAYERS; ++player)
    {
        for (int token = 0; token < MAX_TOKENS_PER_PLAYER; ++token)
        {
            playerTokens[player].push_back(playerStartPositions[player * MAX_TOKENS_PER_PLAYER + token]);
        }
    }

    finishedPlayerTokens.resize(NUM_PLAYERS);
    for (int player = 0; player < NUM_PLAYERS; ++player)
    {
        finishedPlayerTokens[player].resize(MAX_TOKENS_PER_PLAYER, false);
    }




    std::random_device rd;
    randomGenerator.seed(rd());

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(
        sf::VideoMode(GRID_SIZE * TILE_SIZE, GRID_SIZE * TILE_SIZE),
        "Ludo Game",
        sf::Style::Default,
        settings);
    window.setFramerateLimit(30);

    if (!defaultFont.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
    {
        throw std::runtime_error("Font not loaded. Adjust font path.");
    }

    infoText.setFont(defaultFont);
    infoText.setCharacterSize(40);
    infoText.setFillColor(sf::Color::Black);
    infoText.setPosition(10, GRID_SIZE * TILE_SIZE - 30);

    currentPlayer = 0;
    diceValue = 0;
    diceRolled = false;
}

int LudoGame::rollDice()
{
    std::uniform_int_distribution<> dis(1, 6);
    return dis(randomGenerator);
}

sf::Vector2i LudoGame::moveTokenOnBoard(sf::Vector2i token, int player, int tokenIndex)
{
    const std::vector<sf::Vector2i>& path = killers[player] ? killersPath[player] : ludoPath;
    std::cout << "Player " << player << " path size: " << path.size() << std::endl;
   

    // Find the current token position in the path
    auto it = std::find(path.begin(), path.end(), token);
    if (it == path.end())
    {
        // If token is not in the path, find the closest position
        it = std::min_element(path.begin(), path.end(), 
            [&token](const sf::Vector2i& a, const sf::Vector2i& b) {
                return (std::abs(a.x - token.x) + std::abs(a.y - token.y)) < 
                       (std::abs(b.x - token.x) + std::abs(b.y - token.y));
            });
    }

    size_t currentIndex = std::distance(path.begin(), it);
    size_t newIndex; // New index for the token

    // Determine the new index based on whether the player has killed tokens
    if (!killers[player]) {
        // For players who haven't killed, wrap around using modulo
        newIndex = (currentIndex + diceValue) % path.size();
    } else {
        // For players who have killed, increment without wrapping
        newIndex = currentIndex + diceValue;
        
        // Ensure the token doesn't go beyond the path
        if (newIndex >= path.size()) {
            // Check if the token is truly in the home column for this player
            sf::Vector2i lastPosition = path.back();
            bool isFinished = false;
            // check if after the dice roll the token is in the last pos of path
            if (token == lastPosition) {
                isFinished = true;
                std::cout << "Player " << player << " has finished the path." << std::endl;
                playerTokens[player].erase(std::remove(playerTokens[player].begin(), playerTokens[player].end(), token), playerTokens[player].end());
                finishedPlayerTokens[player][tokenIndex] = true;
            }


            
            return isFinished ? token : path.back();

        }
    }

    // Ensure we don't get stuck by finding the next valid move
    while (newIndex < path.size())
    {
        sf::Vector2i newPosition = path[newIndex];

        // Check for blocking by same player's tokens
        int sameColorTokenCount = std::count(playerTokens[player].begin(), 
                                             playerTokens[player].end(), 
                                             newPosition);
        
        // Check for blocking by an opposing player's block of two tokens
        int opposingBlockCount = 0;
        for (int otherPlayer = 0; otherPlayer < NUM_PLAYERS; ++otherPlayer)
        {
            if (otherPlayer == player) continue;

            int opposingTokenCount = std::count(playerTokens[otherPlayer].begin(), 
                                                playerTokens[otherPlayer].end(), 
                                                newPosition);


            if (opposingTokenCount >= 2) {
                opposingBlockCount++;
            }
        }

        // If not blocked, return the new position
        if ((sameColorTokenCount <= 1 || isSafeZone(newPosition)) && 
            (opposingBlockCount == 0 || isSafeZone(newPosition)))
        {
            return newPosition;
        }
        else if (opposingBlockCount > 0 && !isSafeZone(newPosition))
        {
            // If blocked by opposing player's block of two tokens, invalidate the move
            return token;
        }

        
        
        // If we've exhausted the path, return to original token position
        if (newIndex >= path.size()) 
        {
            return token;
        }
    }

    // Fallback: return original token if no valid move found
    return token;
}

void LudoGame::moveToken(int player, int tokenIndex)
{
    std::lock_guard<std::mutex> lock(gameMutex);

    auto &token = playerTokens[player][tokenIndex];
    if (isTokenInYard(token, player))
    {
        if (diceValue == 6)
        {
            token = ludoPath[player * 13]; // Starting point for each player
            std::cout << "Player " << player << " token " << tokenIndex << " spawned." << std::endl;
        }
    }
    else
    {
        
        sf::Vector2i newPosition = moveTokenOnBoard(token, player, tokenIndex);

        // Check if the new position is blocked by other players' tokens
        bool isBlocked = false;
        for (int otherPlayer = 0; otherPlayer < NUM_PLAYERS; ++otherPlayer)
        {
            if (otherPlayer != player)
            {
                int sameColorTokenCount = 0;
                for (const auto &otherToken : playerTokens[otherPlayer])
                {
                    if (otherToken == newPosition)
                    {
                        sameColorTokenCount++;
                    }
                }

                if (sameColorTokenCount > 1 && !isSafeZone(newPosition))
                {
                    isBlocked = true;
                    break;
                }
            }
        }

        if (!isBlocked)
        {
            token = newPosition;
        }
        else
        {
            std::cout << "Move blocked by other player's tokens." << std::endl;
        }
    
    }

    // Check for token capture
    for (int otherPlayer = 0; otherPlayer < NUM_PLAYERS; ++otherPlayer)
    {
        if (otherPlayer != player)
        {
            for (auto &otherToken : playerTokens[otherPlayer])
            {
                if (otherToken == token && !isSafeZone(token) && token != ludoPath.back())
                {
                    std::cout << "Player " << player << " killed player " << otherPlayer << "'s token." << std::endl;
                    killers[player] = true;
                    // Send the captured token back to its yard
                    for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i)
                    {
                        sf::Vector2i yardPosition = playerStartPositions[otherPlayer * MAX_TOKENS_PER_PLAYER + i];
                        bool spotOccupied = false;
                        for (const auto &token : playerTokens[otherPlayer])
                        {
                            if (token == yardPosition)
                            {
                                spotOccupied = true;
                                break;
                            }
                        }
                        if (!spotOccupied)
                        {
                            otherToken = yardPosition;
                            break;
                        }
                    }
                }
            }
        }
    }

    diceRolled = false;
    if (diceValue != 6)
    {
        currentPlayer = (currentPlayer + 1) % NUM_PLAYERS;
    }
}

bool LudoGame::isTokenInYard(const sf::Vector2i &token, int player)
{
    for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i)
    {
        if (token == playerStartPositions[player * MAX_TOKENS_PER_PLAYER + i])
        {
            return true;
        }
    }
    return false;
}

bool LudoGame::isSafeZone(const sf::Vector2i &position)
{
    return std::find(safeZones.begin(), safeZones.end(), position) != safeZones.end();
}

void LudoGame::renderGame()
{
    window.clear(sf::Color::White);

    sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));

    drawBoard();

    sf::CircleShape token(TILE_SIZE / 3.f);
    token.setOutlineThickness(1);
    token.setOutlineColor(sf::Color::Black);

    // Create a star shape
    sf::ConvexShape star;
    star.setPointCount(10);
    float radius = TILE_SIZE / 6.f;
    float innerRadius = radius / 2.5f;
    for (int i = 0; i < 10; ++i)
    {
        float angle = i * 36 * 3.14159f / 180;
        float r = (i % 2 == 0) ? radius : innerRadius;
        star.setPoint(i, sf::Vector2f(r * cos(angle), r * sin(angle)));
    }
    star.setFillColor(sf::Color::Black);

    for (int player = 0; player < NUM_PLAYERS; ++player)
    {
        for (const auto &tokenPos : playerTokens[player])
        {
            int tokenCount = 0;
            for (int otherPlayer = 0; otherPlayer < NUM_PLAYERS; ++otherPlayer)
            {
                tokenCount += std::count(playerTokens[otherPlayer].begin(), playerTokens[otherPlayer].end(), tokenPos);
            }

            if (tokenCount > 1)
            {
                token.setRadius(TILE_SIZE / 4.f);
                star.setScale(0.5f, 0.5f);
            }
            else
            {
                token.setRadius(TILE_SIZE / 3.f);
                star.setScale(1.f, 1.f);
            }

            token.setFillColor(playerColors[player]);

            // Offset tokens if there are multiple tokens at the same position
            float offset = (tokenCount > 1) ? TILE_SIZE / 8.f : 0.f;
            for (int i = 0; i < tokenCount; ++i)
            {
                token.setPosition(tokenPos.y * TILE_SIZE + TILE_SIZE / 6.f + (i * offset),
                                  tokenPos.x * TILE_SIZE + TILE_SIZE / 6.f + (i * offset));
                star.setPosition(token.getPosition() + sf::Vector2f(token.getRadius() - radius / 4.f, token.getRadius() - radius / 4.f));
                window.draw(token);
                window.draw(star);
            }
        }
    }

    infoText.setString("Player " + std::to_string(currentPlayer + 1) +
                       " | Dice: " + std::to_string(diceValue) +
                       (diceRolled ? " | Click to move" : " | Click to roll"));
    window.draw(infoText);

    window.display();
}

void LudoGame::runGame()
{
    // simulate game play by itself
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
        }

        renderGame();
    }
}

void LudoGame::handleMouseClick(int x, int y)
{
    if (!diceRolled)
    {
        diceValue = rollDice();
        diceRolled = true;
    }
    else
    {
        int clickedRow = y / TILE_SIZE;
        int clickedCol = x / TILE_SIZE;

        for (int i = 0; i < MAX_TOKENS_PER_PLAYER; ++i)
        {
            if (playerTokens[currentPlayer][i].x == clickedRow &&
                playerTokens[currentPlayer][i].y == clickedCol)
            {
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

    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            cell.setPosition(i * TILE_SIZE, j * TILE_SIZE);

            // Quadrants
            if (i < 6 && j < 6) // Top-left red quadrant
                cell.setFillColor(red);
            else if (i > 8 && j < 6) // Top-right green quadrant
                cell.setFillColor(green);
            else if (i < 6 && j > 8) // Bottom-left yellow quadrant
                cell.setFillColor(yellow);
            else if (i > 8 && j > 8) // Bottom-right blue quadrant
                cell.setFillColor(blue);
            // Home paths
            else if (j == 7 && i >= 1 && i <= 6)
                cell.setFillColor(red);
            else if (i == 7 && j >= 1 && j <= 6)
                cell.setFillColor(green);
            else if (j == 7 && i >= 8 && i <= 13)
                cell.setFillColor(blue);
            else if (i == 7 && j >= 8 && j <= 13)
                cell.setFillColor(yellow);
            // Center triangle
            else if ((i == 7 && j == 7) || (i == 6 && j == 6) || (i == 6 && j == 8) || (i == 8 && j == 6) || (i == 8 && j == 8))
                cell.setFillColor(black);
            // Spawn and end cells
            else if (i == 1 && j == 6)
                cell.setFillColor(red);
            else if (i == 8 && j == 1)
                cell.setFillColor(green);
            else if (i == 13 && j == 8)
                cell.setFillColor(blue);
            else if (i == 6 && j == 13)
                cell.setFillColor(yellow);
            // End cells
            else if (i == 2 && j == 8)
                cell.setFillColor(red);
            else if (i == 6 && j == 2)
                cell.setFillColor(green);
            else if (i == 12 && j == 6)
                cell.setFillColor(blue);
            else if (i == 8 && j == 12)
                cell.setFillColor(yellow);
            // Default white cells
            else
                cell.setFillColor(white);

            window.draw(cell);
        }
    }
}

void LudoGame::simulateGameplay()
{
    while (window.isOpen())
{
    // simulate game play by itself where the player rolls the dice and moves the token randomly without any user input
    if (!diceRolled)
    {
        diceValue = rollDice();
        diceRolled = true;
    }
    else
    {
        int tokenIndex = 0;
        do
        {
            tokenIndex = std::uniform_int_distribution<>(0, MAX_TOKENS_PER_PLAYER - 1)(randomGenerator);
        } while (finishedPlayerTokens[currentPlayer][tokenIndex]); // Change condition to select tokens that have not finished their path
        
        moveToken(currentPlayer, tokenIndex);
    }

    renderGame();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
}
// explain the code:
// token: a circle shape with a star shape inside it.
// yard: the starting position of the token.
// safe zone: the pink colored path in the game.
// ludo path: the path that the token follows in the game.
// home column: the column where the token enters the home path.
// player: the player who is playing the game.
// dice: a random number generator that determines the number of steps the token moves.
// move token: a function that moves the token on the board based on the dice value.
// is token in yard: a function that checks if the token is in the yard.
// is safe zone: a function that checks if the token is in the safe zone.
// render game: a function that renders the game window.
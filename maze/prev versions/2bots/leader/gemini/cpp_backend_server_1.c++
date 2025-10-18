#include "crow_all.h" // A single include for the entire Crow library
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <string>

// Your original game logic and state struct remain exactly the same.
// This is the "engine" of your game, and it doesn't need to change.
// ===================================================================
using namespace std;
using namespace std::chrono;

const int WIDTH = 21;
const int HEIGHT = 21;

struct GameState {
    vector<vector<char>> maze;
    int playerX, playerY;
    int bot1X, bot1Y;
    int bot2X, bot2Y;
    int exitX, exitY;
    bool gameActive;
    steady_clock::time_point startTime;
    int timeLimit = 120;

    GameState() : maze(HEIGHT, vector<char>(WIDTH, '|')),
                  playerX(1), playerY(1), gameActive(false) {}
};

GameState game;

const vector<pair<int, int>> directions = {
    {-2, 0}, {2, 0}, {0, 2}, {0, -2}
};

bool isValid(int x, int y) {
    return x > 0 && x < HEIGHT - 1 && y > 0 && y < WIDTH - 1;
}

void carveMaze(int x, int y) {
    game.maze[x][y] = ' ';
    vector<pair<int, int>> dirs = directions;
    random_device rd;
    mt19937 g(rd());
    shuffle(dirs.begin(), dirs.end(), g);

    for (auto [dx, dy] : dirs) {
        int nx = x + dx;
        int ny = y + dy;

        if (isValid(nx, ny) && game.maze[nx][ny] == '|') {
            game.maze[x + dx / 2][y + dy / 2] = ' ';
            carveMaze(nx, ny);
        }
    }
}

void placeExit() {
    vector<pair<int, int>> possibleExits;

    for (int i = 1; i < WIDTH - 1; ++i) {
        if (game.maze[1][i] == ' ') possibleExits.push_back({0, i});
        if (game.maze[HEIGHT - 2][i] == ' ') possibleExits.push_back({HEIGHT - 1, i});
    }
    for (int i = 1; i < HEIGHT - 1; ++i) {
        if (game.maze[i][1] == ' ') possibleExits.push_back({i, 0});
        if (game.maze[i][WIDTH - 2] == ' ') possibleExits.push_back({i, WIDTH - 1});
    }

    if (!possibleExits.empty()) {
        random_device rd;
        mt19937 g(rd());
        uniform_int_distribution<> dis(0, possibleExits.size() - 1);
        tie(game.exitX, game.exitY) = possibleExits[dis(g)];
    }
}

void placeBots() {
    vector<pair<int, int>> emptyCells;
    for (int i = 1; i < HEIGHT - 1; ++i) {
        for (int j = 1; j < WIDTH - 1; ++j) {
            if (game.maze[i][j] == ' ' &&
                !(i == 1 && j == 1) &&
                !(i == game.exitX && j == game.exitY)) {
                emptyCells.push_back({i, j});
            }
        }
    }

    if (emptyCells.size() >= 2) {
        random_device rd;
        mt19937 g(rd());
        shuffle(emptyCells.begin(), emptyCells.end(), g);

        tie(game.bot1X, game.bot1Y) = emptyCells[0];
        tie(game.bot2X, game.bot2Y) = emptyCells[1];
    }
}

void moveBots() {
    if (!game.gameActive) return;

    vector<pair<int, int>> dirs = directions;
    random_device rd;
    mt19937 g(rd());

    shuffle(dirs.begin(), dirs.end(), g);
    for (auto [dx, dy] : dirs) {
        int nx = game.bot1X + dx / 2, ny = game.bot1Y + dy / 2;
        if (isValid(nx, ny) && game.maze[nx][ny] == ' ') {
            game.bot1X = nx; game.bot1Y = ny; break;
        }
    }
    
    shuffle(dirs.begin(), dirs.end(), g);
    for (auto [dx, dy] : dirs) {
        int nx = game.bot2X + dx / 2, ny = game.bot2Y + dy / 2;
        if (isValid(nx, ny) && game.maze[nx][ny] == ' ') {
            game.bot2X = nx; game.bot2Y = ny; break;
        }
    }
}

void initNewGame() {
    game.maze.assign(HEIGHT, vector<char>(WIDTH, '|'));
    game.playerX = 1; game.playerY = 1;
    game.gameActive = true;
    game.startTime = steady_clock::now();
    carveMaze(1, 1);
    placeExit();
    placeBots();
    cout << "New game started!" << endl;
}
// ===================================================================
// End of your original game logic

// This function is rewritten to use Crow's JSON tools.
// It's safer and cleaner than building a string by hand.
crow::json::wvalue getGameStateJson() {
    crow::json::wvalue state;

    // Build the maze as a list of lists
    vector<crow::json::wvalue> maze_rows;
    for (int i = 0; i < HEIGHT; ++i) {
        vector<crow::json::wvalue> row;
        for (int j = 0; j < WIDTH; ++j) {
            char cell = game.maze[i][j];
            if (i == game.playerX && j == game.playerY) cell = 'P';
            else if (i == game.bot1X && j == game.bot1Y && game.gameActive) cell = '1';
            else if (i == game.bot2X && j == game.bot2Y && game.gameActive) cell = '2';
            else if (i == game.exitX && j == game.exitY) cell = 'E';
            row.push_back(string(1, cell));
        }
        maze_rows.push_back(row);
    }
    state["maze"] = maze_rows;

    // Calculate time and check for game over
    auto now = steady_clock::now();
    int elapsed = duration_cast<seconds>(now - game.startTime).count();
    int remaining = max(0, game.timeLimit - elapsed);
    if (remaining <= 0 && game.gameActive) {
        game.gameActive = false;
    }
    
    // Add all other game state variables
    state["timeRemaining"] = remaining;
    state["playerX"] = game.playerX;
    state["playerY"] = game.playerY;
    state["bot1X"] = game.bot1X;
    state["bot1Y"] = game.bot1Y;
    state["bot2X"] = game.bot2X;
    state["bot2Y"] = game.bot2Y;
    state["exitX"] = game.exitX;
    state["exitY"] = game.exitY;
    state["gameActive"] = game.gameActive;

    return state;
}


int main() {
    // Create the Crow application object
    crow::SimpleApp app;

    // The "/newgame" route
    CROW_ROUTE(app, "/newgame")([]() {
        initNewGame();
        return getGameStateJson();
    });

    // The "/state" route
    CROW_ROUTE(app, "/state")([]() {
        return getGameStateJson();
    });

    // The "/move" route
    CROW_ROUTE(app, "/move").methods("POST"_method)([](const crow::request& req) {
        if (!game.gameActive) {
            return getGameStateJson();
        }

        // Crow automatically parses the JSON body
        auto data = crow::json::load(req.body);
        if (!data) {
            return crow::response(400); // Bad Request
        }

        // Safely get the move character
        string move_str = data["move"].s();
        if (move_str.empty()) {
            return crow::response(400); // Bad Request
        }
        char move = move_str[0];

        // Your original move logic
        int newX = game.playerX, newY = game.playerY;
        switch (move) {
            case 'w': case 'W': newX--; break;
            case 'a': case 'A': newY--; break;
            case 's': case 'S': newX++; break;
            case 'd': case 'D': newY++; break;
            case 'q': case 'Q':
                game.gameActive = false;
                return getGameStateJson();
        }

        if (newX >= 0 && newX < HEIGHT && newY >= 0 && newY < WIDTH) {
            if (game.maze[newX][newY] == ' ' || (newX == game.exitX && newY == game.exitY)) {
                game.playerX = newX;
                game.playerY = newY;

                if (game.playerX == game.exitX && game.playerY == game.exitY) {
                    game.gameActive = false;
                } else {
                    moveBots();
                    if ((game.bot1X == game.playerX && game.bot1Y == game.playerY) ||
                        (game.bot2X == game.playerX && game.bot2Y == game.playerY)) {
                        game.gameActive = false;
                    }
                }
            }
        }
        
        return getGameStateJson();
    });

    // Set the port and run the server (with multiple threads)
    app.port(8080).multithreaded().run();
}

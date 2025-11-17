#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <string>
#include <sstream>
#include <ctime>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <signal.h>
#endif

using namespace std;
using namespace std::chrono;

const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;

struct Position {
    int x, y;
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Score {
    string name;
    int score;
};

struct GameState {
    vector<Position> snake;
    Position food;
    string direction;
    string nextDirection;
    int score;
    bool gameActive;
    string playerName;
    int highScore;
    steady_clock::time_point lastMoveTime;
    int moveDelay = 200; // milliseconds
};

GameState game;
vector<Score> leaderboard;
mt19937 rng(time(0));

void spawnFood() {
    vector<Position> emptySpaces;
    
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            Position pos = {x, y};
            bool occupied = false;
            
            for (const auto& segment : game.snake) {
                if (segment == pos) {
                    occupied = true;
                    break;
                }
            }
            
            if (!occupied) {
                emptySpaces.push_back(pos);
            }
        }
    }
    
    if (!emptySpaces.empty()) {
        uniform_int_distribution<> dis(0, emptySpaces.size() - 1);
        game.food = emptySpaces[dis(rng)];
    }
}

void initNewGame() {
    game.snake.clear();
    game.snake.push_back({10, 10}); // Head in center
    game.snake.push_back({9, 10});  // Body segment
    game.snake.push_back({8, 10});  // Body segment
    
    game.direction = "right";
    game.nextDirection = "right";
    game.score = 0;
    game.gameActive = true;
    game.lastMoveTime = steady_clock::now();
    
    spawnFood();
    
    cout << "New game started for player: " << game.playerName << endl;
    cout << "Snake spawned at (10, 10)" << endl;
    cout << "Food spawned at (" << game.food.x << ", " << game.food.y << ")" << endl;
}

void moveSnake() {
    auto now = steady_clock::now();
    auto elapsed = duration_cast<milliseconds>(now - game.lastMoveTime).count();
    
    if (elapsed < game.moveDelay || !game.gameActive) {
        return;
    }
    
    game.lastMoveTime = now;
    game.direction = game.nextDirection;
    
    Position newHead = game.snake[0];
    
    if (game.direction == "up") newHead.y--;
    else if (game.direction == "down") newHead.y++;
    else if (game.direction == "left") newHead.x--;
    else if (game.direction == "right") newHead.x++;
    
    // Check wall collision
    if (newHead.x < 0 || newHead.x >= GRID_WIDTH || 
        newHead.y < 0 || newHead.y >= GRID_HEIGHT) {
        game.gameActive = false;
        cout << "Player " << game.playerName << " hit a wall! Game Over." << endl;
        
        // Update leaderboard
        if (leaderboard.size() < 10 || game.score > leaderboard.back().score) {
            if (leaderboard.size() == 10) {
                leaderboard.pop_back();
            }
            leaderboard.push_back({game.playerName, game.score});
            sort(leaderboard.begin(), leaderboard.end(), [](const Score& a, const Score& b) {
                return a.score > b.score;
            });
            cout << "New high score for " << game.playerName << " with score " << game.score << "!" << endl;
        }
        return;
    }
    
    // Check self collision
    for (const auto& segment : game.snake) {
        if (newHead == segment) {
            game.gameActive = false;
            cout << "Player " << game.playerName << " collided with themselves! Game Over." << endl;
            
            // Update leaderboard
            if (leaderboard.size() < 10 || game.score > leaderboard.back().score) {
                if (leaderboard.size() == 10) {
                    leaderboard.pop_back();
                }
                leaderboard.push_back({game.playerName, game.score});
                sort(leaderboard.begin(), leaderboard.end(), [](const Score& a, const Score& b) {
                    return a.score > b.score;
                });
                cout << "New high score for " << game.playerName << " with score " << game.score << "!" << endl;
            }
            return;
        }
    }
    
    game.snake.insert(game.snake.begin(), newHead);
    
    // Check food collision
    if (newHead == game.food) {
        game.score++;
        if (game.score > game.highScore) {
            game.highScore = game.score;
        }
        cout << "Food eaten! Score: " << game.score << endl;
        spawnFood();
        
        // Speed up slightly
        if (game.moveDelay > 50) {
            game.moveDelay = max(50, game.moveDelay - 5);
        }
    } else {
        game.snake.pop_back();
    }
}

string getGameStateJson() {
    moveSnake();
    
    string json = "{";
    json += "\"snake\":[";
    for (size_t i = 0; i < game.snake.size(); i++) {
        json += "{\"x\":" + to_string(game.snake[i].x) + ",\"y\":" + to_string(game.snake[i].y) + "}";
        if (i < game.snake.size() - 1) json += ",";
    }
    json += "],";
    json += "\"food\":{\"x\":" + to_string(game.food.x) + ",\"y\":" + to_string(game.food.y) + "},";
    json += "\"score\":" + to_string(game.score) + ",";
    json += "\"highScore\":" + to_string(game.highScore) + ",";
    json += "\"gameActive\":" + string(game.gameActive ? "true" : "false");
    json += "}";
    
    return json;
}

string getLeaderboardJson() {
    string json = "[";
    for (size_t i = 0; i < leaderboard.size(); ++i) {
        json += "{";
        json += "\"name\":\"" + leaderboard[i].name + "\",";
        json += "\"score\":" + to_string(leaderboard[i].score);
        json += "}";
        if (i < leaderboard.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    return json;
}

void setDirection(const string& dir) {
    // Prevent 180-degree turns
    if (dir == "up" && game.direction != "down") game.nextDirection = dir;
    else if (dir == "down" && game.direction != "up") game.nextDirection = dir;
    else if (dir == "left" && game.direction != "right") game.nextDirection = dir;
    else if (dir == "right" && game.direction != "left") game.nextDirection = dir;
}

string handleRequest(const string& request) {
    string headers = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\n\r\n";
    
    if (request.find("OPTIONS") == 0) return headers + "{}";

    if (request.find("POST /newgame") != string::npos) {
        size_t bodyPos = request.find("\r\n\r\n");
        if (bodyPos != string::npos) {
            string body = request.substr(bodyPos + 4);
            size_t namePos = body.find("\"name\":\"");
            if (namePos != string::npos) {
                size_t start = namePos + 8;
                size_t end = body.find("\"", start);
                if (end != string::npos) {
                    game.playerName = body.substr(start, end - start);
                } else {
                    game.playerName = "Anonymous";
                }
            } else {
                game.playerName = "Anonymous";
            }
        }
        
        // Get high score from leaderboard
        game.highScore = 0;
        if (!leaderboard.empty()) {
            game.highScore = leaderboard[0].score;
        }
        
        initNewGame();
        return headers + getGameStateJson();
    }
    
    if (request.find("POST /move") != string::npos) {
        size_t bodyPos = request.find("\r\n\r\n");
        if (bodyPos != string::npos) {
            string body = request.substr(bodyPos + 4);
            size_t dirPos = body.find("\"direction\":\"");
            if (dirPos != string::npos) {
                size_t start = dirPos + 13;
                size_t end = body.find("\"", start);
                if (end != string::npos) {
                    string direction = body.substr(start, end - start);
                    setDirection(direction);
                    cout << "Direction changed to: " << direction << endl;
                }
            }
        }
        return headers + "{}";
    }
    
    if (request.find("GET /state") != string::npos) {
        return headers + getGameStateJson();
    }
    
    if (request.find("GET /leaderboard") != string::npos) {
        return headers + getLeaderboardJson();
    }

    return "HTTP/1.1 404 Not Found\r\n\r\n";
}

int main() {
    cout << "=== Snake Game Server ===" << endl;
    cout << "Initializing..." << endl;
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed!" << endl;
        return 1;
    }
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cout << "Socket creation failed!" << endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    if (bind(serverSocket, (sockaddr*)&address, sizeof(address)) < 0) {
        cout << "Bind failed! Port might already be in use." << endl;
        return 1;
    }
    
    if (listen(serverSocket, 3) < 0) {
        cout << "Listen failed!" << endl;
        return 1;
    }
    
    cout << "?? Snake Game server running on http://localhost:8080" << endl;
    cout << "Game Rules:" << endl;
    cout << "  - Use Arrow Keys or WASD to control the snake" << endl;
    cout << "  - Eat food to grow and score points" << endl;
    cout << "  - Don't hit walls or yourself!" << endl;
    cout << "Waiting for connections..." << endl;
    
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            char buffer[4096] = {};
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytesRead > 0) {
                string request(buffer, bytesRead);
                string response = handleRequest(request);
                send(clientSocket, response.c_str(), response.length(), 0);
            }
            
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
        }
    }

#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif

    return 0;
}

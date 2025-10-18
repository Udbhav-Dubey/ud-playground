#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
#include <ctime>
#include <queue>
#include <cmath>
#include <climits>
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

// Data structure for a single leaderboard entry
struct Score {
    string name;
    int timeTaken;
};

const int WIDTH = 21;
const int HEIGHT = 21;

struct GameState {
    vector<vector<char>> maze;
    int playerX, playerY;
    int bot1X, bot1Y;
    int bot2X, bot2Y;
    int exitX, exitY;
    int keyX, keyY;          // Key position
    bool hasKey;             // Player has collected the key
    bool keyCollected;       // Key has been collected (for rendering)
    bool gameActive;
    steady_clock::time_point startTime;
    int timeLimit = 60;
    string playerName;
};

GameState game;
vector<Score> leaderboard;
mt19937 rng(time(0));

// Maze generation using recursive backtracking
void carveMaze(int x, int y) {
    game.maze[x][y] = ' ';
    
    vector<pair<int, int>> directions = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}};
    shuffle(directions.begin(), directions.end(), rng);
    
    for (auto [dx, dy] : directions) {
        int nx = x + dx, ny = y + dy;
        int wx = x + dx/2, wy = y + dy/2;
        
        if (nx > 0 && nx < HEIGHT-1 && ny > 0 && ny < WIDTH-1 && game.maze[nx][ny] == '|') {
            game.maze[wx][wy] = ' ';
            carveMaze(nx, ny);
        }
    }
}

// Place the exit (door) in a random location far from start
void placeExit() {
    vector<pair<int, int>> emptySpaces;
    for (int i = 1; i < HEIGHT-1; i++) {
        for (int j = 1; j < WIDTH-1; j++) {
            if (game.maze[i][j] == ' ') {
                // Ensure exit is reasonably far from start (1,1)
                int distance = abs(i - 1) + abs(j - 1);
                if (distance > 10) { // Minimum distance from start
                    emptySpaces.push_back({i, j});
                }
            }
        }
    }
    
    if (!emptySpaces.empty()) {
        uniform_int_distribution<> dis(0, emptySpaces.size() - 1);
        auto [x, y] = emptySpaces[dis(rng)];
        game.exitX = x;
        game.exitY = y;
    } else {
        // Fallback: place at bottom right area
        game.exitX = HEIGHT - 2;
        game.exitY = WIDTH - 2;
    }
}

// Place the key in a random empty location
void placeKey() {
    vector<pair<int, int>> emptySpaces;
    for (int i = 1; i < HEIGHT-1; i++) {
        for (int j = 1; j < WIDTH-1; j++) {
            if (game.maze[i][j] == ' ' && 
                !(i == 1 && j == 1) &&                    // Not at player start
                !(i == game.exitX && j == game.exitY)) {  // Not at exit
                emptySpaces.push_back({i, j});
            }
        }
    }
    
    if (!emptySpaces.empty()) {
        uniform_int_distribution<> dis(0, emptySpaces.size() - 1);
        auto [x, y] = emptySpaces[dis(rng)];
        game.keyX = x;
        game.keyY = y;
    } else {
        // Fallback: place somewhere in the middle
        game.keyX = HEIGHT / 2;
        game.keyY = WIDTH / 2;
    }
    game.hasKey = false;
    game.keyCollected = false;
}

// Place bots in random empty locations
void placeBots() {
    vector<pair<int, int>> emptySpaces;
    for (int i = 1; i < HEIGHT-1; i++) {
        for (int j = 1; j < WIDTH-1; j++) {
            if (game.maze[i][j] == ' ' && 
                !(i == 1 && j == 1) && 
                !(i == game.exitX && j == game.exitY) &&
                !(i == game.keyX && j == game.keyY)) {    // Not at key location
                emptySpaces.push_back({i, j});
            }
        }
    }
    
    if (emptySpaces.size() >= 2) {
        shuffle(emptySpaces.begin(), emptySpaces.end(), rng);
        game.bot1X = emptySpaces[0].first;
        game.bot1Y = emptySpaces[0].second;
        game.bot2X = emptySpaces[1].first;
        game.bot2Y = emptySpaces[1].second;
    }
}

// Check if a position is valid for movement
bool isValidPosition(int x, int y) {
    return x >= 0 && x < HEIGHT && y >= 0 && y < WIDTH && 
           (game.maze[x][y] == ' ' || (x == game.exitX && y == game.exitY));
}

void moveBots() {
    auto moveBot = [](int& botX, int& botY) {
        
        int distanceToPlayer = abs(botX - game.playerX) + abs(botY - game.playerY);
        

        if (distanceToPlayer <= 2) {
            uniform_int_distribution<> catchChance(1, 100);
            if (catchChance(rng) <= 90) {
                
                cout << "Bot caught the player!" << endl;
                return;
            }
        }
        
        vector<pair<int, int>> validMoves;
        vector<pair<int, int>> directions = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}};
        
        for (auto [dx, dy] : directions) {
            int newX = botX + dx, newY = botY + dy;
            if (isValidPosition(newX, newY)) {
                validMoves.push_back({newX, newY});
            }
        }
        
        if (!validMoves.empty()) {
            uniform_int_distribution<> dis(0, validMoves.size() - 1);
            auto [newX, newY] = validMoves[dis(rng)];
            botX = newX;
            botY = newY;
        }
    };
    
    moveBot(game.bot1X, game.bot1Y);
    moveBot(game.bot2X, game.bot2Y);
}
void initNewGame() {
    game.maze.assign(HEIGHT, vector<char>(WIDTH, '|'));
    game.playerX = 1; 
    game.playerY = 1;
    game.gameActive = true;
    game.startTime = steady_clock::now();
    
    carveMaze(1, 1);
    placeExit();
    placeKey();    // Place the key
    placeBots();
    
    cout << "New game started for player: " << game.playerName << endl;
    cout << "Key placed at: (" << game.keyX << ", " << game.keyY << ")" << endl;
    cout << "Exit (door) placed at: (" << game.exitX << ", " << game.exitY << ")" << endl;
    cout << "Bot1 at: (" << game.bot1X << ", " << game.bot1Y << ")" << endl;
    cout << "Bot2 at: (" << game.bot2X << ", " << game.bot2Y << ")" << endl;
}

string getMazeJson() {
    string json = "{\"maze\":[";
    
    for (int i = 0; i < HEIGHT; ++i) {
        json += "[";
        for (int j = 0; j < WIDTH; ++j) {
            char cell = game.maze[i][j];
            
            // Player has highest priority
            if (i == game.playerX && j == game.playerY) {
                cell = 'P';
            }
            // Then bots (only if game is active)
            else if (i == game.bot1X && j == game.bot1Y && game.gameActive) {
                cell = '1';
            }
            else if (i == game.bot2X && j == game.bot2Y && game.gameActive) {
                cell = '2';
            }
            // Key (only if not collected)
            else if (i == game.keyX && j == game.keyY && !game.keyCollected) {
                cell = 'K';  // K for Key
            }
            // Exit/Door
            else if (i == game.exitX && j == game.exitY) {
                cell = game.hasKey ? 'E' : 'D';  // D for locked Door, E for unlocked Exit
            }
            
            json += "\"" + string(1, cell) + "\"";
            if (j < WIDTH - 1) json += ",";
        }
        json += "]";
        if (i < HEIGHT - 1) json += ",";
    }
    
    auto now = steady_clock::now();
    int elapsed = duration_cast<seconds>(now - game.startTime).count();
    int remaining = max(0, game.timeLimit - elapsed);
    if (remaining <= 0 && game.gameActive) {
        game.gameActive = false;
        cout << "Game ended: Time's up!" << endl;
    }
    
    json += "],";
    json += "\"timeRemaining\":" + to_string(remaining) + ",";
    json += "\"playerX\":" + to_string(game.playerX) + ",";
    json += "\"playerY\":" + to_string(game.playerY) + ",";
    json += "\"bot1X\":" + to_string(game.bot1X) + ",";
    json += "\"bot1Y\":" + to_string(game.bot1Y) + ",";
    json += "\"bot2X\":" + to_string(game.bot2X) + ",";
    json += "\"bot2Y\":" + to_string(game.bot2Y) + ",";
    json += "\"exitX\":" + to_string(game.exitX) + ",";
    json += "\"exitY\":" + to_string(game.exitY) + ",";
    json += "\"keyX\":" + to_string(game.keyX) + ",";
    json += "\"keyY\":" + to_string(game.keyY) + ",";
    json += "\"hasKey\":" + string(game.hasKey ? "true" : "false") + ",";
    json += "\"keyCollected\":" + string(game.keyCollected ? "true" : "false") + ",";
    json += "\"gameActive\":" + string(game.gameActive ? "true" : "false");
    json += "}";
    
    return json;
}

string getLeaderboardJson() {
    string json = "[";
    for (size_t i = 0; i < leaderboard.size(); ++i) {
        json += "{";
        json += "\"name\":\"" + leaderboard[i].name + "\",";
        json += "\"time\":" + to_string(leaderboard[i].timeTaken);
        json += "}";
        if (i < leaderboard.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    return json;
}

string handleRequest(const string& request) {
    string headers = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\n\r\n";
    
    cout << "Request: " << request.substr(0, 50) << "..." << endl;
    
    if (request.find("OPTIONS") == 0) return headers + "{}";

    if (request.find("POST /newgame") != string::npos) {
        size_t bodyPos = request.find("\r\n\r\n");
        if (bodyPos != string::npos) {
            string body = request.substr(bodyPos + 4);
            size_t namePos = body.find("\"name\":\"");
            if(namePos != string::npos){
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
        initNewGame();
        return headers + getMazeJson();
    }
    
    if (request.find("POST /move") != string::npos) {
        if (!game.gameActive) return headers + getMazeJson();
        
        size_t bodyPos = request.find("\r\n\r\n");
        if (bodyPos != string::npos) {
            string body = request.substr(bodyPos + 4);
            size_t movePos = body.find("\"move\":\"");
            if (movePos != string::npos && movePos + 8 < body.length()) {
                char move = body[movePos + 8];
                
                int newX = game.playerX, newY = game.playerY;
                
                switch (move) {
                    case 'w': newX--; break;
                    case 'a': newY--; break;
                    case 's': newX++; break;
                    case 'd': newY++; break;
                    case 'e': 
                        game.gameActive = false; 
                        cout << "Player " << game.playerName << " quit the game" << endl;
                        return headers + getMazeJson();
                }
                
                // Improved movement validation
                if (isValidPosition(newX, newY)) {
                    game.playerX = newX;
                    game.playerY = newY;
                    cout << "Player moved to: (" << game.playerX << ", " << game.playerY << ")" << endl;
                    
                    // Check if player collected the key
                    if (game.playerX == game.keyX && game.playerY == game.keyY && !game.keyCollected) {
                        game.hasKey = true;
                        game.keyCollected = true;
                        cout << "Player " << game.playerName << " collected the key! Door is now unlocked." << endl;
                    }
                    
                    // Check win condition - player must have key and be at exit
                    if (game.playerX == game.exitX && game.playerY == game.exitY) {
                        if (game.hasKey) {
                            game.gameActive = false;
                            cout << "Player " << game.playerName << " unlocked the door and won!" << endl;

                            auto now = steady_clock::now();
                            int timeTaken = duration_cast<seconds>(now - game.startTime).count();

                            // Update leaderboard if this is a top 10 score
                            if (leaderboard.size() < 10 || timeTaken < leaderboard.back().timeTaken) {
                                if (leaderboard.size() == 10) {
                                    leaderboard.pop_back();
                                }
                                leaderboard.push_back({game.playerName, timeTaken});
                                sort(leaderboard.begin(), leaderboard.end(), [](const Score& a, const Score& b) {
                                    return a.timeTaken < b.timeTaken;
                                });
                                cout << "New high score for " << game.playerName << " with time " << timeTaken << "s!" << endl;
                            }
                        } else {
                            cout << "Player " << game.playerName << " reached the door but doesn't have the key!" << endl;
                            // Player can stand on the door but can't win without the key
                        }
                    } else {
                        // Move bots and check collisions (only if not at exit)
                        moveBots();
                        cout << "Bots moved to: Bot1(" << game.bot1X << ", " << game.bot1Y << "), Bot2(" << game.bot2X << ", " << game.bot2Y << ")" << endl;
                        
                        if ((game.bot1X == game.playerX && game.bot1Y == game.playerY) ||
                            (game.bot2X == game.playerX && game.bot2Y == game.playerY)) {
                            game.gameActive = false;
                            cout << "Player " << game.playerName << " was caught by a bot!" << endl;
                        }
                    }
                } else {
                    cout << "Invalid move attempted: (" << newX << ", " << newY << ")" << endl;
                }
            }
        }
        return headers + getMazeJson();
    }
    
    if (request.find("GET /state") != string::npos) {
        return headers + getMazeJson();
    }
    
    if (request.find("GET /leaderboard") != string::npos) {
        return headers + getLeaderboardJson();
    }

    return "HTTP/1.1 404 Not Found\r\n\r\n";
}

int main() {
    cout << "=== Maze Chase Game with Key-Door Mechanic ===" << endl;
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
    
    cout << "🚀 Maze server with Key-Door mechanic running on http://localhost:8080" << endl;
    cout << "Game Rules:" << endl;
    cout << "  - Find the Key (K) first" << endl;
    cout << "  - Then go to the Door (D) to unlock it and win" << endl;
    cout << "  - Door becomes Exit (E) when you have the key" << endl;
    cout << "  - Bots move randomly (2 steps at a time)" << endl;
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

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
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
    bool gameActive;
    steady_clock::time_point startTime;
    int timeLimit = 120;
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

// Place the exit in a random location
void placeExit() {
    vector<pair<int, int>> emptySpaces;
    for (int i = 1; i < HEIGHT-1; i++) {
        for (int j = 1; j < WIDTH-1; j++) {
            if (game.maze[i][j] == ' ' && !(i == 1 && j == 1)) {
                emptySpaces.push_back({i, j});
            }
        }
    }
    
    if (!emptySpaces.empty()) {
        uniform_int_distribution<> dis(0, emptySpaces.size() - 1);
        auto [x, y] = emptySpaces[dis(rng)];
        game.exitX = x;
        game.exitY = y;
    }
}

// Place bots in random empty locations
void placeBots() {
    vector<pair<int, int>> emptySpaces;
    for (int i = 1; i < HEIGHT-1; i++) {
        for (int j = 1; j < WIDTH-1; j++) {
            if (game.maze[i][j] == ' ' && !(i == 1 && j == 1) && !(i == game.exitX && j == game.exitY)) {
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

// Simple bot movement AI - move randomly in valid directions
void moveBots() {
    auto moveBot = [](int& botX, int& botY) {
        vector<pair<int, int>> validMoves;
        vector<pair<int, int>> directions = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}};
        
        for (auto [dx, dy] : directions) {
            int newX = botX + dx, newY = botY + dy;
            if (newX >= 0 && newX < HEIGHT && newY >= 0 && newY < WIDTH && 
                game.maze[newX][newY] == ' ') {
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
    placeBots();
    
    cout << "New game started for player: " << game.playerName << endl;
}

string getMazeJson() {
    string json = "{\"maze\":[";
    
    for (int i = 0; i < HEIGHT; ++i) {
        json += "[";
        for (int j = 0; j < WIDTH; ++j) {
            char cell = game.maze[i][j];
            if (i == game.playerX && j == game.playerY) cell = 'P';
            else if (i == game.bot1X && j == game.bot1Y && game.gameActive) cell = '1';
            else if (i == game.bot2X && j == game.bot2Y && game.gameActive) cell = '2';
            else if (i == game.exitX && j == game.exitY) cell = 'E';
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
                    case 'e': game.gameActive = false; return headers + getMazeJson();
                }
                
                if (newX >= 0 && newX < HEIGHT && newY >= 0 && newY < WIDTH) {
                    if (game.maze[newX][newY] == ' ' || (newX == game.exitX && newY == game.exitY)) {
                        game.playerX = newX;
                        game.playerY = newY;
                        
                        // Win condition logic
                        if (game.playerX == game.exitX && game.playerY == game.exitY) {
                            game.gameActive = false;
                            cout << "Player " << game.playerName << " won!" << endl;

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
                            // Move bots and check collisions
                            moveBots();
                            if ((game.bot1X == game.playerX && game.bot1Y == game.playerY) ||
                                (game.bot2X == game.playerX && game.bot2Y == game.playerY)) {
                                game.gameActive = false;
                                cout << "Player " << game.playerName << " was caught by a bot!" << endl;
                            }
                        }
                    }
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
    srand(time(0));
    
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
        cout << "Bind failed!" << endl;
        return 1;
    }
    
    if (listen(serverSocket, 3) < 0) {
        cout << "Listen failed!" << endl;
        return 1;
    }
    
    cout << "🚀 Maze server running on http://localhost:8080" << endl;
    
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

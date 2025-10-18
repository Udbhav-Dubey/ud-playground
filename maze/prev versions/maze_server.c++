#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
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

// Your original game logic - UNTOUCHED!
const int width = 21;
const int height = 21;
vector<vector<char>> maze(height, vector<char>(width, '|'));

const vector<pair<int,int>> directions = {
    {-2,0}, // North
    {2,0},  // South
    {0,2},  // east
    {0,-2}  // west
};

bool isValid(int x, int y) {
    return x > 0 && x < height - 1 && y > 0 && y < width - 1;
}

int exitX, exitY;
int botX, botY;
int playerX = 1, playerY = 1;
auto gameStartTime = steady_clock::now();
bool gameActive = false;
const int time_limit = 120;

void carve_maze(int x, int y) {
    maze[x][y] = ' ';
    vector<pair<int,int>> dirs = directions;
    random_device rd;
    mt19937 g(rd());
    shuffle(dirs.begin(), dirs.end(), g);
    
    for (auto[dx,dy] : dirs) {
        int nx = x + dx;
        int ny = y + dy;
        
        if (isValid(nx, ny) && maze[nx][ny] == '|') {
            maze[x + dx/2][y + dy/2] = ' ';
            carve_maze(nx, ny);
        }
    }
}

void exit_E() {
    vector<pair<int,int>> possible_exits;
    for (int i = 1; i < width - 1; ++i) {
        if (maze[1][i] == ' ') { possible_exits.emplace_back(0, i); }
        if (maze[height - 2][i] == ' ') possible_exits.emplace_back(height - 1, i);
    }
    for (int i = 1; i < height - 1; ++i) {
        if (maze[i][1] == ' ') { possible_exits.emplace_back(i, 0); }
        if (maze[i][width - 2] == ' ') { possible_exits.emplace_back(i, width - 1); }
    }
    
    if (!possible_exits.empty()) {
        random_device rd;
        mt19937 g(rd());
        uniform_int_distribution<> dis(0, possible_exits.size() - 1);
        auto [ex, ey] = possible_exits[dis(g)];
        exitX = ex;
        exitY = ey;
        maze[exitX][exitY] = 'E';
    }
}

void place_bot() {
    vector<pair<int,int>> empty_cells;
    for (int i = 1; i < height - 1; ++i) {
        for (int j = 1; j < width - 1; ++j) {
            if (maze[i][j] == ' ' && !(i == 1 && j == 1) && !(i == exitX && j == exitY)) {
                empty_cells.emplace_back(i, j);
            }
        }
    }
    
    if (empty_cells.empty()) return;
    
    random_device rd;
    mt19937 g(rd());
    uniform_int_distribution<> dis(0, empty_cells.size() - 1);
    auto[bx, by] = empty_cells[dis(g)];
    botX = bx;
    botY = by;
    maze[botX][botY] = 'B';
}

void move_bot() {
    vector<pair<int,int>> dirs = directions;
    random_device rd;
    mt19937 g(rd());
    shuffle(dirs.begin(), dirs.end(), g);
    
    for (auto [dx, dy] : dirs) {
        int nx = botX + dx;
        int ny = botY + dy;
        if (nx >= 0 && nx < height && ny >= 0 && ny < width) {
            if (maze[nx][ny] != '|' && maze[nx][ny] != 'E') {
                if (!(botX == 1 && botY == 1)) { maze[botX][botY] = ' '; }
                botX = nx;
                botY = ny;
                maze[botX][botY] = 'B';
                break;
            }
        }
    }
}

// New HTTP server stuff
string getMazeAsJson() {
    string json = "{\"maze\":[";
    for (int i = 0; i < height; ++i) {
        json += "[";
        for (int j = 0; j < width; ++j) {
            char cell = maze[i][j];
            if (i == playerX && j == playerY) cell = 'P';
            json += "\"" + string(1, cell) + "\"";
            if (j < width - 1) json += ",";
        }
        json += "]";
        if (i < height - 1) json += ",";
    }
    json += "],";
    
    auto now = steady_clock::now();
    int elapsed = duration_cast<seconds>(now - gameStartTime).count();
    int remaining = time_limit - elapsed;
    
    json += "\"timeRemaining\":" + to_string(max(0, remaining)) + ",";
    json += "\"playerX\":" + to_string(playerX) + ",";
    json += "\"playerY\":" + to_string(playerY) + ",";
    json += "\"botX\":" + to_string(botX) + ",";
    json += "\"botY\":" + to_string(botY) + ",";
    json += "\"exitX\":" + to_string(exitX) + ",";
    json += "\"exitY\":" + to_string(exitY) + ",";
    json += "\"gameActive\":" + string(gameActive ? "true" : "false");
    json += "}";
    
    return json;
}

string handleRequest(const string& request) {
    string response;
    string headers = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\nAccess-Control-Max-Age: 86400\r\n\r\n";
    
    // Handle preflight OPTIONS request
    if (request.find("OPTIONS") == 0) {
        return headers + "{}";
    }
    
    if (request.find("GET /newgame") != string::npos) {
        // Reset maze
        maze.assign(height, vector<char>(width, '|'));
        playerX = 1; playerY = 1;
        gameStartTime = steady_clock::now();
        gameActive = true;
        
        carve_maze(1, 1);
        exit_E();
        place_bot();
        
        response = headers + getMazeAsJson();
    }
    else if (request.find("POST /move") != string::npos) {
        if (!gameActive) {
            response = headers + getMazeAsJson();
            return response;
        }
        
        // Check time
        auto now = steady_clock::now();
        int elapsed = duration_cast<seconds>(now - gameStartTime).count();
        if (elapsed >= time_limit) {
            gameActive = false;
            response = headers + getMazeAsJson();
            return response;
        }
        
        // Extract move from POST body
        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != string::npos) {
            string body = request.substr(bodyStart + 4);
            size_t moveStart = body.find("\"move\":\"");
            if (moveStart != string::npos) {
                char move = body[moveStart + 8];
                
                int posx = playerX, posy = playerY;
                if (move == 'w' || move == 'W') posx--;
                else if (move == 'a' || move == 'A') posy--;
                else if (move == 's' || move == 'S') posx++;
                else if (move == 'd' || move == 'D') posy++;
                else if (move == 'q' || move == 'Q') {
                    gameActive = false;
                    response = headers + getMazeAsJson();
                    return response;
                }
                
                if (maze[posx][posy] == ' ' || maze[posx][posy] == 'E') {
                    playerX = posx;
                    playerY = posy;
                    
                    if (playerX == exitX && playerY == exitY) {
                        gameActive = false;
                        response = headers + getMazeAsJson();
                        return response;
                    }
                    
                    move_bot();
                    
                    if (botX == playerX && botY == playerY) {
                        gameActive = false;
                    }
                }
            }
        }
        
        response = headers + getMazeAsJson();
    }
    else if (request.find("GET /state") != string::npos) {
        response = headers + getMazeAsJson();
    }
    else {
        response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }
    
    return response;
}

int main() {
    srand(time(0));
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }
    
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
#else
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
#endif
        perror("setsockopt failed");
        return 1;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        return 1;
    }
    
    cout << "🚀 Maze game server running on http://localhost:8080" << endl;
    cout << "Press Ctrl+C to stop" << endl;
    cout << "Test with: curl http://localhost:8080/newgame" << endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }
        
        char buffer[4096] = {0};
        int bytes_read = recv(client_socket, buffer, 4096, 0);
        if (bytes_read > 0) {
            string request(buffer);
            string response = handleRequest(request);
            send(client_socket, response.c_str(), response.length(), 0);
        }
        
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
    }
    
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return 0;
}

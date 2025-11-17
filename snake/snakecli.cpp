#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <string>
#include <ctime>
#include <thread>

#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>
#else
  #include <termios.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

using namespace std;
using namespace std::chrono;

static const int GRID_WIDTH  = 20;
static const int GRID_HEIGHT = 20;

struct Position {
    int x, y;
    bool operator==(const Position& o) const { return x==o.x && y==o.y; }
};

struct GameState {
    vector<Position> snake;
    Position food{};
    string direction = "right";
    string nextDirection = "right";
    int score = 0;
    bool gameActive = true;
    int highScore = 0;
    steady_clock::time_point lastMoveTime;
    int moveDelay = 200;  // ms
    string playerName = "Player";
};

static GameState game;
static mt19937 rng((uint32_t)time(nullptr));

// ----------------- Platform helpers -----------------
#ifndef _WIN32
struct TermRaw {
    termios orig{};
    bool ok = false;
    TermRaw() {
        if (tcgetattr(STDIN_FILENO, &orig) == 0) {
            termios raw = orig;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN]  = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
            // non-blocking stdin
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
            ok = true;
        }
    }
    ~TermRaw() {
        if (ok) tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    }
};
#endif

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    this_thread::sleep_for(chrono::milliseconds(ms));
#endif
}

// Read one key if available. Returns empty string if none.
// Normalizes to "up","down","left","right","w","a","s","d","q"
static string poll_key() {
#ifdef _WIN32
    if (!_kbhit()) return "";
    int c = _getch();
    if (c == 0 || c == 224) {  // arrows
        int c2 = _getch();
        switch (c2) {
            case 72: return "up";
            case 80: return "down";
            case 75: return "left";
            case 77: return "right";
            default: return "";
        }
    }
    char ch = (char)c;
    if (ch=='w'||ch=='W') return "w";
    if (ch=='a'||ch=='A') return "a";
    if (ch=='s'||ch=='S') return "s";
    if (ch=='d'||ch=='D') return "d";
    if (ch=='q'||ch=='Q') return "q";
    return "";
#else
    char buf[8];
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0) return "";
    // Handle escape sequences for arrows: ESC [ A/B/C/D
    if (buf[0] == 27 && n >= 3 && buf[1] == '[') {
        switch (buf[2]) {
            case 'A': return "up";
            case 'B': return "down";
            case 'C': return "right";
            case 'D': return "left";
        }
    }
    // WASD / q
    for (ssize_t i=0;i<n;i++) {
        char ch = buf[i];
        if (ch=='w'||ch=='W') return "w";
        if (ch=='a'||ch=='A') return "a";
        if (ch=='s'||ch=='S') return "s";
        if (ch=='d'||ch=='D') return "d";
        if (ch=='q'||ch=='Q') return "q";
    }
    return "";
#endif
}

// --------------- Game logic ---------------
static void spawnFood() {
    vector<Position> emptySpaces;
    emptySpaces.reserve(GRID_WIDTH*GRID_HEIGHT);
    for (int y=0; y<GRID_HEIGHT; ++y) {
        for (int x=0; x<GRID_WIDTH; ++x) {
            Position p{x,y};
            bool occ = false;
            for (auto &s : game.snake) { if (s==p) { occ=true; break; } }
            if (!occ) emptySpaces.push_back(p);
        }
    }
    if (!emptySpaces.empty()) {
        uniform_int_distribution<int> dis(0, (int)emptySpaces.size()-1);
        game.food = emptySpaces[dis(rng)];
    }
}

static void initNewGame(const string& name = "Player") {
    game.snake.clear();
    game.snake.push_back({GRID_WIDTH/2, GRID_HEIGHT/2});
    game.snake.push_back({GRID_WIDTH/2-1, GRID_HEIGHT/2});
    game.snake.push_back({GRID_WIDTH/2-2, GRID_HEIGHT/2});
    game.direction = "right";
    game.nextDirection = "right";
    game.score = 0;
    game.gameActive = true;
    game.playerName = name;
    game.lastMoveTime = steady_clock::now();
    game.moveDelay = 200;
    spawnFood();
}

static void setDirection(const string& dir) {
    if (dir=="up"    && game.direction!="down")  game.nextDirection = "up";
    if (dir=="down"  && game.direction!="up")    game.nextDirection = "down";
    if (dir=="left"  && game.direction!="right") game.nextDirection = "left";
    if (dir=="right" && game.direction!="left")  game.nextDirection = "right";
}

static void moveSnakeTick() {
    auto now = steady_clock::now();
    auto elapsed = duration_cast<milliseconds>(now - game.lastMoveTime).count();
    if (!game.gameActive || elapsed < game.moveDelay) return;

    game.lastMoveTime = now;
    game.direction = game.nextDirection;

    Position newHead = game.snake.front();
    if      (game.direction=="up")    newHead.y--;
    else if (game.direction=="down")  newHead.y++;
    else if (game.direction=="left")  newHead.x--;
    else if (game.direction=="right") newHead.x++;

    // wall collision
    if (newHead.x<0 || newHead.x>=GRID_WIDTH || newHead.y<0 || newHead.y>=GRID_HEIGHT) {
        game.gameActive = false;
        return;
    }
    // self collision
    for (auto &s : game.snake) {
        if (newHead==s) { game.gameActive = false; return; }
    }

    game.snake.insert(game.snake.begin(), newHead);

    // food collision
    if (newHead == game.food) {
        game.score++;
        game.highScore = max(game.highScore, game.score);
        if (game.moveDelay > 50) game.moveDelay = max(50, game.moveDelay - 5);
        spawnFood();
    } else {
        game.snake.pop_back();
    }
}

// --------------- Rendering ---------------
static void clear_screen() {
    // ANSI clear works in modern Windows terminals and Linux
    cout << "\x1b[2J\x1b[H";
}

static void render() {
    clear_screen();
    // top border
    cout << "=== Snake (CLI) ===" << "\n Player: " << game.playerName
            << "\n Score: " << game.score
            << " | Best: " << game.highScore
            << " | Delay: " << game.moveDelay << "ms\n";
    cout << "+";
    for (int x=0; x<GRID_WIDTH; ++x) cout << "-";
    cout << "+\n";

    for (int y=0; y<GRID_HEIGHT; ++y) {
        cout << "|";
        for (int x=0; x<GRID_WIDTH; ++x) {
            Position p{x,y};
            if (p == game.food) { cout << "F"; continue; }
            bool body = false;
            for (size_t i=0;i<game.snake.size();++i) {
                if (game.snake[i]==p) { 
                    cout << (i==0 ? "O" : "o"); 
                    body = true; 
                    break; 
                }
            }
            if (!body) cout << " ";
        }
        cout << "|\n";
    }
    cout << "+";
    for (int x=0; x<GRID_WIDTH; ++x) cout << "-";
    cout << "+\n";
    cout << "[WASD / Arrows] | 'q' to quit\n";
    cout.flush();
}

int main() {
#ifndef _WIN32
    TermRaw rawGuard;  // restore terminal on exit
#endif
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    initNewGame("Ud");

    while (true) {
        // input
        string k = poll_key();
        if (!k.empty()) {
            if (k=="q") break;
            if (k=="up"||k=="down"||k=="left"||k=="right") setDirection(k);
            if (k=="w") setDirection("up");
            if (k=="s") setDirection("down");
            if (k=="a") setDirection("left");
            if (k=="d") setDirection("right");
        }

        // tick + draw
        moveSnakeTick();
        render();

        if (!game.gameActive) {
            cout << "\nGAME OVER. Press 'r' to restart or 'q' to quit.\n";
            cout.flush();
            // small wait loop for decision (non-blocking)
            while (true) {
                string k2 = poll_key();
                if (k2=="q") return 0;
#ifdef _WIN32
                if (!k2.empty() && (k2=="w"||k2=="a"||k2=="s"||k2=="d"||k2=="up"||k2=="down"||k2=="left"||k2=="right")) {
                    // ignore direction keys on game over
                    k2.clear();
                }
#endif
                if (!k2.empty()) {
                    char c = k2[0];
                    if (c=='r' || c=='R') { initNewGame(game.playerName); break; }
                }
                sleep_ms(20);
            }
        }

        // frame pacing (render loop); movement is governed by moveDelay internally
        sleep_ms(10);
    }

    return 0;
}

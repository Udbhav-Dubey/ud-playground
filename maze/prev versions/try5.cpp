#include <bits/stdc++.h>
#include <random>
#include <termios.h>
#include <unistd.h>
#include <chrono>
using namespace std::chrono;
using namespace std;

char getch() {
    char buf = 0;
    termios old = {};
    tcgetattr(STDIN_FILENO, &old);
    termios new_t = old;
    new_t.c_lflag &= ~ICANON;
    new_t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
    read(STDIN_FILENO, &buf, 1);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &old);
    return buf;
}

const int width = 41;
const int height = 41;
vector<vector<char>> maze(height, vector<char>(width, '#'));

const vector<pair<int, int>> directions = {
    {-2, 0}, {2, 0}, {0, 2}, {0, -2}
};

bool isValid(int x, int y) {
    return x > 0 && x < height - 1 && y > 0 && y < width - 1;
}

int exitX, exitY;

void exit_E() {
    vector<pair<int, int>> possible_exits;
    for (int i = 1; i < width - 1; ++i) {
        if (maze[1][i] == ' ') possible_exits.emplace_back(0, i);
        if (maze[height - 2][i] == ' ') possible_exits.emplace_back(height - 1, i);
    }
    for (int i = 1; i < height - 1; ++i) {
        if (maze[i][1] == ' ') possible_exits.emplace_back(i, 0);
        if (maze[i][width - 2] == ' ') possible_exits.emplace_back(i, width - 1);
    }
    if (!possible_exits.empty()) {
        random_device rd;
        mt19937 g(rd());
        uniform_int_distribution<> dis(0, possible_exits.size() - 1);
        tie(exitX, exitY) = possible_exits[dis(g)];
        maze[exitX][exitY] = 'E';
    }
}

void carve_maze(int x, int y) {
    maze[x][y] = ' ';
    vector<pair<int, int>> dirs = directions;
    random_device rd;
    mt19937 g(rd());
    shuffle(dirs.begin(), dirs.end(), g);
    for (auto [dx, dy] : dirs) {
        int nx = x + dx, ny = y + dy;
        if (isValid(nx, ny) && maze[nx][ny] == '#') {
            maze[x + dx / 2][y + dy / 2] = ' ';
            carve_maze(nx, ny);
        }
    }
}

void drawUI(int remainingTime) {
    cout << "\033[2J\033[1;1H"; // Clear screen
    cout << "\033[1;36m"; // Cyan color
    cout << "=========== ASCII MAZE GAME ===========\n";
    cout << "\033[0m"; // Reset
    cout << "Time Left: " << remainingTime << " seconds\tUse WASD to move, Q to quit\n\n";
}

void drawMaze(const vector<vector<char>>& maze) {
    for (const auto& row : maze) {
        for (char cell : row) {
            if (cell == '#') cout << "\033[1;90m" << cell << "\033[0m"; // dark grey
            else if (cell == 'P') cout << "\033[1;32m" << cell << "\033[0m"; // green
            else if (cell == 'E') cout << "\033[1;31m" << cell << "\033[0m"; // red
            else cout << cell;
        }
        cout << "\n";
    }
}

void playGame(vector<vector<char>>& maze, int N) {
    int x = 1, y = 1;
    maze[x][y] = 'P';
    const int time_limit = 180; // 3 minutes

    bool firstKey = true;
    steady_clock::time_point start_time;

    while (true) {
        int elapsed = 0;
        if (!firstKey)
            elapsed = duration_cast<seconds>(steady_clock::now() - start_time).count();

        int remaining = time_limit - elapsed;
        if (remaining <= 0) {
            drawUI(0);
            drawMaze(maze);
            cout << "\n\n\033[1;31mTime's up! You lost.\033[0m\n";
            break;
        }

        drawUI(remaining);
        drawMaze(maze);

        char move = getch();
        if (firstKey) {
            start_time = steady_clock::now();
            firstKey = false;
        }

        int posx = x, posy = y;
        if (move == 'w' || move == 'W') posx--;
        else if (move == 'a' || move == 'A') posy--;
        else if (move == 's' || move == 'S') posx++;
        else if (move == 'd' || move == 'D') posy++;
        else if (move == 'Q' || move == 'q') break;

        if (maze[posx][posy] == ' ' || maze[posx][posy] == 'E') {
            maze[x][y] = ' ';
            x = posx; y = posy;
            maze[x][y] = 'P';

            if (x == exitX && y == exitY) {
                drawUI(remaining);
                drawMaze(maze);
                cout << "\n\n\033[1;32mCongratulations! You won!\033[0m\n";
                break;
            }
        }
    }
}

int main() {
    srand(time(0));
    carve_maze(1, 1);
    exit_E();
    playGame(maze, width);
    return 0;
}

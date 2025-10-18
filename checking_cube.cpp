#include <iostream>
#include <cmath>
#include <unistd.h> // usleep (Linux/Mac). On Windows use Sleep from <windows.h>

struct Vec3 {
    float x, y, z;
};

struct Vec2 {
    int x, y;
};

Vec2 project(Vec3 v, float angleX, float angleY, int width, int height) {
    // Rotate around X
    float cosX = cos(angleX), sinX = sin(angleX);
    float y = v.y * cosX - v.z * sinX;
    float z = v.y * sinX + v.z * cosX;

    // Rotate around Y
    float cosY = cos(angleY), sinY = sin(angleY);
    float x = v.x * cosY + z * sinY;
    z = -v.x * sinY + z * cosY;

    // Simple perspective projection
    float fov = 100.0f;
    float scale = fov / (z + 3.0f);

    int px = (int)(width / 2 + x * scale * width / 2);
    int py = (int)(height / 2 - y * scale * height / 2);

    return {px, py};
}

int main() {
    const int width = 80, height = 24;

    // Cube vertices
    Vec3 vertices[8] = {
        {-1, -1, -1}, {1, -1, -1},
        {1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, {1, -1,  1},
        {1,  1,  1}, {-1,  1,  1}
    };

    // Cube edges (pairs of vertex indices)
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    float angleX = 0, angleY = 0;

    while (true) {
        char screen[width * height];
        std::fill_n(screen, width * height, ' ');

        // Project vertices
        Vec2 projected[8];
        for (int i = 0; i < 8; i++) {
            projected[i] = project(vertices[i], angleX, angleY, width, height);
        }

        // Draw edges
        for (auto &edge : edges) {
            Vec2 a = projected[edge[0]];
            Vec2 b = projected[edge[1]];
            int dx = std::abs(b.x - a.x), sx = a.x < b.x ? 1 : -1;
            int dy = -std::abs(b.y - a.y), sy = a.y < b.y ? 1 : -1;
            int err = dx + dy, e2;
            int x = a.x, y = a.y;

            while (true) {
                if (x >= 0 && x < width && y >= 0 && y < height)
                    screen[y * width + x] = '#';
                if (x == b.x && y == b.y) break;
                e2 = 2 * err;
                if (e2 >= dy) { err += dy; x += sx; }
                if (e2 <= dx) { err += dx; y += sy; }
            }
        }

        // Print buffer
        std::cout << "\x1b[H"; // reset cursor
        for (int i = 0; i < width * height; i++) {
            std::cout << (i % width ? screen[i] : '\n');
        }

        angleX += 0.05;
        angleY += 0.03;
        usleep(30000); // ~30 FPS
    }
}


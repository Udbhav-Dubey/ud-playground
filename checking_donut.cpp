#include <iostream>
#include <cmath>
#include <cstring>
#include <unistd.h> // for usleep (Linux/Mac). On Windows, replace with Sleep.

int main() {
    float A = 0, B = 0;
    float i, j;
    int k;
    float z[1760];
    char b[1760];

    std::cout << "\x1b[2J"; // clear screen

    for(;;) {
        std::memset(b,32,1760);
        std::memset(z,0,7040);

        for(j=0; j < 6.28; j += 0.07) { // j goes around circle
            for(i=0; i < 6.28; i += 0.02) { // i goes around donut
                float c = std::sin(i),
                      d = std::cos(j),
                      e = std::sin(A),
                      f = std::sin(j),
                      g = std::cos(A),
                      h = d+2,
                      D = 1/(c*h*e+f*g+5),
                      l = std::cos(i),
                      m = std::cos(B),
                      n = std::sin(B),
                      t = c*h*g - f*e;
                int x = 40 + 30*D*(l*h*m - t*n);
                int y = 12 + 15*D*(l*h*n + t*m);
                int o = x + 80*y;
                int N = 8*((f*e - c*d*g)*m - c*d*e - f*g - l*d*n);

                if(22 > y && y > 0 && x > 0 && x < 80 && D > z[o]) {
                    z[o] = D;
                    b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
                }
            }
        }

        std::cout << "\x1b[H"; // reset cursor
        for(k=0; k < 1760; k++)
            std::putchar(k % 80 ? b[k] : 10);

        A += 0.04;
        B += 0.08;
        usleep(30000); // sleep 30ms
    }
}


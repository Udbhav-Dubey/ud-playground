#include <iostream>
#include <unistd.h>   // for usleep
#include <string>

using namespace std;

int main() {
    string message = ">>> Hello from Ud's custom sl! <<<";
    int width = 80; // assume typical terminal width (you can detect this dynamically later)
    
    // Start with message off the right edge
    for (int pos = width; pos + (int)message.size() >= 0; --pos) {
        cout << "\033[2J";        // clear screen
        cout << "\033[H";         // move cursor to top-left
        for (int i = 0; i < pos; ++i) cout << ' '; // spacing before text
        cout << message << flush; // print message
        usleep(50000);            // control speed (microseconds)
    }
    cout << endl;
    return 0;
}


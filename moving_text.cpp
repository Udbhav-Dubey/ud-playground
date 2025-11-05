#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream> 
using namespace std;

string artBlock = R"(
.____            __           .__      
|    |   _____  |  | __  _____|  |__   
|    |   \__  \ |  |/ / /  ___/  |  \  
|    |___ / __ \|    <  \___ \|   Y  \
|_______ (____  /__|_ \/____  >___|  / 
        \/    \/     \/     \/     \/   
)";


vector<string> splitTextArt(const string& art) {
    vector<string> lines;
    stringstream ss(art);
    string line;
    getline(ss, line); 
    while (getline(ss, line)) {
        if (ss.peek() == EOF) break; 
        lines.push_back(line);
    }
    return lines;
}

int main() {
    vector<string> textArt = splitTextArt(artBlock);
    if (textArt.empty()) return 1; 

    int termWidth = 80; 
    int artWidth = textArt[0].size(); 

    for (int pos = termWidth; pos > -artWidth; --pos) {
        cout << "\033[2J";  
        cout << "\033[H";   
        for (const auto &line : textArt) { 
            for (int i = 0; i < pos; ++i) cout << ' ';
            cout << line << '\n';
        }
        cout.flush();
        usleep(60000); 
    }
    cout << endl;
    return 0;
}

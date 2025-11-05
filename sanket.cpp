//                       __           __   
//  ___________    ____ |  | __ _____/  |_ 
// /  ___/\__  \  /    \|  |/ // __ \   __\
// \___ \  / __ \|   |  \    <\  ___/|  |  
///____  >(____  /___|  /__|_ \\___  >__|  
//     \/      \/     \/     \/    \/      
//    
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream> 
using namespace std;

string artBlock = R"(
                       __           __   
  ___________    ____ |  | __ _____/  |_ 
 /  ___/\__  \  /    \|  |/ // __ \   __\
 \___ \  / __ \|   |  \    <\  ___/|  |  
/____  >(____  /___|  /__|_ \\___  >__|  
     \/      \/     \/     \/    \/      
  
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

    int termWidth = 160; 
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

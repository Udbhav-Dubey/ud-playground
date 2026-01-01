#include <string>
#include <vector>
#include <iostream>
#include <fstream>
int main (){
    std::ifstream in("joke_1.json");
    if (!in){
        std::cout << "no json found\n";
    }
    std::string line;
    while(std::getline(in,line)){
        if (line.find("type")!=std::string::npos){
            std::getline(in,line,':');
            std::getline(in,line);
            std::cout << line << "\n";
            std::getline(in,line,':');
            std::getline(in,line);
            std::cout << line << "\n";
        }
    }
    return 0;
}

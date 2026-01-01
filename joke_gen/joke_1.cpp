#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <chrono>
#include <thread>
#include <cstdlib>
struct joke{
    std::string setup;
    std::string punch;
};
std::vector<joke>loadJokes(){
    std::ifstream in("joke.json");
     if (!in){
        std::cout << "failing in reading .json file";
        exit(1);
    }
    std::string line;
    std::vector<joke> jokes;
    std::getline(in,line);
    int i=0;
    while(std::getline(in,line)){
        if (line.find("type")!=std::string::npos){
            joke j;
            std::getline(in,line,':');
            std::getline(in,line);
            j.setup=line;
            std::getline(in,line,':');
            std::getline(in,line);
            j.punch=line;
            jokes.push_back(j);
        }
    }
    in.close();
    return jokes;
}
    void showRandom(std::vector<joke>&jokes){
    system("clear");
    int idx=std::rand()%jokes.size();
    std::cout << "\n\n" << jokes[idx].setup<<"\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "\n\n" << jokes[idx].punch<<"\n\n";
}
int main (){
    std::vector<joke>jokess=loadJokes();
    std::srand(std::time(nullptr));
       while(true){
        showRandom(jokess);
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        std::cout << "press 1 for next joke\n0 for exit\n";
        bool flag;
        std::cin>>flag;
        if (flag==0){break;}
    }
    return 0;
}

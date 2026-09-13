#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
class songs{
    std::vector<std::string>mp3Files;
    const std::string path{"."};
    public:
    songs(){
        scanDirectory();
    }
    void scanDirectory(){
        mp3Files.clear();
            for (const auto &f:std::filesystem::directory_iterator(path)){
                if (f.is_regular_file()){
                    std::string filename=f.path().filename().string();
                    if (filename.length()>=4&&filename.substr(filename.length()-4)==".mp3"){
                        mp3Files.push_back(filename);
                    }
                }
            }
            std::sort(mp3Files.begin(),mp3Files.end());
    }
    void files(){
        int i=0;
        for (auto s:mp3Files){
            std::cout << i++ << " "<< s << "\n" ;
        }
    }
};
int main(){
    songs s;
    s.files();
    
}

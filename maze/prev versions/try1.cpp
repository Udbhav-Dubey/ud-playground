#include <bits/stdc++.h>
using namespace std;
const int width=21; 
const int height=21;
vector<vector<char>> maze(height,vector<char>(width,'#')); // fill the maze;

void printmaze(){
    for (int i=0;i<height;++i){
        for (int j=0;j<width;++j){
            cout << maze[i][j];
        }
        cout << endl;
    }
}
int main (){
    srand(time(0));//for random genration 
    printmaze();
    return 0;
}

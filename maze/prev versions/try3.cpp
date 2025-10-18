#include <bits/stdc++.h>
#include <random>
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
const vector<pair<int,int>> directions={
    {-2,0}, // North
    {2,0}, // South
    {0,2},   // east
    {0,-2}  // west
};
bool isValid(int x,int y){
    return x>0 && x < height-1 && y>0 && y<width-1;
}
void carve_maze(int x,int y){
    maze[x][y]=' ';
    vector<pair<int,int>>dirs=directions;
    std::random_device rd; // for random genration 
std::mt19937 g(rd()); // we use this 
   shuffle(dirs.begin(),dirs.end(),g);
    for (auto[dx,dy]:dirs){
        int nx=x+dx; // new x 
        int ny=y+dy; // new y

        if (isValid(nx,ny)&&maze[nx][ny]=='#'){
            maze[x+dx/2][y+dy/2]=' ';
            carve_maze(nx,ny);
        }
    }
}
int main (){
    srand(time(0));//for random genration 
    carve_maze(1,1); // 1,1 for starting point 
    printmaze();
    return 0;
}

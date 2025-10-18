#include <bits/stdc++.h>
#include <random>
#include <termios.h>
#include <unistd.h>
char getch() {
    char buf = 0;
    termios old = {};
    if (tcgetattr(STDIN_FILENO, &old) < 0)
        perror("tcsetattr()");
    termios new_t = old;
    new_t.c_lflag &= ~ICANON;
    new_t.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_t) < 0)
        perror("tcsetattr ICANON");
    if (read(STDIN_FILENO, &buf, 1) < 0)
        perror("read()");
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}
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
    random_device rd; // for random genration 
    mt19937 g(rd()); // we use this 
   shuffle(dirs.begin(),dirs.end(),g);
    for (auto[dx,dy]:dirs){
        int nx=x+dx; // new x 
        int ny=y+dy; // new y

        if (isValid(nx,ny)&&maze[nx][ny]=='#'){ // first it should be in constraints then if it is a wall we carve it 
          maze[x+dx/2][y+dy/2]=' ';   // by 2 + 1 to ensure even 
            carve_maze(nx,ny); // call the function again 
        }
    }
}
int exitX =0,exitY=0;
void exit_E(){
    vector<pair<int,int>>possible_exits;
    for (int i=1;i<width-1;++i){
        if (maze[1][i]==' '){possible_exits.emplace_back(0,i);}
        if (maze[height-2][i]==' ')possible_exits.emplace_back(height-1,i);
    }
    for (int i=1;i<height-1;++i){
        if (maze[i][1]==' '){possible_exits.emplace_back(i,0);}
        if (maze[i][width-2]==' '){possible_exits.emplace_back(i,width-1);}
        }
    
    if (!possible_exits.empty()) {
        random_device rd;
        mt19937 g(rd());
        uniform_int_distribution<> dis(0, possible_exits.size() - 1);
        auto [ex, ey] = possible_exits[dis(g)];
        exitX = ex;
        exitY = ey;
        maze[exitX][exitY] = 'E'; 
    }
}
void playGame(vector<vector<char>>&maze,int N){
    int x=1,y=1;
    maze[x][y]='P';
    while(true){
        system("clear");
   cout << "complete the maze using wasd under 3 minutes and to exit enter Q \n\n\n";

    for (int i=0;i<N;++i){
        for (int j=0;j<N;++j){
            cout << maze[i][j];
        }
        cout << "\n";
    }
    char move=getch();
    int posx=x;
    int posy=y;
    if (move=='w' ||move=='W') posx--;
    else if (move=='a'||move=='A')posy--;
    else if (move=='s'||move=='S')posx++;
    else if (move=='d'||move=='D')posy++;
    else if (move=='Q') break; // quit
    
    if (maze[posx][posy]==' ' || maze[posx][posy]=='E'){
        maze[x][y]=' '; // make previous position empty 
        x=posx; 
        y=posy;
        maze[x][y]='P'; // player moved
        if (x==exitX && y==exitY){
            system("clear");
            cout<<"\n\nyou win " ;
            break;
        }
    } 
}
}
int main (){
    int N=21;
    srand(time(0));//for random genration 
    carve_maze(1,1); // 1,1 for starting point 
    printmaze();
    exit_E();
    playGame(maze,N); 
    return 0;
}

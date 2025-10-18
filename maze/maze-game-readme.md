# Maze Chase Game - Key & Door Edition

A fast-paced maze escape game where players must find a key and reach the exit while avoiding AI-controlled bots, all within a 60-second time limit. Features procedurally generated mazes and competitive leaderboards.

## Game Overview

**Objective**: Find the key (K), unlock the door (D), and escape through the exit (E) while avoiding two roaming bots - all within 60 seconds!

### Game Mechanics
- **Procedural Generation**: Each game features a unique randomly generated maze
- **Key-Door System**: Players must collect the key before they can unlock the door to win
- **AI Bots**: Two bots patrol the maze with random movement patterns
- **Time Pressure**: 60-second time limit adds urgency to every move
- **Leaderboard**: Top 10 fastest completion times are saved and displayed

## Technology Stack

### Backend (C++)
- **Language**: C++17
- **Networking**: Raw socket programming (cross-platform Windows/Linux)
- **Game Engine**: Custom maze generation using recursive backtracking algorithm
- **API**: RESTful HTTP endpoints for game state management

### Frontend (HTML/CSS/JavaScript)
- **UI Framework**: Vanilla HTML5 with modern CSS3
- **Styling**: Custom CSS with animations and responsive design
- **Networking**: Fetch API for server communication
- **Visual Effects**: CSS animations, starfield background, glowing effects

## Features

### 🎮 Core Gameplay
- **WASD Movement**: Intuitive keyboard controls
- **Real-time Updates**: Live game state synchronization
- **Visual Feedback**: Animated sprites and status indicators
- **Sound-free Design**: Visual-only feedback system

### 🏆 Competitive Elements
- **Leaderboard System**: Persistent top 10 high scores
- **Player Names**: Custom player identification
- **Time Tracking**: Precise completion time measurement
- **Score Persistence**: Leaderboard survives server restarts

### 🎨 Visual Design
- **Space Theme**: Animated starfield background
- **Modern UI**: Glassmorphism effects and gradient styling
- **Responsive Design**: Mobile-friendly layout
- **Game Status**: Real-time connection and key status indicators

## Installation & Setup

### Prerequisites
- **C++ Compiler**: GCC 7+ or MSVC 2017+ (C++17 support required)
- **Operating System**: Windows or Linux
- **Network**: Port 8080 must be available

### Backend Setup

1. **Compile the server**:
   ```bash
   # Linux/Mac
   g++ -std=c++17 -o maze_server cpp_backend_server_1.cpp
   
   # Windows (Visual Studio)
   cl /EHsc /std:c++17 cpp_backend_server_1.cpp /Fe:maze_server.exe
   ```

2. **Run the server**:
   ```bash
   # Linux/Mac
   ./maze_server
   
   # Windows
   maze_server.exe
   ```

3. **Verify server is running**:
   - Server will display: "🚀 Maze server with Key-Door mechanic running on http://localhost:8080"
   - Port 8080 should be accessible

### Frontend Setup

1. **Open the game**:
   - Open `maze_frontend_client(2).html` in any modern web browser
   - Ensure the backend server is running first

2. **Alternative serving methods**:
   ```bash
   # Using Python's built-in server
   python -m http.server 3000
   
   # Using Node.js http-server
   npx http-server -p 3000
   
   # Using PHP
   php -S localhost:3000
   ```

## API Endpoints

The game server provides the following REST endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/newgame` | POST | Start a new game with player name |
| `/move` | POST | Send player movement command |
| `/state` | GET | Get current game state |
| `/leaderboard` | GET | Retrieve top 10 scores |

### Request/Response Examples

**Start New Game**:
```json
POST /newgame
{
  "name": "PlayerName"
}
```

**Player Movement**:
```json
POST /move
{
  "move": "w"  // w/a/s/d for movement, e for quit
}
```

## Game Controls

| Key | Action |
|-----|--------|
| `W` | Move Up |
| `A` | Move Left |
| `S` | Move Down |
| `D` | Move Right |
| `ESC` | Quit Game |

## Game Elements

### Visual Legend
- **P** - Player (green, glowing)
- **K** - Key (golden, rotating)
- **D** - Locked Door (brown, pulsing)
- **E** - Unlocked Exit (orange, beacon effect)
- **1, 2** - AI Bots (red, hunting animation)
- **|** - Walls (dark)
- **Space** - Walkable paths (light gray)

### Game States
1. **Menu Screen**: Player name entry and leaderboard display
2. **Active Game**: Real-time maze navigation
3. **Game Over**: Time expired or caught by bot
4. **Victory**: Successfully escaped with key

## Architecture Details

### Maze Generation Algorithm
The game uses **recursive backtracking** to create unique mazes:
1. Start with a grid filled with walls
2. Carve paths by randomly selecting directions
3. Ensure all areas are reachable
4. Place game elements in valid positions

### Bot AI Behavior
- **Movement Pattern**: Random direction selection every turn
- **Step Size**: 2 cells per move for balanced difficulty
- **Collision Detection**: Game ends if bot occupies same cell as player
- **Path Validation**: Bots respect maze walls

### Network Architecture
- **Protocol**: HTTP over TCP sockets
- **CORS Enabled**: Cross-origin requests supported
- **Stateful Server**: Game state maintained server-side
- **JSON Communication**: Structured data exchange

## Development Notes

### Performance Considerations
- Maze generation is O(n) where n is the number of cells
- Bot movement calculations are O(1) per bot per turn
- Network requests are asynchronous to prevent UI blocking

### Cross-Platform Compatibility
- Socket code includes Windows and Linux implementations
- Frontend uses standard web technologies for universal browser support
- No external dependencies required

## Troubleshooting

### Common Issues

**Server won't start**:
- Check if port 8080 is already in use
- Verify compiler supports C++17
- Ensure proper permissions for network binding

**Frontend can't connect**:
- Confirm server is running and accessible
- Check browser console for CORS errors
- Verify the API_BASE URL in the HTML file

**Game performance issues**:
- Close other applications using network resources
- Try a different web browser
- Check system firewall settings

## Contributing

This project is open for contributions. Areas for improvement:
- Additional maze generation algorithms
- More sophisticated bot AI
- Sound effects and music
- Mobile touch controls
- Multiplayer functionality

## License

This project is available for educational and personal use. Please respect the Developer Student Club Thapar branding present in the frontend.

---

**Developed for Developer Student Club Thapar Institute of Engineering and Technology**
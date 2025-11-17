using UnityEngine;
using UnityEngine.SceneManagement;
using System.Collections.Generic;
using System.Linq;

public class GameManager : MonoBehaviour
{
    public static GameManager Instance { get; private set; }
    
    [Header("Game Settings")]
    public int mazeWidth = 21;
    public int mazeHeight = 21;
    public float timeLimit = 60f;
    
    [Header("Prefabs")]
    public GameObject wallPrefab;
    public GameObject floorPrefab;
    public GameObject playerPrefab;
    public GameObject keyPrefab;
    public GameObject doorPrefab;
    public GameObject botPrefab;
    
    [Header("References")]
    public Transform mazeParent;
    public UIManager uiManager;
    
    // Game state
    private float timeRemaining;
    private bool gameActive = false;
    private bool hasKey = false;
    private string playerName;
    
    // Maze data
    private char[,] maze;
    private Vector2Int exitPos;
    private Vector2Int keyPos;
    
    // Game objects
    private GameObject player;
    private GameObject key;
    private GameObject door;
    private List<GameObject> bots = new List<GameObject>();
    
    void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            Destroy(gameObject);
        }
    }
    
    void Update()
    {
        if (!gameActive) return;
        
        timeRemaining -= Time.deltaTime;
        uiManager.UpdateTimer(Mathf.Max(0, timeRemaining));
        
        if (timeRemaining <= 0)
        {
            GameOver("Time's Up!");
        }
    }
    
    public void StartNewGame(string name)
    {
        playerName = name;
        gameActive = true;
        hasKey = false;
        timeRemaining = timeLimit;
        
        GenerateMaze();
        SpawnGameObjects();
        
        uiManager.ShowGameUI();
        uiManager.UpdateKeyStatus(false);
    }
    
    void GenerateMaze()
    {
        maze = new char[mazeHeight, mazeWidth];
        
        // Initialize with walls
        for (int i = 0; i < mazeHeight; i++)
        {
            for (int j = 0; j < mazeWidth; j++)
            {
                maze[i, j] = '|';
            }
        }
        
        // Carve maze using recursive backtracking
        CarveMaze(1, 1);
        
        // Place exit far from start
        PlaceExit();
        
        // Place key
        PlaceKey();
    }
    
    void CarveMaze(int x, int y)
    {
        maze[x, y] = ' ';
        
        // Define directions: up, down, left, right (2 steps)
        List<Vector2Int> directions = new List<Vector2Int>
        {
            new Vector2Int(-2, 0),
            new Vector2Int(2, 0),
            new Vector2Int(0, -2),
            new Vector2Int(0, 2)
        };
        
        // Shuffle directions
        directions = directions.OrderBy(x => Random.Range(0, 1000)).ToList();
        
        foreach (var dir in directions)
        {
            int nx = x + dir.x;
            int ny = y + dir.y;
            int wx = x + dir.x / 2;
            int wy = y + dir.y / 2;
            
            if (nx > 0 && nx < mazeHeight - 1 && ny > 0 && ny < mazeWidth - 1 && maze[nx, ny] == '|')
            {
                maze[wx, wy] = ' ';
                CarveMaze(nx, ny);
            }
        }
    }
    
    void PlaceExit()
    {
        List<Vector2Int> validPositions = new List<Vector2Int>();
        
        for (int i = 1; i < mazeHeight - 1; i++)
        {
            for (int j = 1; j < mazeWidth - 1; j++)
            {
                if (maze[i, j] == ' ')
                {
                    int distance = Mathf.Abs(i - 1) + Mathf.Abs(j - 1);
                    if (distance > 10)
                    {
                        validPositions.Add(new Vector2Int(i, j));
                    }
                }
            }
        }
        
        if (validPositions.Count > 0)
        {
            exitPos = validPositions[Random.Range(0, validPositions.Count)];
        }
        else
        {
            exitPos = new Vector2Int(mazeHeight - 2, mazeWidth - 2);
        }
    }
    
    void PlaceKey()
    {
        List<Vector2Int> validPositions = new List<Vector2Int>();
        
        for (int i = 1; i < mazeHeight - 1; i++)
        {
            for (int j = 1; j < mazeWidth - 1; j++)
            {
                if (maze[i, j] == ' ' && !(i == 1 && j == 1) && !(i == exitPos.x && j == exitPos.y))
                {
                    validPositions.Add(new Vector2Int(i, j));
                }
            }
        }
        
        if (validPositions.Count > 0)
        {
            keyPos = validPositions[Random.Range(0, validPositions.Count)];
        }
    }
    
    void SpawnGameObjects()
    {
        // Clear existing objects
        foreach (Transform child in mazeParent)
        {
            Destroy(child.gameObject);
        }
        bots.Clear();
        
        // Spawn maze
        for (int i = 0; i < mazeHeight; i++)
        {
            for (int j = 0; j < mazeWidth; j++)
            {
                Vector3 pos = new Vector3(j, -i, 0);
                
                if (maze[i, j] == '|')
                {
                    Instantiate(wallPrefab, pos, Quaternion.identity, mazeParent);
                }
                else
                {
                    Instantiate(floorPrefab, pos, Quaternion.identity, mazeParent);
                }
            }
        }
        
        // Spawn player at (1,1)
        player = Instantiate(playerPrefab, new Vector3(1, -1, 0), Quaternion.identity);
        
        // Spawn key
        key = Instantiate(keyPrefab, new Vector3(keyPos.y, -keyPos.x, 0), Quaternion.identity);
        
        // Spawn door
        door = Instantiate(doorPrefab, new Vector3(exitPos.y, -exitPos.x, 0), Quaternion.identity);
        
        // Spawn bots
        SpawnBots(2);
        
        // Center camera
        Camera.main.transform.position = new Vector3(mazeWidth / 2f, -mazeHeight / 2f, -10);
    }
    
    void SpawnBots(int count)
    {
        List<Vector2Int> validPositions = new List<Vector2Int>();
        
        for (int i = 1; i < mazeHeight - 1; i++)
        {
            for (int j = 1; j < mazeWidth - 1; j++)
            {
                if (maze[i, j] == ' ' && 
                    !(i == 1 && j == 1) && 
                    !(i == exitPos.x && j == exitPos.y) &&
                    !(i == keyPos.x && j == keyPos.y))
                {
                    validPositions.Add(new Vector2Int(i, j));
                }
            }
        }
        
        validPositions = validPositions.OrderBy(x => Random.Range(0, 1000)).ToList();
        
        for (int i = 0; i < Mathf.Min(count, validPositions.Count); i++)
        {
            Vector2Int pos = validPositions[i];
            GameObject bot = Instantiate(botPrefab, new Vector3(pos.y, -pos.x, 0), Quaternion.identity);
            bots.Add(bot);
        }
    }
    
    public bool IsValidPosition(int x, int y)
    {
        if (x < 0 || x >= mazeHeight || y < 0 || y >= mazeWidth) return false;
        return maze[x, y] == ' ' || (x == exitPos.x && y == exitPos.y);
    }
    
    public void CollectKey()
    {
        if (!hasKey)
        {
            hasKey = true;
            Destroy(key);
            uiManager.UpdateKeyStatus(true);
            
            // Update door sprite to show it's unlocked
            SpriteRenderer doorSprite = door.GetComponent<SpriteRenderer>();
            if (doorSprite != null)
            {
                doorSprite.color = Color.yellow; // Visual indicator
            }
        }
    }
    
    public void ReachExit()
    {
        if (hasKey)
        {
            gameActive = false;
            float timeTaken = timeLimit - timeRemaining;
            SaveScore(playerName, timeTaken);
            uiManager.ShowWinScreen(timeTaken);
        }
    }
    
    public void GameOver(string reason)
    {
        gameActive = false;
        uiManager.ShowGameOverScreen(reason);
    }
    
    void SaveScore(string name, float time)
    {
        List<ScoreEntry> scores = LoadScores();
        scores.Add(new ScoreEntry { name = name, time = time });
        scores = scores.OrderBy(s => s.time).Take(10).ToList();
        
        string json = JsonUtility.ToJson(new ScoreList { scores = scores });
        PlayerPrefs.SetString("Leaderboard", json);
        PlayerPrefs.Save();
    }
    
    public List<ScoreEntry> LoadScores()
    {
        string json = PlayerPrefs.GetString("Leaderboard", "");
        if (string.IsNullOrEmpty(json))
        {
            return new List<ScoreEntry>();
        }
        
        ScoreList scoreList = JsonUtility.FromJson<ScoreList>(json);
        return scoreList.scores;
    }
    
    public bool IsGameActive()
    {
        return gameActive;
    }
    
    public void RestartGame()
    {
        SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);
    }
}

[System.Serializable]
public class ScoreEntry
{
    public string name;
    public float time;
}

[System.Serializable]
public class ScoreList
{
    public List<ScoreEntry> scores;
}
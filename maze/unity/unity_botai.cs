using UnityEngine;
using System.Collections.Generic;
using System.Linq;

public class BotAI : MonoBehaviour
{
    [Header("Movement Settings")]
    public float moveSpeed = 3f;
    public float moveInterval = 1.5f;
    
    private Vector2Int gridPosition;
    private Vector3 targetPosition;
    private bool isMoving = false;
    private float moveTimer = 0f;
    
    void Start()
    {
        // Initialize grid position based on spawn location
        gridPosition = new Vector2Int(
            Mathf.RoundToInt(-transform.position.y),
            Mathf.RoundToInt(transform.position.x)
        );
        targetPosition = transform.position;
        moveTimer = Random.Range(0f, moveInterval);
    }
    
    void Update()
    {
        if (!GameManager.Instance.IsGameActive()) return;
        
        // Smooth movement
        if (isMoving)
        {
            transform.position = Vector3.MoveTowards(transform.position, targetPosition, moveSpeed * Time.deltaTime);
            
            if (Vector3.Distance(transform.position, targetPosition) < 0.01f)
            {
                transform.position = targetPosition;
                isMoving = false;
            }
        }
        else
        {
            // Move at intervals
            moveTimer += Time.deltaTime;
            if (moveTimer >= moveInterval)
            {
                moveTimer = 0f;
                TryMove();
            }
        }
    }
    
    void TryMove()
    {
        // Get valid adjacent positions
        List<Vector2Int> validMoves = new List<Vector2Int>();
        
        Vector2Int[] directions = new Vector2Int[]
        {
            new Vector2Int(-1, 0), // up
            new Vector2Int(1, 0),  // down
            new Vector2Int(0, -1), // left
            new Vector2Int(0, 1)   // right
        };
        
        foreach (var dir in directions)
        {
            Vector2Int newPos = gridPosition + dir;
            if (GameManager.Instance.IsValidPosition(newPos.x, newPos.y))
            {
                validMoves.Add(newPos);
            }
        }
        
        // Move to random valid position
        if (validMoves.Count > 0)
        {
            gridPosition = validMoves[Random.Range(0, validMoves.Count)];
            targetPosition = new Vector3(gridPosition.y, -gridPosition.x, 0);
            isMoving = true;
        }
    }
    
    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.CompareTag("Player"))
        {
            GameManager.Instance.GameOver("Caught by Bot!");
        }
    }
}
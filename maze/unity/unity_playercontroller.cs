using UnityEngine;

public class PlayerController : MonoBehaviour
{
    [Header("Movement Settings")]
    public float moveSpeed = 5f;
    
    private Vector2Int gridPosition;
    private Vector3 targetPosition;
    private bool isMoving = false;
    
    void Start()
    {
        gridPosition = new Vector2Int(1, 1);
        targetPosition = transform.position;
    }
    
    void Update()
    {
        if (!GameManager.Instance.IsGameActive()) return;
        
        // Smooth movement to target position
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
            // Handle input only when not moving
            HandleInput();
        }
    }
    
    void HandleInput()
    {
        Vector2Int moveDirection = Vector2Int.zero;
        
        if (Input.GetKeyDown(KeyCode.W) || Input.GetKeyDown(KeyCode.UpArrow))
        {
            moveDirection = new Vector2Int(-1, 0);
        }
        else if (Input.GetKeyDown(KeyCode.S) || Input.GetKeyDown(KeyCode.DownArrow))
        {
            moveDirection = new Vector2Int(1, 0);
        }
        else if (Input.GetKeyDown(KeyCode.A) || Input.GetKeyDown(KeyCode.LeftArrow))
        {
            moveDirection = new Vector2Int(0, -1);
        }
        else if (Input.GetKeyDown(KeyCode.D) || Input.GetKeyDown(KeyCode.RightArrow))
        {
            moveDirection = new Vector2Int(0, 1);
        }
        else if (Input.GetKeyDown(KeyCode.Escape))
        {
            GameManager.Instance.GameOver("Player Quit");
            return;
        }
        
        if (moveDirection != Vector2Int.zero)
        {
            TryMove(moveDirection);
        }
    }
    
    void TryMove(Vector2Int direction)
    {
        Vector2Int newGridPos = gridPosition + direction;
        
        if (GameManager.Instance.IsValidPosition(newGridPos.x, newGridPos.y))
        {
            gridPosition = newGridPos;
            targetPosition = new Vector3(gridPosition.y, -gridPosition.x, 0);
            isMoving = true;
        }
    }
    
    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.CompareTag("Key"))
        {
            GameManager.Instance.CollectKey();
        }
        else if (other.CompareTag("Exit"))
        {
            GameManager.Instance.ReachExit();
        }
        else if (other.CompareTag("Bot"))
        {
            GameManager.Instance.GameOver("Caught by Bot!");
        }
    }
    
    public Vector2Int GetGridPosition()
    {
        return gridPosition;
    }
}
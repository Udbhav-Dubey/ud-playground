using UnityEngine;
using UnityEngine.UI;
using TMPro;
using System.Collections.Generic;

public class UIManager : MonoBehaviour
{
    [Header("Screens")]
    public GameObject homeScreen;
    public GameObject nameInputScreen;
    public GameObject gameScreen;
    public GameObject gameOverScreen;
    public GameObject winScreen;
    
    [Header("Home Screen")]
    public Transform leaderboardContent;
    public GameObject leaderboardEntryPrefab;
    
    [Header("Name Input")]
    public TMP_InputField nameInputField;
    
    [Header("Game Screen")]
    public TextMeshProUGUI timerText;
    public TextMeshProUGUI keyStatusText;
    public Image keyStatusIcon;
    
    [Header("Game Over Screen")]
    public TextMeshProUGUI gameOverMessageText;
    
    [Header("Win Screen")]
    public TextMeshProUGUI winMessageText;
    
    void Start()
    {
        ShowHomeScreen();
    }
    
    public void ShowHomeScreen()
    {
        HideAllScreens();
        homeScreen.SetActive(true);
        UpdateLeaderboard();
    }
    
    public void ShowNameInputScreen()
    {
        HideAllScreens();
        nameInputScreen.SetActive(true);
        nameInputField.text = "";
    }
    
    public void ShowGameUI()
    {
        HideAllScreens();
        gameScreen.SetActive(true);
    }
    
    public void ShowGameOverScreen(string reason)
    {
        gameOverScreen.SetActive(true);
        gameOverMessageText.text = reason;
    }
    
    public void ShowWinScreen(float timeTaken)
    {
        winScreen.SetActive(true);
        winMessageText.text = $"You escaped in {timeTaken:F1} seconds!\nCongratulations!";
    }
    
    void HideAllScreens()
    {
        homeScreen.SetActive(false);
        nameInputScreen.SetActive(false);
        gameScreen.SetActive(false);
        gameOverScreen.SetActive(false);
        winScreen.SetActive(false);
    }
    
    public void UpdateTimer(float timeRemaining)
    {
        timerText.text = $"Time: {Mathf.CeilToInt(timeRemaining)}s";
        
        // Change color when time is running out
        if (timeRemaining <= 10)
        {
            timerText.color = Color.red;
        }
        else if (timeRemaining <= 30)
        {
            timerText.color = Color.yellow;
        }
        else
        {
            timerText.color = Color.green;
        }
    }
    
    public void UpdateKeyStatus(bool hasKey)
    {
        if (hasKey)
        {
            keyStatusText.text = "🗝️ Has Key!";
            keyStatusText.color = Color.green;
        }
        else
        {
            keyStatusText.text = "🔒 Find Key";
            keyStatusText.color = Color.yellow;
        }
    }
    
    void UpdateLeaderboard()
    {
        // Clear existing entries
        foreach (Transform child in leaderboardContent)
        {
            Destroy(child.gameObject);
        }
        
        List<ScoreEntry> scores = GameManager.Instance.LoadScores();
        
        if (scores.Count == 0)
        {
            GameObject entry = Instantiate(leaderboardEntryPrefab, leaderboardContent);
            entry.GetComponent<TextMeshProUGUI>().text = "No scores yet. Be the first!";
            return;
        }
        
        for (int i = 0; i < scores.Count; i++)
        {
            GameObject entry = Instantiate(leaderboardEntryPrefab, leaderboardContent);
            entry.GetComponent<TextMeshProUGUI>().text = 
                $"{i + 1}. {scores[i].name} - {scores[i].time:F1}s";
        }
    }
    
    // Button callbacks
    public void OnPlayButtonClicked()
    {
        ShowNameInputScreen();
    }
    
    public void OnStartGameButtonClicked()
    {
        string playerName = nameInputField.text.Trim();
        
        if (string.IsNullOrEmpty(playerName))
        {
            // Show error or use default name
            playerName = "Anonymous";
        }
        
        GameManager.Instance.StartNewGame(playerName);
    }
    
    public void OnRestartButtonClicked()
    {
        GameManager.Instance.RestartGame();
    }
    
    public void OnQuitButtonClicked()
    {
        Application.Quit();
    }
}
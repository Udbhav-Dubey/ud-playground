import requests
response = requests.get('https://v2.jokeapi.dev/joke/Any')
print(response.json()) # For JSON data
print(response.text)   # For raw text


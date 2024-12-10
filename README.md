# XOXO-GAME
NETWORKING

# Networking
### Tic-Tac-Toe Game
## Overview
This is a simple multiplayer Tic-Tac-Toe game implemented using UDP  and TCPsockets in C. The game allows two players to play against each other in a turn-based manner. Players can make their moves, and the server checks for wins or draws.
## Features
1. Two-player gameplay: Supports two players, 'X' and 'O'.
2. Real-time board updates: Players receive updates after each move.
3. Win and draw detection: The server checks for a winner or a draw after each turn.
4. Replay option: Players can choose to play again after a game concludes.
## How to run the game
``gcc -o server server.c``
``gcc -o client client.c``
1. Run the server code in a terminal:``./server``
2. Open two additional terminal windows for the clients and run the client code:``./client``
## How to Play
1. Players take turns entering their moves by specifying the row and column (0-2).
2. The board is displayed after each move.
3. The game continues until one player wins or the game ends in a draw.
4. After the game, players can choose to play again by responding with "yes" or "no".

## Game Rules
1. The game board is a 3x3 grid.
2. Players alternate turns, placing their marker ('X' or 'O') in an empty cell.
3. The first player to align three markers vertically, horizontally, or diagonally wins.
4. If all cells are filled without a winner, the game is declared a draw.

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



## Networking

### XOXO [15 points]
You are required to design and implement a simple multiplayer Tic-Tac-Toe game using networking concepts. The game will have a server to manage the game state and two clients (players) who will play against each other.

#### Overview

- **Server**: The server will manage the game state (the Tic-Tac-Toe board), handle communication between the two players, and determine if there’s a winner or if the game ends in a draw.
- **Clients**: Two clients will connect to the server and send their moves. The server will update the game board and broadcast the updated game state to both players.

#### Game Rules:
- **The Board**: The tic-tac-toe board is a 3x3 grid.
- Players take turns to place their symbol (‘X’ or ‘O’) in an empty spot.
- The game begins with an empty board, and the server will send this initial state to both players once the game starts.

#### Starting the Game:
- The game will start when both players (clients) have connected to the server and confirmed they are ready to play.
- The server will assign Player 1 to use ‘X’ and Player 2 to use ‘O’.

#### Game Flow:
- The game alternates between Player 1 and Player 2. The server has to ensure this.
- Reject invalid moves and ask the player to try again.
- Each player takes turns selecting a position on the grid by specifying the row and column number (e.g., 1 1 for the top-left corner).
- After each move, the server updates the board and sends the current state of the board to both players. The clients should display it in a user-friendly manner.
- The server also informs the next player that it’s their turn to make a move.

#### Winning and Drawing:
- A player wins if they successfully place three of their symbols (‘X’ or ‘O’) in a row, column, or diagonal.
- If the grid is full and no player has won, the game ends in a draw.
- After a win or a draw, the server informs both players of the outcome, displaying the final board and the appropriate message:
  - “Player 1 Wins!”
  - “Player 2 Wins!”
  - “It’s a Draw!”

#### After the Game:
- Once the game ends, both players are asked if they would like to play again.
- If both say yes, the server will reset the board and start a new game.
- If both say no, the server closes the connection for both.
- If one player says yes and the other says no, the player who wanted to continue is informed that their opponent did not wish to play, and the connection for both is closed.

You are expected to implement this twice: once using TCP sockets and once with UDP.

---

### 2. Fake it till you make it [15 points]

We can’t really ask you to implement the entire TCP/IP stack from scratch for about twenty marks (But, for the ones interested here’s a repo on it - https://github.com/saminiir/level-ip).

In this specification, you will implement some TCP functionality using UDP.

#### Functionalities that you have to implement are:
- **Data Sequencing**: The sender (client or server - both should be able to send as well as receive data) must divide the data (assume some text) into smaller chunks (using chunks of fixed size or using a fixed number of chunks). Each chunk is assigned a number which is sent along with the transmission (use structs). The sender should also communicate the total number of chunks being sent [1]. After the receiver has data from all the chunks, it should aggregate them according to their sequence number and display the text.
  
- **Retransmissions**: The receiver must send an ACK packet for every data chunk received (The packet must reference the sequence number of the received chunk). If the sender doesn’t receive the acknowledgment for a chunk within a reasonable amount of time (say 0.1 seconds), it must resend the data. However, the sender shouldn’t wait for receiving acknowledgment for a previously sent chunk before transmitting the next chunk [2].

[1] Regardless of whether you use a fixed number of chunks.

[2] For implementation’s sake, send ACK messages randomly to check whether retransmission is working - say, skip every third chunk’s ACK. (Please comment out this code in your final submission)

Note: Simulating a single client and a single server is sufficient.

Submit your implementation for the server and client in `<mini-project2-directory>/networks/partB`.

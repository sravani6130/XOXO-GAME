#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
int row=-1, col=-1;

char board[3][3];
char currentplayer = 'X';

void initialize_board()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            board[i][j] = '-';
        }
    }
}

void print_board()
{
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("---|---|---\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("---|---|---\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
    printf("\n");
}

void make_move(int row, int col)
{
    if (board[row][col] == '-')
    {
        board[row][col] = currentplayer;
    }
}

int check_winner()
{
    for (int i = 0; i < 3; ++i)
    {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != '-')
        {
            return 1;
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != '-')
        {
            return 1;
        }
    }
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != '-')
    {
        return 1;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != '-')
    {
        return 1;
    }
    return 0;
}

int check_draw()
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (board[i][j] == '-')
            {
                return 0;
            }
        }
    }
    return 1;
}

void switch_player()
{
    currentplayer = (currentplayer == 'X') ? 'O' : 'X';
}

void send_board(int client1, int client2)
{
    send(client1, board, sizeof(board), 0);
    send(client2, board, sizeof(board), 0);
}

void handle_client_turn(int client, int other_client)
{
    // row = -1;
    // col = -1;
    // int flag = 1;
    while (1)
    {
        char buffer[BUFFER_SIZE] = {0};
        // if (flag = 1)
        // {
        //     send(client, "Your move", sizeof("Your move"), 0);
        // }
        recv(client, buffer, BUFFER_SIZE, 0);
        sscanf(buffer, "%d %d", &row, &col);

        if (board[row][col] == '-' && (row <= 2) && (col <= 2) && (row >= 0) && (col >= 0))
        {
            make_move(row, col);
            // flag = 1;
            printf("updated board:\n");
            print_board();
            break;
        }

        else
        {
            // flag = 0;
            send(client, "Invalid move. Try again.", sizeof("Invalid move. Try again."), 0);
        }
    }

    // send_board(client, other_client);
}

void handle_game(int client1, int client2)
{
    row = -1;
    col = -1;
    while (1)
    {

        char buffer[BUFFER_SIZE] = {0};
        sprintf(buffer, "Your move%d%d", row, col);

        send(client1, buffer, sizeof(buffer), 0);

        handle_client_turn(client1, client2);

        if (check_winner())
        {
            send(client1, "You win!", sizeof("You win!"), 0);
            send(client2, "You lose!", sizeof("You lose!"), 0);
            break;
        }

        if (check_draw())
        {
            send(client1, "Draw!", sizeof("Draw!"), 0);
            send(client2, "Draw!", sizeof("Draw!"), 0);
            break;
        }

        switch_player();

        sprintf(buffer, "Your move%d%d", row, col);
        send(client2, buffer, sizeof(buffer), 0);
        handle_client_turn(client2, client1);

        if (check_winner())
        {
            send(client2, "You win!", sizeof("You win!"), 0);
            send(client1, "You lose!", sizeof("You lose!"), 0);
            break;
        }

        if (check_draw())
        {
            send(client1, "Draw!", sizeof("Draw!"), 0);
            send(client2, "Draw!", sizeof("Draw!"), 0);
            break;
        }

        switch_player();
    }
}

int main()
{
    int server_fd, client1, client2;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Setting up the address struct
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listening for incoming connections
    if (listen(server_fd, 2) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");

    // Accepting two connections
    if ((client1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Player 1 connected.\n");
    // Assigning symbols to players
    send(client1, "X", 1, 0);
    if ((client2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Player 2 connected.\n");

    // Assigning symbols to players
    send(client2, "O", 1, 0);

    while (1)
    {
        initialize_board();
        print_board();

        // send_board(client1, client2);
        handle_game(client1, client2);

        // Ask both clients if they want to play again
        // send(client1, "Do you want to play again? (yes/no): ", sizeof("Do you want to play again? (yes/no): "), 0);

        recv(client1, buffer, BUFFER_SIZE, 0);
        int client1_wants_to_play = (strcmp(buffer, "yes") == 0);

        // send(client2, "Do you want to play again? (yes/no): ", sizeof("Do you want to play again? (yes/no): "), 0);

        recv(client2, buffer, BUFFER_SIZE, 0);
        int client2_wants_to_play = (strcmp(buffer, "yes") == 0);
        // printf("11:%d 22:%d", client1_wants_to_play, client2_wants_to_play);
        if (client1_wants_to_play && client2_wants_to_play)
        {
            send(client1, "agreed", sizeof("agreed"), 0);
            send(client2, "agreed", sizeof("agreed"), 0);
            continue; // Restart the game
        }
        else if (!client1_wants_to_play && !client2_wants_to_play)
        {
            send(client1, "Both players have quit. Closing connections.", sizeof("Both players have quit. Closing connections."), 0);
            send(client2, "Both players have quit. Closing connections.", sizeof("Both players have quit. Closing connections."), 0);
            break;
        }
        else if (client1_wants_to_play)
        {
            send(client1, "Your opponent does not want to play again. Closing connections.", sizeof("Your opponent does not want to play again. Closing connections."), 0);
            send(client2, "You have quit. Closing connections.", sizeof("You have quit. Closing connections."), 0);
            break;
        }
        else
        {
            send(client2, "Your opponent does not want to play again. Closing connections.", sizeof("Your opponent does not want to play again. Closing connections."), 0);
            send(client1, "You have quit. Closing connections.", sizeof("You have quit. Closing connections."), 0);
            break;
        }
    }

    close(client1);
    close(client2);
    close(server_fd);

    return 0;
}
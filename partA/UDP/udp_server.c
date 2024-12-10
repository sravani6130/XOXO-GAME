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

int row = -1, col = -1;
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

void send_board(int sock, struct sockaddr_in *client1_addr, struct sockaddr_in *client2_addr)
{
    sendto(sock, board, sizeof(board), 0, (struct sockaddr *)client1_addr, sizeof(*client1_addr));
    sendto(sock, board, sizeof(board), 0, (struct sockaddr *)client2_addr, sizeof(*client2_addr));
}

void handle_client_turn(int sock, struct sockaddr_in *client_addr, struct sockaddr_in *other_client_addr)
{
    while (1)
    {
        char buffer[BUFFER_SIZE] = {0};
        socklen_t addr_len = sizeof(*client_addr);

        recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)client_addr, &addr_len);
        sscanf(buffer, "%d %d", &row, &col);

        if (board[row][col] == '-' && (row <= 2) && (col <= 2) && (row >= 0) && (col >= 0))
        {
            make_move(row, col);
            printf("Updated board:\n");
            print_board();
            break;
        }
        else
        {
            sendto(sock, "Invalid move. Try again.", sizeof("Invalid move. Try again."), 0, (struct sockaddr *)client_addr, addr_len);
        }
    }
}

void handle_game(int sock, struct sockaddr_in *client1_addr, struct sockaddr_in *client2_addr)
{
    // printf("1\n");
    row = -1;
    col = -1;
    while (1)
    {
        char buffer[BUFFER_SIZE] = {0};
        sprintf(buffer, "Your move%d%d", row, col);

        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)client1_addr, sizeof(*client1_addr));

        handle_client_turn(sock, client1_addr, client2_addr);

        if (check_winner())
        {
            sendto(sock, "You win!", sizeof("You win!"), 0, (struct sockaddr *)client1_addr, sizeof(*client1_addr));
            sendto(sock, "You lose!", sizeof("You lose!"), 0, (struct sockaddr *)client2_addr, sizeof(*client2_addr));
            break;
        }

        if (check_draw())
        {
            sendto(sock, "Draw!", sizeof("Draw!"), 0, (struct sockaddr *)client1_addr, sizeof(*client1_addr));
            sendto(sock, "Draw!", sizeof("Draw!"), 0, (struct sockaddr *)client2_addr, sizeof(*client2_addr));
            break;
        }

        switch_player();

        sprintf(buffer, "Your move%d%d", row, col);
        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)client2_addr, sizeof(*client2_addr));
        handle_client_turn(sock, client2_addr, client1_addr);

        if (check_winner())
        {
            sendto(sock, "You win!", sizeof("You win!"), 0, (struct sockaddr *)client2_addr, sizeof(*client2_addr));
            sendto(sock, "You lose!", sizeof("You lose!"), 0, (struct sockaddr *)client1_addr, sizeof(*client1_addr));
            break;
        }

        if (check_draw())
        {
            sendto(sock, "Draw!", sizeof("Draw!"), 0, (struct sockaddr *)client1_addr, sizeof(*client1_addr));
            sendto(sock, "Draw!", sizeof("Draw!"), 0, (struct sockaddr *)client2_addr, sizeof(*client2_addr));
            break;
        }

        switch_player();
    }
}

int main()
{
    int sock;
    struct sockaddr_in server_addr, client1_addr, client2_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Setting up the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Binding the socket to the address and port
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");

    // Accepting two connections
    recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client1_addr, &addr_len);
    printf("Player 1 connected.\n");
    sendto(sock, "X", 1, 0, (struct sockaddr *)&client1_addr, addr_len);

    recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client2_addr, &addr_len);
    printf("Player 2 connected.\n");
    sendto(sock, "O", 1, 0, (struct sockaddr *)&client2_addr, addr_len);

    while (1)
    {
        initialize_board();
        print_board();

        handle_game(sock, &client1_addr, &client2_addr);

        // Ask both clients if they want to play again
        // sendto(sock, "Do you want to play again? (yes/no): ", sizeof("Do you want to play again? (yes/no): "), 0, (struct sockaddr *)&client1_addr, addr_len);
        memset(buffer, 0, BUFFER_SIZE);
        recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client1_addr, &addr_len);
        int client1_wants_to_play = (strcmp(buffer, "yes") == 0);

        // sendto(sock, "Do you want to play again1? (yes/no): ", sizeof("Do you want to play again? (yes/no): "), 0, (struct sockaddr *)&client2_addr, addr_len);
        memset(buffer, 0, BUFFER_SIZE);
        recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client2_addr, &addr_len);
        int client2_wants_to_play = (strcmp(buffer, "yes") == 0);

        if (client1_wants_to_play && client2_wants_to_play)
        {
            // printf("herre\n");
            sendto(sock, "agreed", sizeof("agreed"), 0, (struct sockaddr *)&client1_addr, addr_len);
            sendto(sock, "agreed", sizeof("agreed"), 0, (struct sockaddr *)&client2_addr, addr_len);
        }
        else if (!client1_wants_to_play && !client2_wants_to_play)
        {
            sendto(sock, "Both players have quit. Closing connections.", sizeof("Both players have quit. Closing connections."), 0, (struct sockaddr *)&client1_addr, addr_len);
            sendto(sock, "Both players have quit. Closing connections.", sizeof("Both players have quit. Closing connections."), 0, (struct sockaddr *)&client2_addr, addr_len);
            break;
        }
        else if (client1_wants_to_play)
        {
            sendto(sock, "Your opponent does not want to play again. Closing connections.", sizeof("Your opponent does not want to play again. Closing connections."), 0, (struct sockaddr *)&client1_addr, addr_len);
            sendto(sock, "Your opponent does not want to play again. Closing connections.", sizeof("Your opponent does not want to play again. Closing connections."), 0, (struct sockaddr *)&client2_addr, addr_len);
            break;
        }
        else
        {
            sendto(sock, "Your opponent does not want to play again. Closing connections.", sizeof("Your opponent does not want to play again. Closing connections."), 0, (struct sockaddr *)&client1_addr, addr_len);
            sendto(sock, "Your opponent does not want to play again. Closing connections.", sizeof("Your opponent does not want to play again. Closing connections."), 0, (struct sockaddr *)&client2_addr, addr_len);
            break;
        }
    }

    close(sock);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024
char board[3][3];

void print_board()
{
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("---|---|---\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("---|---|---\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
    printf("\n");
}

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

int main()
{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char symbol;
    int row, col;
    socklen_t addr_len = sizeof(serv_addr);

    // Create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;   // IPv4
    serv_addr.sin_port = htons(PORT); // Port

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    printf("Connected to server\n");

    // Send initial connection message to the server
    sendto(sock, "Client connected", strlen("Client connected"), 0, (struct sockaddr *)&serv_addr, addr_len);

    // Receive a message from the server indicating which symbol to use ('X' or 'O')
    recvfrom(sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
    symbol = buffer[0];
    printf("You are playing as '%c'\n", symbol);
    initialize_board();

    while (1)
    {
        // Receive the updated game state from the server
        recvfrom(sock, buffer, BUF_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
        // printf("%c %c\n", buffer[9], buffer[10]);
        char new1[2];
        new1[0] = buffer[9];
        new1[1] = '\0';
        char new2[2];
        new2[0] = buffer[10];
        new2[1] = '\0';
        row = atoi(new1);
        col = atoi(new2);
        if (buffer[9] == '-')
        {
            row = -1;
            col = -1;
        }
        if (row != -1)
        {
            if (symbol == 'X')
            {
                board[row][col] = 'O';
            }
            else
            {
                board[row][col] = 'X';
            }
        }

        print_board();
        if (strstr(buffer, "win") || strstr(buffer, "lose") || strstr(buffer, "Draw"))
        {
            printf("%s\n", buffer);
            printf("Do you want to play again? (yes/no): ");
            scanf("%s", buffer);
            sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, addr_len);
            char buffer1[BUF_SIZE];
            recvfrom(sock, buffer1, BUF_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
        // printf("bufer1: %s\n",buffer1);
            if (strcmp(buffer1, "agreed") == 0)
            {
                // printf("erieir\n");
                initialize_board();
                continue;
            }
            else
            {
                // printf("eerrorr\n");
                printf("%s\n", buffer1);
                break;
            }
        }
        else if (strcmp(buffer, "Invalid move. Try again.") == 0)
        {
            printf("%s\n", buffer);
            printf("Your move (row and column): ");
            scanf("%d %d", &row, &col);
            sprintf(buffer, "%d %d", row, col);
            sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, addr_len);
            board[row][col] = symbol;
            print_board();
        }
        else if (strncmp(buffer, "Your move", 9) == 0)
        {
            printf("Your move (row and column): ");
            scanf("%d %d", &row, &col);
            sprintf(buffer, "%d %d", row, col);
            sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, addr_len);
            if (board[row][col] == '-')
            {
                board[row][col] = symbol;
                print_board();
            }
            printf("waiting for opponent to play\n");
        }
    }

    close(sock);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 512
#define CHUNK_SIZE 7
#define TIMEOUT 0.1
typedef struct
{
    int seq_num;
    int total_chunks;
    char data[CHUNK_SIZE];
} Chunk;

typedef struct
{
    int seq_num;
} Ack;

void set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void receive_data(int sockfd, struct sockaddr_in *addr, socklen_t addr_len)
{
    Chunk chunks[100];
    int total_chunks = -1;
    int ack[100] = {0};
    int flag = 0;
    int count = 0;
    printf("------RECEIVING------\n");

    while (1)
    {
        Chunk chunk;
        int n = recvfrom(sockfd, &chunk, sizeof(chunk), MSG_WAITALL, (struct sockaddr *)addr, &addr_len);
        if (n > 0)
        {
            if (total_chunks == -1)
            {
                total_chunks = chunk.total_chunks;
                // printf("Total chunks to be received: %d\n", total_chunks);
            }
            // printf("Received chunk %d: %s\n", chunk.seq_num, chunk.data);

            // Simulating packet loss
            // if (chunk.seq_num == 2 && flag == 0) {
            //     flag++;
            //     printf("Skipped ACK for the chunk %d\n", chunk.seq_num);
            //     continue; // Skip sending ACK for this chunk
            // }
            count++;
            ack[chunk.seq_num] = 1;
            sendto(sockfd, &chunk.seq_num, sizeof(chunk.seq_num), MSG_CONFIRM, (const struct sockaddr *)addr, addr_len);
            // printf("ACK sent for the chunk %d\n", chunk.seq_num);
            chunks[chunk.seq_num] = chunk;
        }

        if (count == total_chunks)
        {
            // printf("Received chunks of total %d.\n", total_chunks);
            break;
        }
    }

    char reassembled_text[total_chunks * CHUNK_SIZE + 1];
    reassembled_text[0] = '\0';

    for (int i = 0; i < total_chunks; ++i)
    {
        strncat(reassembled_text, chunks[i].data, CHUNK_SIZE);
    }

    printf("Final data: %s\n", reassembled_text);
}
void send_data(int sockfd, struct sockaddr_in *addr, socklen_t addr_len, const char *text)
{
    int text_len = strlen(text);
    int total_chunks = (text_len + CHUNK_SIZE - 1) / CHUNK_SIZE;
    Chunk chunks[total_chunks];

    // Preparing chunks for sending
    for (int i = 0; i < total_chunks; ++i)
    {
        chunks[i].seq_num = i;
        chunks[i].total_chunks = total_chunks;
        strncpy(chunks[i].data, text + i * CHUNK_SIZE, CHUNK_SIZE);
        // printf("Chunk %d sent with chunk data: %s\n", i, chunks[i].data);
        sendto(sockfd, &chunks[i], sizeof(chunks[i]), MSG_CONFIRM, (const struct sockaddr *)addr, addr_len);
    }

    int acked[total_chunks];
    memset(acked, 0, sizeof(acked));

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < total_chunks; ++i)
    {
        while (!acked[i])
        {
            fd_set readfds;
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = TIMEOUT * 1000000;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);

            // Wait for ACK with timeout
            int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

            if (activity > 0)
            {
                int ack;
                recvfrom(sockfd, &ack, sizeof(ack), MSG_WAITALL, (struct sockaddr *)addr, &addr_len);
                // printf("Received ack_num for chunk %d\n", ack);
                acked[ack] = 1; // Mark this chunk as acknowledged
            }
            else
            {
                gettimeofday(&end, NULL);
                double elapsed = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
                if (elapsed >= TIMEOUT)
                {
                    // printf("Timeout occurred. Retransmitting the not acked chunks.\n");
                    sendto(sockfd, &chunks[i], sizeof(chunks[i]), MSG_CONFIRM, (const struct sockaddr *)addr, addr_len);
                    // printf("Chunk %d sent with chunk data: %s\n", i, chunks[i].data);
                    gettimeofday(&start, NULL);
                }
            }
        }
    }

    printf("All chunks are sent and acknowledged.\n");
}
int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(client_addr);
    set_nonblocking(sockfd);

    receive_data(sockfd, &client_addr, addr_len);
    const char *message = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    send_data(sockfd, &client_addr, addr_len, message);

    close(sockfd);
    return 0;
}
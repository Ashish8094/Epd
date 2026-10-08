#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // Socket variables
    int serverSocket, clientSocket;

    // Array to store 12-bit Hamming code
    int h[13] = {0};

    // Array to store 8 original data bits
    int data[9] = {0};

    int i;
    int choice;

    // Parity bits
    int p1, p2, p4, p8;

    // Stores the position of the error
    int errorPosition;

    // Stores decimal value of binary data
    int value = 0;

    // Stores received Hamming code as a string
    char codeword[13];

    // Structures for server and client address
    struct sockaddr_in server, client;

    // Stores size of client address
    socklen_t length = sizeof(client);

    printf("===== HAMMING CODE RECEIVER =====\n");

    // Create TCP socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Set server address family to IPv4
    server.sin_family = AF_INET;

    // Set port number as 9000
    server.sin_port = htons(9000);

    // Set server IP address
    server.sin_addr.s_addr = inet_addr("127.0.0.14");

    // Bind socket with IP address and port
    bind(serverSocket,
         (struct sockaddr *)&server,
         sizeof(server));

    // Start listening for incoming connections
    listen(serverSocket, 5);

    printf("\nWaiting for sender...\n");

    // Accept connection from sender
    clientSocket = accept(serverSocket,
                          (struct sockaddr *)&client,
                          &length);

    // Clear the codeword array
    memset(codeword, 0, sizeof(codeword));

    // Receive Hamming code from sender
    recv(clientSocket,
         codeword,
         sizeof(codeword) - 1,
         0);

    // Convert received characters into integer bits
    // Example: '1' - '0' = 1
    for(i = 1; i <= 12; i++)
        h[i] = codeword[i - 1] - '0';

    // Ask user whether to introduce an error
    printf("\n1. Data received without Error");
    printf("\n2. Data received with Error");
    printf("\nEnter choice: ");
    scanf("%d", &choice);

    // If choice is 2, introduce a random error
    if(choice == 2)
    {
        // Initialize random number generator
        srand(time(NULL));

        // Select a random position from 1 to 12
        errorPosition = rand() % 12 + 1;

        // Flip the selected bit
        // 0 becomes 1 and 1 becomes 0
        h[errorPosition] = h[errorPosition] ^ 1;

        // Display corrupted Hamming code
        printf("\nReceived Hamming Code: ");

        for(i = 1; i <= 12; i++)
            printf("%d", h[i]);

        printf("\n");
    }

    // Calculate P1 parity
    p1 = h[1] ^ h[3] ^ h[5] ^ h[7] ^ h[9] ^ h[11];

    // Calculate P2 parity
    p2 = h[2] ^ h[3] ^ h[6] ^ h[7] ^ h[10] ^ h[11];

    // Calculate P4 parity
    p4 = h[4] ^ h[5] ^ h[6] ^ h[7] ^ h[12];

    // Calculate P8 parity
    p8 = h[8] ^ h[9] ^ h[10] ^ h[11] ^ h[12];

    // Display parity bits in P8 P4 P2 P1 order
    printf("\nParity Bits (P8 P4 P2 P1): %d%d%d%d",
           p8, p4, p2, p1);

    // Calculate error position using parity bits
    // P1 = 1, P2 = 2, P4 = 4, P8 = 8
    errorPosition = p1 + (2 * p2) + (4 * p4) + (8 * p8);

    // If error position is 0, there is no error
    if(errorPosition == 0)
    {
        printf("\nNo Error Detected\n");
    }
    else
    {
        // Display the position of the error
        printf("\nError Detected at Position: %d",
               errorPosition);

        // Flip the wrong bit again to correct the error
        h[errorPosition] = h[errorPosition] ^ 1;

        printf("\nError Corrected");

        // Display corrected Hamming code
        printf("\nCorrected Hamming Code: ");

        for(i = 1; i <= 12; i++)
            printf("%d", h[i]);

        printf("\n");
    }

    // Extract the original 8 data bits
    // Data bits are stored at positions 3,5,6,7,9,10,11,12
    data[1] = h[3];
    data[2] = h[5];
    data[3] = h[6];
    data[4] = h[7];
    data[5] = h[9];
    data[6] = h[10];
    data[7] = h[11];
    data[8] = h[12];

    // Display recovered binary data
    printf("\nRecovered Binary Data: ");

    for(i = 1; i <= 8; i++)
    {
        // Print each data bit
        printf("%d", data[i]);

        // Convert binary number into decimal value
        value = value * 2 + data[i];
    }

    // Convert ASCII value back into character
    printf("\nRecovered Character: %c", value);

    // Display recovered ASCII value
    printf("\nASCII Value: %d\n", value);

    // Close connection with sender
    close(clientSocket);

    // Close server socket
    close(serverSocket);

    return 0;
}
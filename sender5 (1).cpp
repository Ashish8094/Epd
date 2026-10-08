#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

// Generate 12-bit Hamming Code
void generateHammingCode(int h[], int data[])
{
    // Place 8 data bits
    h[3]  = data[1];
    h[5]  = data[2];
    h[6]  = data[3];
    h[7]  = data[4];
    h[9]  = data[5];
    h[10] = data[6];
    h[11] = data[7];
    h[12] = data[8];

    // Calculate parity bits
    h[1] = h[3] ^ h[5] ^ h[7] ^ h[9] ^ h[11];
    h[2] = h[3] ^ h[6] ^ h[7] ^ h[10] ^ h[11];
    h[4] = h[5] ^ h[6] ^ h[7] ^ h[12];
    h[8] = h[9] ^ h[10] ^ h[11] ^ h[12];
}

int main()
{
    int sock;
    int h[13] = {0};
    int data[9] = {0};
    int ascii;
    int i;

    char ch;
    char codeword[13];

    struct sockaddr_in receiver;

    printf("===== HAMMING CODE SENDER =====\n");

    // Take one character as input
    printf("Enter first character of your name: ");
    scanf(" %c", &ch);

    // Get ASCII value
    ascii = (int)ch;

    printf("\nCharacter: %c", ch);
    printf("\nASCII Value: %d", ascii);

    // Convert ASCII to 8-bit binary
    for(i = 8; i >= 1; i--)
    {
        data[i] = ascii % 2;
        ascii = ascii / 2;
    }

    printf("\nBinary Data: ");

    for(i = 1; i <= 8; i++)
        printf("%d", data[i]);

    // Generate Hamming Code
    generateHammingCode(h, data);

    printf("\nGenerated Hamming Code: ");

    for(i = 1; i <= 12; i++)
    {
        printf("%d", h[i]);

        // Convert bit into character
        codeword[i - 1] = h[i] + '0';
    }

    codeword[12] = '\0';

    // Create TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Receiver address
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);

    // Roll No. = 11
    receiver.sin_addr.s_addr = inet_addr("127.0.0.14");

    // Connect to receiver
    connect(sock,
            (struct sockaddr *)&receiver,
            sizeof(receiver));

    // Send Hamming Code
    send(sock, codeword, strlen(codeword), 0);

    printf("\nHamming Code Sent Successfully\n");

    // Close socket
    close(sock);

    return 0;
}

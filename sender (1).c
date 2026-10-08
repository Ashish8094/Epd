#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

// Receiver port
#define PORT 9000

// Sender port
#define SENDER_PORT 9001

// Total number of frames
#define TOTAL_FRAMES 8

// Maximum frames that can be sent at once
#define WINDOW_SIZE 4

int main()
{
    int sock;
    int choice;

    // Base of the current sliding window
    // represents the first unacknowledged frame in the current window
    int base = 0;

    // Stores the next frame that needs to be sent.
    int nextFrame = 0;

    // Frame that will be intentionally lost
    int lostFrame;

    // Used to make sure only one frame is lost
    int lossDone = 0;

    // Stores received ACK number
    int ack;

    // message is used for messages like:
        // GBN
        // SR
    char message[50];
    // buffer is used for data such as:
    // DATA:3
    // ACK:3
    char buffer[50];

    // Structures for sender and receiver addresses
    struct sockaddr_in sender, receiver;

    // Size of receiver address
    socklen_t length = sizeof(receiver);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // ---------------- SENDER ADDRESS ----------------

    // Use IPv4
    sender.sin_family = AF_INET;

    // Set sender port
    sender.sin_port = htons(SENDER_PORT);

    // Set sender IP address
    sender.sin_addr.s_addr = inet_addr("127.0.0.14");

    // Bind socket to sender address
    bind(sock, (struct sockaddr *)&sender, sizeof(sender));

    // ---------------- RECEIVER ADDRESS ----------------

    // Use IPv4
    receiver.sin_family = AF_INET;

    // Set receiver port
    receiver.sin_port = htons(PORT);

    // Set receiver IP address
    receiver.sin_addr.s_addr = inet_addr("127.0.0.14");

    printf("===== SLIDING WINDOW SENDER =====\n");

    // Ask user to select protocol
    printf("\n1. Go Back N");
    printf("\n2. Selective Repeat");
    printf("\nEnter choice: ");
    scanf("%d", &choice);

    // Store selected protocol in a message
    if(choice == 1)
        strcpy(message, "GBN");
    else
        strcpy(message, "SR");

    // Send selected protocol to receiver
    sendto(sock, message, strlen(message), 0,
           (struct sockaddr *)&receiver, length);

    printf("\nProtocol: %s\n", message);

    // Initialize random number generator
    srand(time(NULL));

    // Select one random frame for loss
    lostFrame = rand() % TOTAL_FRAMES;

    printf("Random frame selected for loss: %d\n", lostFrame);


    /*
       =================================
              GO BACK N PROTOCOL
       =================================
    */

    if(choice == 1)
    {
        printf("\n--- Go Back N Transmission ---\n");

        // Continue until all frames are acknowledged
        while(base < TOTAL_FRAMES)
        {
            // Send all frames inside the current window
            while(nextFrame < base + WINDOW_SIZE &&
                  nextFrame < TOTAL_FRAMES)
            {
                // Create data message
                sprintf(buffer, "DATA:%d", nextFrame);

                // Simulate loss of one frame
                if(nextFrame == lostFrame && lossDone == 0)
                {
                    printf("Frame %d lost\n", nextFrame);

                    // Mark that loss has already happened
                    lossDone = 1;
                }
                else
                {
                    // Send frame to receiver
                    sendto(sock, buffer, strlen(buffer), 0,
                           (struct sockaddr *)&receiver, length);

                    printf("Sent Frame %d\n", nextFrame);
                }

                // Move to next frame
                nextFrame++;
            }

            // Set timeout of 2 seconds
            struct timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;

            // Apply timeout to socket
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                       &timeout, sizeof(timeout));

            // Wait for ACK from receiver
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&receiver, &length);

            // If ACK is received
            if(n > 0)
            {
                // Add string ending character
                buffer[n] = '\0';

                // Extract ACK number
                sscanf(buffer, "ACK:%d", &ack);

                printf("Received ACK %d\n", ack);

                // Move window forward
                if(ack >= base)
                    base = ack + 1;
            }
            else
            {
                // No ACK received within timeout
                printf("\nTimeout occurred\n");

                // Go Back N retransmits all frames
                // starting from the missing frame
                printf("Retransmitting frames from %d\n", base);

                for(int i = base; i < nextFrame; i++)
                {
                    // Create frame message again
                    sprintf(buffer, "DATA:%d", i);

                    // Retransmit frame
                    sendto(sock, buffer, strlen(buffer), 0,
                           (struct sockaddr *)&receiver, length);

                    printf("Retransmitted Frame %d\n", i);
                }
            }
        }
    }


    /*
       =================================
            SELECTIVE REPEAT PROTOCOL
       =================================
    */

    else if(choice == 2)
    {
        // Stores whether each frame has been acknowledged
        int acknowledged[TOTAL_FRAMES] = {0};

        // Stores whether each frame has already been sent
        int sent[TOTAL_FRAMES] = {0};

        printf("\n--- Selective Repeat Transmission ---\n");

        // Continue until all frames are acknowledged
        while(base < TOTAL_FRAMES)
        {
            // Send frames inside the current window
            for(int i = base;
                i < base + WINDOW_SIZE && i < TOTAL_FRAMES;
                i++)
            {
                // Send only if frame has not been sent before
                if(sent[i] == 0)
                {
                    // Create data message
                    sprintf(buffer, "DATA:%d", i);

                    // Simulate loss of one frame
                    if(i == lostFrame && lossDone == 0)
                    {
                        printf("Frame %d lost\n", i);

                        // Mark loss as completed
                        lossDone = 1;
                    }
                    else
                    {
                        // Send frame to receiver
                        sendto(sock, buffer, strlen(buffer), 0,
                               (struct sockaddr *)&receiver, length);

                        printf("Sent Frame %d\n", i);
                    }

                    // Mark frame as sent
                    sent[i] = 1;
                }
            }

            // Set timeout of 2 seconds
            struct timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;

            // Apply timeout to socket
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                       &timeout, sizeof(timeout));

            // Wait for ACK
            int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&receiver, &length);

            // If ACK is received
            if(n > 0)
            {
                // Add string ending character
                buffer[n] = '\0';

                // Extract ACK number
                sscanf(buffer, "ACK:%d", &ack);

                printf("Received ACK %d\n", ack);

                // Mark this frame as acknowledged
                acknowledged[ack] = 1;

                // Move window forward while frames
                // are continuously acknowledged
                while(base < TOTAL_FRAMES &&
                      acknowledged[base] == 1)
                {
                    base++;
                }
            }
            else
            {
                // No ACK received within timeout
                printf("\nTimeout occurred\n");

                // Retransmit only the frames
                // which are not acknowledged
                for(int i = base;
                    i < base + WINDOW_SIZE && i < TOTAL_FRAMES;
                    i++)
                {
                    // Check if frame is not acknowledged
                    if(acknowledged[i] == 0)
                    {
                        // Create frame message
                        sprintf(buffer, "DATA:%d", i);

                        // Retransmit only this frame
                        sendto(sock, buffer, strlen(buffer), 0,
                               (struct sockaddr *)&receiver, length);

                        printf("Retransmitted Frame %d\n", i);
                    }
                }
            }
        }
    }

    // All frames have been successfully transmitted
    printf("\nAll frames transmitted successfully.\n");

    // Close UDP socket
    close(sock);

    return 0;
}
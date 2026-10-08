#include <stdio.h>          // For printf(), sprintf(), sscanf()
#include <stdlib.h>         // For general utility functions
#include <string.h>         // For strcpy(), strcmp(), strlen()
#include <arpa/inet.h>      // For socket address functions
#include <unistd.h>         // For close()

#define PORT 9000           // Port number used for communication
#define TOTAL_FRAMES 8      // Total number of frames

int main()
{
    int sock;                               // Socket descriptor
    int expected = 0;                       // Next frame expected
    int received[TOTAL_FRAMES] = {0};       // Stores received frames

    char buffer[50];                        // Buffer for messages
    char protocol[10];                      // Stores protocol name

    // Structures for receiver and sender addresses
    struct sockaddr_in receiver, sender;

    // Stores size of sender address
    socklen_t length = sizeof(sender);

    // Create a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Set receiver address family
    receiver.sin_family = AF_INET;

    // Set receiver port
    receiver.sin_port = htons(PORT);

    // Set receiver IP address
    receiver.sin_addr.s_addr = inet_addr("127.0.0.11");

    // Bind socket to receiver address
    bind(sock, (struct sockaddr *)&receiver, sizeof(receiver));

    // Display receiver heading
    printf("===== SLIDING WINDOW RECEIVER =====\n");

    // Wait for sender
    printf("\nWaiting for sender...\n");

    // Receive protocol name from sender
    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)&sender, &length);

    // Add string terminating character
    buffer[n] = '\0';

    // Copy protocol name into protocol variable
    strcpy(protocol, buffer);

    // Display selected protocol
    printf("\nProtocol: %s\n", protocol);


    /*
       ============================
       GO BACK N
       ============================
    */

    // Check if Go Back N is selected
    if(strcmp(protocol, "GBN") == 0)
    {
        // Display Go Back N heading
        printf("\n--- Go Back N Receiver ---\n");

        // Continue until all frames are received
        while(expected < TOTAL_FRAMES)
        {
            // Receive a frame from sender
            n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&sender, &length);

            // Add string terminating character
            buffer[n] = '\0';

            int frame;              // Stores received frame number

            // Extract frame number from DATA message
            sscanf(buffer, "DATA:%d", &frame);

            // Display received frame
            printf("Received Frame %d\n", frame);

            // Check if correct frame is received
            if(frame == expected)
            {
                // Accept the correct frame
                printf("Frame %d accepted\n", frame);

                // Create ACK message
                sprintf(buffer, "ACK:%d", frame);

                // Send ACK to sender
                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&sender, length);

                // Move to next expected frame
                expected++;
            }
            else
            {
                // Discard out-of-order frame
                printf("Frame %d discarded\n", frame);

                // ACK last correctly received frame
                sprintf(buffer, "ACK:%d", expected - 1);

                // Send ACK to sender
                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&sender, length);
            }
        }
    }


    /*
       ============================
       SELECTIVE REPEAT
       ============================
    */

    // Check if Selective Repeat is selected
    else if(strcmp(protocol, "SR") == 0)
    {
        // Display Selective Repeat heading
        printf("\n--- Selective Repeat Receiver ---\n");

        // Continue until all frames are received
        while(expected < TOTAL_FRAMES)
        {
            // Receive a frame from sender
            n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&sender, &length);

            // Add string terminating character
            buffer[n] = '\0';

            int frame;              // Stores received frame number

            // Extract frame number from DATA message
            sscanf(buffer, "DATA:%d", &frame);

            // Display received frame
            printf("Received Frame %d\n", frame);

            // Check if frame is not already received
            if(received[frame] == 0)
            {
                // Mark frame as received
                received[frame] = 1;

                // Display accepted frame
                printf("Frame %d accepted and buffered\n",
                       frame);

                // Create ACK for this frame
                sprintf(buffer, "ACK:%d", frame);

                // Send ACK to sender
                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&sender, length);
            }

            // Deliver consecutive frames in correct order
            while(expected < TOTAL_FRAMES &&
                  received[expected] == 1)
            {
                // Deliver the expected frame
                printf("Frame %d delivered\n", expected);

                // Move to next expected frame
                expected++;
            }
        }
    }

    // Display successful completion message
    printf("\nAll frames received successfully.\n");

    // Close the socket
    close(sock);

    // End the program
    return 0;
}
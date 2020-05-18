// Name: Anishkumar SS
// ID No: 2017A7PS0069P
// Question 1

#include "packet.h"

int main()
{
    // All necessary declarations
    int socket_1, socket_2, add_len, activity, ret, i, sd, max_sd, offset;
    PKT datapack1, datapack2, ackpack;
    char data[PACKET_SIZE];
    struct sockaddr_in serv_addr;

    // Creating 2 sockets on the client side
    if ((socket_1 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        die("Error in creating socket 1/n");
    if ((socket_2 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        die("Error in creating socket 2\n");

    printf("2 sockets Created\n");

    // Information about the server address
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5001);

    // Establishing the connection
    if (connect(socket_1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        die("Connection error at socket 1");
    if (connect(socket_2, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        die("Connection error at socket 2");

    printf("Connection Established in 2 Sockets\n");

    // Opening the source file
    FILE *fp = fopen("source_file.txt", "rb");
    if (fp == NULL)
    {
        printf("File open error");
        return 1;
    }
    printf("File opened\n");

    struct timeval *tv = (struct timeval *)malloc(sizeof(struct timeval));
    max_sd = (socket_1 > socket_2) ? socket_1 : socket_2;
    fd_set rset;

    //Sending the first set of data outside the while loop
    offset = 0; // Initially the offset it set to be zero.

    // Constructing the datapacket to be sent
    int bytes_read = fread(data, 1, PACKET_SIZE, fp);
    datapack1.size = bytes_read;
    datapack1.sq_no = offset;
    datapack1.lastflg = 0;
    datapack1.ackdataflg = 0;
    strcpy(datapack1.data, data);

    if (bytes_read != PACKET_SIZE) // If the size of data is not PACKET_SIZE, then it is the last packet of the file transfer.
        datapack1.lastflg = 1;

    offset += bytes_read; // Updating offset
    send(socket_1, &datapack1, sizeof(datapack1), 0);
    printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 1\n", datapack1.sq_no, datapack1.size);

    if (bytes_read == PACKET_SIZE) //
    {
        // Sending Data from 2nd Socket.
        bytes_read = fread(data, 1, sizeof(data), fp);
        datapack2.size = bytes_read;
        datapack2.sq_no = offset;
        datapack2.lastflg = 0;
        datapack2.ackdataflg = 0;
        strcpy(datapack2.data, data);

        if (bytes_read != PACKET_SIZE)
            datapack1.lastflg = 1;

        offset += bytes_read;
        send(socket_2, &datapack2, sizeof(datapack2), 0);
        printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 2\n", datapack2.sq_no, datapack2.size);
    }

    // Entering the Loop
    while (1)
    {

        // Setting out timeout variables declared in header file.
        tv->tv_sec = TIMEOUT;
        tv->tv_usec = 0;
        FD_ZERO(&rset);
        FD_SET(socket_1, &rset);
        FD_SET(socket_2, &rset);

        activity = select(max_sd, &rset, NULL, NULL, tv);

        if (activity == 0) // Both processes have timed out, so have to resend both
        {
            printf("Timeout Occured\n Resending both packets\n");
            send(socket_1, &datapack1, sizeof(datapack1), 0);
            printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 1\n", datapack1.sq_no, datapack1.size);
            if (datapack1.size == PACKET_SIZE)
            {
                send(socket_2, &datapack2, sizeof(datapack2), 0);
                printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 2\n", datapack2.sq_no, datapack2.size);
            }
            continue;
        }

        if (FD_ISSET(socket_1, &rset)) // If reply detected in socket_1
        {
            if ((ret = recv(socket_1, &ackpack, sizeof(ackpack), 0)) == -1)
                die("Error in recieving ACK");

            if (ackpack.sq_no == datapack1.size + datapack1.sq_no) // When it recieves the ACK for the correct packet
            {
                printf("RCVD ACK: for PKT with Seq_no: %d from channel = 1\n", ackpack.sq_no);
                if(ackpack.lastflg == 1)
                    end_conn(socket_1, socket_2, fp);

                bytes_read = fread(data, 1, sizeof(data), fp);
                datapack1.size = bytes_read;
                datapack1.sq_no = offset;
                datapack1.lastflg = 0;
                datapack1.ackdataflg = 0;
                strcpy(datapack1.data, data);
                if (bytes_read != PACKET_SIZE)
                    datapack1.lastflg = 1;
                offset += bytes_read;

                // When the ACK of the 1st packet from channel 1 is detected
                // Due to the inability of maintaining two timers.
                // We choose to send the next batch of data through channel 1
                // and resend the packet from channel 2
                send(socket_2, &datapack2, sizeof(datapack2), 0);
                printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 2\n", datapack2.sq_no, datapack2.size);
                printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 1\n", datapack1.sq_no, datapack1.size);
                send(socket_1, &datapack1, sizeof(datapack1), 0);

                continue;
            }
            else // Ignore if the ACK is for a previous packet
                continue;
        }

        if (FD_ISSET(socket_2, &rset))
        {
            if ((ret = recv(socket_2, &ackpack, sizeof(ackpack), 0)) == -1)
                die("Error in recieving ACK in Socket 2\n");

            if (ackpack.sq_no == datapack2.size + datapack2.sq_no)
            {
                printf("RCVD ACK: for PKT with Seq_no: %d from channel = 2\n", ackpack.sq_no);
                if(ackpack.lastflg == 1)
                    end_conn(socket_1, socket_2, fp);

                bytes_read = fread(data, 1, sizeof(data), fp); // The next set of bytes are captured;
                datapack2.size = bytes_read;
                datapack2.sq_no = offset;
                datapack2.lastflg = 0;
                datapack2.ackdataflg = 0;
                strcpy(datapack2.data, data);
                if (bytes_read != PACKET_SIZE)
                    datapack2.lastflg = 1;
                offset += bytes_read;

                send(socket_1, &datapack1, sizeof(datapack1), 0);
                printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 1\n", datapack1.sq_no, datapack1.size);
                send(socket_2, &datapack2, sizeof(datapack2), 0);
                printf("SENT PKT: Sq_No: %d , Size = %d ,Channel = 2\n", datapack2.sq_no, datapack2.size);

                continue;
            }
            else
                continue;
        }
    }

    close(socket_1);
    close(socket_2);
    fclose(fp);
}
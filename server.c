// Name: Anishkumar SS
// ID No: 2017A7PS0069P
// Question 1

#include "packet.h"

int main()
{
    // All necessary declarations
    srand(time(0)); 
    int master_sock, add_len, activity, ret, i, sd, max_sd;
    int new_socket, client[2] = {0};
    int exp_offset = 0;                                     
    PKT datapack, ackpack;
    struct buffer_system *buff_head = (struct buffer_system *)malloc(sizeof(struct buffer_system));
    buff_head->size = 0;
    struct sockaddr_in serv_addr;
    fd_set readfds;

    // Creating sockets for server
    if ((master_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        die("Error creating Socket 1 ");

    printf("Server sockets created successfully \n");

    // Details of the server address.
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5001);
    add_len = sizeof(serv_addr);

    // Binding of Socket to server address
    ret = bind(master_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
        die("Error while binding");

    printf("Binding Complete \n");

    // Listening on the Master socket
    if (listen(master_sock, 3) == -1)
        die("Failed at listen");
    printf("Listening\n");

    // Creating the destination file.
    FILE *fp;
    fp = fopen("destination_file.txt", "w+");
    if (NULL == fp)
        die("Error opening file");
    printf("Created the destination file\n");

    // Establishing connection on the outside
    if ((client[0] = accept(master_sock, (struct sockaddr *)&serv_addr, (socklen_t *)&add_len)) < 0)
        die("select error");
    printf("New connection : Socket fd is %d , ip : %s ,port: %d \n", client[0], inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
    if ((client[1] = accept(master_sock, (struct sockaddr *)&serv_addr, (socklen_t *)&add_len)) < 0)
        die("select error");
    printf("New connection: Socket fd is %d , ip : %s, port: %d \n", client[1], inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
    printf("Both connections have been established\n");

    // Beggining with the Loop
    max_sd = (client[0] > client[1]) ? client[0] : client[1];
    while (1)
    {
        // Monitoring both the client sockets using fd_set
        FD_ZERO(&readfds);
        FD_SET(client[0], &readfds);
        FD_SET(client[1], &readfds);

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR))
            die("Error in Select");

        for (i = 0; i < 2; i++)
        {
            sd = client[i];
            if (FD_ISSET(sd, &readfds)) // If the activity is detected in respective channel
            {
                if ((ret = recv(sd, &datapack, sizeof(datapack), 0)) == -1)
                    die("Error in recieving data");

                // For dropping packets with the probability of PDR defined as Macro
                if ((rand() % 100) < PDR)
                {
                    printf("Packet Dropped\n");
                    break;
                }

                printf("RCVD PKT: Sq_No: %d , Size = %d ,Channel = %d\n", datapack.sq_no, datapack.size, i + 1);

                if (exp_offset == datapack.sq_no) // If message is what was expected, there is a direct write option without any buffer
                {
                    fwrite(datapack.data, 1, datapack.size, fp);
                    exp_offset += datapack.size;
                    
                    // Looking up at buffer to rewrite whatever fits in
                    while (buff_head && buff_head->packet.sq_no == exp_offset)
                    {
                        fwrite(buff_head->packet.data, 1, buff_head->packet.size, fp);
                        exp_offset += buff_head->packet.size;

                        if (buff_head->next)
                            buff_head->next->size = buff_head->size - 1;

                        buff_head = buff_head->next;
                    }

                    // Now to send the ACK to client
                    ackpack.sq_no = exp_offset;
                    ackpack.ackdataflg = 1;

                    if (datapack.size != PACKET_SIZE)
                        ackpack.lastflg = 1;

                    send(sd, &ackpack, sizeof(ackpack), 0);
                    printf("SENT ACK: for PKT with Seq no: %d in Channel: %d \n", ackpack.sq_no, i + 1);

                    // If the recieved packet does not have data of size PACKET_SIZE, then it is considered to be the last packet sent.
                    // Alternatively it can also be checked with datapack.lastflag == 1.
                    if (datapack.size != PACKET_SIZE) 
                        end_conn(client[0], client[1], fp);

                }
                else if(exp_offset < datapack.sq_no)  // When the offset of packet recieved does not match with expected offset, then we buffer the packet in a Linked List. 
                {
                    printf("Buffering data\n");
                    struct buffer_system *new_buff = (struct buffer_system *)malloc(sizeof(struct buffer_system));
                    new_buff->packet = datapack;
                    new_buff->next = NULL;

                    // If the linked List is empty, we choose new_buff as the head of linked list
                    if (buff_head->size == 0) 
                    {
                        buff_head = new_buff;
                        break;
                    }

                    // If the new_buffer replaces the original LL header as it has a smaller sq_no
                    if (buff_head->packet.sq_no > new_buff->packet.sq_no) 
                    {
                        new_buff->next = buff_head;
                        buff_head = new_buff;
                        buff_head->size = buff_head->next->size + 1;
                        break;
                    }
                    else 
                    {
                        // We add the new buffer value in an ascending sort manner, 
                        // So that it becomes easier while writing back

                        buff_head->size++;
                        struct buffer_system *front = buff_head->next;
                        struct buffer_system *back = buff_head;

                        while (front && (!(new_buff->packet.sq_no > back->packet.sq_no && new_buff->packet.sq_no < front->packet.sq_no)))
                        {
                            back = front;
                            front = front->next;
                        }

                        back->next = new_buff;
                        new_buff->next = front;
                        break;
                    }
                }
            }
        }
    }
}

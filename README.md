# Socket Programming 
An attempt at solving Computer Network (CSF303) assignment written in C from scratch.

## EXECUTION INSTRUCTIONS:

1. Run the following commands to create an executable for client and server
    $gcc server.c -o server 
    $gcc client.c -o client 

2. Create a source file and name it as "source_file.txt", and fill it with relevant text information.

3. Once output files are created, use two different terminals to run the executables in the given order.
    Terminal 1 : $./server
    Terminal 2 : $./client

4. Open "destination_file.txt" containing the data that has been transferred through the network.

## METHODOLOGY:

1. In order to emulate two communication channels, we create 2 sockets in the client and server side, which communicates independently. 

2. Independent connection is established between socket-socket pairs on server and client, so that Packets and corresponding ACK's travel through the same channel.

3. The packets transmitted are structures which contain information specified in the question

4. Timeout duration , Packet size and Packet drop rates have been declared as Macros in "packet.h" and can be altered.

5. The packet dropping has been integrated in the server program using rand() function.

6. The buffer feature has been integrated in the server program using linked list. Every new entry in this linked list is stored in the ascending order of sequence number, making it easy to write in the end.

7. Timeouts in the client sides are handled using select() and timeval. 
  However, the program has been designed with a single timer. So in the event where we have succesfull ACK response from both channels, the one that is being detected first by select() is the only one considered, the packet from other channel has to be resent. 


## GENERAL WORKING PROCESS:

1. Sockets are created and connected on both ends.

2. Client sends the first 2 packets with 1 packet each using 2 channels. Eg. 1st packet has first 100 bytes and 2nd packet has the next 100 bytes.

3. Once the server recieves the packet, it determines whether or not the packet has to be dropped using rand() and PDR. 

4. If the packet is accepted in the server side, then it checks if contains the information that has to be written down next:
    C1: If there is a mismatch in sequence number and expected offset.
        > The packet is buffered in a linked list or ignored if 
        > Send ACK with expected offset which is not changed.

    C2; If the packet matches the expected offset
        > Write data from current packet and update offset
        > Look to see if the buffer contains information that can be used with the updated expected offset
        > Send ACK with updated expected offset.

5. On the client side, we use select() with a timeout to monitor channel. The cases there 
    C1: No activity in the duration of Timeout.
        > We resend both packets from the respective channel, without reconstucting them.

    C2: Activity detected on a channel
        > We check to see if it's the ACK for recently sent packet from that channel, 
          If it's an ACK from previous packet, then we ignore it.
          else we send the next batch of data through that channel.
        > The packet from the other channel is resent by default.

6. The program terminates when the ACK of the final byte is sent back.

##### NOTE: If error arises after multiple runs of the program, try running them on new terminals


    

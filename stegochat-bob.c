/**
 * StegoChat (Bob's side)
 * Connects to another program and sends the message via modifying the TTLs of network packets
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
	char dickens[19][100];
	strcpy(dickens[0], "It was the best of times, it was the worst of times,");
	strcpy(dickens[1], "it was the age of wisdom, it was the age of foolishn");
	strcpy(dickens[2], "ess, it was the epoch of belief, it was the epoch of");
	strcpy(dickens[3], " incredulity, it was the season of Light, it was the");
	strcpy(dickens[4], " season of Darkness, it was the spring of hope, it w");
	strcpy(dickens[5], "as the winter of despair, we had everything before u");
	strcpy(dickens[6], "s, we had nothing before us, we were all going direc");
	strcpy(dickens[7], "t to Heaven, we were all going direct the other way—");
	strcpy(dickens[8], " in short, the period was so far like the present pe");
	strcpy(dickens[9], "riod, that some of its noisiest authorities insisted");
	strcpy(dickens[10], " on its being received, for good or for evil, in the");
	strcpy(dickens[11], " superlative degree of comparison only. There were a");
	strcpy(dickens[12], " king with a large jaw and a queen with a plain face");
	strcpy(dickens[13], ", on the throne of England; there were a king with a");
	strcpy(dickens[14], " large jaw and a queen with a fair face, on the thro");
	strcpy(dickens[15], "ne of France. In both countries it was clearer than ");
	strcpy(dickens[16], "crystal to the lords of the State preserves of loave");
	strcpy(dickens[17], "s and fishes, that things in general were settled fo");
	strcpy(dickens[18], "r ever.");
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	int fd;
	struct sockaddr_in addr;
	struct msghdr msg; 
	struct iovec iov[1];  
	int one = 1;
	char pkt[1024];
	char buf[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;
	int *ttlptr=NULL;

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1) {
   	perror( "socket" );
   	return 1;
	}

	if (setsockopt(fd,IPPROTO_IP,IP_RECVTTL,&one,sizeof(int)) == -1) {
		perror( "setsockopt" );
		return 1;
	}

	/* listen on port 20077 */
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(20077);
	bind(fd,(struct sockaddr *)&addr,sizeof(addr));	

	/* initialize the message for recvmsg */
	struct sockaddr_in alice; // Struct for Alice's contact info
	alice.sin_addr.s_addr = inet_addr("127.0.0.1");

	memset(&msg, '\0', sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	iov[0].iov_base = pkt; /* starting point of buffer */
	iov[0].iov_len = sizeof(pkt);
	msg.msg_control = buf; // Assign buffer space for control header + header data/value
	msg.msg_controllen = sizeof(buf); //just initializing it
	msg.msg_name = &alice;
	msg.msg_namelen = sizeof(alice);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	int keep_communicating = 1; // Do we still communicate
	int is_done = 0; // The message is done
	int counter = 0; // Index to put the hidden char in secret_message
	int ttl_done = 200; // There's no more data coming if you see this TTL
	char ttlchar; // Char the TTL represents
	char secret_message[1000] = ""; // TTL-hidden message
	char passage[100]; // Dickens passage
	int dickens_counter = 0; // Which Dickens passage do we use?
	int index, c; // Placeholders
	char quit[] = "QUIT"; // Message to end chat

	while (keep_communicating == 1) {// Do we still communicate?
		// Receive data
		while (is_done == 0) { // While there's still data to get
			// actually receive the messages
			if (recvmsg(fd, &msg, 0) == -1) {
				perror( "recvmsg" );
				close(fd);
				return 1;
			} // End recvmsg if
			// loop through control messages
			for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg,cmsg)) {
				if ( (cmsg->cmsg_level == IPPROTO_IP) && (cmsg->cmsg_type == IP_TTL) && (cmsg->cmsg_len) ) {
					ttlptr = (int *) CMSG_DATA(cmsg); // Get the TTL
					//printf("received_ttl = %d \n", *ttlptr );
					if (*ttlptr == ttl_done) { // Message over
						is_done = 1;
						break;
					}
					else {
						ttlchar = *ttlptr; // Get the TTL's char value
						secret_message[counter] = ttlchar; // Put it in secret_message
						counter++; // Move to the next slot
					}
				} // End if
			} // End for
		} // End recv while
		if ((secret_message[0] == quit[0]) && (secret_message[1] == quit[1]) && (secret_message[2] == quit[2]) && (secret_message[3] == quit[3])) {
			printf("Other user quit.\n");
			keep_communicating = 0; // If the message is QUIT, break off the program
			break;
		}
		printf("Message received: %s\n", secret_message); // Print it
		memset(secret_message, 0, sizeof(secret_message)); // Clear secret_message for the next one
		is_done = 0; // Reset vars for next recv
		counter = 0; 

		/////////////////////////////////////////////////////////////////////////////////

		// Send data
		char message[1000]; 
		printf("What's the message to encode? : "); // Get message to hide in TTL
		gets(message);
		if ((message[0] == quit[0]) && (message[1] == quit[1]) && (message[2] == quit[2]) && (message[3] == quit[3])) {
			printf("Goodbye.\n");
			keep_communicating = 0; // If the message is QUIT, break off the program
		}
		for (index = 0; index < strlen(message); index++) { // For each char in message
			c = message[index];
			strcpy(passage, dickens[dickens_counter]);
			setsockopt(fd, IPPROTO_IP, IP_TTL, &c, sizeof(c)); // Send a random Dickens passage with our modified TTL
			if ((numbytes = sendto(fd, passage, strlen(passage), 0, (struct sockaddr*)&alice, sizeof(alice))) == -1) {
				perror("bob: sendto");
				exit(1);
			}
			dickens_counter++; // Tweak Dickens data
			if (dickens_counter > 18) dickens_counter = 0;
		}
		setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl_done, sizeof(ttl_done)); // Our message is done
		if ((numbytes = sendto(fd, passage, strlen(passage), 0, (struct sockaddr*)&alice, sizeof(alice))) == -1) {
			perror("bob: sendto");
			exit(1);
		}
		dickens_counter++; // Tweak Dickens data
		if (dickens_counter > 18) dickens_counter = 0;
		memset(message, 0, sizeof(message)); // Clear message for the next one
	} // End comm while
	/* we really never get here */
	close( fd );
	return 0;
}

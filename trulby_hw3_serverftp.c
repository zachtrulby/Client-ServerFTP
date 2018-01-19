/* 
 * Server FTP program
 *
 * Purpose: To emulate the server side of client-server processing with FTP protocol.
 * Recieves a message from dummy client-side which includes the unix command, as well as
 * an appropriate argument (where applicable). Server elicits appropriate response
 * depending on whether valid/invalid entry, including success/error three-digit code in
 * addition to the action carried out to the specified file/directory. 
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*Data Ports for transfer between server and client*/
#define SERVER_FTP_PORT 2236
#define DATA_CONNECTION_PORT SERVER_FTP_PORT +1

/* Definitions/declarations for Error and OK codes to be used by the progam */
#define OK 0
#define ER_INVALID_HOST_NAME -1
#define ER_CREATE_SOCKET_FAILED -2
#define ER_BIND_FAILED -3
#define ER_CONNECT_FAILED -4
#define ER_SEND_FAILED -5
#define ER_RECEIVE_FAILED -6
#define ER_ACCEPT_FAILED -7


/* Function prototypes for initializing the server and sending/receiving messages*/

int getDataSocket(int *s);
int svcInitServer(int *s);
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);
int dataConnect(char *servername, int *s);


/* List of all global variables to be used by the program */

char userCmd[1024];	/* user typed ftp command line received from client */
char *cmd;		/* ftp command (without argument) extracted from userCmd */
char *argument;		/* argument (without ftp command) extracted from userCmd */
char replyMsg[4096];    /* buffer to send reply message to client */
char *delimiter = " ";
char buffer[4096];
FILE *myfile;
char users[1024];  /* list of users */
char user[1024];   /*current user active*/
char pass[1024];   /*password of current*/


/*
 * main
 *
 * Function to listen for connection request from client
 * Receive ftp command one at a time from client
 * Process received command
 * Send a reply message to the client after processing the command with staus of
 * performing (completing) the command
 * On receiving QUIT ftp command, send reply to client and then close all sockets
 *
 * Parameters
 * argc		- Count of number of arguments passed to main (input)
 * argv  	- Array of pointer to input parameters to main (input)
 *		   It is not required to pass any parameter to main
 *		   Can use it if needed.
 *
 * Return status
 *	0			- Successful execution until QUIT command from client 
 *	ER_ACCEPT_FAILED	- Accepting client connection request failed
 *	N			- Failed status, value of N depends on the command processed
 */

int main(	
	int argc,
	char *argv[]
	)
{
	/* List of local varibales */

	int msgSize;        /* Size of message received in octets (bytes) */
	int listenSocket;   /* listening server ftp socket for client connect request */
	int ccSocket;        /* Control connection socket - to be used in all client communication */
	int status;


	/*
	 * NOTE: without \n at the end of format string in printf,
         * UNIX will buffer (not flush)
	 * output to display and you will not see it on monitor.
	*/
	printf("Started execution of server ftp\n");


	 /*initialize server ftp*/
	printf("Initialize ftp server\n");	/* changed text */

	status=svcInitServer(&listenSocket);
	if(status != 0)
	{
		printf("Exiting server ftp due to svcInitServer returned error\n");
		exit(status);
	}


	printf("ftp server is waiting to accept connection\n");

	/* wait until connection request comes from client ftp */
	ccSocket = accept(listenSocket, NULL, NULL);

	printf("Came out of accept() function \n");

	if(ccSocket < 0)
	{
		perror("cannot accept connection:");
		printf("Server ftp is terminating after closing listen socket.\n");
		close(listenSocket);  /* close listen socket */
		return (ER_ACCEPT_FAILED);  // error exist
	}

	printf("Connected to client, calling receiveMsg to get ftp cmd from client \n");


	/* Receive and process ftp commands from client until quit command.
 	 * On receiving quit command, send reply to client and 
         * then close the control connection socket "ccSocket". 
	 */
	do
	{
	    /* Receive client ftp commands until */
 	    status=receiveMessage(ccSocket, userCmd, sizeof(userCmd), &msgSize);
	    if(status < 0)
	    {
		printf("Receive message failed. Closing control connection \n");
		printf("Server ftp is terminating.\n");
		break;
	    }
	    /* Seperate command and argument from userCmd */

/*
 * Homework 2 Assignment Begins Here:
 * 
 * Conditional if/if else statement to compare the userCmd string to cmd strtok. If
 * the userCmd is the same, replyMsg is changed to what current userCmd is entered,
 * verifying it as such. In the case that it is not (see else) replyMsg is returned
 * as invalid.
 */
	if(strchr(userCmd, ' ') == NULL) strcpy(cmd, userCmd);
	else {
	    strcpy(cmd, strtok(userCmd, delimiter));
	    strcpy(argument, strtok(NULL, delimiter));
	            printf("User cmd is: %s\n", cmd);
		printf("Argument is: %s\n", argument);
	}
	/* List of users and passwords delimited by white space */
	strcpy(users, "zach password\n"
		      "catie bubblegum\n"
		      "martin guitar\n"
		      "andrew frosty\n");
    //pwd command
	if(strcmp(cmd, "pwd") == 0) {
	    memset(buffer, '\0', sizeof(buffer)); //flushing buffer
	    system("pwd > /tmp/pwd.txt");
	    myfile = fopen("/tmp/pwd.txt", "r");  /* read the file to buffer sent */
	    status = fread(buffer, sizeof(buffer), sizeof(char), myfile);
	    sprintf(replyMsg, "Cmd 250 okay %s\n", buffer);
	    fclose(myfile);
	    system("rm /tmp/pwd.txt");		  /* delete the txt file created */
	}
    //ls command
	else if(strcmp(cmd, "ls") == 0){
	    memset(buffer, '\0', sizeof(buffer)); /* flushing buffer */
	    system("ls > /tmp/ls.txt");
	    myfile = fopen("/tmp/ls.txt", "r");   /* read the file to buffer sent */
	    status = fread(buffer, sizeof(buffer), sizeof(char), myfile);
	    sprintf(replyMsg, "Cmd 250 okay %s\n", buffer);
	    fclose(myfile);
	    system("rm /tmp/ls.txt");		  /* delete the txt file created */
	}
    //mkdir command
	else if(strcmp(cmd, "mkdir")==0) {
	    memset(buffer, '\0', sizeof(buffer)); /* flushing buffer */
	    if(strlen(argument) == 0) {		  /* checks to see if there is an argument;
						     if not, reports as error */
		printf("Error: No argument found. Please retry with valid argument.");
		memset(cmd, '\0', sizeof(cmd));   /* resets command and argument
		memset(argument, '/0', sizeof(argument));
	}
	char subcmd[1024];
	memset(replyMsg, '\0', sizeof(replyMsg));
	sprintf(subcmd, "mkdir %s", argument);
	status = system(subcmd);		  /* makes system call to new directory
						     created */
	sprintf(replyMsg, "Cmd 212 successfully created directory: %s\n", argument);
	memset(cmd, '\0', sizeof(cmd));   	  /* resets command and argument
	memset(argument, '/0', sizeof(cmd));
    //rmdir command
	else if(strcmp(cmd, "rmdir") == 0) {
	    char subcmd[1024];
	    memset(replyMsg, '\0', sizeof(replyMsg));
	    if(strlen(argument) == 0) {		  /* checks to see if there is an argument;
						     if not, reports as error */
	         sprintf(replyMsg, "Error: No argument found. Please retry with valid argument.\n");
	    }
	    sprintf(subcmd, "rmdir %s", argument);
	    status = system(subcmd);		  /* makes system call to delete
						     specified directory in argument */
	    if(status < 0) {
		sprintf(replyMsg, "Error: Diriectory not found.\n");
	    }
	    sprintf(replyMsg, "Cmd 212 successfully removed directory: %s\n", argument);
	    memset(cmd, '\0', sizeof(cmd));   /* resets command and argument
	    memset(argument, '/0', sizeof(argument));
	}
    //rm function
        else if(strcmp(cmd, "dele") == 0) {
	    char subcmd[1024];
	    memset(replyMsg, '\0', sizeof(replyMsg));
	    if(strlen(argument) == 0) {		  /* checks to see if there is an argument;
						     if not, reports as error */
	        sprintf(replyMsg, "Error: No argument found. Please retry with valid argument.\n");
	    }
	    sprintf(subcmd, "rm %s", argument);
	    status = system(subcmd);		  /* makes system call to delete file
						     specified in argument */
	    if(strlen(argument) == 0) {		  /* checks to see if there is an argument;
						     if not, reports an error */
	        sprintf(replyMsg, "Error: No argument found. Please retry with valid argument.\n");
	    }
	    sprintf(subcmd, "dele %s", argument); /* makes system call to delete specified
						     file */
	    status = system(subcmd);
	    if(status < 0) {			  /* if not OK status return, returns error */
	        sprintf(replyMsg, "Error: Not found.\n");
	    }
	    sprintf(replyMsg, "Cmd 211 okay, deleted file: %s\n", argument);
	    memset(cmd, '\0', sizeof(cmd));
	    memset(argument, '\0', sizeof(argument));
        }
    //cd function
	else if(strcmp(cmd, "cd") == 0) {	  
	    memset(replyMsg, '\0', sizeof(replyMsg));
	    status = chdir(argument);
	    if(status < 0) {			  /* if not OK status return, returns error
		sprintf(replyMsg, "Error: Directory not found.\n");
	    }
	    memset(cmd, '\0', sizeof(cmd));
	    memset(argument, '\0', sizeof(argument));  /* in order to move backwards in 
							  current directory */
	}
    //stat function
        else if(strcmp(cmd, "stat") == 0) {
	    memset(replyMsg, '\0', sizeof(replyMsg));
	    memset(buffer, '\0', sizeof(buffer)); /* flushing buffer */
	    if(strlen(argument) > 0) {            /* checks to see if argument is present;
						     if there is, throws exception
	        printf("Error: Command stat does not require an argument. Please try again.");
	    }
	    system("stat > /tmp/stat.txt");
	    myfile = fopen("/tmp/stat.txt", "r"); /* read the file to buffer sent */
	    fread(buffer, sizeof(buffer), sizeof(char), myfile);
	    strcpy(replyMsg, buffer);
	    fclose(myfile);
	    system("rm /tmp/stat.txt");
	}
    //print users
	else if(strcmp(cmd, "user") == 0) {
	    char line[1024];
	    char *thisln;
	    int found = 0;
	    strcpy(line, users);		  /* make sure users isn't overwritten,
						     makes new variable if so */
	    thisline = strtok(line, "\n");
	    do {
	        printf("line: %s\n", thisln);
		sscanf(thisln, "%s %s", user, pass);
		if(strcmp(argument, user) == 0) { /* if no password, throws no password
						     exception */
		    sprintf(replyMsg, "Cmd 331 username okay, need password.\n");
		    found = 1;
		    break;
		}
		thisln = strtok(NULL, "\n");
		memset(replyMsg, '\0', sizeof(replyMsg));
	    } while(thisln != NULL);
	    if(found == 0) {
		sprintf(replyMsg, "Cmd 332 username not found. Please enter a valid username.\n");
	    }
	}
    //help function: copies FTP commands into the replyMsg
	else if(strcmp(cmd, "help") == 0) {
	    strcpy(replyMsg, "Commands\t Use \t\t\t Syntax\n"
			     "pwd   \t\t prints current directory \t pwd\n"
			     "cd    \t\t change directory         \t cd dir\n"
			     "ls    \t\t print files in directory \t ls\n"
			     "dele  \t\t remove specified file    \t dele filename\n"
			     "stat  \t\t print stats		  \t stat\n"
			     "mkdir \t\t make a directory	  \t mkdir dir\n"
			     "rmdir \t\t remove a directory       \t rmdir dir\n"
			     "user  \t\t login username		  \t user username\n"
			     "pass  \t\t login password		  \t pass password\n"
			     );
	}
	else if(strcmp(cmd, "pass") == 0) {
	    memset(replyMsg, '\0', sizeof(replyMsg));
	    if(pass[0] == '\0') sprintf(replyMsg, "Cmd 332 account required for login\n");
	    if(strcmp(argument, pass) == 0) sprintf(replyMsg, "Cmd 231 password correct... logging in.");
	    else sprintf(replyMsg, "Error: password incorrect. Try again.");
	}
	//passes replyMsg
	else if(strcmp(cmd, "quit") == 0) {
	    memset(replyMsg, '\0', sizeof(replyMsg));
	    strcpy(replyMsg, "Cmd 231 okay, logging out...\n");
	}
	else if(strcmp("recv", cmd) == 0) {
	    FILE *afile;
	    char buff[201];
	    char command[201];
	    int s;
	    dataConnect("134.241.37.12", &s);
	    FILE *newfile = fopen(argument, "w");
	    if(newfile == NULL) {
		while(1) {				   /* while recieving messages,
							      write to a file */
		    if(msgSize == 0) break;
		    status = recieveMessage(s, buff, sizeof(buff), &msgSize);
		    fwrite(buff, sizeof(char), msgSize, newfile);
		    fflush(newfile);
		    memset(buff, '\0', sizeof(buff));
		}
		close(s);
		fclose(newfile);
	    }
	    else {
	        sprintf(replyMsg, "Cmd 202 Error: Not a valid command.\n");

	    /*
 	     * ftp server sends only one reply message to the client for 
	     * each command received in this implementation.
	     */
	    /*strcpy(replyMsg,"200 cmd okay\n");*/  /* Should have appropriate reply msg starting HW2 */
	    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);	/* Added 1 to include NULL character in */
				/* the reply string strlen does not count NULL character */
	    if(status < 0)
	    {
		break;  /* exit while loop */
	    }
	}
	while(strcmp(cmd, "quit") != 0);

	printf("Closing control connection socket.\n");
	close (ccSocket);  /* Close client control connection socket */

	printf("Closing listen socket.\n");
	close(listenSocket);  /*close listen socket */

	printf("Existing from server ftp main \n");

	return (status);
}


/*
 * svcInitServer
 *
 * Function to create a socket and to listen for connection request from client
 *    using the created listen socket.
 *
 * Parameters
 * s		- Socket to listen for connection request (output)
 *
 * Return status
 *	OK			- Successfully created listen socket and listening
 *	ER_CREATE_SOCKET_FAILED	- socket creation failed
 */

int dataConnect (
	char *serverName, /* server IP address in dot notation (input) */
	int *s 		  /* control connection socket number (output) */
	)
{
	int sock;	/* local variable to keep socket number */

	struct sockaddr_in clientAddress;  	/* local client IP address */
	struct sockaddr_in serverAddress;	/* server IP address */
	struct hostent	   *serverIPstructure;	/* host entry having server IP address in binary */


	/* Get IP address os server in binary from server name (IP in dot natation) */
	if((serverIPstructure = gethostbyname(serverName)) == NULL)
	{
		printf("%s is unknown server. \n", serverName);
		return (ER_INVALID_HOST_NAME);  /* error return */
	}

	/* Create control connection socket */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket ");
		return (ER_CREATE_SOCKET_FAILED);	/* error return */
	}

	/* initialize client address structure memory to zero */
	memset((char *) &clientAddress, 0, sizeof(clientAddress));

	/* Set local client IP address, and port in the address structure */
	clientAddress.sin_family = AF_INET;	/* Internet protocol family */
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY is 0, which means */
						 /* let the system fill client IP address */
	clientAddress.sin_port = 0;  /* With port set to 0, system will allocate a free port */
			  /* from 1024 to (64K -1) */

	/* Associate the socket with local client IP address and port */
	if(bind(sock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}


	/* Initialize serverAddress memory to 0 */
	memset((char *) &serverAddress, 0, sizeof(serverAddress));

	/* Set ftp server ftp address in serverAddress */
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr, 
			serverIPstructure->h_length);
	serverAddress.sin_port = htons(DATA_CONNECTION_PORT);

	/* Connect to the server */
	if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("Cannot connect to server ");
		close (sock); 	/* close the control connection socket */
		return(ER_CONNECT_FAILED);  	/* error return */
	}

	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /* successful return */
}
int svcInitServer (
	int *s 		/*Listen socket number returned from this function */
	)
{
	int sock;
	struct sockaddr_in svcAddr;
	int qlen;

	/*create a socket endpoint */
	if( (sock=socket(AF_INET, SOCK_STREAM,0)) <0)
	{
		perror("cannot create socket");
		return(ER_CREATE_SOCKET_FAILED);
	}

	/*initialize memory of svcAddr structure to zero. */
	memset((char *)&svcAddr,0, sizeof(svcAddr));

	/* initialize svcAddr to have server IP address and server listen port#. */
	svcAddr.sin_family = AF_INET;
	svcAddr.sin_addr.s_addr=htonl(INADDR_ANY);  /* server IP address */
	svcAddr.sin_port=htons(SERVER_FTP_PORT);    /* server listen port # */

	/* bind (associate) the listen socket number with server IP and port#.
	 * bind is a socket interface function 
	 */
	if(bind(sock,(struct sockaddr *)&svcAddr,sizeof(svcAddr))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}

	/* 
	 * Set listen queue length to 1 outstanding connection request.
	 * This allows 1 outstanding connect request from client to wait
	 * while processing current connection request, which takes time.
	 * It prevents connection request to fail and client to think server is down
	 * when in fact server is running and busy processing connection request.
	 */
	qlen=1; 


	/* 
	 * Listen for connection request to come from client ftp.
	 * This is a non-blocking socket interface function call, 
	 * meaning, server ftp execution does not block by the 'listen' funcgtion call.
	 * Call returns right away so that server can do whatever it wants.
	 * The TCP transport layer will continuously listen for request and
	 * accept it on behalf of server ftp when the connection requests comes.
	 */

	listen(sock,qlen);  /* socket interface function call */

	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /*successful return */
}


/*
 * sendMessage
 *
 * Function to send specified number of octet (bytes) to client ftp
 *
 * Parameters
 * s		- Socket to be used to send msg to client (input)
 * msg  	- Pointer to character arrary containing msg to be sent (input)
 * msgSize	- Number of bytes, including NULL, in the msg to be sent to client (input)
 *
 * Return status
 *	OK		- Msg successfully sent
 *	ER_SEND_FAILED	- Sending msg failed
 */

int sendMessage(
	int    s,	/* socket to be used to send msg to client */
	char   *msg, 	/* buffer having the message data */
	int    msgSize 	/* size of the message/data in bytes */
	)
{
	int i;
	/* Print the message to be sent byte by byte as character */
	for(i=0; i<msgSize; i++)
	{
		printf("%c",msg[i]);
	}
	printf("\n");

	if((send(s, msg, msgSize, 0)) < 0) /* socket interface call to transmit */
	{
		perror("unable to send ");
		return(ER_SEND_FAILED);
	}

	return(OK); /* successful send */
}


/*
 * receiveMessage
 *
 * Function to receive message from client ftp
 *
 * Parameters
 * s		- Socket to be used to receive msg from client (input)
 * buffer  	- Pointer to character arrary to store received msg (input/output)
 * bufferSize	- Maximum size of the array, "buffer" in octent/byte (input)
 *		    This is the maximum number of bytes that will be stored in buffer
 * msgSize	- Actual # of bytes received and stored in buffer in octet/byes (output)
 *
 * Return status
 *	OK			- Msg successfully received
 *	R_RECEIVE_FAILED	- Receiving msg failed
 */


int receiveMessage (
	int s, 		/* socket */
	char *buffer, 	/* buffer to store received msg */
	int bufferSize, /* how large the buffer is in octet */
	int *msgSize 	/* size of the received msg in octet */
	)
{
	int i;

	*msgSize=recv(s,buffer,bufferSize,0); /* socket interface call to receive msg */

	if(*msgSize<0)
	{
		perror("unable to receive");
		return(ER_RECEIVE_FAILED);
	}

	/* Print the received msg byte by byte */
	for(i=0;i<*msgSize;i++)
	{
		printf("%c", buffer[i]);
	}
	printf("\n");

	return (OK);
}
int getDataSocket (int *s)
{
  int sock;
	struct sockaddr_in svcAddr;
	int qlen;

	if( (sock=socket(AF_INET, SOCK_STREAM,0)) <0)
	{
		perror("cannot create socket");
		return(ER_CREATE_SOCKET_FAILED);
	}

	memset((char *)&svcAddr,0, sizeof(svcAddr));

	/* initialize svcAddr to have server IP address and server listen port#. */
	svcAddr.sin_family = AF_INET;
	svcAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	svcAddr.sin_port=htons(DATA_CONNECTION_PORT+1);

	/* bind (associate) the listen socket number with server IP and port#.
	 * bind is a socket interface function 
	 */
	if(bind(sock,(struct sockaddr *)&svcAddr,sizeof(svcAddr))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}

	/* 
	 * Set listen queue length to 1 outstanding connection request.
	 * This allows 1 outstanding connect request from client to wait
	 * while processing current connection request, which takes time.
	 * It prevents connection request to fail and client to think server is down
	 * when in fact server is running and busy processing connection request.
	 */
	qlen=1; 


	/* 
	 * Listen for connection request to come from client ftp.
	 * This is a non-blocking socket interface function call, 
	 * meaning, server ftp execution does not block by the 'listen' funcgtion call.
	 * Call returns right away so that server can do whatever it wants.
	 * The TCP transport layer will continuously listen for request and
	 * accept it on behalf of server ftp when the connection requests comes.
	 */

	listen(sock,qlen);  /* socket interface function call */

	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /*successful return */
}
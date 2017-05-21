////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////        INCLUDES          ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <mraa/aio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////        DEFINES          /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE 4
#define THRESHOLD 100

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////        GLOBALS          /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

char buf[BUFFER_SIZE];
int client_socket_fd, portno;


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////        PROTOTYPES          //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void error(const char *msg);
void initClient(int argc, char *argv[]);
void getPassword(char *buf);
void sendToServer(char *password);
void recvAuthentication(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////       FUNCTION DEFINITIONS          ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////


void handler(int sig)
{
	if(sig == SIGINT)
		exit(1);
}

void
error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void
initClient(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    // Read command line arguments, need to get the host IP address and port
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    
    // Convert the arguments to the appropriate data types
    portno = atoi(argv[2]);
    
    // setup the socket
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // check if the socket was created successfully. If it wasnt, display an error and exit
    if(client_socket_fd < 0) {
        error("ERROR opening socket");
    }
    
    // check if the IP entered by the user is valid
    server = gethostbyname(argv[1]);
    //server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // clear our the serv_addr buffer
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    // set up the socket
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    // try to connect to the server
    if (connect(client_socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
    }
}

void getPassword(char *buf) {
    //int x = 1;
    int i;
  	uint16_t value;
  	mraa_aio_context light;
  	light = mraa_aio_init(3);
  
    double val = mraa_aio_read(light);
    if(val > THRESHOLD){
      strcat(buf, "1");
    }
    else{
      strcat(buf, "0");
    }
    //printf("%.2f\n", val);
      
	  mraa_aio_close(light);
}

void sendToServer(char *password) {
    ssize_t nwrite, nread;
    
    // send user input to the server
    nwrite = write(client_socket_fd, password,strlen(password));
    // n contains how many bytes were received by the server
    // if n is less than 0, then there was an error
    if (nwrite < 0) {
        error("ERROR writing to socket");
    }
    
    // clear out the buffer
    memset(buf, 0, BUFFER_SIZE);
    
    // get the response from the server and print it to the user
    nread = read(client_socket_fd, buf, 255);
    if (nread < 0) {
        error("ERROR reading from socket");
    }
    printf("%s\n",buf);
    
    // clean up the file descriptors
    close(client_socket_fd);
}

int
main (int argc, char *argv[]) {

    signal(SIGINT, handler);
    
    // Set up client connection with server
    initClient(argc, argv);
    
    int i;
    memset(buf, 0, BUFFER_SIZE);
    
    // Prompt user for password
    printf("\nPlease enter your 4-bit password in 5 seconds.\n");
    sleep(2);
    printf("Cover the light for a 0, otherwise it's a 1.\n");
    for(i = 4; i > 0; i--){
      printf("In %d seconds...\n", i);
      sleep(3);
      // Get password from user
      getPassword(buf);
      printf("You entered %s so far.\n",buf);
    }
    
    char send_buf[28];
    memset(send_buf, 0, 28);
    strcat(send_buf, "ID = Group3 Password = ");
    strcat(send_buf, buf);
    strcat(send_buf, "\n");
    
    sendToServer(send_buf);
    
    //printf("final buffer: %s\n",buf);
    //printf("final buffer: %s\n",send_buf);
    return EXIT_SUCCESS;
}




















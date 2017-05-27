////////////////////////////////////////////////////////////////////////////
/////////////////////////// Identification /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Group Members:
// Katrina Duong    (ID: 004-598-670)
// Jason Less       (ID: 404-640-158)
// Erynn-Marie Phan (ID: 504-757-459)
// Yun Xu           (ID: 304-635-157)
// Title: Secure Implementation

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#i#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h> /* time_t, struct tm, time, localtime, current_time, previous_time */
#include <mraa/aio.h>
#include <mraa/gpio.h>

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Buffer size for username/password
#define SEND_BUFFER_SIZE 32
// Buffer size for password
#define PWD_BUFFER_SIZE 9
// Buffer size for Group ID
#define ID_BUFFER_SIZE 1
// Buffer size to hold message from server
#define RECV_BUFFER_SIZE 4
// Light threshold distinguising between high or low light intensity
#define THRESHOLD 100

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Globals ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/* Programmable Variables */
// Light sensors used to create password
mraa_aio_context light0;
mraa_aio_context light1;
mraa_aio_context light2;
mraa_aio_context light3;
// Button used as a MUX to choose which light sensor to use
mraa_gpio_context button;
// Variables to connect to server as client
int client_socket_fd, portno;
// Buffer to store the Group ID
char id_buf[ID_BUFFER_SIZE];
// Buffer to store the password
char pwd_buf[PWD_BUFFER_SIZE];
// Buffer containing username/password to send to server
char send_buf[SEND_BUFFER_SIZE];
// Buffer containing message from server
char receive_buf[RECV_BUFFER_SIZE];
// Variable storing the state of the button
int button_state = 0;
// Number of retries remaining before lockout
int retry_count = 3;

/* Flags */
// Flag to indicate whether the password can be retried before being locked out
int retry_password_flag = 1;
// Flag to indicate whether the button state can be altered
// Serves basic function as a lock
int button_flag = 0;

/* Debug */
int run_flag = 1;
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Prototypes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Print usage upon unrecognized arguments
void print_usage(void);
// Signal handler (SIGINT ^C)
void handler(int signum);
// Initialize the client to the server
void initClient(int argc, char *argv[]);
// Initialize sensors
void initSensors(void);
// Shutdown sensors
void shutdownSensors(void);
// Process pressing of the button sensor
void button_handler(void);
// Prompt user for the password (giving prompt of how to generate password)
void userPrompt(void);
// Acquire password from the user (via light sensors and button MUX)
void getPassword(char *buf);
// Format the username/password buffer to send to server
void createUsernamePassword(void);
// Send username/password over to the server
void sendToServer(char *password);

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Function Definitions ///////////////////////
////////////////////////////////////////////////////////////////////////////
void
print_usage(void) {
    fprintf(stderr, "Usage: main [hostname] [--port=portnum]\n");
    exit(EXIT_FAILURE);
}

void
handler(int signum) {
    if(signum == SIGINT) {
        //debugging
        run_flag = 0;
    }
    exit(EXIT_SUCCESS);
}

void
initClient(int argc, char *argv[]) {
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    // Read command line arguments, need to get the host IP address and port
    if (argc < 3) {
        print_usage();
    }
    
    // Convert the arguments to the appropriate data types
    portno = atoi(argv[2]);
    
    // setup the socket
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // check if the socket was created successfully. If it wasnt, display an error and exit
    if(client_socket_fd < 0) {
        fprintf(stderr, "Error opening socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // check if the IP entered by the user is valid
    server = gethostbyname(argv[1]);
    //server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }
    
    // clear our the serv_addr buffer
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    // set up the socket
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    // try to connect to the server
    if (connect(client_socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        fprintf(stderr, "Error connecting to server.\n");
        exit(EXIT_FAILURE);
    }
}

void
initSensors(void) {
    // Initialize mraa
    mraa_init();
    
    // Set button sensor to digital input 3
    button = mraa_gpio_init(3);
    // Set light sensor 0 to analog input 0
    light0 = mraa_aio_init(0);
    // Set light sensor 1 to analog input 1
    light1 = mraa_aio_init(1);
    // Set light sensor 2 to analog input 2
    light2 = mraa_aio_init(2);
    // Set light sensor 3 to analog input 3
    light3 = mraa_aio_init(3);
    
    // Indicating the button is a general-purpose input
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    // Register the button, so that when pressed, it will be properly serviced
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, (void *)&button_handler, NULL);

}

void
shutdownSensors(void) {
    mraa_gpio_close(button);
    mraa_aio_close(light0);
    mraa_aio_close(light1);
    mraa_aio_close(light2);
    mraa_aio_close(light3);

}

void
button_handler(void) {
    // State of the button to indicate which light sensor is high (and others are low)
    // Four states corresponding to the four light sensors
    
    // Print out the button state so that the user knows which light sensor is active
    // Button should only be allowed to change state when prompted
    // Lock button state critical section when not active
    if (button_flag == 1) {
        if (button_state == 3) {
            button_state = 0;
        }
        else {
            button_state++;
        }
        fprintf(stdout, "\tLight sensor currently active: %d\n", button_state);
    }
}

void
userPrompt(void) {
    // Explain how to the user how to generate the ID and password using the sensors
    
    // Explain the values that can be produced from each of the four light sensors
    fprintf(stdout, "===================================================================\n");
    fprintf(stdout, "There are four light sensors each having two different values:\n");
    fprintf(stdout, "\tLight sensor 0 is either 0 or 1\n");
    fprintf(stdout, "\tLight sensor 1 is either 2 or 3\n");
    fprintf(stdout, "\tLight sensor 2 is either 4 or 5\n");
    fprintf(stdout, "\tLight sensor 3 is either 6 or 7\n\n");
    fprintf(stdout, "===================================================================\n");
    sleep(8);
    
    // Explain how to produce the given value for the light sensor
    fprintf(stdout, "How to produce values for light sensors:\n\n");
    fprintf(stdout, "1. Cover light sensor to produce the lower number for that sensor \n\t(i.e. Cover light sensor 0 to produce a 0).\n\n");
    fprintf(stdout, "2. Otherwise light sensor produces the higher number for that sensor \n\t(i.e. Don't cover light sensor 2 to produce a 5).\n\n");
    fprintf(stdout, "===================================================================\n");
    sleep(8);
    
    // Explain how to use the button to choose which light sensor to use
    fprintf(stdout, "To select which light sensor is active, press the button to change\n between light sensors: \n\n");
    fprintf(stdout, "- By default, light sensor 0 starts active.\n");
    fprintf(stdout, "- To generate a 2 or a 3, press the button once to switch to light sensor 1.\n");
    fprintf(stdout, "- If the button is pressed again, then light sensor 2 becomes active\n (having values 4 or 5).\n\n");
    fprintf(stdout, "===================================================================\n");
    sleep(8);
    
    // Prompt user to enter the ID
    fprintf(stdout, "Light sensor currently active: %d\n", button_state);
    fprintf(stdout, "\nPress the button to select which light sensor is active.\n");

    /* Critical section */
    // Allow for user to change the state of the button for 5 seconds
    time_t endwait;
    int seconds = 5; // end loop after this time has elapsed
    endwait = time(NULL) + seconds;
    while (time(NULL) < endwait) {
        // Activate button
        button_flag = 1;
    }
    // Deactivate button
    button_flag = 0;

    fprintf(stdout, "\n===================================================================\n");
    
    int i;
    for (i = 3; i > 0; i--) {
        fprintf(stdout, "In %d seconds, please enter your 1-digit group ID.\n", i);
        sleep(1);
        if (i == 1) {
            getPassword(id_buf);
            fprintf(stdout, "The Group ID entered: %s.\n", id_buf);
        }
    }
    
    sleep(8);
    
    // Prompt user to enter the password
    fprintf(stdout, "===================================================================\n");
    fprintf(stdout, "Light sensor currently active: %d\n", button_state);
    fprintf(stdout, "\nIn 5 seconds, please enter your 8-digit password.\n");
    sleep(5);
    
    for (i = 8; i > 0; i--) {
        fprintf(stdout, "\n===================================================================\n");
        fprintf(stdout, "\nPress the button to select which light sensor is active.\n");
        
        /* Critical section */
        // Allow for user to change the state of the button for 5 seconds
        time_t endwait;
        int seconds = 5; // end loop after this time has elapsed
        endwait = time(NULL) + seconds;
        while (time(NULL) < endwait) {
            // Activate button
            button_flag = 1;
        }
        // Deactivate button
        button_flag = 0;
        
        fprintf(stdout, "In %d seconds...\n", i);
        sleep(5);
        getPassword(pwd_buf);
        fprintf(stdout, "You entered %s so far.\n", pwd_buf);
    }
}

void
getPassword(char *buf) {
    double light;
    switch (button_state) {
        // Light sensor 0 is active (values 0 or 1)
        case 0:
            light = mraa_aio_read(light0);
            if (light > THRESHOLD) {
                strcat(buf, "1");
            }
            else {
                strcat(buf, "0");
            }
            break;
        // Light sensor 1 is active (values 2 or 3)
        case 1:
            light = mraa_aio_read(light1);
            if (light > THRESHOLD) {
                strcat(buf, "3");
            }
            else {
                strcat(buf, "2");
            }
            break;
        // Light sensor 2 is active (values 4 or 5)
        case 2:
            light = mraa_aio_read(light2);
            if (light > THRESHOLD) {
                strcat(buf, "5");
            }
            else {
                strcat(buf, "4");
            }
            break;
        // Light sensor 3 is active (values 6 or 7)
        case 3:
            light = mraa_aio_read(light3);
            if (light > THRESHOLD) {
                strcat(buf, "7");
            }
            else {
                strcat(buf, "6");
            }
            break;
        default:
            fprintf(stderr, "Error processing buttons.\n");
            exit(EXIT_FAILURE);
    }
}

void
createUsernamePassword(void) {
    memset(send_buf, 0, SEND_BUFFER_SIZE);
    
    // Format username
    strcat(send_buf, "ID = ");
    strcat(send_buf, id_buf);
    // Format password
    strcat(send_buf, " Password = ");
    strcat(send_buf, pwd_buf);
    strcat(send_buf, "\n");
}

void
sendToServer(char *password) {
    ssize_t nwrite, nread;
    
    // send user input to the server
    nwrite = write(client_socket_fd, password, strlen(password));
    // n contains how many bytes were received by the server
    // if n is less than 0, then there was an error
    if (nwrite < 0) {
        fprintf(stderr, "Error writing to socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // clear out the buffer
    memset(receive_buf, 0, RECV_BUFFER_SIZE);
    
    // get the response from the server and print it to the user
    nread = read(client_socket_fd, receive_buf, RECV_BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading from socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // Print the response (YES or NO) to STDOUT
    printf("%s\n", receive_buf);
    
    // If correct password, then door is opened, exit program
    if (strcmp(receive_buf, "YES") == 0) {
        retry_password_flag = 0;
    }
    // If incorrect password, door still locked, retry if attempts remain
    else if (strcmp(receive_buf, "NO") == 0) {
        retry_count--;
        fprintf(stdout, "Incorrect username/password combination given.\n");
        fprintf(stdout, "Number of retries remaining before lockout: %d\n", retry_count);
    }
    
    // clean up the file descriptors
    close(client_socket_fd);
}


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[]) {
    // Register SIGINT to the signal handler
    signal(SIGINT, handler);
    
    // Set up client connection with server
    initClient(argc, argv);
    
    // Initialize the button and light sensors
    initSensors();
    
    // Initialize buffers that will store the Group ID and password
    memset(id_buf, 0, ID_BUFFER_SIZE);
    memset(pwd_buf, 0, PWD_BUFFER_SIZE);
    
    
    // Enter password: if success, then door opened, exit success
    // If failure, can retry a total of three times before being locked out
    // for 5 minutes
    while (retry_password_flag && (retry_count != 0)) {
        // Initiate prompt to user and prompt user for password
        userPrompt();
    
        // Format username/password buffer
        createUsernamePassword();
    
        // Send username/password combination to server
        sendToServer(send_buf);
    
        // Shutdown the button and light sensors
        shutdownSensors();
    }
    
    /* Debugging */
    //printf("final buffer: %s\n",buf);
    //printf("final buffer: %s\n",send_buf);
    
    // If no errors encountered, success
    return EXIT_SUCCESS;
}


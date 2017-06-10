#include "iot_secure_door_unlock.h"

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

SSL *ssl_init(void){
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    const SSL_METHOD *sslMethod = TLSv1_client_method();
    SSL_CTX *sslContext = SSL_CTX_new(sslMethod);
    
    if (sslContext == NULL)
    perror("ERROR can't get SSL context\n");
    
    
    SSL * sslClient = SSL_new(sslContext);
    if (sslClient == NULL)
    perror("ERROR can't get SSL structure\n");
    
    return sslClient;
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
    
    // SSL setup
    SSL_library_init();
    sslClient = ssl_init();
    
    // setup the socket
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // check if the socket was created successfully. If it wasnt, display an error and exit
    if(client_socket_fd < 0) {
        fprintf(stderr, "Error opening socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // check if the IP entered by the user is valid
    server = gethostbyname(argv[1]);
    
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
    if (!SSL_set_fd(sslClient, client_socket_fd))
    perror("ERROR can't connect SSL to socket\n");
    if (SSL_connect(sslClient) != 1)
    perror("ERROR server rejected connection\n");
    
    
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
    
    // clean up the file descriptors
    SSL_free(sslClient);
    close(client_socket_fd);
    
    // Exit success
    exit(EXIT_SUCCESS);
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
    strcat(send_buf, "ID = Group");
    strcat(send_buf, id_buf);
    // Format password
    strcat(send_buf, " Password = ");
    strcat(send_buf, pwd_buf);
    strcat(send_buf, "\n");
}

void
sendToServer(char *password) {
    // clear out the buffer
    memset(receive_buf, 0, RECV_BUFFER_SIZE);
    
    ssize_t nwrite, nread;
    
    // send user input to the server
    nwrite = SSL_write(sslClient, password, strlen(password));
    // n contains how many bytes were received by the server
    // if n is less than 0, then there was an error
    if (nwrite < 0) {
        fprintf(stderr, "Error writing to socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // get the response from the server and print it to the user
    nread = SSL_read(sslClient, receive_buf, RECV_BUFFER_SIZE);
    if (nread < 0) {
        fprintf(stderr, "Error reading from socket.\n");
        exit(EXIT_FAILURE);
    }
    
    // Print the response (YES or NO) to STDOUT
    printf("%s\n", receive_buf);
    sleep(3);
    
    // If correct password, then door is opened, exit program
    if (strcmp(receive_buf, "Group3 YES") == 0) {
        sleep(4);
        shutdownSensors();
    }
    // If incorrect password, door still locked, retry if attempts remain
    else if (strcmp(receive_buf, " NO\n") == 0) {
    //else {
        retry_count--;
        fprintf(stdout, "Incorrect username/password combination given.\n");
        fprintf(stdout, "Number of retries remaining before lockout: %d\n", retry_count);
        sleep(3);
    }
    
    // clear out the buffer
    memset(receive_buf, 0, RECV_BUFFER_SIZE);
}


////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Main Function //////////////////////////////
////////////////////////////////////////////////////////////////////////////
int
main (int argc, char *argv[]) {
    // Note: This was just used for debugging purposes
    // Register SIGINT to the signal handler
    //signal(SIGINT, handler);
    
    // Set up client connection with server
    initClient(argc, argv);
    
    // Initialize the button and light sensors
    initSensors();
    
    /* Enter password: if success, then door opened, exit success.
       If failure, can retry a total of three times before being locked out
       for 5 minutes.
     */
    while (retry_count > 0) {
        // Initialize buffers that will store the Group ID and password
        memset(id_buf, 0, ID_BUFFER_SIZE);
        memset(pwd_buf, 0, PWD_BUFFER_SIZE);
        
        // Initiate prompt to user and prompt user for password
        userPrompt();
        
        // Format username/password buffer
        createUsernamePassword();
        
        // Send username/password combination to server
        sendToServer(send_buf);
    }
    
    // Shutdown the button and light sensors
    shutdownSensors();
    
    // If no errors encountered, success
    return EXIT_SUCCESS;
}



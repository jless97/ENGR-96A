// E96A: IoT Security System

#ifndef MAIN_H
#define MAIN_H

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Includes ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <mraa/aio.h>
#include <mraa/gpio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Defines ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Buffer size for username/password
#define SEND_BUFFER_SIZE 64
// Buffer size for password
#define PWD_BUFFER_SIZE 9
// Buffer size for Group ID
#define ID_BUFFER_SIZE 1
// Buffer size to hold message from server
#define RECV_BUFFER_SIZE 16
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
SSL *sslClient;
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
// for debugging (SSL connection)
void error(const char *msg);
// Signal handler (SIGINT ^C)
void handler(int signum);
// Initialize SSL connection
SSL * ssl_init(void);
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


#endif //MAIN_H

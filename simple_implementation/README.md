# e96_iot_security_system

The goal of this project was to implement a simple IoT system using the Intel Edison (and a light sensor) to unlock a door by using
a server and a simple four-digit password. This project served as the barebones necessary for a password-authentication system. In
the upcoming weeks, our group will add extra features to the implementation to make it more secure and slightly more complex.

### Description of included files

#### main.c
Our project consists of a single source file that consists of several functions that divides up how our program works. First of all,
we registered the interrupt signal (SIGINT) at the beginning of the program, so that the user can quit the program (via ^C) due to
any errors that might have arisen when inputting the password. Next, we set up the client portion of the server-client relationship
using the standard socket inititialization functions. 

After setting the system up properly, the program merely prompts the user for their four-digit password, which is to be inputted 
using the light sensor. This is accomplished by utilizing different light intensities to act as a binary input. The password was
stored in a buffer of length 4, and then the proper Username/Password string was concatenated together into a buffer to be sent to
the server for authentication.

Lastly, the correctly formatted string was sent over to the server to validate the correctness of the password. The server simply
replies with a yes or no answer depending on if the correct group-specific username and password were sent over in the desired 
format.

#### main
The compiled executable

#### success.jpg
Screenshot showing a successful password validation

#### fail.jpg
Screenshot showing an unsuccessful password validation

## Group Information

* Group 3
* Katrina Duong       (ID: 004-598-670) 
* Jason Less          (ID: 404-640-158)
* Erynn-Marie Phan    (ID: 504-757-459) 
* Yun Xu              (ID: 304-635-157)

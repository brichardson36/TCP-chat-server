Brayden Richardson
brichardson36@gatech.edu

Files:
chatserver.c - Main file for running the chat server
chatclient.c - Code for connecting to the server as a client simulating a user
Makefile - Make file for compiling the code and creating executables

Instructions:
Run "make" to compile and create the executables. Then use the command "./chatserver -start -port 5001" to start the server on port 5001.
Run "./chatclient -join -host <hostname> -user <username> -passcode cs3251secret" to join the chatroom with the passed in username. 
If another value for passcode is passed in, the server will refused connection.
Run "make clean" to remove all executables.

Details:
This was built on Ubuntu 20.04, but it should run properly on 18.04.
The makefile contains information about the compiler flags and c standard used.
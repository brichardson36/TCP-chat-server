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

ASSIGNMENT SPEC:
Objective:
  Understand creation of sockets
  Understand that application protocols often are simple plain text messages with special meanings 
  Understand how to parse simple text commands
Introduction:
  In this assignment, you will create a chat room on a single computer where you and your (imaginary) friends can chat with each other. The following steps will be required to     achieve this:

  Create a server program that is always running on a specific port (5001).
  Create a client program that can join this server.
  The client needs a display name and a passcode to enter the chat room. (Assume all clients use the same passcode but different display name).
  The job of the server is to accept connections from clients, get their display name and passcode (in plaintext), verify that the passcode is correct and then allow clients       into the chat room.
  When any client sends a message, the display name is shown before the message, and the message is delivered to all other current clients. 
  Clients can type any text message, or can type one of the following shortcut codes to display specific text to everyone:
  Type :) to display [feeling happy]
  Type :( to display [feeling sad]
  Type :mytime to display the current time
  Type :+1hr to display the current time + 1 hour
  Type :Exit to close your connection and terminate the client
What will you learn?
  Basic socket programming to create a client-server application
  How do multiple clients connect to a single server?
  How to keep multiple persistent TCP connections alive?
  Text parsing to develop a custom application layer protocol. (This is, in spirit, very similar to how HTTP works, for example)

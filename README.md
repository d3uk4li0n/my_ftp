# my_ftp
ftp client and server with multithreading

# Description
A thread pool has been used to handle multiple clients  
Code for the client has been implemented too

# Installation
make  
to clean up after usage: make clean 

# Usage
You’re going to need two terminals, and you’re going to run on localhost  
In the first terminal run: ./server 3600    
In the second terminal run: ./client 127.0.0.1 3600

You will be asked to login, and then prompted to choose an action:  
1. list  
2. files  
2. get file  
3. put file  
4. exit

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

class FTPClient{
    public:
  FTPClient(const char *host, int port);
  ~FTPClient();
  bool login(const char *username, const char *password);
  string readLine(int sockfd);
  void list();
  void get(const char *filename);
  void put(const char *filename);
  void quit();
  void mode();

  int openSocket(const char *host, int port);
private:
  int sockfd;
  
  int datafd;
};

#endif
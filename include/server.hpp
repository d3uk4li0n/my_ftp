#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "threadPool.hpp"
#include <sstream>
#include <fcntl.h>

using namespace std;

class FTPServer {
public:
  FTPServer(int port, const string& path);
  void Start();
  void HandleClientConnection(int clientSocket);
  bool AuthenticateUser(const string& username);
  bool AuthenticatePass(const string& password);
  void TransferData(int cmdSocket, int clientSocket, const string& filename);
  void ReceiveData(int cmdSocket, int clientSocket, const string& filename);
private:
  int m_port;
  string m_path;
  ThreadPool m_threadPool;

  void sendString(int clientSocket, const string& string);
  string receiveString(int clientSocket);
  string list();
  vector<int> openDataChannel();
};

#endif
#include "../include/client.hpp"

#include <sstream>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

using namespace std;

FTPClient::FTPClient(const char *host, int port) {
  sockfd = openSocket(host, port);
}

FTPClient::~FTPClient() {
  close(sockfd);
}

bool FTPClient::login(const char *username, const char *password) {
    stringstream ss;
    ss << "USER " << username << "\n";
    string str = ss.str();
    write(sockfd, str.c_str(), str.length());

    string response = readLine(sockfd);
    if(response != "230"){
        //cout << "Info: " << response << endl;
        cout << "Login failed" << endl;
        return false;
    }

    stringstream ss2;
    ss2 << "PASS " << password << "\n";
    string str2 = ss2.str();

    write(sockfd, str2.c_str(), str2.length());
    response = readLine(sockfd);
    if(response != "230"){
        cout << "Login failed" << endl;
        return false;
    }

    return true;
}

string FTPClient::readLine(int fd) {
  char buffer[1024];
  int n = read(fd, buffer, sizeof(buffer));
  if (n < 0) {
    cout << "Error reading from server" << endl;
    return "";
  }

  return string(buffer, n);
}

void FTPClient::list() {
  write(sockfd, "LIST\n", strlen("LIST\n"));
  cout << readLine(datafd) << endl;
}

void FTPClient::get(const char *filename) {
    stringstream ss;
    ss << "RETR " << filename << "\n";
    string str = ss.str();

    write(sockfd, str.c_str(), str.length());  

    string response = readLine(sockfd);
    if (response != "150") {
        cout << "File unavailable" << endl;
        return;
    }

    char buffer[1024];
    memset(buffer, '\0', 1024);

    long file_size;
    read(datafd, buffer, 20);
    sscanf(buffer, "%ld", &file_size);
    

    int fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        cout << "Error opening file" << endl;
        return;
    }

    int n;
    long total_bytes = 0; 
    memset(buffer, '\0', 1024);

    while (total_bytes < file_size && (n = read(datafd, buffer, sizeof(buffer))) > 0) {
        total_bytes += n;

        write(fd, buffer, n);
        memset(buffer, '\0', 1024);    
    }

    close(fd);
}


void FTPClient::put(const char *filename) {
    stringstream ss;
    ss << "STOR " << filename << "\n";
    string str = ss.str();
    write(sockfd, str.c_str(), str.length());

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        cout << "Error opening file" << endl;
        return;
    }

    struct stat st;
    stat(filename, &st);
    long file_size = st.st_size;

    char buffer[1024];
    memset(buffer, '\0', 1024);

    sprintf(buffer, "%ld", file_size);  
    write(datafd, buffer, 20);

    int n;
    memset(buffer, '\0', 1024);
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        write(datafd, buffer, n);
        memset(buffer, '\0', 1024);
    }

    close(fd);
}

void FTPClient::quit(){
  write(sockfd, "QUIT\n", strlen("quit\n"));

  string response = readLine(sockfd);
  if (response != "221") {
    cout << "Error quit. Code = " << response << endl;
  }
}

void FTPClient::mode(){
  write(sockfd, "PASV\n", strlen("PASV\n"));

  string response = readLine(sockfd);
  if (response != "227") {
    cout << "Error quit. Code = " << response << endl;
  }

  string ip_port = readLine(sockfd);
  stringstream ss(ip_port);

  string ip;
  int port;
  ss >> ip >> port;

  datafd = openSocket(ip.c_str(), port);
}

int FTPClient::openSocket(const char *host, int port) {
  int new_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (new_socket_fd < 0) {
    cout << "Error creating socket" << endl;
    return -1;
  }

  struct sockaddr_in addr;  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(host);
  if (connect(new_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    cout << "Error connecting to server" << endl;
    return -1;
  }

  return new_socket_fd;
}
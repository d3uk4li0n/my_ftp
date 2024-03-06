#include "../include/server.hpp"

#include <asm-generic/socket.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

FTPServer::FTPServer(int port, const string& path):
 m_port(port), m_path(path), m_threadPool(5){
}

void FTPServer::Start(){
  srand(time(0));
  int opt = 1;

  int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket == -1) {
    cout << "Error creating socket." << endl;
    exit(1);
  }

    /*if(setsockopt(listen_socket, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)) < 0 ){
                cout << "Error setsockopt." << endl;
                exit(EXIT_FAILURE);
        }*/      

    if(setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        cout << "Error setsockopt." << endl;
        exit(EXIT_FAILURE);
    }
    
    if(setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        cout << "Error setsockopt." << endl;
        exit(EXIT_FAILURE);
    }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    cout << "Error binding socket." << endl;
    exit(1);
  }

  listen(listen_socket, 10);

  mutex m;

  while (true) {
    int client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == -1) {
      cout << "Error accepting connection." << endl;
      continue;
    }

    m_threadPool.Enqueue([&, client_socket]() {
      lock_guard<mutex> guard(m);

      HandleClientConnection(client_socket);
    });
  }
}

bool FTPServer::AuthenticateUser(const string& username){
  if (username == "USER Anonymous\n") {
    return true;
  }

  return false;
}

bool FTPServer::AuthenticatePass(const string& password){
  if (password == "PASS \n") {
    return true;
  }

  return false;
}

void FTPServer::sendString(int clientSocket, const string& str) {
  const char* bytes = str.c_str();
  int length = strlen(bytes);

  send(clientSocket, bytes, length, 0);
}

string FTPServer::receiveString(int clientSocket) {
  char bytes[1024];
  int length = recv(clientSocket, bytes, sizeof(bytes), 0);

  string str(bytes, length);

  return str;
}

void FTPServer::HandleClientConnection(int clientSocket) {
  string username = receiveString(clientSocket);

  bool authenticated = AuthenticateUser(username);

  if (authenticated) {
    sendString(clientSocket, "230");
  } else {
    sendString(clientSocket, "530");
    return;
  }

  string password = receiveString(clientSocket);

  authenticated = AuthenticatePass(password);

  if (authenticated) {
    sendString(clientSocket, "230");
  } else {
    sendString(clientSocket, "202");
    return;
  }  

  string mode = receiveString(clientSocket);

  if (mode == "PASV\n")
  {
    sendString(clientSocket, "227");
  }else{
    sendString(clientSocket, "500");
    return;
  }

  vector<int> channel = openDataChannel();
  string ipAddr = "127.0.0.1";
  int port = channel[0];
  int data_socket = channel[1];

  stringstream ss;
  ss << ipAddr << " " << port << endl;
  sendString(clientSocket, ss.str());

  int client_data_socket = accept(data_socket, NULL, NULL);
  if (client_data_socket == -1) {
    cout << "Error accepting connection." << endl;
    return;
  }

  string command = receiveString(clientSocket);
  while (command != "QUIT\n"){
    if (command == "LIST\n"){
      string filenames = list();
      sendString(client_data_socket, filenames);

    }else if (command.length() > 4 && command.substr(0, 4) == "RETR"){
      string filename = command.substr(5, command.length() - 6);
      TransferData(clientSocket, client_data_socket, filename);

    }else if (command.length() > 4 && command.substr(0, 4) == "STOR")
    {
      string filename = command.substr(5, command.length() - 6);
      ReceiveData(clientSocket, client_data_socket, filename);
    }
    command = receiveString(clientSocket);
  }

  sendString(clientSocket, "221"); 
  close(clientSocket);

  close(data_socket);
  close(client_data_socket);
}

void FTPServer::TransferData(int cmdSocket, int clientSocket, const string& filename){
  stringstream ss;
  ss << m_path << "/" << filename;
  string full_path = ss.str();

  int fd = open(full_path.c_str(), O_RDONLY);
  if (fd < 0) {
    cout << "Error opening file" << endl;
    sendString(cmdSocket, "550"); 
    return;
  }

  sendString(cmdSocket, "150"); 

  struct stat st;
  stat(full_path.c_str(), &st);
  long file_size = st.st_size;

  char buffer[1024];
  memset(buffer, '\0', 1024);

  sprintf(buffer, "%ld", file_size);  
  write(clientSocket, buffer, 20);
  
  int n;
  memset(buffer, '\0', 1024);
  while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
    write(clientSocket, buffer, n);
    memset(buffer, '\0', 1024);
  }

  close(fd);
}

void FTPServer::ReceiveData(int cmdSocket, int clientSocket, const string& filename){
  stringstream ss;
  ss << m_path << "/" << filename;
  string full_path = ss.str();

  (void)cmdSocket;

  int fd = open(full_path.c_str(), O_WRONLY | O_CREAT, 0644);
  if (fd < 0) {
    cout << "Error opening file" << endl;
    return;
  }

  char buffer[1024];
  memset(buffer, '\0', 1024);

  long file_size;
  read(clientSocket, buffer, 20);
  sscanf(buffer, "%ld", &file_size);

  long total_bytes = 0; 

  int n;
  memset(buffer, '\0', 1024);
  while (total_bytes < file_size && (n = read(clientSocket, buffer, sizeof(buffer))) > 0) {
    write(fd, buffer, n);

    total_bytes += n;
    memset(buffer, '\0', 1024);
  }

  close(fd);
}

string FTPServer::list(){
  DIR *dir_ptr;
  struct dirent *diread;
  stringstream ss;

  if ((dir_ptr = opendir(m_path.c_str())) != nullptr) {
      while ((diread = readdir(dir_ptr)) != nullptr) {
          ss << diread->d_name << endl;
      }
      closedir(dir_ptr);
  } 

  ss << endl;
  
  return ss.str();
}

vector<int> FTPServer::openDataChannel(){
  int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_socket == -1) {
    cout << "Error creating socket." << endl;
    exit(1);
  }

  //random port
  int port = 5000 + rand() % 5000;

  while (true)
  {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
      //ignore
    }else{
      break; //success
    }

    port = 5000 + rand() % 100;
  }

  listen(listen_socket, 1);

  //return port and socket
  vector<int> data;
  data.push_back(port);
  data.push_back(listen_socket);

  return data;
}

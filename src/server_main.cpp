#include "../include/server.hpp"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "Usage: server <port> <home_dir>" << endl;
    return 1;
  }

  int port = atoi(argv[1]);
  string home_dir = argv[2];

  FTPServer server(port, home_dir);
  server.Start();

  return 0;
}
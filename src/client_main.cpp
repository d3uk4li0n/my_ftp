#include "../include/client.hpp"

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cout << "Usage: client <server address> <port>" << endl;
        return 1;
    }

    const char* server_addr = argv[1];
    int port = atoi(argv[2]);  
    string command;
    string filename;

    FTPClient client(server_addr, port);

    string username;
    string password;  

    cout << "HELP: Enter Anonymous and empty password to login" << endl;

    while (true){
        cout << "Enter username: ";
        getline(cin, username);
        cout << "Enter password: ";
        getline(cin, password);

        if (client.login(username.c_str(), password.c_str())){
        break;
        }
    }

    cout << "login successfully" << endl;

    client.mode();  

    while (1){
        printf("1. list files\n");
        printf("2. get file\n");
        printf("3. put file\n");
        printf("4. exit\n");
        printf("Enter your choice: ");

        cin >> command;

        if (command == "1"){
            client.list();
        }else if (command == "2"){
            cout << "Enter the name of file: ";
            cin >> filename;

            client.get(filename.c_str());
        }else if (command == "3"){
            cout << "Enter the name of file: ";
            cin >> filename;
            client.put(filename.c_str());
        }else if (command == "4") {
            break;
        }else{
            cout << "Invalid selection" << endl;
        }
    }

    client.quit();
    
    return 0;  
}
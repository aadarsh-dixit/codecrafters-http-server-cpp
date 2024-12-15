#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <netdb.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<bits/stdc++.h>

using namespace std;

vector<string> split_message(string path,  const string& deli){
  vector<string>tok;
  stringstream ss(path);
  string word;
  while(getline(ss,word,*deli.begin())){
    tok.push_back(word);
  }
  return tok;
}

vector<string> rn_seperated_header;
string get_path(string request){
  rn_seperated_header = split_message(request,"\r\n");
  vector<string> temp2 = split_message(rn_seperated_header[0]," ");
  return temp2[1];
}

string get_header_data(string header){
  vector<string> temp2 = split_message(header," ");
  return temp2[1];
}
#pragma comment(lib, "Ws2_32.lib")
int main(int argc, char **argv) {

  WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);   //   TCP
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  if(client_fd<0){
    close(client_fd);
    close(server_fd);
    std::cout<<" Client connection failed/n";
    return 1;
  }
  std::cout << "Client connected\n";
  std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";

  char buf[1024];
	int rc = recv(client_fd, buf, sizeof(buf), 0);
  string request(buf);
  cout<<request<<endl;

  string path  = get_path(request);
 vector<string>tokens = split_message(path,"/");

  if(path == "/") response = "HTTP/1.1 200 OK\r\n\r\n";
  else if(tokens[1] == "echo") {
    response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(tokens[2].length()) + "\r\n";
    response += "\r\n"; // End of headers
    response += tokens[2]; // Body
  }
  else if(tokens[1]=="user-agent"){
    string user_agent  = get_header_data(rn_seperated_header[2]);
    response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(user_agent.length()) + "\r\n";
    response += "\r\n"; // End of headers
    response += user_agent; // Body
  }

  cout<<response<<endl;

  int data = send(client_fd, response.c_str(),response.size(),0);

  close(client_fd);
  close(server_fd);

    WSACleanup(); 
  return 0;
}

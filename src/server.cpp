#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>
#include<bits/stdc++.h>

using namespace std;

// #pragma comment(lib, "ws2_32.lib")

// std::vector<std::string> split_message( std::string message, const std::string& delim) {
//   std::vector<std::string> toks;
//   stringstream ss(message);
//   std::string line;
//   while (getline(ss, line, *delim.begin())) {
//     toks.push_back(line);
//     // ss.ignore(delim.length() - 1);
//   }
//   return toks;
// }

vector<string> split_message(string path,  const string& deli){
  vector<string>tok;
  stringstream ss(path);
  string word;
  while(getline(ss,word,*deli.begin())){
    tok.push_back(word);
  }
  return tok;
}

string get_path(string request){
  vector<string> temp1 = split_message(request,"\r\n");
  vector<string> temp2 = split_message(temp1[0]," ");
  return temp2[1];
}


int main(int argc, char **argv) {

  // WSADATA wsaData;
  //   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
  //       std::cerr << "Failed to initialize Winsock." << std::endl;
  //       return 1;
  //   }
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
  else if(tokens[1] == "echo") response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "+to_string(tokens[2].length())+"\r\n\r\n"+tokens[2];
  
  cout<<response<<endl;

  int data = write(client_fd, response.c_str(),sizeof(response));

  close(client_fd);
  close(server_fd);

    // WSACleanup(); 
  return 0;
}

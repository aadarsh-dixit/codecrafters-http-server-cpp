#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <fstream>
// #include <winsock2.h>
// #include <ws2tcpip.h>
#include <thread>
#include <zlib.h>
#include <bits/stdc++.h>

using namespace std;

std::string gzipCompress(const std::string &input)
{
  // Initialize the zlib structures
  z_stream zs;
  memset(&zs, 0, sizeof(zs));

  if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
  {
    throw std::runtime_error("Failed to initialize zlib.");
  }

  zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.data()));
  zs.avail_in = input.size();

  int ret;
  char outbuffer[32768];
  std::string compressedData;

  // Compress the data
  do
  {
    zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);

    if (compressedData.size() < zs.total_out)
    {
      compressedData.append(outbuffer, zs.total_out - compressedData.size());
    }
  } while (ret == Z_OK);

  deflateEnd(&zs);

  if (ret != Z_STREAM_END)
  {
    throw std::runtime_error("Failed to compress the data.");
  }

  return compressedData;
}

vector<string> split_message(const string &path, const string &deli)
{
  vector<string> tok;
  size_t start = 0, end;

  // Use `string::find` to locate the delimiter
  while ((end = path.find(deli, start)) != string::npos)
  {
    tok.push_back(path.substr(start, end - start)); // Extract substring
    start = end + deli.length();                    // Move past the delimiter
  }
  // Add the last part of the string (after the last delimiter)
  tok.push_back(path.substr(start));
  return tok;
}

vector<string> rn_seperated_header;
vector<string> rq_line_data;
string get_path(string request)
{
  rn_seperated_header = split_message(request, "\r\n");
  rq_line_data = split_message(rn_seperated_header[0], " ");
  return rq_line_data[1];
}

string get_header_data(string header)
{
  vector<string> temp2 = split_message(header, " ");
  return temp2[1];
}

std::string read_file_as_string(const std::string &file_path)
{
  std::ifstream file(file_path);
  std::stringstream buffer;
  buffer << file.rdbuf(); // Read the entire file into the buffer
  file.close();           // Close the file after reading
  return buffer.str();    // Return the file content as a string
}

void handling_each_client(int client_fd, string directory_path)
{
  std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
  std::string accept_encoding = "invalid-encoding";

  char buf[1024];
  int rc = recv(client_fd, buf, sizeof(buf), 0);
  string request(buf);
  cout << request << endl;

  string path = get_path(request);
  vector<string> tokens = split_message(path, "/");

  vector<vector<string>> individual_space_sperated;

  for (int i = 0; i < rn_seperated_header.size(); i++)
  {
    vector<string> temp = split_message(rn_seperated_header[i], " ");
    for (int j = 0; j < temp.size(); j++)
    {
      individual_space_sperated.push_back(temp);
    }
  }

  for (int i = 0; i < individual_space_sperated.size(); i++)
  {
    if (individual_space_sperated[i][0] == "Accept-Encoding:")
    {
      for (int j = 0; j < individual_space_sperated[i].size(); j++)
      {
        if (individual_space_sperated[i][j] == "gzip," || individual_space_sperated[i][j] == "gzip")
        {
          accept_encoding = "gzip";
          break;
        }
      }
      break;
    }
  }

  if (path == "/")
    response = "HTTP/1.1 200 OK\r\n\r\n";
  else if (tokens[1] == "echo")
  {
    response = "HTTP/1.1 200 OK\r\n";
    if (accept_encoding == "gzip")
      response += "Content-Encoding: gzip\r\n";
    response += "Content-Type: text/plain\r\n";
    if (accept_encoding == "gzip")
    {
      string compressed_body = gzipCompress(tokens[2]);
      response += "Content-Length: " + std::to_string(compressed_body.length()) + "\r\n";
      response += "\r\n"; // End of headers
      response += compressed_body;
    }
    else
    {
      response += "Content-Length: " + std::to_string(tokens[2].length()) + "\r\n";
      response += "\r\n";    // End of headers
      response += tokens[2]; // Body
    }
  }
  else if (tokens[1] == "user-agent")
  {
    string user_agent = get_header_data(rn_seperated_header[2]);
    response = "HTTP/1.1 200 OK\r\n";
    if (accept_encoding == "gzip")
      response += "Content-Encoding: gzip\r\n";
    response += "Content-Type: text/plain\r\n";
    if (accept_encoding == "gzip")
    {
      string compressed_body = gzipCompress(user_agent);
      response += "Content-Length: " + std::to_string(compressed_body.length()) + "\r\n";
      response += "\r\n"; // End of headers
      response += compressed_body;
    }
    else
    {
      response += "Content-Length: " + std::to_string(user_agent.length()) + "\r\n";
      response += "\r\n";     // End of headers
      response += user_agent; // Body
    }
  }
  else if (tokens[1] == "files")
  {
    string file_path = directory_path + tokens[2];
    if (rq_line_data[0] == "GET")
    {
      if (filesystem::exists(file_path))
      {
        string data_from_file = read_file_as_string(file_path);
        response = "HTTP/1.1 200 OK\r\n";
        if (accept_encoding == "gzip")
          response += "Content-Encoding: gzip\r\n";
        response += "Content-Type: application/octet-stream\r\n";
        if (accept_encoding == "gzip")
        {
          string compressed_body = gzipCompress(data_from_file);
          response += "Content-Length: " + std::to_string(compressed_body.length()) + "\r\n";
          response += "\r\n"; // End of headers
          response += compressed_body;
        }
        else
        {
          response += "Content-Length: " + std::to_string(data_from_file.length()) + "\r\n";
          response += "\r\n";         // End of headers
          response += data_from_file; // Body
        }
      }
    }
    else if (rq_line_data[0] == "POST")
    {
      std::ofstream file(file_path);
      if (file.is_open())
      {
        // Write data to the file
        int size = rn_seperated_header.size();
        cout << "*************************" << endl;
        for (int i = 0; i < size; i++)
        {
          cout << rn_seperated_header[i] << endl;
        }
        cout << endl;
        file << rn_seperated_header[size - 1];
        std::cout << "File created and data written successfully at: " << file_path << std::endl;
        // Close the file
        file.close();
      }
      else
      {
        // Handle the error if the file could not be created
        std::cerr << "Failed to create the file at path: " << file_path << std::endl;
      }
      response = "HTTP/1.1 201 Created\r\n\r\n";
    }
  }

  cout << response << endl;

  int data = send(client_fd, response.c_str(), response.size(), 0);
  close(client_fd);
}

// #pragma comment(lib, "Ws2_32.lib")
int main(int argc, char **argv)
{

  // WSADATA wsaData;
  // if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
  //     std::cerr << "Failed to initialize Winsock." << std::endl;
  //     return 1;
  // }
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage

  // argc no of argument passed including the program name
  // argv is char* array containt the arguments

  string directory_path = "";
  for (int i = 0; i < argc; i++)
  {
    if (string(argv[i]) == "--directory")
    {
      if (i + 1 < argc)
      {
        directory_path = string(argv[i + 1]);
      }
    }
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0); //   TCP
  if (server_fd < 0)
  {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
  {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
  {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  while (true)
  {
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
    if (client_fd < 0)
    {
      close(client_fd);
      close(server_fd);
      std::cout << " Client connection failed/n";
      return 1;
    }
    std::cout << "Client connected\n";

    thread t1(handling_each_client, client_fd, directory_path);
    t1.detach();
  }

  close(server_fd);
  // WSACleanup();
  return 0;
}

#include "EchoAssignment.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>

// !IMPORTANT: allowed system calls.
// !DO NOT USE OTHER NETWORK SYSCALLS (send, recv, select, poll, epoll, fork
// etc.)
//  * socket
//  * bind
//  * listen
//  * accept
//  * read
//  * write
//  * close
//  * getsockname
//  * getpeername
// See below for their usage.
// https://github.com/ANLAB-KAIST/KENSv3/wiki/Misc:-External-Resources#linux-manuals

int EchoAssignment::serverMain(const char *bind_ip, int port,
                               const char *server_hello) {
  // Your server code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for serverMain.

  int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_fd == -1) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in *server_addr = new sockaddr_in;

  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(port);
  server_addr->sin_addr.s_addr = inet_addr(bind_ip);

  if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) ==
      -1) {
    perror("bind");
    return -1;
  }

  if (listen(server_fd, 5) == -1) {
    perror("listen");
    return -1;
  }

  struct sockaddr_in *client_addr = new sockaddr_in;
  socklen_t client_addr_len = sizeof(sockaddr_in);

  int client_fd =
      accept(server_fd, (struct sockaddr *)client_addr, &client_addr_len);
  if (client_fd == -1) {
    perror("accept");
    return -1;
  }

  if (getpeername(client_fd, (struct sockaddr *)(client_addr),
                  &client_addr_len)) {
    delete client_addr;
    return -1;
  }

  char client_addr_string[INET_ADDRSTRLEN] = {0};
  inet_ntop(AF_INET, &(client_addr->sin_addr.s_addr), client_addr_string,
            INET_ADDRSTRLEN);

  char *buf = (char *)calloc(10, sizeof(char));

  int read_len = read(client_fd, buf, 10);

  if (read_len == -1) {
    perror("read");
    return -1;
  }

  int write_len = write(client_fd, buf, read_len);
  if (write_len == -1) {
    perror("write");
    return -1;
  }

  submitAnswer(client_addr_string, buf);
  return 0;
}

int EchoAssignment::clientMain(const char *server_ip, int port,
                               const char *command) {
  // Your client code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for clientMain.

  int client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_fd == -1) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in *server_addr = new sockaddr_in;
  socklen_t server_addr_len = sizeof(sockaddr_in);

  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(port);
  server_addr->sin_addr.s_addr = inet_addr(server_ip);

  if (connect(client_fd, (struct sockaddr *)server_addr,
              sizeof(struct sockaddr_in)) == -1) {
    perror("connect");
    close(client_fd);

    return -1;
  }

  if (getpeername(client_fd, (struct sockaddr *)(server_addr),
                  &server_addr_len)) {
    delete server_addr;
    return -1;
  }

  char server_addr_string[INET_ADDRSTRLEN] = {0};
  inet_ntop(AF_INET, &(server_addr->sin_addr.s_addr), server_addr_string,
            INET_ADDRSTRLEN);

  char *buf = (char *)calloc(10, sizeof(char));

  int write_len = write(client_fd, command, strlen(command));
  if (write_len == -1) {
    perror("write");
    close(client_fd);
    return -1;
  }

  int read_len = read(client_fd, buf, 10);
  if (read_len == -1) {
    perror("read");
    close(client_fd);
    return -1;
  }

  submitAnswer(server_addr_string, buf);

  close(client_fd);
  return 0;
}

static void print_usage(const char *program) {
  printf("Usage: %s <mode> <ip-address> <port-number> <command/server-hello>\n"
         "Modes:\n  c: client\n  s: server\n"
         "Client commands:\n"
         "  hello : server returns <server-hello>\n"
         "  whoami: server returns <client-ip>\n"
         "  whoru : server returns <server-ip>\n"
         "  others: server echos\n"
         "Note: each command is terminated by newline character (\\n)\n"
         "Examples:\n"
         "  server: %s s 0.0.0.0 9000 hello-client\n"
         "  client: %s c 127.0.0.1 9000 whoami\n",
         program, program, program);
}

int EchoAssignment::Main(int argc, char *argv[]) {

  if (argc == 0)
    return 1;

  if (argc != 5) {
    print_usage(argv[0]);
    return 1;
  }

  int port = atoi(argv[3]);
  if (port == 0) {
    printf("Wrong port number\n");
    print_usage(argv[0]);
  }

  switch (*argv[1]) {
  case 'c':
    return clientMain(argv[2], port, argv[4]);
  case 's':
    return serverMain(argv[2], port, argv[4]);
  default:
    print_usage(argv[0]);
    return 1;
  }
}

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

  struct sockaddr_in *bind_addr = new sockaddr_in;

  bind_addr->sin_family = AF_INET;
  bind_addr->sin_port = htons(port);
  bind_addr->sin_addr.s_addr = inet_addr(bind_ip);

  if (bind(server_fd, (struct sockaddr *)bind_addr, sizeof(*bind_addr)) == -1) {
    perror("bind");
    return -1;
  }

  if (listen(server_fd, 100) == -1) {
    perror("listen");
    return -1;
  }

  struct sockaddr_in *client_addr = new sockaddr_in;
  socklen_t client_addr_len = sizeof(sockaddr_in);

  int client_fd;
  while ((client_fd = accept(server_fd, (struct sockaddr *)client_addr,
                             &client_addr_len)) >= 0) {
    if (client_fd == -1) {
      perror("accept");
      return -1;
    }

    char client_addr_string[INET_ADDRSTRLEN] = {0};

    inet_ntop(AF_INET, &(client_addr->sin_addr.s_addr), client_addr_string,
              INET_ADDRSTRLEN);

    char *buf = (char *)calloc(1024, sizeof(char));

    int read_len = read(client_fd, buf, 1024);

    if (read_len == -1) {
      perror("read");
      return -1;
    }

    submitAnswer(client_addr_string, buf);

    if (!strcmp(buf, "hello")) {
      strcpy(buf, server_hello);
    } else if (!strcmp(buf, "whoami")) {
      strcpy(buf, client_addr_string);
    } else if (!strcmp(buf, "whoru")) {
      strcpy(buf, bind_ip);
    } else {
      // echo buf
    }

    int write_len = write(client_fd, buf, strlen(buf));
    if (write_len == -1) {
      perror("write");
      return -1;
    }
  }

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

  char server_addr_string[INET_ADDRSTRLEN] = {0};
  inet_ntop(AF_INET, &(server_addr->sin_addr.s_addr), server_addr_string,
            INET_ADDRSTRLEN);

  char *buf = (char *)calloc(1024, sizeof(char));

  int write_len = write(client_fd, command, strlen(command));
  if (write_len == -1) {
    perror("write");
    close(client_fd);
    return -1;
  }

  int read_len = read(client_fd, buf, 1024);
  if (read_len == -1) {
    perror("read");
    close(client_fd);
    return -1;
  }

  if (!strcmp(buf, "0.0.0.0")) {
    submitAnswer(server_addr_string, server_ip);
  } else {
    submitAnswer(server_addr_string, buf);
  }

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

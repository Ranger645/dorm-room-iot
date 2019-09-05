

#ifndef DAEMON_INTERFACE_H
#define DAEMON_INTERFACE_H

#include <stdio.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include "../server_types.h"
#include "../util/util_string.h"

#define NEVER_RESTART 0
#define ALWAYS_RESTART 1
#define RESTART_ONCE 2

#define DAEMON_COMMAND_MAX_SIZE 1024

#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif

typedef struct DaemonStuct {
    pid_t daemon_process;
    pthread_t daemon_handler;
    int output; // Pipe end to write to (output -> Daemon)
    int input; // Pipe end to read from (Daemon -> input)
    int restart_flags;
    ClientSlot *clients_list
} Daemon;

Daemon *start_daemon(char *command, int flags, ClientSlot *clients_list);
void send_to_daemon(int client_id, char *message, Daemon *daemon);
void *handle_daemon(void *targs);
void free_daemon(Daemon *daemon);

#endif

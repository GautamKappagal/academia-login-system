// server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_USERS 100

typedef struct {
    int sock;
    struct sockaddr_in addr;
} client_data;

void handle_admin(int sock);
void handle_student(int sock);
void handle_faculty(int sock);

void *client_handler(void *arg) {
    client_data *data = (client_data *)arg;
    int sock = data->sock;
    free(data);

    char role[16], username[32], password[32];
    read(sock, role, sizeof(role));
    read(sock, username, sizeof(username));
    read(sock, password, sizeof(password));

    // Simple auth check (can be replaced with file-based auth)
    if (strcmp(role, "admin") == 0 && strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        write(sock, "success", 8);
        handle_admin(sock);
    } else if (strcmp(role, "student") == 0 && strcmp(password, "student123") == 0) {
        write(sock, "success", 8);
        handle_student(sock);
    } else if (strcmp(role, "faculty") == 0 && strcmp(password, "faculty123") == 0) {
        write(sock, "success", 8);
        handle_faculty(sock);
    } else {
        write(sock, "failure", 8);
    }

    close(sock);
    pthread_exit(NULL);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        socklen_t addrlen = sizeof(address);
        int client_sock = accept(server_fd, (struct sockaddr *)&address, &addrlen);

        client_data *data = malloc(sizeof(client_data));
        data->sock = client_sock;
        data->addr = address;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, data);
        pthread_detach(tid);
    }

    return 0;
}

// Implement these with file I/O and locking
void handle_admin(int sock) {
    char buffer[256];
    while (1) {
        read(sock, buffer, sizeof(buffer));
        if (strcmp(buffer, "exit") == 0) break;
        write(sock, "admin-action-received", 23);
    }
}

void handle_student(int sock) {
    char buffer[256];
    while (1) {
        read(sock, buffer, sizeof(buffer));
        if (strcmp(buffer, "exit") == 0) break;
        write(sock, "student-action-received", 25);
    }
}

void handle_faculty(int sock) {
    char buffer[256];
    while (1) {
        read(sock, buffer, sizeof(buffer));
        if (strcmp(buffer, "exit") == 0) break;
        write(sock, "faculty-action-received", 24);
    }
}
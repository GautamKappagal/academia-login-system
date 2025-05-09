#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_LEN 100

// Function to validate admin login
int validateAdmin(const char *username_input, const char *password_input) {
    int fd = open("admins.txt", O_RDONLY);
    if (fd == -1) {
        write(2, "Error opening admin credentials file\n", 37);
        return 0;
    }

    char buffer[MAX_LEN];
    int bytesRead;
    char username[MAX_LEN], password[MAX_LEN];
    int idx = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            if (buffer[i] == ' ') {
                username[idx] = '\0';  // End the username string
                idx = 0;

                // Now, read the password part
                int j = 0;
                while (i + 1 < bytesRead && buffer[i + 1] != '\n' && buffer[i + 1] != '\0') {
                    password[j++] = buffer[++i];
                }
                password[j] = '\0';  // End the password string

                // Compare with user input
                if (strcmp(username_input, username) == 0 && strcmp(password_input, password) == 0) {
                    close(fd);
                    return 1;  // Login successful
                }
            } else if (idx < MAX_LEN - 1) {
                username[idx++] = buffer[i];  // Collect username characters
            }
        }
    }

    close(fd);
    return 0;  // Login failed
}

// Function to validate faculty login
int validateFaculty(const char *username_input, const char *password_input) {
    int fd = open("faculties.txt", O_RDONLY);
    if (fd == -1) {
        write(2, "Error opening faculty credentials file\n", 39);
        return 0;
    }

    char buffer[MAX_LEN];
    int bytesRead;
    char username[MAX_LEN], password[MAX_LEN];
    int idx = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                username[idx] = '\0';  // End the username string
                idx = 0;

                // Now, read the password part
                int j = 0;
                while (i + 1 < bytesRead && buffer[i + 1] != '\n' && buffer[i + 1] != '\0') {
                    password[j++] = buffer[++i];
                }
                password[j] = '\0';  // End the password string

                // Compare with user input
                if (strcmp(username_input, username) == 0 && strcmp(password_input, password) == 0) {
                    close(fd);
                    return 1;  // Login successful
                }
            } else if (idx < MAX_LEN - 1) {
                username[idx++] = buffer[i];  // Collect username characters
            }
        }
    }

    close(fd);
    return 0;  // Login failed
}

// Function to validate student login
int validateStudent(const char *username_input, const char *password_input) {
    int fd = open("students.txt", O_RDONLY);
    if (fd == -1) {
        write(2, "Error opening student credentials file\n", 39);
        return 0;
    }

    char buffer[MAX_LEN];
    int bytesRead;
    char username[MAX_LEN], password[MAX_LEN];
    int idx = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                username[idx] = '\0';  // End the username string
                idx = 0;

                // Now, read the password part
                int j = 0;
                while (i + 1 < bytesRead && buffer[i + 1] != '\n' && buffer[i + 1] != '\0') {
                    password[j++] = buffer[++i];
                }
                password[j] = '\0';  // End the password string

                // Compare with user input
                if (strcmp(username_input, username) == 0 && strcmp(password_input, password) == 0) {
                    close(fd);
                    return 1;  // Login successful
                }
            } else if (idx < MAX_LEN - 1) {
                username[idx++] = buffer[i];  // Collect username characters
            }
        }
    }

    close(fd);
    return 0;  // Login failed
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char username[MAX_LEN] = {0}, password[MAX_LEN] = {0};
    char response[MAX_LEN] = {0};
    int len;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

    // Read username
    len = read(new_socket, username, MAX_LEN);
    username[len] = '\0';

    // Read password
    len = read(new_socket, password, MAX_LEN);
    password[len] = '\0';

    // Validate
    if (validateAdmin(username, password)) {
        strcpy(response, "Admin login successful!\n");
    } else if (validateFaculty(username, password)) {
        strcpy(response, "Faculty login successful!\n");
    } else if (validateStudent(username, password)) {
        strcpy(response, "Student login successful!\n");
    } else {
        strcpy(response, "Login failed. Invalid username or password.\n");
    }

    write(new_socket, response, strlen(response));

    close(new_socket);
    close(server_fd);
    return 0;
}
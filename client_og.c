#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to add student
void add_student(int sock) {
    char username[BUFFER_SIZE] = {0};
    char password[BUFFER_SIZE] = {0};

    // Get student username
    write(STDOUT_FILENO, "Enter student username: ", 25);
    int ulen = read(STDIN_FILENO, username, BUFFER_SIZE);
    if (ulen > 0 && username[ulen - 1] == '\n') {
        username[ulen - 1] = '\0';
    }

    // Get student password
    write(STDOUT_FILENO, "Enter student password: ", 25);
    int plen = read(STDIN_FILENO, password, BUFFER_SIZE);
    if (plen > 0 && password[plen - 1] == '\n') {
        password[plen - 1] = '\0';
    }

    // Send username and password separately
    write(sock, username, BUFFER_SIZE);
    write(sock, password, BUFFER_SIZE);

    // Read whether the student was added successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "Username already exists.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
    } else {
        write(STDOUT_FILENO, "Student added successfully.\n", 28);
    }
}

void add_faculty(int sock) {
    char username[BUFFER_SIZE] = {0};
    char password[BUFFER_SIZE] = {0};

    // Get faculty username
    write(STDOUT_FILENO, "Enter faculty username: ", 25);
    int ulen = read(STDIN_FILENO, username, BUFFER_SIZE);
    if (ulen > 0 && username[ulen - 1] == '\n') {
        username[ulen - 1] = '\0';
    }

    // Get faculty password
    write(STDOUT_FILENO, "Enter faculty password: ", 25);
    int plen = read(STDIN_FILENO, password, BUFFER_SIZE);
    if (plen > 0 && password[plen - 1] == '\n') {
        password[plen - 1] = '\0';
    }

    // Send username and password separately
    write(sock, username, BUFFER_SIZE);
    write(sock, password, BUFFER_SIZE);

    // Read whether the faculty was added successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "Username already exists.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
    } else {
        write(STDOUT_FILENO, "Faculty added successfully.\n", 28);
    }
}

void view_all_courses(int sock) {
    // Here you would typically send a request to the server to get the list of courses
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "No courses available.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
    } else {
        write(STDOUT_FILENO, "Available courses:\n", 19);
        write(STDOUT_FILENO, response, strlen(response));
        write(STDOUT_FILENO, "\n", 1);
    }
}

int view_student_details(int sock, char *username) {
    write(sock, username, BUFFER_SIZE);
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "Student not found.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
        return 0;  // Indicate failure
    } else {
        write(STDOUT_FILENO, "Student courses: ", 17);
        write(STDOUT_FILENO, response, strlen(response));
        write(STDOUT_FILENO, "\n", 1);
        return 1;  // Success
    }
}

int view_faculty_details(int sock, char *username) {
    // Send faculty ID to server
    write(sock, username, BUFFER_SIZE);
    // Here you would typically wait for a response from the server
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "Faculty not found.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
        return 0;  // Indicate failure
    } else {
        write(STDOUT_FILENO, response, strlen(response));
        write(STDOUT_FILENO, "\n", 1);
        return 1;  // Success
    }
}

void add_course(int sock) {
    char course_name[BUFFER_SIZE] = {0};

    // Get course name
    write(STDOUT_FILENO, "Enter course code: ", 19);
    int clen = read(STDIN_FILENO, course_name, BUFFER_SIZE);
    if (clen > 0 && course_name[clen - 1] == '\n') {
        course_name[clen - 1] = '\0';
    }

    write(sock, course_name, BUFFER_SIZE);

    // Read whether the course was added successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

void enroll_course(int sock) {
    char course_name[BUFFER_SIZE] = {0};

    // Get course name
    write(STDOUT_FILENO, "Enter course code: ", 19);
    int clen = read(STDIN_FILENO, course_name, BUFFER_SIZE);
    if (clen > 0 && course_name[clen - 1] == '\n') {
        course_name[clen - 1] = '\0';
    }

    write(sock, course_name, BUFFER_SIZE);

    // Read whether the course was enrolled successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

void delete_course(int sock) {
    char course_name[BUFFER_SIZE] = {0};

    // Get course name
    write(STDOUT_FILENO, "Enter course code: ", 19);
    int clen = read(STDIN_FILENO, course_name, BUFFER_SIZE);
    if (clen > 0 && course_name[clen - 1] == '\n') {
        course_name[clen - 1] = '\0';
    }

    write(sock, course_name, BUFFER_SIZE);

    // Read whether the course was deleted successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

void view_courses(int sock) {
    // Here you would typically send a request to the server to get the list of courses
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);

    if (strcmp(response, "No courses available.\n") == 0) {
        write(STDERR_FILENO, response, strlen(response));
    } else {
        write(STDOUT_FILENO, "Available courses: ", 19);
        write(STDOUT_FILENO, response, strlen(response));
        write(STDOUT_FILENO, "\n", 1);
    }
}

void activate_student(int sock) {
    char username[BUFFER_SIZE] = {0};

    // Get student username
    write(STDOUT_FILENO, "Enter student username: ", 25);
    int ulen = read(STDIN_FILENO, username, BUFFER_SIZE);
    if (ulen > 0 && username[ulen - 1] == '\n') {
        username[ulen - 1] = '\0';
    }

    write(sock, username, BUFFER_SIZE);

    // Read whether the student was activated successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

void block_student(int sock) {
    char username[BUFFER_SIZE] = {0};

    // Get student username
    write(STDOUT_FILENO, "Enter student username: ", 25);
    int ulen = read(STDIN_FILENO, username, BUFFER_SIZE);
    if (ulen > 0 && username[ulen - 1] == '\n') {
        username[ulen - 1] = '\0';
    }

    write(sock, username, BUFFER_SIZE);

    // Read whether the student was blocked successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

void update_course(int sock) {
    char old_course[BUFFER_SIZE] = {0};
    char new_course[BUFFER_SIZE] = {0};

    // Get old course name
    write(STDOUT_FILENO, "Enter old course code: ", 23);
    int clen = read(STDIN_FILENO, old_course, BUFFER_SIZE);
    if (clen > 0 && old_course[clen - 1] == '\n') {
        old_course[clen - 1] = '\0';
    }

    // Get new course name
    write(STDOUT_FILENO, "Enter new course code: ", 23);
    clen = read(STDIN_FILENO, new_course, BUFFER_SIZE);
    if (clen > 0 && new_course[clen - 1] == '\n') {
        new_course[clen - 1] = '\0';
    }

    write(sock, old_course, BUFFER_SIZE);
    write(sock, new_course, BUFFER_SIZE);

    // Read whether the course was updated successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

void change_password(int sock){
    char new_password[BUFFER_SIZE] = {0};

    // Get new password
    write(STDOUT_FILENO, "Enter new password: ", 20);
    int clen = read(STDIN_FILENO, new_password, BUFFER_SIZE);
    if (clen > 0 && new_password[clen - 1] == '\n') {
        new_password[clen - 1] = '\0';
    }

    write(sock, new_password, BUFFER_SIZE);

    // Read whether the password was changed successfully
    char response[BUFFER_SIZE] = {0};
    read(sock, response, BUFFER_SIZE);
    write(STDOUT_FILENO, response, strlen(response));
}

// Handle user input for admin, faculty, or student
void handle_admin_input(int sock, char *buffer) {
    // Display menu options
    write(STDOUT_FILENO, "\n....... Welcome to Admin Menu .......\n", 39);
    write(STDOUT_FILENO, "1. Add Student\n", 15);
    write(STDOUT_FILENO, "2. View Student Details\n", 24);
    write(STDOUT_FILENO, "3. Add Faculty\n", 15);
    write(STDOUT_FILENO, "4. View Faculty Details\n", 25);
    write(STDOUT_FILENO, "5. Activate Student\n", 20);
    write(STDOUT_FILENO, "6. Block Student\n", 18);
    write(STDOUT_FILENO, "7. Logout and Exit\n", 20);

    // Take user choice
    write(STDOUT_FILENO, "Enter your choice: ", 19);

    // Clear buffer and read user input
    memset(buffer, 0, BUFFER_SIZE);
    int len = read(STDIN_FILENO, buffer, BUFFER_SIZE);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';  // Remove the newline character
    }

    write(sock, buffer, strlen(buffer));

    // Handle user choice based on input
    if (strcmp(buffer, "1") == 0) {
        add_student(sock);
    } else if (strcmp(buffer, "2") == 0) {
        char username[BUFFER_SIZE] = {0};
        write(STDOUT_FILENO, "Enter student username: ", 25);
        len = read(STDIN_FILENO, username, BUFFER_SIZE);
        if (len > 0 && username[len - 1] == '\n') {
            username[len - 1] = '\0';
        }
        if (!view_student_details(sock, username)) return;
    } else if (strcmp(buffer, "3") == 0) {
        add_faculty(sock);
    } else if (strcmp(buffer, "4") == 0) {
        char username[BUFFER_SIZE] = {0};
        write(STDOUT_FILENO, "Enter faculty username: ", 25);
        len = read(STDIN_FILENO, username, BUFFER_SIZE);
        if (len > 0 && username[len - 1] == '\n') {
            username[len - 1] = '\0';
        }
        if (!view_faculty_details(sock, username)) return;
    } else if (strcmp(buffer, "5") == 0) {
        activate_student(sock);
    } else if (strcmp(buffer, "6") == 0) {
        block_student(sock);
    } else if (strcmp(buffer, "7") == 0) {
        write(STDOUT_FILENO, "Logging out and exiting...\n", 28);
        close(sock);
        exit(0);
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }
}

void handle_faculty_input(int sock, char *username, char *password) {
    // Display menu options
    write(STDOUT_FILENO, "\n....... Welcome to Faculty Menu .......\n", 41);
    write(STDOUT_FILENO, "1. View Offering Courses\n", 26);
    write(STDOUT_FILENO, "2. Add New Course\n", 19);
    write(STDOUT_FILENO, "3. Remove Course from Catalog\n", 31);
    write(STDOUT_FILENO, "4. Update Course Details\n", 26);
    write(STDOUT_FILENO, "5. Change Password\n", 20);
    write(STDOUT_FILENO, "6. Logout and Exit\n", 20);

    // Take user choice
    char buffer[BUFFER_SIZE] = {0};
    write(STDOUT_FILENO, "Enter your choice: ", 19);

    // Clear buffer and read user input
    memset(buffer, 0, BUFFER_SIZE);
    int len = read(STDIN_FILENO, buffer, BUFFER_SIZE);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';  // Remove the newline character
    }

    // Send choice to server
    write(sock, buffer, strlen(buffer));

    // Handle user choice
    if (strcmp(buffer, "1") == 0) {
        view_faculty_details(sock, username);
    } else if (strcmp(buffer, "2") == 0) {
        add_course(sock);
    } else if (strcmp(buffer, "3") == 0) {
        delete_course(sock);
    } else if (strcmp(buffer, "4") == 0) {
        update_course(sock);
    } else if (strcmp(buffer, "5") == 0) {
        change_password(sock);
    } else if (strcmp(buffer, "6") == 0) {
        write(STDOUT_FILENO, "Logging out and exiting...\n", 28);
        close(sock);
        exit(0);
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }
}

void handle_student_input(int sock, char *username, char *password) {
    // Display menu options
    write(STDOUT_FILENO, "\n....... Welcome to Student Menu .......\n", 41);
    write(STDOUT_FILENO, "1. View Courses\n", 16);
    write(STDOUT_FILENO, "2. Enroll (pick) New Course\n", 28);
    write(STDOUT_FILENO, "3. Drop Course\n", 16);
    write(STDOUT_FILENO, "4. View Enrolled Course Details\n", 33);
    write(STDOUT_FILENO, "5. Change Password\n", 20);
    write(STDOUT_FILENO, "6. Logout and Exit\n", 20);

    // Take user choice
    char buffer[BUFFER_SIZE] = {0};
    write(STDOUT_FILENO, "Enter your choice: ", 19);

    // Clear buffer and read user input
    memset(buffer, 0, BUFFER_SIZE);
    int len = read(STDIN_FILENO, buffer, BUFFER_SIZE);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';  // Remove the newline character
    }
    // Send choice to server
    write(sock, buffer, strlen(buffer));

    // Handle user choice
    if (strcmp(buffer, "1") == 0) {
        view_all_courses(sock);
    } else if (strcmp(buffer, "2") == 0) {
        enroll_course(sock);
    } else if (strcmp(buffer, "3") == 0) {
        delete_course(sock);
    } else if (strcmp(buffer, "4") == 0) {
        view_student_details(sock, username);
    } else if (strcmp(buffer, "5") == 0) {
        change_password(sock);
    } else if (strcmp(buffer, "6") == 0) {
        write(STDOUT_FILENO, "Logging out and exiting...\n", 28);
        close(sock);
        exit(0);
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char role_input;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Zero out the structure
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        return -1;
    }

    // Try connecting
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    // Write a message to terminal using system call
    write(STDOUT_FILENO, "Connection established.\n", 24);

    // Take input for user role
    write(STDOUT_FILENO, "....................Welcome Back to Academia :: Course Registration....................\n", 89);
    write(STDOUT_FILENO, "Login Type\n", 11);
    write(STDOUT_FILENO, "Enter Your Choice { 1.Admin , 2.Professor, 3. Student } : ", 58);
    read(STDIN_FILENO, &role_input, 1);

    // Clear the newline character left in the input buffer
    while (getchar() != '\n');  // This consumes the newline character

    // Handle role input (just printing it for now)
    write(STDOUT_FILENO, "You selected role: ", 19);
    write(STDOUT_FILENO, &role_input, 1);
    write(STDOUT_FILENO, "\n", 1);

    // Send selected role to server
    write(sock, &role_input, sizeof(role_input));

    // Handle login and other logic for different roles
    if (role_input == '1') {
        // Admin login
        write(STDOUT_FILENO, "Username: ", 10);
        scanf("%s", buffer);
        write(STDOUT_FILENO, "Password: ", 10);
        scanf("%s", buffer + 50);  // Store password after username in buffer

        char *username = buffer;
        char *password = buffer + 50;  // Password is stored after username in buffer

        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
            write(STDOUT_FILENO, "Login successful.\n", 18);
            write(sock, "Admin login successful.\n", 25);
        } else {
            write(STDOUT_FILENO, "Invalid credentials.\n", 21);
            write(sock, "Invalid credentials.\n", 21);
            close(sock);
            return 0;
        }

        while (1) {
            handle_admin_input(sock, buffer); // Handle Admin tasks
        }
    } else if (role_input == '2') {
        // Professor login
        write(STDOUT_FILENO, "Username: ", 10);
        scanf("%s", buffer);
        write(STDOUT_FILENO, "Password: ", 10);
        scanf("%s", buffer + 50);  // Store password after username in buffer

        char *username = buffer;
        char *password = buffer + 50;  // Password is stored after username in buffer

        write(sock, username, BUFFER_SIZE);
        write(sock, password, BUFFER_SIZE);

        char response[BUFFER_SIZE] = {0};
        read(sock, response, BUFFER_SIZE);

        write(STDOUT_FILENO, response, strlen(response));
        if (strcmp(response, "Faculty login successful.\n") == 0) {
            while (1) {
                handle_faculty_input(sock, username, password); // Handle Faculty tasks
            }
        } else {
            close(sock);
            return 0;
        }
    } else if (role_input == '3') {
        // Student login
        write(STDOUT_FILENO, "Username: ", 10);
        scanf("%s", buffer);
        write(STDOUT_FILENO, "Password: ", 10);
        scanf("%s", buffer + 50);  // Store password after username in buffer

        char *username = buffer;
        char *password = buffer + 50;  // Password is stored after username in buffer

        write(sock, username, BUFFER_SIZE);
        write(sock, password, BUFFER_SIZE);

        char response[BUFFER_SIZE] = {0};
        read(sock, response, BUFFER_SIZE);

        write(STDOUT_FILENO, response, strlen(response));
        if (strcmp(response, "Student login successful.\n") == 0) {
            while (1) {
                handle_student_input(sock, username, password); // Handle Student tasks
            }
        } else {
            close(sock);
            return 0;
        }
    } else {
        write(STDOUT_FILENO, "Invalid role selected.\n", 23);
    }
    return 0;
}

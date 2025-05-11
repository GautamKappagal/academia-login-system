#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int student_exists(const char *received_username) {
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return 0;  // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username (before first colon)
            char *colon_pos = strchr(line, ':');
            if (colon_pos != NULL) {
                *colon_pos = '\0'; // temporarily terminate at colon
                if (strcmp(line, received_username) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

int faculty_exists(const char *received_username) {
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return 0;  // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username (before first colon)
            char *colon_pos = strchr(line, ':');
            if (colon_pos != NULL) {
                *colon_pos = '\0'; // temporarily terminate at colon
                if (strcmp(line, received_username) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

void add_student(int sock) {// Receive username and password
    char received_username[BUFFER_SIZE] = {0};
    char received_password[BUFFER_SIZE] = {0};

    read(sock, received_username, BUFFER_SIZE);
    read(sock, received_password, BUFFER_SIZE);

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, received_username, strlen(received_username));
    write(STDOUT_FILENO, "\n", 1);

    // Check if username already exists in database
    if (student_exists(received_username)) {
        char *err = "Username already exists.\n";
        write(sock, err, strlen(err));
        return;
    }

    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);


    // Construct student entry
    char entry[BUFFER_SIZE];
    int entry_len = snprintf(entry, BUFFER_SIZE, "%s:%s: :1\n", received_username, received_password);

    // Open students.txt in append mode, create if doesn't exist
    int fd = open("students.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    // Lock file for writing
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLKW, &lock);

    // Write the new entry
    write(fd, entry, entry_len);

    // Unlock file
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);

    // Notify client
    char *msg = "Student added successfully.\n";
    write(sock, msg, strlen(msg));
}

void add_faculty(int sock) {
    char received_username[BUFFER_SIZE] = {0};
    char received_password[BUFFER_SIZE] = {0};

    read(sock, received_username, BUFFER_SIZE);
    read(sock, received_password, BUFFER_SIZE);

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, received_username, strlen(received_username));
    write(STDOUT_FILENO, "\n", 1);

    // Check if username already exists in database
    if (faculty_exists(received_username)) {
        char *err = "Username already exists.\n";
        write(sock, err, strlen(err));
        return;
    }

    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);

    // Construct faculty entry
    char entry[BUFFER_SIZE];
    int entry_len = snprintf(entry, BUFFER_SIZE, "%s:%s:x\n", received_username, received_password);

    // Open students.txt in append mode, create if doesn't exist
    int fd = open("faculties.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    // Lock file for writing
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLKW, &lock);

    // Write the new entry
    write(fd, entry, entry_len);

    // Unlock file
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);

    close(fd);

    // Notify client
    char *msg = "Faculty added successfully.\n";
    write(sock, msg, strlen(msg));
}

void add_course(int sock, const char* faculty_username) {
    char course_name[BUFFER_SIZE] = {0};
    read(sock, course_name, BUFFER_SIZE);

    // Print received course
    write(STDOUT_FILENO, "Received course code: ", 23);
    write(STDOUT_FILENO, course_name, strlen(course_name));
    write(STDOUT_FILENO, "\n", 1);

    int fd_orig = open("faculties.txt", O_RDONLY);
    int fd_temp = open("faculties_tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_orig < 0 || fd_temp < 0) {
        char *err = "Error opening files\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(fd_temp, F_SETLKW, &lock);

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read;
    int found = 0;

    while ((bytes_read = read(fd_orig, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 2) {
            line[idx] = '\0';

            char copy[BUFFER_SIZE];
            strcpy(copy, line);
            char *username = strtok(copy, ":");
            char *password = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");

            if (username && strcmp(username, faculty_username) == 0) {
                found = 1;
                // check if x or existing courses
                if (!courses || strcmp(courses, "x") == 0) {
                    snprintf(line, BUFFER_SIZE, "%s:%s:%s", username, password, course_name);
                } else {
                    // append course with comma
                    char new_courses[BUFFER_SIZE];
                    snprintf(new_courses, BUFFER_SIZE, "%s,%s", courses, course_name);
                    snprintf(line, BUFFER_SIZE, "%s:%s:%s", username, password, new_courses);
                }
            }

            write(fd_temp, line, strlen(line));
            write(fd_temp, "\n", 1);
            idx = 0;
        } else {
            line[idx++] = buffer[0];
        }
    }

    // Handle last line without newline
    if (idx > 0) {
        line[idx] = '\0';
        char copy[BUFFER_SIZE];
        strcpy(copy, line);
        char *username = strtok(copy, ":");
        char *password = strtok(NULL, ":");
        char *courses = strtok(NULL, ":");

        if (username && strcmp(username, faculty_username) == 0) {
            found = 1;
            if (!courses || strcmp(courses, "x") == 0) {
                snprintf(line, BUFFER_SIZE, "%s:%s:%s", username, password, course_name);
            } else {
                char new_courses[BUFFER_SIZE];
                snprintf(new_courses, BUFFER_SIZE, "%s,%s", courses, course_name);
                snprintf(line, BUFFER_SIZE, "%s:%s:%s", username, password, new_courses);
            }
        }
        write(fd_temp, line, strlen(line));
        write(fd_temp, "\n", 1);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd_temp, F_SETLK, &lock);

    close(fd_orig);
    close(fd_temp);

    // Replace old file
    rename("faculties_tmp.txt", "faculties.txt");

    if (found) {
        char *msg = "Course added successfully.\n";
        write(sock, msg, strlen(msg));
    } else {
        char *msg = "Faculty not found.\n";
        write(sock, msg, strlen(msg));
    }
}

void enroll_course(int sock, const char *student_username) {
    char course_name[BUFFER_SIZE] = {0};
    read(sock, course_name, BUFFER_SIZE);

    // Check if course exists in faculties.txt
    int fd_fac = open("faculties.txt", O_RDONLY);
    if (fd_fac < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, found_course = 0;
    while (read(fd_fac, buffer, 1) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 2) {
            line[idx] = '\0';

            // Get course section (after 2nd colon)
            int colon_count = 0, i = 0;
            while (line[i] != '\0') {
                if (line[i] == ':') colon_count++;
                if (colon_count == 2) break;
                i++;
            }

            if (colon_count == 2) {
                char *courses = line + i + 1;
                if (strcmp(courses, "x") != 0) {
                    char *token = strtok(courses, ",");
                    while (token) {
                        if (strcmp(token, course_name) == 0) {
                            found_course = 1;
                            break;
                        }
                        token = strtok(NULL, ",");
                    }
                }
            }

            if (found_course) break;
            idx = 0;
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd_fac);

    if (!found_course) {
        char *msg = "No such course found.\n";
        write(sock, msg, strlen(msg));
        return;
    }

    // Update students.txt
    int fd_stu = open("students.txt", O_RDONLY);
    int fd_tmp = open("students_tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_stu < 0 || fd_tmp < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(fd_tmp, F_SETLKW, &lock);

    idx = 0;
    int updated = 0;
    while (read(fd_stu, buffer, 1) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 2) {
            line[idx] = '\0';

            char copy[BUFFER_SIZE];
            strcpy(copy, line);
            char *username = strtok(copy, ":");
            char *password = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");

            if (username && strcmp(username, student_username) == 0) {
                updated = 1;
                if (!courses || strcmp(courses, "x") == 0) {
                    snprintf(line, BUFFER_SIZE, "%s:%s:%s:1", username, password, course_name);
                } else {
                    char new_courses[BUFFER_SIZE];
                    snprintf(new_courses, BUFFER_SIZE, "%s,%s", courses, course_name);
                    snprintf(line, BUFFER_SIZE, "%s:%s:%s:1", username, password, new_courses);
                }
            }

            write(fd_tmp, line, strlen(line));
            write(fd_tmp, "\n", 1);
            idx = 0;
        } else {
            line[idx++] = buffer[0];
        }
    }

    // Handle last line
    if (idx > 0) {
        line[idx] = '\0';
        char copy[BUFFER_SIZE];
        strcpy(copy, line);
        char *username = strtok(copy, ":");
        char *password = strtok(NULL, ":");
        char *courses = strtok(NULL, ":");

        if (username && strcmp(username, student_username) == 0) {
            updated = 1;
            if (!courses || strcmp(courses, "x") == 0) {
                snprintf(line, BUFFER_SIZE, "%s:%s:%s:1", username, password, course_name);
            } else {
                char new_courses[BUFFER_SIZE];
                snprintf(new_courses, BUFFER_SIZE, "%s,%s", courses, course_name);
                snprintf(line, BUFFER_SIZE, "%s:%s:%s:1", username, password, new_courses);
            }
        }

        write(fd_tmp, line, strlen(line));
        write(fd_tmp, "\n", 1);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd_tmp, F_SETLK, &lock);

    close(fd_stu);
    close(fd_tmp);
    rename("students_tmp.txt", "students.txt");

    if (updated) {
        char *msg = "Enrolled successfully.\n";
        write(sock, msg, strlen(msg));
    } else {
        char *msg = "Student not found.\n";
        write(sock, msg, strlen(msg));
    }
}

void delete_course(int sock, const char* username_to_edit, const char* filename) {
    char course_code[BUFFER_SIZE] = {0};
    read(sock, course_code, BUFFER_SIZE);

    char msg[BUFFER_SIZE];
    int fd_orig = open(filename, O_RDONLY);
    char tmp_filename[BUFFER_SIZE];
    snprintf(tmp_filename, sizeof(tmp_filename), "%s_tmp.txt", filename);
    int fd_temp = open(tmp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_orig < 0 || fd_temp < 0) {
        char *err = "Error opening files\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(fd_temp, F_SETLKW, &lock);

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read;
    int found_user = 0, found_course = 0;

    while ((bytes_read = read(fd_orig, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 2) {
            line[idx] = '\0';

            char copy[BUFFER_SIZE];
            strcpy(copy, line);
            char *username = strtok(copy, ":");
            char *password = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");
            char *status = strtok(NULL, ":");

            if (username && strcmp(username, username_to_edit) == 0) {
                found_user = 1;

                char updated_courses[BUFFER_SIZE] = "";
                if (courses) {
                    char *token = strtok(courses, ",");
                    while (token) {
                        if (strcmp(token, course_code) != 0) {
                            if (strlen(updated_courses) > 0) strcat(updated_courses, ",");
                            strcat(updated_courses, token);
                        } else {
                            found_course = 1;
                        }
                        token = strtok(NULL, ",");
                    }
                }

                if (strlen(updated_courses) == 0) strcpy(updated_courses, "x");
                snprintf(line, BUFFER_SIZE, "%s:%s:%s:%s", username, password, updated_courses, status ? status : "activated");
            }

            write(fd_temp, line, strlen(line));
            write(fd_temp, "\n", 1);
            idx = 0;
        } else {
            line[idx++] = buffer[0];
        }
    }

    // Handle last line without newline
    if (idx > 0) {
        line[idx] = '\0';

        char copy[BUFFER_SIZE];
        strcpy(copy, line);
        char *username = strtok(copy, ":");
        char *password = strtok(NULL, ":");
        char *courses = strtok(NULL, ":");
        char *status = strtok(NULL, ":");

        if (username && strcmp(username, username_to_edit) == 0) {
            found_user = 1;

            char updated_courses[BUFFER_SIZE] = "";
            if (courses) {
                char *token = strtok(courses, ",");
                while (token) {
                    if (strcmp(token, course_code) != 0) {
                        if (strlen(updated_courses) > 0) strcat(updated_courses, ",");
                        strcat(updated_courses, token);
                    } else {
                        found_course = 1;
                    }
                    token = strtok(NULL, ",");
                }
            }

            if (strlen(updated_courses) == 0) strcpy(updated_courses, "x");
            snprintf(line, BUFFER_SIZE, "%s:%s:%s:%s", username, password, updated_courses, status ? status : "activated");
        }

        write(fd_temp, line, strlen(line));
        write(fd_temp, "\n", 1);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd_temp, F_SETLK, &lock);

    close(fd_orig);
    close(fd_temp);

    rename(tmp_filename, filename);

    if (!found_user) {
        snprintf(msg, sizeof(msg), "User '%s' not found.\n", username_to_edit);
    } else if (!found_course) {
        snprintf(msg, sizeof(msg), "Course '%s' not found for user.\n", course_code);
    } else {
        snprintf(msg, sizeof(msg), "Course '%s' deleted successfully.\n", course_code);
    }

    write(sock, msg, strlen(msg));
}

void view_student_details(int sock) {
    char username[BUFFER_SIZE] = {0};

    // Get student username
    read(sock, username, BUFFER_SIZE);
    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, username, strlen(username));
    write(STDOUT_FILENO, "\n", 1);

    // Open students.txt in read mode
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read, found = 0;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string
            idx = 0;

            // Extract username (before first colon)
            char temp_line[BUFFER_SIZE];
            strcpy(temp_line, line); // to preserve original
            char *token = strtok(temp_line, ":"); // username
            if (token && strcmp(token, username) == 0) {
                found = 1;

                // Skip username and password
                token = strtok(NULL, ":");
                token = strtok(NULL, ":");
                // Extract courses
                if (strcmp(token, "x") != 0){
                    write(sock, token, strlen(token));
                } else {
                    write(sock, "No courses found.\n", 18);
                }

                break;
            }
        } else {
            line[idx++] = buffer[0];
        }
    }

    if (!found) {
        char *err = "Student not found.\n";
        write(STDERR_FILENO, err, strlen(err));
    }

    close(fd);
}

void view_faculty_details(int sock){
    // Open faculties.txt in read mode
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read;
    char details[BUFFER_SIZE] = {0};

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string
            idx = 0;

            // Extract username (before first colon)
            char temp_line[BUFFER_SIZE];
            strcpy(temp_line, line); // to preserve original
            char *token = strtok(temp_line, ":"); // username

            sprintf(details, "Faculty: %s\n", token);
            // Skip password
            token = strtok(NULL, ":");
            // Extract courses
            token = strtok(NULL, ":");
            if (strcmp(token, "x") != 0){
                strcat(details, token);
                strcat(details, "\n");
            } else {
                strcat(details, "No courses found.\n");
            }
        } else {
            line[idx++] = buffer[0];
        }
    }

    if (strlen(details) > 0) {
        write(sock, details, strlen(details));
    } else {
        char *err = "No faculty details found.\n";
        write(STDERR_FILENO, err, strlen(err));
    }

    close(fd);
}

void change_password(int sock, char role, const char *username) {
    char new_password[BUFFER_SIZE] = {0};
    read(sock, new_password, BUFFER_SIZE);

    const char *file = (role == '2') ? "faculties.txt" :
                       (role == '3') ? "students.txt"  : NULL;

    if (!file) {
        char *msg = "Invalid role.\n";
        write(sock, msg, strlen(msg));
        return;
    }

    int fd = open(file, O_RDONLY);
    int tmp_fd = open("temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0 || tmp_fd < 0) {
        char *err = "Error opening file.\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(tmp_fd, F_SETLKW, &lock);

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, updated = 0;
    while (read(fd, buffer, 1) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 2) {
            line[idx] = '\0';

            char copy[BUFFER_SIZE];
            strcpy(copy, line);
            char *uname = strtok(copy, ":");
            char *old_pass = strtok(NULL, ":");
            char *rest = strtok(NULL, "\n");

            if (uname && strcmp(uname, username) == 0) {
                if (rest)
                    snprintf(line, BUFFER_SIZE, "%s:%s:%s", uname, new_password, rest);
                else
                    snprintf(line, BUFFER_SIZE, "%s:%s", uname, new_password);
                updated = 1;
            }

            write(tmp_fd, line, strlen(line));
            write(tmp_fd, "\n", 1);
            idx = 0;
        } else {
            line[idx++] = buffer[0];
        }
    }

    if (idx > 0) {
        line[idx] = '\0';
        char copy[BUFFER_SIZE];
        strcpy(copy, line);
        char *uname = strtok(copy, ":");
        char *old_pass = strtok(NULL, ":");
        char *rest = strtok(NULL, "\n");

        if (uname && strcmp(uname, username) == 0) {
            if (rest)
                snprintf(line, BUFFER_SIZE, "%s:%s:%s", uname, new_password, rest);
            else
                snprintf(line, BUFFER_SIZE, "%s:%s", uname, new_password);
            updated = 1;
        }

        write(tmp_fd, line, strlen(line));
        write(tmp_fd, "\n", 1);
    }

    lock.l_type = F_UNLCK;
    fcntl(tmp_fd, F_SETLK, &lock);
    close(fd);
    close(tmp_fd);
    rename("temp.txt", file);

    char *msg = updated ? "Password updated successfully.\n"
                        : "User not found.\n";
    write(sock, msg, strlen(msg));
}

int validate_student(const char *username, const char *password) {
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        write(STDERR_FILENO, "Error opening students.txt\n", 28);
        return 0; // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username and password
            char *token = strtok(line, ":");
            if (token && strcmp(token, username) == 0) {
                token = strtok(NULL, ":");
                if (token && strcmp(token, password) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

int validate_faculty(const char *username, const char *password) {
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        write(STDERR_FILENO, "Error opening faculties.txt\n", 28);
        return 0; // Assume doesn't exist if file can't be opened
    }

    char buffer[1];
    char line[BUFFER_SIZE];
    int idx = 0;
    int bytes_read;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate the string

            // Extract username and password
            char *token = strtok(line, ":");
            if (token && strcmp(token, username) == 0) {
                token = strtok(NULL, ":");
                if (token && strcmp(token, password) == 0) {
                    close(fd);
                    return 1; // found match
                }
            }

            idx = 0; // reset for next line
        } else {
            line[idx++] = buffer[0];
        }
    }

    close(fd);
    return 0; // not found
}

int run_admin_menu(int sock){
    // Receive task choice from client
    char task_choice;
    read(sock, &task_choice, sizeof(task_choice));
    printf("Received task choice: %c\n", task_choice);

    // Process task choice

    if (task_choice == '1') {
        add_student(sock);
    }
    else if (task_choice == '2') {
        view_student_details(sock);
    }
    else if (task_choice == '3') {
        add_faculty(sock);
    }
    else if (task_choice == '4') {
        write(STDOUT_FILENO, "Viewing faculty details...\n", 28);
    }
    else if (task_choice == '5') {
        write(STDOUT_FILENO, "Activating student...\n", 22);
    }
    else if (task_choice == '6') {
        write(STDOUT_FILENO, "Blocking student...\n", 21);
    }
    else if (task_choice == '7') {
        write(STDOUT_FILENO, "Modifying student details...\n", 30);
    }
    else if (task_choice == '8') {
        write(STDOUT_FILENO, "Modifying faculty details...\n", 30);
    }
    else if (task_choice == '9'){
        char *msg = "Logging out and exiting...\n";
        write(sock, msg, strlen(msg));
        close(sock);
        return 1;
    }
    else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }

    return 0;
}

int run_professor_menu(int sock, char role, const char *received_username) {
    char task_choice;
    read(sock, &task_choice, sizeof(task_choice));
    printf("Received task choice: %c\n", task_choice);

    // Process task choice
    if (task_choice == '1') {
        // View courses
        view_faculty_details(sock);
    } else if (task_choice == '2') {
        // Add course
        add_course(sock, received_username);
    } else if (task_choice == '3') {
        // Remove course
        delete_course(sock, received_username, "faculties.txt");
    } else if (task_choice == '4') {
        // View enrolled students
        write(STDOUT_FILENO, "Viewing enrolled students...\n", 30);
    } else if (task_choice == '5') {
        // Change password
        change_password(sock, role, received_username);
    } else if (task_choice == '6') {
        char *msg = "Logging out and exiting...\n";
        write(sock, msg, strlen(msg));
        close(sock);
        return 1;
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }

    return 0;
}

int run_student_menu(int sock, char role, const char *received_username) {
    char task_choice;
    read(sock, &task_choice, sizeof(task_choice));
    printf("Received task choice: %c\n", task_choice);

    // Process task choice
    if (task_choice == '1') {
        // View courses
        view_student_details(sock);
    } else if (task_choice == '2') {
        // Enroll in course
        enroll_course(sock, received_username);
    } else if (task_choice == '3') {
        // Drop course
        delete_course(sock, received_username, "students.txt");
    } else if (task_choice == '4') {
        // View enrolled courses
        write(STDOUT_FILENO, "Viewing enrolled courses...\n", 28);

    } else if (task_choice == '5') {
        // Change password
        change_password(sock, role, received_username);
    } else if (task_choice == '6'){
        char *msg = "Logging out and exiting...\n";
        write(sock, msg, strlen(msg));
        close(sock);
        return 1;
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }

    return 0;
}

int main() {
    int server_fd, sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully.\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    address.sin_port = htons(PORT);

    // Binding socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Server bound to port %d.\n", PORT);

    // Listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    // Accepting client connection
    sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (sock < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");

    // Receive role input from client
    char role_input;
    read(sock, &role_input, sizeof(role_input));
    printf("Received role input: %c\n", role_input);

    if (role_input == '1') {
        // Admin menu
        while(1) {
            int ret = run_admin_menu(sock);
            if (ret == 1) {
                close(sock);
                return 0; // Exit after admin menu
            }
        }
    } else if (role_input == '2') {
        // Professor menu

        // Receive username and password for validation
        char received_username[BUFFER_SIZE] = {0};
        char received_password[BUFFER_SIZE] = {0};

        read(sock, received_username, BUFFER_SIZE);
        read(sock, received_password, BUFFER_SIZE);

        write(STDOUT_FILENO, "Received username: ", 19);
        write(STDOUT_FILENO, received_username, strlen(received_username));
        write(STDOUT_FILENO, "\n", 1);
        write(STDOUT_FILENO, "Received password: ", 19);
        write(STDOUT_FILENO, received_password, strlen(received_password));
        write(STDOUT_FILENO, "\n", 1);

        if (!faculty_exists(received_username)){
            write(STDOUT_FILENO, "Faculty does not exist.\n", 24);
            write(sock, "Faculty not found.\n", 20);
            return 1;
        }

        if (validate_faculty(received_username, received_password)) {
            write(STDOUT_FILENO, "Faculty login successful.\n", 26);
            write(sock, "Faculty login successful.\n", 26);
        } else {
            write(STDOUT_FILENO, "Invalid credentials.\n", 21);
            write(sock, "Invalid credentials.\n", 21);
            return 1;
        }

        while(1) {
            int ret = run_professor_menu(sock, role_input, received_username);
            if (ret == 1) {
                close(sock);
                return 0; // Exit after professor menu
            }
        }
    } else if (role_input == '3') {
        // Student menu

        // Receive username and password for validation
        char received_username[BUFFER_SIZE] = {0};
        char received_password[BUFFER_SIZE] = {0};

        read(sock, received_username, BUFFER_SIZE);
        read(sock, received_password, BUFFER_SIZE);

        write(STDOUT_FILENO, "Received username: ", 19);
        write(STDOUT_FILENO, received_username, strlen(received_username));
        write(STDOUT_FILENO, "\n", 1);
        write(STDOUT_FILENO, "Received password: ", 19);
        write(STDOUT_FILENO, received_password, strlen(received_password));
        write(STDOUT_FILENO, "\n", 1);

        if (!student_exists(received_username)){
            write(STDOUT_FILENO, "Student does not exist.\n", 24);
            write(sock, "Student not found.\n", 20);
            return 1;
        }

        if (validate_student(received_username, received_password)) {
            write(STDOUT_FILENO, "Student login successful.\n", 26);
            write(sock, "Student login successful.\n", 26);
        } else {
            write(STDOUT_FILENO, "Invalid credentials.\n", 21);
            write(sock, "Invalid credentials.\n", 21);
            return 1;
        }

        while(1) {
            int ret = run_student_menu(sock, role_input, received_username);
            if (ret == 1) {
                close(sock);
                return 0; // Exit after student menu
            }
        }
    } else if (role_input == '9') {
        write(STDOUT_FILENO, "Client disconnected.\n", 23);
        close(sock);
        return 0; // Exit after client disconnect
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define ADMIN_USERNAME "admin"
#define ADMIN_PASSWORD "admin123"

// Structure to pass client data to threads
typedef struct {
    int sock;
    struct sockaddr_in address;
} client_data_t;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

int course_in_list(const char *courses, const char *course_name) {
    if (!courses || !course_name || strcmp(courses, "x") == 0) {
        return 0;
    }

    char courses_copy[BUFFER_SIZE];
    strncpy(courses_copy, courses, BUFFER_SIZE - 1);
    courses_copy[BUFFER_SIZE - 1] = '\0';

    char *token = strtok(courses_copy, ",");
    while (token) {
        if (strcmp(token, course_name) == 0) {
            return 1;
        }
        token = strtok(NULL, ",");
    }

    return 0;
}

int valid_field(const char *value) {
    return value && value[0] != '\0' && strchr(value, ':') == NULL && strchr(value, ',') == NULL;
}

int replace_course_code(const char *courses, const char *old_course, const char *new_course,
                        char *updated_courses, size_t updated_size) {
    int found = 0;
    updated_courses[0] = '\0';

    if (!courses || strcmp(courses, "x") == 0) {
        snprintf(updated_courses, updated_size, "%s", courses ? courses : "x");
        return 0;
    }

    char courses_copy[BUFFER_SIZE];
    strncpy(courses_copy, courses, BUFFER_SIZE - 1);
    courses_copy[BUFFER_SIZE - 1] = '\0';

    char *token = strtok(courses_copy, ",");
    while (token) {
        const char *value = token;
        if (strcmp(token, old_course) == 0) {
            value = new_course;
            found = 1;
        }

        if (!course_in_list(updated_courses, value)) {
            if (updated_courses[0] != '\0') {
                strncat(updated_courses, ",", updated_size - strlen(updated_courses) - 1);
            }
            strncat(updated_courses, value, updated_size - strlen(updated_courses) - 1);
        }

        token = strtok(NULL, ",");
    }

    if (updated_courses[0] == '\0') {
        snprintf(updated_courses, updated_size, "x");
    }

    return found;
}

int check_if_blocked(const char *username) {
    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening students.txt");
        return -1; // File error
    }

    char ch, line[BUFFER_SIZE];
    int idx = 0, bytes_read;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0';
            idx = 0;

            char line_copy[BUFFER_SIZE];
            strcpy(line_copy, line);

            char *token = strtok(line_copy, ":");
            if (token && strcmp(token, username) == 0) {
                // Skip password and courses
                token = strtok(NULL, ":"); // password
                token = strtok(NULL, ":"); // courses
                token = strtok(NULL, ":"); // active flag

                close(fd);
                if (token && strcmp(token, "0") == 0) {
                    return 1; // blocked
                } else {
                    return 0; // active
                }
            }
        } else {
            line[idx++] = ch;
        }
    }

    if (idx > 0) {
        line[idx] = '\0';

        char line_copy[BUFFER_SIZE];
        strcpy(line_copy, line);

        char *token = strtok(line_copy, ":");
        if (token && strcmp(token, username) == 0) {
            strtok(NULL, ":");
            strtok(NULL, ":");
            token = strtok(NULL, ":");

            close(fd);
            return (token && strcmp(token, "0") == 0) ? 1 : 0;
        }
    }

    close(fd);
    return -1; // username not found
}

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

    if (idx > 0) {
        line[idx] = '\0';
        char *colon_pos = strchr(line, ':');
        if (colon_pos != NULL) {
            *colon_pos = '\0';
            if (strcmp(line, received_username) == 0) {
                close(fd);
                return 1;
            }
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

    if (idx > 0) {
        line[idx] = '\0';
        char *colon_pos = strchr(line, ':');
        if (colon_pos != NULL) {
            *colon_pos = '\0';
            if (strcmp(line, received_username) == 0) {
                close(fd);
                return 1;
            }
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

    if (!valid_field(received_username) || !valid_field(received_password)) {
        char *err = "Invalid username or password.\n";
        write(sock, err, strlen(err));
        return;
    }

    pthread_mutex_lock(&file_mutex);

    // Check if username already exists in database
    if (student_exists(received_username)) {
        char *err = "Username already exists.\n";
        write(sock, err, strlen(err));
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    write(STDOUT_FILENO, "Received password: ", 19);
    write(STDOUT_FILENO, received_password, strlen(received_password));
    write(STDOUT_FILENO, "\n", 1);


    // Construct student entry
    char entry[BUFFER_SIZE];
    int entry_len = snprintf(entry, BUFFER_SIZE, "%s:%s:x:1\n", received_username, received_password);

    // Open students.txt in append mode, create if doesn't exist
    int fd = open("students.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        pthread_mutex_unlock(&file_mutex);
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
    pthread_mutex_unlock(&file_mutex);

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

    if (!valid_field(received_username) || !valid_field(received_password)) {
        char *err = "Invalid username or password.\n";
        write(sock, err, strlen(err));
        return;
    }

    pthread_mutex_lock(&file_mutex);

    // Check if username already exists in database
    if (faculty_exists(received_username)) {
        char *err = "Username already exists.\n";
        write(sock, err, strlen(err));
        pthread_mutex_unlock(&file_mutex);
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
        pthread_mutex_unlock(&file_mutex);
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
    pthread_mutex_unlock(&file_mutex);

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

    if (!valid_field(course_name)) {
        char *msg = "Invalid course code.\n";
        write(sock, msg, strlen(msg));
        return;
    }

    pthread_mutex_lock(&file_mutex);

    int fd_orig = open("faculties.txt", O_RDONLY);
    int fd_temp = open("faculties_tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_orig < 0 || fd_temp < 0) {
        char *err = "Error opening files\n";
        write(STDERR_FILENO, err, strlen(err));
        if (fd_orig >= 0) close(fd_orig);
        if (fd_temp >= 0) close(fd_temp);
        pthread_mutex_unlock(&file_mutex);
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
    int duplicate_course = 0;

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
                if (course_in_list(courses, course_name)) {
                    duplicate_course = 1;
                } else if (!courses || strcmp(courses, "x") == 0) {
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
            if (course_in_list(courses, course_name)) {
                duplicate_course = 1;
            } else if (!courses || strcmp(courses, "x") == 0) {
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
    pthread_mutex_unlock(&file_mutex);

    if (duplicate_course) {
        char *msg = "Course already exists.\n";
        write(sock, msg, strlen(msg));
    } else if (found) {
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

    // Print received course
    write(STDOUT_FILENO, "Received course code: ", 23);
    write(STDOUT_FILENO, course_name, strlen(course_name));
    write(STDOUT_FILENO, "\n", 1);

    if (!valid_field(course_name)) {
        char *msg = "Invalid course code.\n";
        write(sock, msg, strlen(msg));
        return;
    }

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
    pthread_mutex_lock(&file_mutex);

    int fd_stu = open("students.txt", O_RDONLY);
    int fd_tmp = open("students_tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_stu < 0 || fd_tmp < 0) {
        char *err = "Error opening students.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        if (fd_stu >= 0) close(fd_stu);
        if (fd_tmp >= 0) close(fd_tmp);
        pthread_mutex_unlock(&file_mutex);
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
    int already_enrolled = 0;
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
                if (course_in_list(courses, course_name)) {
                    already_enrolled = 1;
                } else if (!courses || strcmp(courses, "x") == 0) {
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
            if (course_in_list(courses, course_name)) {
                already_enrolled = 1;
            } else if (!courses || strcmp(courses, "x") == 0) {
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
    pthread_mutex_unlock(&file_mutex);

    if (already_enrolled) {
        char *msg = "Already enrolled in this course.\n";
        write(sock, msg, strlen(msg));
    } else if (updated) {
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

    if (!valid_field(course_code)) {
        char *err = "Invalid course code.\n";
        write(sock, err, strlen(err));
        return;
    }

    pthread_mutex_lock(&file_mutex);

    char msg[BUFFER_SIZE];
    int fd_orig = open(filename, O_RDONLY);
    char tmp_filename[BUFFER_SIZE];
    snprintf(tmp_filename, sizeof(tmp_filename), "%s_tmp.txt", filename);
    int fd_temp = open(tmp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_orig < 0 || fd_temp < 0) {
        char *err = "Error opening files\n";
        write(STDERR_FILENO, err, strlen(err));
        if (fd_orig >= 0) close(fd_orig);
        if (fd_temp >= 0) close(fd_temp);
        pthread_mutex_unlock(&file_mutex);
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
                snprintf(line, BUFFER_SIZE, "%s:%s:%s:%s", username, password, updated_courses, status ? status : "1");
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
            snprintf(line, BUFFER_SIZE, "%s:%s:%s:%s", username, password, updated_courses, status ? status : "1");
        }

        write(fd_temp, line, strlen(line));
        write(fd_temp, "\n", 1);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd_temp, F_SETLK, &lock);

    close(fd_orig);
    close(fd_temp);

    rename(tmp_filename, filename);
    pthread_mutex_unlock(&file_mutex);

    if (!found_user) {
        snprintf(msg, sizeof(msg), "User '%s' not found.\n", username_to_edit);
    } else if (!found_course) {
        snprintf(msg, sizeof(msg), "Course '%s' not found for user.\n", course_code);
    } else {
        snprintf(msg, sizeof(msg), "Course '%s' deleted successfully.\n", course_code);
    }

    write(sock, msg, strlen(msg));
}

void view_all_courses(int sock) {
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read;

    char output[BUFFER_SIZE * 10] = ""; // Buffer to collect all courses

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0'; // terminate line
            idx = 0;

            char temp_line[BUFFER_SIZE];
            strcpy(temp_line, line);

            // Format: username:password:course1,course2,...
            char *username = strtok(temp_line, ":");
            char *password = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");

            if (username && password && courses) {
                char *course = strtok(courses, ",");
                while (course) {
                    if (strcmp(course, "x") != 0) {
                        size_t used = strlen(output);
                        if (used < sizeof(output) - 1) {
                            snprintf(output + used, sizeof(output) - used, "Course: %s (Faculty: %s)\n", course, username);
                        }
                    }
                    course = strtok(NULL, ",");
                }
            }

        } else {
            line[idx++] = buffer[0];
        }
    }

    if (idx > 0) {
        line[idx] = '\0';
        char temp_line[BUFFER_SIZE];
        strcpy(temp_line, line);

        char *username = strtok(temp_line, ":");
        char *password = strtok(NULL, ":");
        char *courses = strtok(NULL, ":");

        if (username && password && courses) {
            char *course = strtok(courses, ",");
            while (course) {
                if (strcmp(course, "x") != 0) {
                    size_t used = strlen(output);
                    if (used < sizeof(output) - 1) {
                        snprintf(output + used, sizeof(output) - used, "Course: %s (Faculty: %s)\n", course, username);
                    }
                }
                course = strtok(NULL, ",");
            }
        }
    }

    if (strlen(output) == 0) {
        strcpy(output, "No courses available.\n");
    }

    write(sock, output, strlen(output));
    close(fd);
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
                if (token && strcmp(token, "x") != 0){
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

    if (!found && idx > 0) {
        line[idx] = '\0';
        char temp_line[BUFFER_SIZE];
        strcpy(temp_line, line);
        char *token = strtok(temp_line, ":");
        if (token && strcmp(token, username) == 0) {
            found = 1;
            strtok(NULL, ":");
            token = strtok(NULL, ":");
            if (token && strcmp(token, "x") != 0) {
                write(sock, token, strlen(token));
            } else {
                write(sock, "No courses found.\n", 18);
            }
        }
    }

    if (!found) {
        char *err = "Student not found.\n";
        write(STDERR_FILENO, err, strlen(err));
        write(sock, err, strlen(err));
    }

    close(fd);
}

void view_faculty_details(int sock) {
    char username[BUFFER_SIZE] = {0};

    // Read faculty username from socket
    read(sock, username, BUFFER_SIZE);
    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, username, strlen(username));
    write(STDOUT_FILENO, "\n", 1);

    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        char *err = "Error opening faculties.txt\n";
        write(STDERR_FILENO, err, strlen(err));
        return;
    }

    char buffer[1], line[BUFFER_SIZE];
    int idx = 0, bytes_read;
    int found = 0;

    while ((bytes_read = read(fd, buffer, 1)) > 0) {
        if (buffer[0] == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0';
            idx = 0;

            char temp_line[BUFFER_SIZE];
            strcpy(temp_line, line);
            char *token = strtok(temp_line, ":");  // username

            if (token && strcmp(token, username) == 0) {
                found = 1;
                char details[BUFFER_SIZE] = {0};
                sprintf(details, "Faculty: %s\n", token);

                token = strtok(NULL, ":");  // skip password
                token = strtok(NULL, ":");  // courses

                if (token && strcmp(token, "x") != 0) {
                    strcat(details, token);
                    strcat(details, "\n");
                } else {
                    strcat(details, "No courses found.\n");
                }

                write(sock, details, strlen(details));
                break;
            }
        } else {
            line[idx++] = buffer[0];
        }
    }

    if (!found && idx > 0) {
        line[idx] = '\0';

        char temp_line[BUFFER_SIZE];
        strcpy(temp_line, line);
        char *token = strtok(temp_line, ":");

        if (token && strcmp(token, username) == 0) {
            found = 1;
            char details[BUFFER_SIZE] = {0};
            snprintf(details, BUFFER_SIZE, "Faculty: %s\n", token);

            strtok(NULL, ":");
            token = strtok(NULL, ":");

            if (token && strcmp(token, "x") != 0) {
                strncat(details, token, BUFFER_SIZE - strlen(details) - 1);
                strncat(details, "\n", BUFFER_SIZE - strlen(details) - 1);
            } else {
                strncat(details, "No courses found.\n", BUFFER_SIZE - strlen(details) - 1);
            }

            write(sock, details, strlen(details));
        }
    }

    if (!found) {
        char *msg = "Faculty not found.\n";
        write(sock, msg, strlen(msg));
    }

    close(fd);
}

void update_course(int sock, const char* username) {
    char old_course[BUFFER_SIZE] = {0};
    char new_course[BUFFER_SIZE] = {0};

    // Read inputs from socket
    read(sock, old_course, BUFFER_SIZE);
    read(sock, new_course, BUFFER_SIZE);

    old_course[strcspn(old_course, "\n")] = '\0';
    new_course[strcspn(new_course, "\n")] = '\0';

    if (!valid_field(old_course) || !valid_field(new_course)) {
        write(sock, "Invalid course code.\n", 21);
        return;
    }

    pthread_mutex_lock(&file_mutex);

    // ----------- Update faculties.txt ----------
    int fd = open("faculties.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening faculties.txt");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int temp_fd = open("faculties_temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("Error creating faculties_temp.txt");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(temp_fd, F_SETLKW, &lock);

    char ch, line[BUFFER_SIZE];
    int idx = 0, bytes_read, faculty_found = 0, course_found = 0, duplicate_course = 0;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0';
            idx = 0;

            char original[BUFFER_SIZE];
            strcpy(original, line);

            char *u = strtok(line, ":");
            char *p = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");

            if (u && strcmp(u, username) == 0) {
                faculty_found = 1;
                char new_courses[BUFFER_SIZE] = {0};
                if (!course_in_list(courses, old_course)) {
                    dprintf(temp_fd, "%s\n", original);
                } else if (course_in_list(courses, new_course) && strcmp(old_course, new_course) != 0) {
                    duplicate_course = 1;
                    dprintf(temp_fd, "%s\n", original);
                } else {
                    course_found = replace_course_code(courses, old_course, new_course, new_courses, sizeof(new_courses));
                    dprintf(temp_fd, "%s:%s:%s\n", u, p, new_courses);
                }
            } else {
                dprintf(temp_fd, "%s\n", original);
            }
        } else {
            line[idx++] = ch;
        }
    }

    if (idx > 0) {
        line[idx] = '\0';

        char original[BUFFER_SIZE];
        strcpy(original, line);

        char *u = strtok(line, ":");
        char *p = strtok(NULL, ":");
        char *courses = strtok(NULL, ":");

        if (u && strcmp(u, username) == 0) {
            faculty_found = 1;
            char new_courses[BUFFER_SIZE] = {0};
            if (!course_in_list(courses, old_course)) {
                dprintf(temp_fd, "%s\n", original);
            } else if (course_in_list(courses, new_course) && strcmp(old_course, new_course) != 0) {
                duplicate_course = 1;
                dprintf(temp_fd, "%s\n", original);
            } else {
                course_found = replace_course_code(courses, old_course, new_course, new_courses, sizeof(new_courses));
                dprintf(temp_fd, "%s:%s:%s\n", u, p, new_courses);
            }
        } else {
            dprintf(temp_fd, "%s\n", original);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(temp_fd, F_SETLK, &lock);
    close(fd);
    close(temp_fd);

    if (faculty_found && course_found && !duplicate_course) {
        rename("faculties_temp.txt", "faculties.txt");
    } else {
        unlink("faculties_temp.txt");
        if (!faculty_found) {
            write(sock, "Faculty not found.\n", 20);
        } else if (duplicate_course) {
            write(sock, "Course already exists.\n", 23);
        } else {
            write(sock, "Course not found for faculty.\n", 30);
        }
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    // ----------- Update students.txt ----------
    fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening students.txt");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    temp_fd = open("students_temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("Error creating students_temp.txt");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(temp_fd, F_SETLKW, &lock);

    idx = 0;
    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0';
            idx = 0;

            char original[BUFFER_SIZE];
            strcpy(original, line);

            char *u = strtok(line, ":");
            char *p = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");
            char *flag = strtok(NULL, ":");

            if (u && p && courses && flag) {
                char new_courses[BUFFER_SIZE] = {0};
                replace_course_code(courses, old_course, new_course, new_courses, sizeof(new_courses));
                dprintf(temp_fd, "%s:%s:%s:%s\n", u, p, new_courses, flag);
            } else {
                dprintf(temp_fd, "%s\n", original);
            }
        } else {
            line[idx++] = ch;
        }
    }

    if (idx > 0) {
        line[idx] = '\0';

        char original[BUFFER_SIZE];
        strcpy(original, line);

        char *u = strtok(line, ":");
        char *p = strtok(NULL, ":");
        char *courses = strtok(NULL, ":");
        char *flag = strtok(NULL, ":");

        if (u && p && courses && flag) {
            char new_courses[BUFFER_SIZE] = {0};
            replace_course_code(courses, old_course, new_course, new_courses, sizeof(new_courses));
            dprintf(temp_fd, "%s:%s:%s:%s\n", u, p, new_courses, flag);
        } else {
            dprintf(temp_fd, "%s\n", original);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(temp_fd, F_SETLK, &lock);
    close(fd);
    close(temp_fd);
    rename("students_temp.txt", "students.txt");
    pthread_mutex_unlock(&file_mutex);

    // Final acknowledgment
    write(sock, "Course ID updated for faculty and students.\n", 45);
}

void change_password(int sock, char role, const char *username) {
    char new_password[BUFFER_SIZE] = {0};
    read(sock, new_password, BUFFER_SIZE);

    if (!valid_field(new_password)) {
        char *msg = "Invalid password.\n";
        write(sock, msg, strlen(msg));
        return;
    }

    const char *file = (role == '2') ? "faculties.txt" :
                       (role == '3') ? "students.txt"  : NULL;

    if (!file) {
        char *msg = "Invalid role.\n";
        write(sock, msg, strlen(msg));
        return;
    }

    pthread_mutex_lock(&file_mutex);

    int fd = open(file, O_RDONLY);
    int tmp_fd = open("temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0 || tmp_fd < 0) {
        char *err = "Error opening file.\n";
        write(STDERR_FILENO, err, strlen(err));
        if (fd >= 0) close(fd);
        if (tmp_fd >= 0) close(tmp_fd);
        pthread_mutex_unlock(&file_mutex);
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
            strtok(NULL, ":");
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
        strtok(NULL, ":");
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
    pthread_mutex_unlock(&file_mutex);

    char *msg = updated ? "Password updated successfully.\n"
                        : "User not found.\n";
    write(sock, msg, strlen(msg));
}

void activate(int sock, const char *username) {
    pthread_mutex_lock(&file_mutex);

    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening students.txt");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int temp_fd = open("students_temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("Error creating temporary file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, username, strlen(username));
    write(STDOUT_FILENO, "\n", 1);

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(temp_fd, F_SETLKW, &lock);

    char ch, line[BUFFER_SIZE];
    int idx = 0, bytes_read, found = 0;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0';
            idx = 0;

            char line_copy[BUFFER_SIZE];
            strcpy(line_copy, line);

            char *token = strtok(line, ":");
            if (token && strcmp(token, username) == 0) {
                found = 1;
                char *password = strtok(NULL, ":");
                char *courses = strtok(NULL, ":");

                // Write updated line to temp file with active flag = 1
                dprintf(temp_fd, "%s:%s:%s:1\n", username, password, courses);
            } else {
                // Write original line to temp file
                dprintf(temp_fd, "%s\n", line_copy);
            }
        } else {
            line[idx++] = ch;
        }
    }

    if (idx > 0) {
        line[idx] = '\0';

        char line_copy[BUFFER_SIZE];
        strcpy(line_copy, line);

        char *token = strtok(line, ":");
        if (token && strcmp(token, username) == 0) {
            found = 1;
            char *password = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");
            dprintf(temp_fd, "%s:%s:%s:1\n", username, password ? password : "", courses ? courses : "x");
        } else {
            dprintf(temp_fd, "%s\n", line_copy);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(temp_fd, F_SETLK, &lock);

    close(fd);
    close(temp_fd);

    if (found) {
        rename("students_temp.txt", "students.txt");
    } else {
        unlink("students_temp.txt");
        write(STDERR_FILENO, "Student not found.\n", 20);
    }

    pthread_mutex_unlock(&file_mutex);

    char *msg = found ? "Student activated successfully.\n"
                      : "Student not found.\n";
    write(sock, msg, strlen(msg));
}

void block(int sock, const char *username) {
    pthread_mutex_lock(&file_mutex);

    int fd = open("students.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening students.txt");
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    int temp_fd = open("students_temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("Error creating temporary file");
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        return;
    }

    write(STDOUT_FILENO, "Received username: ", 19);
    write(STDOUT_FILENO, username, strlen(username));
    write(STDOUT_FILENO, "\n", 1);

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    fcntl(temp_fd, F_SETLKW, &lock);

    char ch, line[BUFFER_SIZE];
    int idx = 0, bytes_read, found = 0;

    while ((bytes_read = read(fd, &ch, 1)) > 0) {
        if (ch == '\n' || idx >= BUFFER_SIZE - 1) {
            line[idx] = '\0';
            idx = 0;

            char line_copy[BUFFER_SIZE];
            strcpy(line_copy, line);

            char *token = strtok(line, ":");
            if (token && strcmp(token, username) == 0) {
                found = 1;
                char *password = strtok(NULL, ":");
                char *courses = strtok(NULL, ":");

                // Write updated line to temp file with active flag = 0
                dprintf(temp_fd, "%s:%s:%s:0\n", username, password, courses);
            } else {
                // Write original line to temp file
                dprintf(temp_fd, "%s\n", line_copy);
            }
        } else {
            line[idx++] = ch;
        }
    }

    if (idx > 0) {
        line[idx] = '\0';

        char line_copy[BUFFER_SIZE];
        strcpy(line_copy, line);

        char *token = strtok(line, ":");
        if (token && strcmp(token, username) == 0) {
            found = 1;
            char *password = strtok(NULL, ":");
            char *courses = strtok(NULL, ":");
            dprintf(temp_fd, "%s:%s:%s:0\n", username, password ? password : "", courses ? courses : "x");
        } else {
            dprintf(temp_fd, "%s\n", line_copy);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(temp_fd, F_SETLK, &lock);

    close(fd);
    close(temp_fd);

    if (found) {
        rename("students_temp.txt", "students.txt");
    } else {
        unlink("students_temp.txt");
        write(STDERR_FILENO, "Student not found.\n", 20);
    }

    pthread_mutex_unlock(&file_mutex);

    char *msg = found ? "Student blocked successfully.\n"
                      : "Student not found.\n";
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

    if (idx > 0) {
        line[idx] = '\0';
        char *token = strtok(line, ":");
        if (token && strcmp(token, username) == 0) {
            token = strtok(NULL, ":");
            if (token && strcmp(token, password) == 0) {
                close(fd);
                return 1;
            }
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

    if (idx > 0) {
        line[idx] = '\0';
        char *token = strtok(line, ":");
        if (token && strcmp(token, username) == 0) {
            token = strtok(NULL, ":");
            if (token && strcmp(token, password) == 0) {
                close(fd);
                return 1;
            }
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
        view_faculty_details(sock);
    }
    else if (task_choice == '5') {
        write(STDOUT_FILENO, "Activating student...\n", 22);
        char username[BUFFER_SIZE] = {0};
        read(sock, username, BUFFER_SIZE);
        activate(sock, username);
    }
    else if (task_choice == '6') {
        write(STDOUT_FILENO, "Blocking student...\n", 21);
        char username[BUFFER_SIZE] = {0};
        read(sock, username, BUFFER_SIZE);
        block(sock, username);
    }
    else if (task_choice == '7') {
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
        // Update course
        update_course(sock, received_username);
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
        view_all_courses(sock);
    } else if (task_choice == '2') {
        // Enroll in course
        enroll_course(sock, received_username);
    } else if (task_choice == '3') {
        // Drop course
        delete_course(sock, received_username, "students.txt");
    } else if (task_choice == '4') {
        // View enrolled courses
        view_student_details(sock);
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

void* handle_client(void* arg) {
    client_data_t* data = (client_data_t*)arg;
    int sock = data->sock;
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(data->address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("New connection from %s\n", client_ip);

    // Receive role input from client
    char role_input;
    read(sock, &role_input, sizeof(role_input));
    printf("Received role input: %c from %s\n", role_input, client_ip);

    if (role_input == '1') {
        // Admin menu
        char received_username[BUFFER_SIZE] = {0};
        char received_password[BUFFER_SIZE] = {0};
        read(sock, received_username, BUFFER_SIZE);
        read(sock, received_password, BUFFER_SIZE);

        if (strcmp(received_username, ADMIN_USERNAME) != 0 ||
            strcmp(received_password, ADMIN_PASSWORD) != 0) {
            write(sock, "Invalid credentials.\n", 21);
            close(sock);
            free(data);
            pthread_exit(NULL);
        }

        write(sock, "Admin login successful.\n", 24);
        write(STDOUT_FILENO, "Admin login successful.\n", 24);

        while(1) {
            int ret = run_admin_menu(sock);
            if (ret == 1) {
                break; // Exit after admin menu
            }
        }
    } else if (role_input == '2') {
        // Professor menu
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

        pthread_mutex_lock(&file_mutex);
        int faculty_exist = faculty_exists(received_username);
        int valid = validate_faculty(received_username, received_password);
        pthread_mutex_unlock(&file_mutex);

        if (!faculty_exist){
            write(STDOUT_FILENO, "Faculty does not exist.\n", 24);
            write(sock, "Faculty not found.\n", 20);
            close(sock);
            free(data);
            pthread_exit(NULL);
        }

        if (valid) {
            write(STDOUT_FILENO, "Faculty login successful.\n", 26);
            write(sock, "Faculty login successful.\n", 26);
        } else {
            write(STDOUT_FILENO, "Invalid credentials.\n", 21);
            write(sock, "Invalid credentials.\n", 21);
            close(sock);
            free(data);
            pthread_exit(NULL);
        }

        while(1) {
            int ret = run_professor_menu(sock, role_input, received_username);
            if (ret == 1) {
                break; // Exit after professor menu
            }
        }
    } else if (role_input == '3') {
        // Student menu
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

        pthread_mutex_lock(&file_mutex);
        int student_exist = student_exists(received_username);
        int valid = validate_student(received_username, received_password);
        int blocked = check_if_blocked(received_username);
        pthread_mutex_unlock(&file_mutex);

        if (!student_exist){
            write(STDOUT_FILENO, "Student does not exist.\n", 24);
            write(sock, "Student not found.\n", 20);
            close(sock);
            free(data);
            pthread_exit(NULL);
        }

        if (valid) {
            write(STDOUT_FILENO, "Student login successful.\n", 26);
            if (blocked) {
                write(STDOUT_FILENO, "Student is blocked.\n", 21);
                write(sock, "Student is blocked.\n", 21);
                close(sock);
                free(data);
                pthread_exit(NULL);
            }

            write(sock, "Student login successful.\n", 26);
        } else {
            write(STDOUT_FILENO, "Invalid credentials.\n", 21);
            write(sock, "Invalid credentials.\n", 21);
            close(sock);
            free(data);
            pthread_exit(NULL);
        }

        while(1) {
            int ret = run_student_menu(sock, role_input, received_username);
            if (ret == 1) {
                break; // Exit after student menu
            }
        }
    } else if (role_input == '9') {
        write(STDOUT_FILENO, "Client disconnected.\n", 23);
    } else {
        write(STDOUT_FILENO, "Invalid choice. Please try again.\n", 35);
    }

    close(sock);
    free(data);
    printf("Client %s disconnected\n", client_ip);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    printf("Server socket created successfully.\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Server bound to port %d.\n", PORT);

    // Listening for connections
    if (listen(server_fd, 10) < 0) {  // Increased backlog to 10
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accepting client connection
        client_data_t *client_data = malloc(sizeof(client_data_t));
        client_data->sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_data->sock < 0) {
            perror("accept failed");
            free(client_data);
            continue;
        }

        memcpy(&client_data->address, &address, sizeof(address));

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)client_data) != 0) {
            perror("Failed to create thread");
            close(client_data->sock);
            free(client_data);
            continue;
        }

        // Detach the thread so we don't need to join it
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}

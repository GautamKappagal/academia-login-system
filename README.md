# Course Registration Portal (Academia)

## 📌 Overview
This project is a **Course Registration Portal** implemented in C using **socket programming and system calls**.  
It simulates an academic system with three roles:
- Admin
- Faculty
- Student

The server maintains all data, and multiple clients can connect to perform operations.

---

## ⚙️ Features

### 👨‍💼 Admin
- Add student / faculty
- Activate / block student
- View details

### 👨‍🏫 Faculty
- Add / remove courses
- View offered courses
- Update course details
- Change password

### 👨‍🎓 Student
- Enroll in courses
- Drop courses
- View enrolled courses
- Change password

---

## 🧠 Concepts Used
- Socket Programming (TCP)
- File Handling using system calls (`open`, `read`, `write`)
- File Locking (`fcntl`)
- Concurrency (basic handling)
- Inter-process communication

---

## 📂 Data Storage
- `students.txt` → student credentials and enrolled courses  
- `faculties.txt` → faculty credentials and offered courses  

---

## 🚀 How to Run

### 1. Compile
```bash
gcc server.c -o server -lpthread
gcc client.c -o client
```

### 2. Run server
```bash
./server
```

### 3. Run client (in another terminal)
```bash
./client
```

---

## 🔑 Default Example Credentials
You can use sample entries from the text files, e.g.:

- Student:
  - username: gautam
  - password: gautamkappagal

- Faculty:
  - username: murali
  - password: murali123

---

## 📌 Notes
- Server runs on port **8080**
- Data is stored persistently in text files
- File locking ensures safe concurrent access

---

## 📷 Sample Output
The system provides CLI-based menus for Admin, Faculty, and Student roles.

---

## 👤 Author
Gautam Kappagal


# VITGo – Transport Booking System 🚆✈️
*A Console-Based Train & Flight Booking Application*

## 📌 Project Overview
VITGo is a robust C++ console-based transport booking system designed to simulate real-world ticketing for both trains and flights. It offers user registration, admin control, booking and cancellation features, seat management, and a simulated UPI-based payment system—all within a file-driven backend architecture.

## ✨ Core Features

### 🔐 User Roles
- **Admin**
  - Add/Delete transport schedules
  - View all bookings
  - Manage UPI ID and admin profile
- **User**
  - Register/Login
  - Book or cancel train/flight tickets
  - View personal booking history

### 🚆 Transport Management
- **Trains**
  - Classes: General, Sleeper, 3AC, 2AC, 1AC
- **Flights**
  - Classes: Economy, Business, First Class

### 🎫 Booking System
- Multi-passenger booking support
- Dynamic seat availability tracking
- Automated price calculation per class
- Simulated UPI payment confirmation

### 💾 File-Based Storage
All data is securely stored using text files:
```
📁 data/
├── users.txt          // User account info
├── bookings.txt       // Booking history
├── train_data.txt     // Train schedules
├── flight_data.txt    // Flight schedules
└── config.cfg         // Admin configurations
```

## ⚙️ Installation & Setup

### Prerequisites
- C++ Compiler (GCC / Clang / MSVC)
- OS: Windows, Linux, or macOS (Tested on Windows using g++)

### Compilation & Execution
```sh
# Clone the repository
git clone https://github.com/yourusername/VITGo-Booking-System.git
cd VITGo-Booking-System

# Compile the source code
g++ main.cpp -o VITGo

# Run the application
./VITGo      # On Windows: VITGo.exe
```

### 🛡 Default Admin Credentials
- **Email**: `admin@vitgo.com`
- **Password**: `admin123`

## 👤 Usage Guide

### ➤ For Users:
1. Register a new account
2. Log in and select between Train or Flight booking
3. Choose your route, class, and number of passengers
4. Confirm booking with UPI (simulated)
5. View or cancel bookings via "My Tickets"

### ➤ For Admins:
1. Log in using admin credentials
2. Add or delete transport schedules (train/flight)
3. View all bookings by users
4. Update profile information and UPI ID

## 🧑‍🤝‍🧑 Team Roles

| Member | Role | Responsibilities |
|--------|------|------------------|
| SUSMIT ROY | **Project Admin & Backend Developer** | Core logic, file handling, system integration |
| SHARANYA SINGHA ROY | **Transport System Specialist** | Transport class logic, seat management, pricing |
| UNNATI SHARMA | **UI/UX Developer** | Menu flows, input validation, output formatting |
| SRIJAN ROY | **Testing & Documentation Lead** | Testing, documentation, bug tracking, user guides |

## 🚧 Planned Future Enhancements
- 🔹 Graphical User Interface (GUI) using **Qt** or **SFML**
- 🔹 Integration with **SQLite/MySQL** for persistent database storage
- 🔹 Email/SMS booking notifications
- 🔹 Refund system for ticket cancellations

> *Note: This is a console-based application. A full GUI version can be developed using additional frameworks.*

## 📁 Repository & License
- **GitHub**: (https://github.com/sus130/VITGo---Online-Ticket-Booking-System)
- **Developed by**: Kryptonix
- **License**: MIT (Open Source)

## 🚀 Happy Booking with VITGo! ✈️🚆

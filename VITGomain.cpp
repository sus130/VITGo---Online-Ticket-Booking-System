#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <conio.h>
#include <regex>
#include <sys/stat.h>
#include <direct.h>

using namespace std;

// Constants and Configuration
const string DATA_DIR = "data";
const string USERS_FILE = DATA_DIR + "/users.txt";
const string BOOKINGS_FILE = DATA_DIR + "/bookings.txt";
const string TRAIN_DATA_FILE = DATA_DIR + "/train_data.txt";
const string FLIGHT_DATA_FILE = DATA_DIR + "/flight_data.txt";
const string CONFIG_FILE = DATA_DIR + "/config.cfg";

const string DEFAULT_ADMIN_EMAIL = "roysus@vitgo.ac.in";
const string DEFAULT_ADMIN_NAME = "SUSMIT ROY";
const string DEFAULT_ADMIN_PASSWORD = "roysus@1906";
const string DEFAULT_UPI_ID = "7872434869@ybl";

// Data Structures
struct User {
    string name;
    string email;
    string contact;
    string password;
    bool isAdmin = false;
};

struct Booking {
    int bookingId;
    string userEmail;
    string transportType;
    string from;
    string to;
    int transportNumber;
    string travelClass;
    float price;
    vector<string> passengerNames;
};

struct Transport {
    int number;
    string from;
    string to;
    vector<string> classes;
    vector<int> seats;
    vector<float> prices;
};

// Utility Functions
void createDataDirectory() {
    #ifdef _WIN32
    _mkdir(DATA_DIR.c_str());
    #else
    mkdir(DATA_DIR.c_str(), 0777);
    #endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pressAnyKeyToContinue() {
    cout << "\nPress any key to continue...";
    _getch();
    clearScreen();
}

string getMaskedInput(const string& prompt) {
    string input;
    cout << prompt;

    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b' && !input.empty()) {
            cout << "\b \b";
            input.pop_back();
        } else if (ch != '\b') {
            input.push_back(ch);
            cout << '*';
        }
    }
    cout << endl;
    return input;
}

string toLower(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

bool isValidEmail(const string& email) {
    const regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return regex_match(email, pattern);
}

bool isValidPhone(const string& phone) {
    const regex pattern(R"(\d{10,15})");
    return regex_match(phone, pattern);
}

vector<string> readFileLines(const string& filename) {
    vector<string> lines;
    ifstream file(filename);
    if (file) {
        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
    }
    return lines;
}

bool writeFileLines(const string& filename, const vector<string>& lines) {
    ofstream file(filename);
    if (!file) return false;
    
    for (const auto& line : lines) {
        file << line << endl;
    }
    return true;
}

// Data Management
class DataManager {
private:
    string adminEmail;
    string adminName;
    string adminPassword;
    string upiId;
    int nextBookingId = 1;

    void loadConfig() {
        ifstream configFile(CONFIG_FILE);
        if (configFile) {
            getline(configFile, adminEmail);
            getline(configFile, adminName);
            getline(configFile, adminPassword);
            getline(configFile, upiId);
            
            string bookingIdStr;
            if (getline(configFile, bookingIdStr)) {
                nextBookingId = stoi(bookingIdStr);
            }
        } else {
            adminEmail = DEFAULT_ADMIN_EMAIL;
            adminName = DEFAULT_ADMIN_NAME;
            adminPassword = DEFAULT_ADMIN_PASSWORD;
            upiId = DEFAULT_UPI_ID;
            saveConfig();
        }
    }

public:
    DataManager() {
        createDataDirectory();
        loadConfig();
    }

    void saveConfig() {
        ofstream configFile(CONFIG_FILE);
        if (configFile) {
            configFile << adminEmail << endl;
            configFile << adminName << endl;
            configFile << adminPassword << endl;
            configFile << upiId << endl;
            configFile << nextBookingId << endl;
        }
    }

    int getNextBookingId() { return nextBookingId++; }
    string getUpiId() const { return upiId; }
    string getAdminEmail() const { return adminEmail; }
    string getAdminName() const { return adminName; }
    string getAdminPassword() const { return adminPassword; }

    void setAdminCredentials(const string& email, const string& name, const string& password) {
        adminEmail = email;
        adminName = name;
        adminPassword = password;
        saveConfig();
    }

    void setUpiId(const string& id) {
        upiId = id;
        saveConfig();
    }
};

// User Management
class UserManager {
private:
    DataManager& dataManager;

    User parseUser(const string& line) {
        istringstream iss(line);
        User user;
        string temp;
        vector<string> tokens;

        while (iss >> temp) {
            tokens.push_back(temp);
        }

        if (tokens.size() < 4) return user;

        int adminFlagPos = tokens.size() - 1;
        if (tokens[adminFlagPos] == "1") {
            user.isAdmin = true;
            adminFlagPos--;
        }

        user.password = tokens[adminFlagPos--];
        user.contact = tokens[adminFlagPos--];
        user.email = tokens[adminFlagPos--];

        user.name = tokens[0];
        for (size_t i = 1; i <= adminFlagPos; i++) {
            user.name += " " + tokens[i];
        }

        return user;
    }

    string serializeUser(const User& user) {
        ostringstream oss;
        oss << user.name << " " << user.email << " " << user.contact << " " << user.password;
        if (user.isAdmin) {
            oss << " 1";
        }
        return oss.str();
    }

public:
    UserManager(DataManager& dm) : dataManager(dm) {}

    bool emailExists(const string& email) {
        auto lines = readFileLines(USERS_FILE);
        for (const auto& line : lines) {
            User user = parseUser(line);
            if (toLower(user.email) == toLower(email)) {
                return true;
            }
        }
        return false;
    }

    bool registerUser(const User& newUser) {
        if (emailExists(newUser.email)) {
            return false;
        }

        ofstream file(USERS_FILE, ios::app);
        if (!file) return false;

        file << serializeUser(newUser) << endl;
        return true;
    }

    User* authenticateUser(const string& email, const string& password) {
        auto lines = readFileLines(USERS_FILE);
        for (const auto& line : lines) {
            User user = parseUser(line);
            if (toLower(user.email) == toLower(email)) {
                if (user.password == password) {
                    return new User(user);
                }
                return nullptr;
            }
        }
        return nullptr;
    }

    User* authenticateAdmin(const string& email, const string& password) {
        if (toLower(email) == toLower(dataManager.getAdminEmail()) && 
            password == dataManager.getAdminPassword()) {
            User* admin = new User();
            admin->name = dataManager.getAdminName();
            admin->email = dataManager.getAdminEmail();
            admin->isAdmin = true;
            return admin;
        }
        return nullptr;
    }

    void recoverPassword(const string& email) {
        auto lines = readFileLines(USERS_FILE);
        for (const auto& line : lines) {
            User user = parseUser(line);
            if (toLower(user.email) == toLower(email)) {
                cout << "\nAccount found!\n";
                cout << "Your password is: " << user.password << "\n";
                return;
            }
        }
        cout << "\nNo account found with that email.\n";
    }
};

// Transport Management
class TransportManager {
public:
    Transport parseTransport(const string& line, bool isTrain) {
        istringstream iss(line);
        Transport transport;

        iss >> transport.number >> transport.from >> transport.to;

        if (isTrain) {
            transport.classes = {"General", "Sleeper", "3rdAC", "2ndAC", "1stAC"};
        } else {
            transport.classes = {"Economy", "Business", "FirstClass"};
        }

        transport.seats.clear();
        transport.prices.clear();

        for (size_t i = 0; i < transport.classes.size(); i++) {
            int seats;
            float price;
            if (!(iss >> seats >> price)) {
                seats = 0;
                price = 0.0f;
            }
            transport.seats.push_back(seats);
            transport.prices.push_back(price);
        }

        return transport;
    }

    string serializeTransport(const Transport& transport) {
        ostringstream oss;
        oss << transport.number << " " << transport.from << " " << transport.to << " ";

        for (size_t i = 0; i < transport.classes.size(); i++) {
            oss << transport.seats[i] << " " << transport.prices[i] << " ";
        }

        return oss.str();
    }

    vector<Transport> getTransports(const string& filename, bool isTrain) {
        vector<Transport> transports;
        auto lines = readFileLines(filename);
        for (const auto& line : lines) {
            transports.push_back(parseTransport(line, isTrain));
        }
        return transports;
    }

    bool addTransport(const string& filename, const Transport& transport, bool isTrain) {
        ofstream file(filename, ios::app);
        if (!file) return false;
        file << serializeTransport(transport) << endl;
        return true;
    }

    bool updateTransport(const string& filename, const Transport& updatedTransport, bool isTrain) {
        auto transports = getTransports(filename, isTrain);
        bool found = false;

        for (auto& transport : transports) {
            if (transport.number == updatedTransport.number && 
                transport.from == updatedTransport.from && 
                transport.to == updatedTransport.to) {
                transport = updatedTransport;
                found = true;
                break;
            }
        }

        if (!found) {
            transports.push_back(updatedTransport);
        }

        vector<string> lines;
        for (const auto& transport : transports) {
            lines.push_back(serializeTransport(transport));
        }

        return writeFileLines(filename, lines);
    }

    bool deleteTransport(const string& filename, int transportNumber, bool isTrain) {
        auto transports = getTransports(filename, isTrain);
        auto it = remove_if(transports.begin(), transports.end(),
            [transportNumber](const Transport& t) {
                return t.number == transportNumber;
            });

        if (it == transports.end()) {
            return false;
        }

        transports.erase(it, transports.end());

        vector<string> lines;
        for (const auto& transport : transports) {
            lines.push_back(serializeTransport(transport));
        }

        return writeFileLines(filename, lines);
    }
};

// Booking Management
class BookingManager {
private:
    DataManager& dataManager;
    TransportManager& transportManager;

    Booking parseBooking(const string& line) {
        istringstream iss(line);
        Booking booking;

        iss >> booking.bookingId >> booking.userEmail >> booking.transportType 
            >> booking.from >> booking.to >> booking.transportNumber 
            >> booking.travelClass >> booking.price;

        string passenger;
        while (iss >> passenger) {
            replace(passenger.begin(), passenger.end(), '_', ' ');
            booking.passengerNames.push_back(passenger);
        }

        return booking;
    }

    string serializeBooking(const Booking& booking) {
        ostringstream oss;
        oss << booking.bookingId << " " << booking.userEmail << " " 
            << booking.transportType << " " << booking.from << " " << booking.to << " "
            << booking.transportNumber << " " << booking.travelClass << " "
            << booking.price;

        for (const auto& name : booking.passengerNames) {
            string storageName = name;
            replace(storageName.begin(), storageName.end(), ' ', '_');
            oss << " " << storageName;
        }

        return oss.str();
    }

public:
    BookingManager(DataManager& dm, TransportManager& tm) 
        : dataManager(dm), transportManager(tm) {}

    vector<Booking> getUserBookings(const string& userEmail) {
        vector<Booking> userBookings;
        auto lines = readFileLines(BOOKINGS_FILE);
        for (const auto& line : lines) {
            Booking booking = parseBooking(line);
            if (toLower(booking.userEmail) == toLower(userEmail)) {
                userBookings.push_back(booking);
            }
        }
        return userBookings;
    }

    bool createBooking(const Booking& booking) {
        ofstream file(BOOKINGS_FILE, ios::app);
        if (!file) return false;
        file << serializeBooking(booking) << endl;
        return true;
    }

    bool cancelBooking(int bookingId, const string& userEmail) {
        auto bookings = getAllBookings();
        bool found = false;

        for (auto it = bookings.begin(); it != bookings.end(); ) {
            if (it->bookingId == bookingId && toLower(it->userEmail) == toLower(userEmail)) {
                updateSeatAvailability(it->transportType == "Train" ? TRAIN_DATA_FILE : FLIGHT_DATA_FILE,
                                     it->transportNumber, it->from, it->to, it->travelClass, 1);
                it = bookings.erase(it);
                found = true;
            } else {
                ++it;
            }
        }

        if (!found) return false;

        vector<string> lines;
        for (const auto& booking : bookings) {
            lines.push_back(serializeBooking(booking));
        }

        return writeFileLines(BOOKINGS_FILE, lines);
    }

    vector<Booking> getAllBookings() {
        vector<Booking> allBookings;
        auto lines = readFileLines(BOOKINGS_FILE);
        for (const auto& line : lines) {
            allBookings.push_back(parseBooking(line));
        }
        return allBookings;
    }

    bool updateSeatAvailability(const string& filename, int transportNumber, 
                               const string& from, const string& to, 
                               const string& cls, int seatChange) {
        bool isTrain = (filename == TRAIN_DATA_FILE);
        auto transports = transportManager.getTransports(filename, isTrain);
        bool found = false;

        for (auto& transport : transports) {
            if (transport.number == transportNumber && 
                transport.from == from && 
                transport.to == to) {
                for (size_t i = 0; i < transport.classes.size(); i++) {
                    if (transport.classes[i] == cls) {
                        transport.seats[i] += seatChange;
                        found = true;
                        break;
                    }
                }
                break;
            }
        }

        if (!found) return false;

        vector<string> lines;
        for (const auto& transport : transports) {
            lines.push_back(transportManager.serializeTransport(transport));
        }

        return writeFileLines(filename, lines);
    }
};

// UI Components
class UI {
private:
    DataManager dataManager;
    UserManager userManager;
    TransportManager transportManager;
    BookingManager bookingManager;

    void displayHeader(const string& title) {
        clearScreen();
        cout << "\n========================================\n";
        cout << "       " << title << "\n";
        cout << "========================================\n";
    }

    void displayTransport(const Transport& transport, bool isTrain) {
        cout << (isTrain ? "Train" : "Flight") << " Number: " << transport.number << "\n";
        cout << "Route: " << transport.from << " -> " << transport.to << "\n";
        cout << "Classes and Availability:\n";
        for (size_t i = 0; i < transport.classes.size(); i++) {
            cout << "  " << (i+1) << ". " << transport.classes[i] << ": Rs " 
                 << transport.prices[i] << " (" << transport.seats[i] << " seats)\n";
        }
        cout << "----------------------------------------\n";
    }

    void displayBooking(const Booking& booking) {
        cout << "\nBooking ID: " << booking.bookingId << "\n";
        cout << booking.transportType << " " << booking.transportNumber << ": " 
             << booking.from << " -> " << booking.to << "\n";
        cout << "Class: " << booking.travelClass << ", Price: Rs " << booking.price << "\n";
        cout << "Passengers: ";
        for (const auto& name : booking.passengerNames) {
            cout << name << ", ";
        }
        cout << "\n----------------------------------------\n";
    }

public:
    UI() : userManager(dataManager), bookingManager(dataManager, transportManager) {}

    void showMainMenu() {
        int choice;
        do {
            displayHeader("WELCOME TO VITGo BOOKING SYSTEM");
            cout << "1. Register\n";
            cout << "2. Login as User\n";
            cout << "3. Login as Admin\n";
            cout << "4. Password Recovery\n";
            cout << "5. Exit\n";
            cout << "========================================\n";
            cout << "Enter your choice: ";
            
            while (!(cin >> choice) || choice < 1 || choice > 5) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid choice. Please enter 1-5: ";
            }

            clearScreen();
            switch (choice) {
                case 1: registerUser(); break;
                case 2: loginUser(); break;
                case 3: loginAdmin(); break;
                case 4: recoverPassword(); break;
                case 5: cout << "Thank you for using VITGo. Goodbye!\n"; break;
            }
        } while (choice != 5);
    }

    void registerUser() {
        displayHeader("USER REGISTRATION");
        
        User newUser;
        cout << "Enter Full Name: ";
        getline(cin >> ws, newUser.name);
        
        cout << "Enter Email: ";
        cin >> newUser.email;
        if (!isValidEmail(newUser.email)) {
            cout << "Invalid email format.\n";
            pressAnyKeyToContinue();
            return;
        }

        if (userManager.emailExists(newUser.email)) {
            cout << "Email already registered. Please use a different email.\n";
            pressAnyKeyToContinue();
            return;
        }

        cout << "Enter Contact: ";
        cin >> newUser.contact;
        if (!isValidPhone(newUser.contact)) {
            cout << "Invalid phone number format.\n";
            pressAnyKeyToContinue();
            return;
        }

        newUser.password = getMaskedInput("Enter Password: ");

        if (userManager.registerUser(newUser)) {
            cout << "\nRegistration successful, " << newUser.name << "!\n";
        } else {
            cout << "\nRegistration failed. Please try again.\n";
        }
        pressAnyKeyToContinue();
    }

    void loginUser() {
        displayHeader("USER LOGIN");
        
        string email, password;
        cout << "Enter Email: ";
        cin >> email;
        password = getMaskedInput("Enter Password: ");

        User* user = userManager.authenticateUser(email, password);
        if (user) {
            cout << "\nLogin successful! Welcome, " << user->name << "!\n";
            pressAnyKeyToContinue();
            showUserMenu(*user);
            delete user;
        } else {
            cout << "\nLogin failed. Please check your credentials.\n";
            cout << "1. Try again\n";
            cout << "2. Recover password\n";
            cout << "3. Back to main menu\n";
            cout << "Choice: ";
            
            int choice;
            cin >> choice;
            switch (choice) {
                case 1: loginUser(); break;
                case 2: recoverPassword(); break;
                default: break;
            }
        }
    }

    void loginAdmin() {
        displayHeader("ADMIN LOGIN");
        
        string email, password;
        cout << "Enter Email: ";
        cin >> email;
        password = getMaskedInput("Enter Password: ");

        User* admin = userManager.authenticateAdmin(email, password);
        if (admin) {
            cout << "\nAdmin login successful!\n";
            pressAnyKeyToContinue();
            showAdminMenu(*admin);
            delete admin;
        } else {
            cout << "Admin login failed. Invalid credentials.\n";
            pressAnyKeyToContinue();
        }
    }

    void recoverPassword() {
        displayHeader("PASSWORD RECOVERY");
        string email;
        cout << "Enter your registered email: ";
        cin >> email;
        
        userManager.recoverPassword(email);
        pressAnyKeyToContinue();
    }

    void showUserMenu(User& user) {
        int choice;
        do {
            displayHeader("WELCOME, " + user.name);
            cout << "1. Book Train Ticket\n";
            cout << "2. Book Flight Ticket\n";
            cout << "3. View Upcoming Tickets\n";
            cout << "4. Cancel Ticket\n";
            cout << "5. Logout\n";
            cout << "========================================\n";
            cout << "Enter your choice: ";
            
            while (!(cin >> choice) || choice < 1 || choice > 5) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid choice. Please enter 1-5: ";
            }

            clearScreen();
            switch (choice) {
                case 1: bookTransport(true, user); break;
                case 2: bookTransport(false, user); break;
                case 3: viewUserBookings(user.email); break;
                case 4: cancelUserBooking(user.email); break;
                case 5: cout << "Logging out...\n"; break;
            }
        } while (choice != 5);
    }

    void bookTransport(bool isTrain, User& user) {
        displayHeader(isTrain ? "BOOK TRAIN TICKET" : "BOOK FLIGHT TICKET");
        
        auto transports = transportManager.getTransports(
            isTrain ? TRAIN_DATA_FILE : FLIGHT_DATA_FILE, isTrain);

        if (transports.empty()) {
            cout << "No available " << (isTrain ? "trains" : "flights") << " found.\n";
            pressAnyKeyToContinue();
            return;
        }

        cout << "Available " << (isTrain ? "Trains" : "Flights") << ":\n";
        for (const auto& transport : transports) {
            displayTransport(transport, isTrain);
        }

        int transportNumber;
        cout << "\nEnter " << (isTrain ? "Train" : "Flight") << " Number: ";
        while (!(cin >> transportNumber)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number: ";
        }

        Transport selectedTransport;
        bool found = false;
        for (const auto& transport : transports) {
            if (transport.number == transportNumber) {
                selectedTransport = transport;
                found = true;
                break;
            }
        }

        if (!found) {
            cout << "Invalid " << (isTrain ? "train" : "flight") << " number.\n";
            pressAnyKeyToContinue();
            return;
        }

        cout << "\nAvailable Classes:\n";
        for (size_t i = 0; i < selectedTransport.classes.size(); i++) {
            cout << i + 1 << ". " << selectedTransport.classes[i] << " - Rs " 
                 << selectedTransport.prices[i] << " (Seats: " 
                 << selectedTransport.seats[i] << ")\n";
        }

        int classChoice;
        cout << "\nChoose class (1-" << selectedTransport.classes.size() << "): ";
        while (!(cin >> classChoice) || classChoice < 1 || classChoice > selectedTransport.classes.size()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid choice. Please enter between 1-" << selectedTransport.classes.size() << ": ";
        }

        string selectedClass = selectedTransport.classes[classChoice - 1];
        float price = selectedTransport.prices[classChoice - 1];
        int availableSeats = selectedTransport.seats[classChoice - 1];

        if (availableSeats <= 0) {
            cout << "Sorry, no seats available in this class.\n";
            pressAnyKeyToContinue();
            return;
        }

        int passengerCount;
        cout << "\nNumber of passengers (max " << availableSeats << "): ";
        while (!(cin >> passengerCount) || passengerCount < 1 || passengerCount > availableSeats) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid number. Please enter between 1-" << availableSeats << ": ";
        }

        vector<string> passengerNames;
        cin.ignore();
        for (int i = 0; i < passengerCount; i++) {
            string name;
            cout << "Passenger " << i + 1 << " Name: ";
            getline(cin, name);
            passengerNames.push_back(name);
        }

        float total = price * passengerCount;
        cout << "\nTotal Amount: Rs " << total << endl;
        cout << "Pay to UPI ID: " << dataManager.getUpiId() << endl;
        cout << "Enter 'confirm' to proceed with payment: ";
        string confirm;
        cin >> confirm;
        if (toLower(confirm) != "confirm") {
            cout << "Payment not confirmed. Booking canceled.\n";
            pressAnyKeyToContinue();
            return;
        }

        Booking newBooking;
        newBooking.bookingId = dataManager.getNextBookingId();
        newBooking.userEmail = user.email;
        newBooking.transportType = isTrain ? "Train" : "Flight";
        newBooking.from = selectedTransport.from;
        newBooking.to = selectedTransport.to;
        newBooking.transportNumber = selectedTransport.number;
        newBooking.travelClass = selectedClass;
        newBooking.price = total;
        newBooking.passengerNames = passengerNames;

        if (bookingManager.createBooking(newBooking)) {
            bookingManager.updateSeatAvailability(
                isTrain ? TRAIN_DATA_FILE : FLIGHT_DATA_FILE,
                selectedTransport.number, selectedTransport.from, 
                selectedTransport.to, selectedClass, -passengerCount);

            cout << "\n========================================\n";
            cout << "          TICKET CONFIRMED\n";
            cout << "========================================\n";
            displayBooking(newBooking);
            cout << "========================================\n";
        } else {
            cout << "Booking failed. Please try again.\n";
        }
        pressAnyKeyToContinue();
    }

    void viewUserBookings(const string& userEmail) {
        displayHeader("YOUR UPCOMING TICKETS");
        
        auto bookings = bookingManager.getUserBookings(userEmail);
        if (bookings.empty()) {
            cout << "No upcoming tickets found.\n";
        } else {
            for (const auto& booking : bookings) {
                displayBooking(booking);
            }
        }
        pressAnyKeyToContinue();
    }

    void cancelUserBooking(const string& userEmail) {
        displayHeader("CANCEL BOOKING");
        
        auto bookings = bookingManager.getUserBookings(userEmail);
        if (bookings.empty()) {
            cout << "You have no bookings to cancel.\n";
            pressAnyKeyToContinue();
            return;
        }

        cout << "Your Bookings:\n";
        for (const auto& booking : bookings) {
            displayBooking(booking);
        }

        int cancelId;
        cout << "Enter Booking ID to cancel (0 to go back): ";
        while (!(cin >> cancelId)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number: ";
        }

        if (cancelId == 0) {
            return;
        }

        if (bookingManager.cancelBooking(cancelId, userEmail)) {
            cout << "Booking canceled successfully.\n";
        } else {
            cout << "Failed to cancel booking or booking not found.\n";
        }
        pressAnyKeyToContinue();
    }

    void showAdminMenu(User& admin) {
        int choice;
        do {
            displayHeader("ADMIN DASHBOARD - " + admin.name);
            cout << "1. Add Train Schedule\n";
            cout << "2. Add Flight Schedule\n";
            cout << "3. View Train Schedules\n";
            cout << "4. View Flight Schedules\n";
            cout << "5. Delete Train Schedule\n";
            cout << "6. Delete Flight Schedule\n";
            cout << "7. View All Bookings\n";
            cout << "8. Update Admin Profile\n";
            cout << "9. Logout\n";
            cout << "========================================\n";
            cout << "Enter choice: ";
            
            while (!(cin >> choice) || choice < 1 || choice > 9) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid choice. Please enter 1-9: ";
            }

            clearScreen();
            switch (choice) {
                case 1: addTransport(true); break;
                case 2: addTransport(false); break;
                case 3: viewTransports(true); break;
                case 4: viewTransports(false); break;
                case 5: deleteTransport(true); break;
                case 6: deleteTransport(false); break;
                case 7: viewAllBookings(); break;
                case 8: updateAdminProfile(); break;
                case 9: cout << "Logging out...\n"; break;
            }
        } while (choice != 9);
    }

    void addTransport(bool isTrain) {
        displayHeader(isTrain ? "ADD TRAIN SCHEDULE" : "ADD FLIGHT SCHEDULE");
        
        Transport newTransport;
        cout << "Enter " << (isTrain ? "Train" : "Flight") << " Number: ";
        while (!(cin >> newTransport.number)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number: ";
        }

        cout << "From: ";
        cin >> newTransport.from;
        cout << "To: ";
        cin >> newTransport.to;

        if (isTrain) {
            newTransport.classes = {"General", "Sleeper", "3rdAC", "2ndAC", "1stAC"};
        } else {
            newTransport.classes = {"Economy", "Business", "FirstClass"};
        }

        cout << "\nEnter seats and prices for each class:\n";
        for (const string& cls : newTransport.classes) {
            int seats;
            float price;
            cout << cls << " Class:\n";
            cout << "  Seats: ";
            while (!(cin >> seats)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number: ";
            }
            cout << "  Price: Rs ";
            while (!(cin >> price)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number: ";
            }
            newTransport.seats.push_back(seats);
            newTransport.prices.push_back(price);
        }

        if (transportManager.addTransport(
            isTrain ? TRAIN_DATA_FILE : FLIGHT_DATA_FILE, newTransport, isTrain)) {
            cout << (isTrain ? "Train" : "Flight") << " schedule added successfully.\n";
        } else {
            cout << "Failed to add schedule.\n";
        }
        pressAnyKeyToContinue();
    }

    void viewTransports(bool isTrain) {
        displayHeader(isTrain ? "TRAIN SCHEDULES" : "FLIGHT SCHEDULES");
        
        auto transports = transportManager.getTransports(
            isTrain ? TRAIN_DATA_FILE : FLIGHT_DATA_FILE, isTrain);

        if (transports.empty()) {
            cout << "No " << (isTrain ? "trains" : "flights") << " found.\n";
        } else {
            for (const auto& transport : transports) {
                displayTransport(transport, isTrain);
            }
        }
        pressAnyKeyToContinue();
    }

    void deleteTransport(bool isTrain) {
        displayHeader(isTrain ? "DELETE TRAIN SCHEDULE" : "DELETE FLIGHT SCHEDULE");
        
        int transportNumber;
        cout << "Enter " << (isTrain ? "Train" : "Flight") << " Number to delete: ";
        while (!(cin >> transportNumber)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number: ";
        }

        if (transportManager.deleteTransport(
            isTrain ? TRAIN_DATA_FILE : FLIGHT_DATA_FILE, transportNumber, isTrain)) {
            cout << (isTrain ? "Train" : "Flight") << " schedule deleted successfully.\n";
        } else {
            cout << "Failed to delete schedule or schedule not found.\n";
        }
        pressAnyKeyToContinue();
    }

    void viewAllBookings() {
        displayHeader("ALL BOOKINGS");
        
        auto bookings = bookingManager.getAllBookings();
        if (bookings.empty()) {
            cout << "No bookings found.\n";
        } else {
            for (const auto& booking : bookings) {
                displayBooking(booking);
            }
        }
        pressAnyKeyToContinue();
    }

    void updateAdminProfile() {
        displayHeader("UPDATE ADMIN PROFILE");
        
        string newEmail, newName, newPassword, newUpi;
        cout << "Current Admin Email: " << dataManager.getAdminEmail() << endl;
        cout << "New Admin Email (leave blank to keep current): ";
        cin.ignore();
        getline(cin, newEmail);

        cout << "Current Admin Name: " << dataManager.getAdminName() << endl;
        cout << "New Admin Name (leave blank to keep current): ";
        getline(cin, newName);

        cout << "New Admin Password (leave blank to keep current): ";
        string password = getMaskedInput("");

        cout << "Current UPI ID: " << dataManager.getUpiId() << endl;
        cout << "New UPI ID (leave blank to keep current): ";
        getline(cin, newUpi);

        if (!newEmail.empty() && isValidEmail(newEmail)) {
            dataManager.setAdminCredentials(newEmail, 
                newName.empty() ? dataManager.getAdminName() : newName,
                password.empty() ? dataManager.getAdminPassword() : password);
        }

        if (!newUpi.empty()) {
            dataManager.setUpiId(newUpi);
        }

        cout << "Admin profile updated successfully.\n";
        pressAnyKeyToContinue();
    }
};

int main() {
    UI ui;
    ui.showMainMenu();
    return 0;
}
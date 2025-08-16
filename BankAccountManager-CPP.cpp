#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <conio.h>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <fstream>
#include <sqlite3.h>

using namespace std;

const int MIN_PIN_NUMBER = 0;
const int MAX_PIN_NUMBER = 9999;
const int MIN_NAME_LENGTH = 4;
const int MAX_NAME_LENGTH = 15;
const int MAX_ATTEMPTS = 3;
const double MAX_BALANCE = 1000000;
const double MAX_NEGATIVE_BALANCE = -1000000;
const double START_BALANCE = 1000.00;

string getMaskedPin(const string& text) {
    cout << text;
    string input;
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b";
            }
        }
        else if (isdigit(ch)) {
            input.push_back(ch);
            cout << '*';
        }
    }
    cout << "\n";
    if (input == "") return "-1";
    return input;
}

double getAmount(const string& text) {
    cout << text;

    string input;
    char ch;
    bool hasDecimal = false;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if(input.back() == '.') {
                hasDecimal = false;
			}
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b";
            }
        }
        else if (isdigit(ch)) {
            input.push_back(ch);
            cout << ch;
        }
        else if (ch == '.' && !hasDecimal) {
            input.push_back(ch);
            cout << ch;
            hasDecimal = true;
        }
        else if (ch == '.' && hasDecimal) {
            continue;
        }
    }
    cout << "\n";
    if (input == "") return 0;
    return stod(input);
}

string toLower(const string& str) {
    string lowerStr = str;
    for (char& c : lowerStr) {
        c = tolower(c);
    }
    return lowerStr;
}

enum Command {
    LOGIN,
    REGISTER,
    QUIT,
    HELP,
    DEPOSIT,
    WITHDRAW,
    BALANCE,
    NEGATIVE_STATUS,
    SET_NEGATIVE,
    SET_PIN,
    LOG_OFF,
    TRANSFER,
    UNKNOWN
};

Command parseCommand(const string& input) {
    string command = toLower(input);
    if (command == "login") return LOGIN;
    if (command == "register") return REGISTER;
    if (command == "quit") return QUIT;
    if (command == "help") return HELP;
    if (command == "deposit") return DEPOSIT;
    if (command == "withdraw") return WITHDRAW;
    if (command == "balance") return BALANCE;
    if (command == "negative?") return NEGATIVE_STATUS;
    if (command == "set_negative") return SET_NEGATIVE;
    if (command == "set_pin") return SET_PIN;
    if (command == "log_off") return LOG_OFF;
    if (command == "transfer") return TRANSFER;
    return UNKNOWN;
}

string hashPin(int &pin) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        cerr << "Error creating EVP context\n";
        return "";
    }

    if (!EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
        cerr << "Error initializing digest\n";
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (!EVP_DigestUpdate(ctx, &pin, sizeof(pin))) {
        cerr << "Error updating digest\n";
        EVP_MD_CTX_free(ctx);
        return "";
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length;
    if (!EVP_DigestFinal_ex(ctx, hash, &length)) {
        cerr << "Error finalizing digest\n";
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    stringstream ss;
    for (unsigned int i = 0; i < length; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool isTaken(sqlite3* db, const string& name, int &id_) {
    sqlite3_stmt* stmt;
    int id = -1;

    const char* sql = "SELECT ID FROM Accounts WHERE Name = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_ = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);
    return false;
}

string getPin(sqlite3* db, const string& name) {
    sqlite3_stmt* stmt;
    string pin = "";

    const char* sql = "SELECT Pin FROM Accounts WHERE Name = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement " << sqlite3_errmsg(db) << endl;;
        return "";
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        pin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    else {
        cout << "No account found for name: " << name << endl;
    }

    sqlite3_finalize(stmt);
    return pin;
}

double getBalance(sqlite3* db, int id) {
    sqlite3_stmt* stmt;
    double balance = 0.0;

    const char* sql = "SELECT Balance FROM Accounts WHERE ID = ?;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        balance = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return balance;
}

int getAttempts(sqlite3* db, const string& name) {
    sqlite3_stmt* stmt;
    int attempts = 0;

    const char* sql = "SELECT Attempts FROM Accounts WHERE Name = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement " << sqlite3_errmsg(db) << endl;;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        attempts = sqlite3_column_int(stmt, 0);
    }
    else {
        cout << "Account not found for name: " << name << endl;
    }
    sqlite3_finalize(stmt);
    return attempts;
}

bool getNegativeStatus(sqlite3* db, const int& id) {
    sqlite3_stmt* stmt;
    bool negativeStatus = false;

    const char* sql = "SELECT Negative FROM Accounts WHERE ID = ?;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        negativeStatus = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return negativeStatus;
}

void updateBalance(sqlite3* db, int& id, double& newBalance) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE Accounts SET Balance = ? WHERE ID = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare update statement " << sqlite3_errmsg(db) << endl;;
        return;
    }

    sqlite3_bind_double(stmt, 1, newBalance);
    sqlite3_bind_int(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to update balance " << sqlite3_errmsg(db) << endl;;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}

void updateAttempts(sqlite3* db, const string& name, int attempts) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE Accounts SET Attempts = ? WHERE Name = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare update statement\n";
        return;
    }

    sqlite3_bind_int(stmt, 1, attempts);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to update attempts " << sqlite3_errmsg(db) << endl;;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}

void updateNegativeStatus(sqlite3* db, int& id, bool negativeStatus) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE Accounts SET Negative = ? WHERE ID = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare update statement " << sqlite3_errmsg(db) << endl;;
        return;
    }

    sqlite3_bind_int(stmt, 1, negativeStatus);
    sqlite3_bind_int(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to update negative status" << sqlite3_errmsg(db) << endl;;
        sqlite3_finalize(stmt);
        return;
    }

	cout << "Negative status updated successfully\n";
    sqlite3_finalize(stmt);
    return;
}

void updatePin(sqlite3* db, const int& id, const string& newPin) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE Accounts SET Pin = ? WHERE ID = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare update statement " << sqlite3_errmsg(db) << endl;;
        return;
    }

    sqlite3_bind_text(stmt, 1, newPin.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to update pin " << sqlite3_errmsg(db) << endl;;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
    return;
}

string getPinById(sqlite3* db, int id) {
    sqlite3_stmt* stmt;
    string pin = "";
    const char* sql = "SELECT Pin FROM Accounts WHERE ID = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement " << sqlite3_errmsg(db) << endl;;
        return "";
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        pin = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    else {
        cout << "No account found for ID: " << id << endl;
    }

    sqlite3_finalize(stmt);
    return pin;
}

int getAttemptsById(sqlite3* db, int id) {
    sqlite3_stmt* stmt;
    int attempts = 0;
    const char* sql = "SELECT Attempts FROM Accounts WHERE ID = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare statement " << sqlite3_errmsg(db) << endl;;
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        attempts = sqlite3_column_int(stmt, 0);
    }
    else {
        cout << "Account not found for ID: " << id << " " << sqlite3_errmsg(db) << endl;;
    }

    sqlite3_finalize(stmt);
    return attempts;
}

void updateAttemptsById(sqlite3* db, int id, int attempts) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE Accounts SET Attempts = ? WHERE ID = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Failed to prepare update statement " << sqlite3_errmsg(db) << endl;;
        return;
    }

    sqlite3_bind_int(stmt, 1, attempts);
    sqlite3_bind_int(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Failed to update attempts\n";
    }

    sqlite3_finalize(stmt);
}

bool checkWithdrawValid(double amount, bool isNegativeAllowed) {
    if (amount < 0 && !isNegativeAllowed) {
		cout << "You can`t withdraw money, negative balance is not allowed\n";
        return false;
    }
    if (amount < MAX_NEGATIVE_BALANCE) {
        cout << "You can`t withdraw that much money, maximum negative balance is " << MAX_NEGATIVE_BALANCE << "$\n";
        return false;
    }
	return true;
}

bool checkDepositValid(double amount) {
    if(amount > MAX_BALANCE) {
        cout << "You can`t deposit that much money, maximum balance is " << MAX_BALANCE << "$\n";
        return false;
	}
    return true;
}

class Bank {
private:
    sqlite3* db;
public:
    int currentId;

    Bank(sqlite3* db) {
        this->db = db;
    }

    void registerAccount(string& name, int& pin) const {
        if (name.size() > MAX_NAME_LENGTH || name.size() < MIN_NAME_LENGTH) {
            cout << "You need to enter name with minimum 4 or maximum 15 characters, please try again\n";
            return;
        }

        int tempId;
        if (isTaken(db, name, tempId)) {
            cout << "Name already taken, please try again\n";
            return;
        }

        if (pin < MIN_PIN_NUMBER || pin > MAX_PIN_NUMBER) {
            cout << "You need to enter pin that is minimum number 0 or maximum number 9999, please try again\n";
            return;
        }

        string hashedPin = hashPin(pin);
        if (hashedPin.empty()) {
            cout << "Error hashing pin, please try again\n";
            return;
		}

        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO Accounts (Name, Pin) VALUES (?, ?);";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Failed to prepare insert statement " << sqlite3_errmsg(db) << endl;;
            return;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPin.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Failed to insert account: " << sqlite3_errmsg(db) << endl;;
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_finalize(stmt);

        cout << "*********************************\n";
        cout << name << " welcome to the bank";
        cout << "\n*********************************\n";

        return;
    }

    bool loginSession(string& name, int& pin) {
        int id;
        if (!isTaken(db, name, id)) {
            cout << "This name does not exist, please try again\n";
            return false;
        }

        if (getAttempts(db, name) >= MAX_ATTEMPTS) {
            cout << "This account is locked please try again another time\n";
            return false;
        }

        string hashedPin = hashPin(pin);

        if (hashedPin != getPinById(db, id)) {
			updateAttemptsById(db, id, getAttemptsById(db, id) + 1);
            return false;
        }
        updateAttemptsById(db, id, 0);
		this->currentId = id;
        return true;
    }

    void runLoggedInSession() {
        double amount, tempD;
        int tempI;
        string temp, targetName;
        while (true) {
            cout << "Choose command, if need type help: ";
            cin >> temp;
            temp = toLower(temp);

            Command command = parseCommand(temp);

            switch (command) {
            case HELP:
                cout << "\n\n\n************************\n"
                    << "log_off\ndeposit\nwithdraw\ntransfer\nbalance\nnegative?\nset_negative\nset_pin\n"
                    << "************************\n";
                break;

            case LOG_OFF:
				cout << "You have logged off successfully\n";
                return;

            case DEPOSIT:
                tempD = getAmount("Please enter amount to deposit: ");
                amount = getBalance(db, this->currentId) + tempD;

				if (!checkDepositValid(amount)) { break; }

                updateBalance(db, this->currentId, amount);

                cout << "You successfully deposited " << tempD << "$\n";
                break;

            case WITHDRAW:
                tempD = getAmount("Please enter amount to withdraw: ");
				amount = getBalance(db, this->currentId) - tempD;

                if (!checkWithdrawValid(amount, getNegativeStatus(db, this->currentId))) { break; }

                updateBalance(db, this->currentId, amount);

				cout << "You successfully withdrew " << tempD << "$\n";
                break;

            case BALANCE:
				cout << "Your current balance is: " << getBalance(db, this->currentId) << "$\n";
                break;

            case NEGATIVE_STATUS:
                cout << "Your negative status is: " 
					<< (getNegativeStatus(db, this->currentId) ? "Enabled" : "Disabled") << "\n";
                break;

            case SET_NEGATIVE:
                (getNegativeStatus(db, this->currentId) ? updateNegativeStatus(db, this->currentId, false)
                    : updateNegativeStatus(db, this->currentId, true));
                break;

            case SET_PIN:
                temp = getMaskedPin("Please enter your new pin (0-9999): ");
                if (temp == "-1") {
                    cout << "You need to enter a valid pin\n";
                    break;
                }

                tempI = stoi(temp);
                if (tempI < MIN_PIN_NUMBER || tempI > MAX_PIN_NUMBER) {
                    cout << "You need to enter a pin that is minimum number 0 or maximum number 9999, please try again\n";
                    break;
                }

                temp = hashPin(tempI);
                if (temp.empty()) {
                    cout << "Error hashing pin, please try again\n";
                    break;
                }

				updatePin(db, this->currentId, temp);

				cout << "Pin updated successfully\n";
                break;

            case TRANSFER:
                tempD = getAmount("Please enter amount to transfer: ");
                cout << "Please enter the name of the account you want to transfer to: ";
                cin >> targetName;
                if (!isTaken(this->db, targetName, tempI)) {
                    cout << "This account does not exist, please try again\n";
                    break;
                }

                amount = getBalance(this->db, this->currentId) - tempD;

                if(amount < 0 && !getNegativeStatus(this->db, this->currentId)) {
                    cout << "You can`t transfer money, negative balance is not allowed\n";
                    break;
				}

				if (amount < MAX_NEGATIVE_BALANCE) {
                    cout << "You can`t transfer that much money, maximum negative balance is " << MAX_NEGATIVE_BALANCE << "$\n";
                    break;
				}

                updateBalance(this->db, this->currentId, amount);

				amount = getBalance(this->db, tempI) + tempD;

                if (amount > MAX_BALANCE) {
					cout << "You can`t transfer that much money, maximum balance is " << MAX_BALANCE << "$\n";
                }

				updateBalance(this->db, tempI, amount);
                
				cout << "Transfer successful, you transferred " << tempD << "$ to " << targetName << "\n";
                break;

            case UNKNOWN:
                cout << "Unknown command, please try again\n";
                break;
            }
        }
    }
};

void registerAccount(Bank& bank) {
    string tname, spin; int tpin;

    cout << "Please enter your name (4-15 characters): ";
    cin >> tname;

    spin = getMaskedPin("Please enter your pin (0-9999):");
    tpin = stoi(spin);

    bank.registerAccount(tname, tpin);
}

void loginAccount(Bank& bank) {
    string tname, spin; int tpin;

    cout << "Please enter your name: ";
    cin >> tname;

    spin = getMaskedPin("Please enter your pin:");
    tpin = stoi(spin);

    if (bank.loginSession(tname, tpin)) {
        bank.runLoggedInSession();
    }
}

int main() {
    sqlite3* db;
    int rc = sqlite3_open("bank.db", &db);
    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 0;
    }

    Bank bank(db);

    string temp;
    while (true) {
        cout << "*********************************\n";
        cout << "Options: login/register/quit";
        cout << "\n*********************************\n";
        cin >> temp;

        temp = toLower(temp);

        Command command = parseCommand(temp);

        switch (command) {
        case REGISTER:
            registerAccount(bank);
            break;

        case LOGIN:
            loginAccount(bank);
            break;

        case QUIT:
            cout << "Thank you for using our bank, goodbye!\n";
            return 0;

        case UNKNOWN:
            cout << "Unknown command, please try again\n";
            break;
        }
    }

    sqlite3_close(db);

    return 0;
}
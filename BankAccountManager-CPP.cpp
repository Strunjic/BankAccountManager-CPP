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

using namespace std;

const double MIN_BALANCE_FOR_NEGATIVE = 10000.00;

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
        if(ch == '\b') {
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

class BankAccount {
private:
    int attempt = 0;
    string hashed_pin;
    bool logged = false;
    double balance = 1000.00;
    bool negative = false;

    static string hashPin(int pin) {
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
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
public:
    string name;

    string getInfo() {
        string s = name + "{" + this->hashed_pin
            + "{" + to_string(this->balance) + "{"
            + to_string(this->attempt) + "{"
            + to_string(this->negative) + "{\n";
        return s;
    }

    BankAccount(string name, int pin) {
        this->name = name;
        this->hashed_pin = hashPin(pin);
    }
    BankAccount() {}
    BankAccount(string name, string hashed_pin, double balance, int attempt, bool negative) {
        this->name = name;
        this->hashed_pin = hashed_pin;
        this->balance = balance;
        this->attempt = attempt;
        this->negative = negative;
    }

    bool login(int pin) {
        if (this->hashed_pin == hashPin(pin)) {
            this->logged = true;
            cout << "You are in, " << name << "\n";
            return true;
        }
        cout << "Wrong pin\n";
        return false;
    }
    void log_off() {
        this->logged = false;
        cout << "You are out " << name << "\n";
    }
    void deposit(double amount) {
        if (!this->logged) {
            cout << "You need to login first\n";
            return;
        }
        if (amount < 0) {
            cout << "You can`t deposit negative money";
            return;
        }
        this->balance += amount;
        cout << "You successfully deposited " << amount << "$\n";
    }
    void withdraw(double amount) {
        if (!this->logged) {
            cout << "You need to login first\n";
            return;
        }
        if (amount < 0) {
            cout << "You can`t withdraw negative money";
            return;
        }
        if (negative) {
            balance -= amount;
            cout << "You successfully withdraw " << amount << "$\n";
            return;
        }
        else if (balance - amount >= 0) {
            balance -= amount;
            cout << "You successfully withdraw " << amount << "$\n";
            return;
        }
        cout << "You can`t withdraw that much money :(\n";
    }
    void getBalance() {
        if (!this->logged) {
            cout << "You need to login first\n";
            return;
        }
		printf("Yout balance is: %.2f$\n", balance);
    }
    void getNegative() {
        if (!this->logged) {
            cout << "You need to login first\n";
            return;
        }
        cout << this->negative << "\n";
    }
    void setNegative() {
        if (!this->logged) {
            cout << "You need to login first\n";
            return;
        }

        if (this->negative) {
            negative = false;
            cout << "You successfully changed the negative status\n";
            return;
        }

        if (this->balance < MIN_BALANCE_FOR_NEGATIVE) {
            cout << "You can`t change negative status\n";
            return;
        }
        this->negative = true;
        cout << "You successfully changed the negative status\n";
    }
    void setPin() {
        if (!this->logged) {
            cout << "You need to login first\n";
            return;
        }
        int tpin; string spin;

        spin = getMaskedPin("Please enter your pin (0-9999):");
        tpin = stoi(spin);

        if (tpin < 0 || tpin > 9999) {
            cout << "You need to enter pin that is minimu number 0 or maximum number 9999, please try again\n";
            return;
        }
        this->hashed_pin = hashPin(tpin);
        cout << "You successfully changed the pin\n";
    }

    int getAttempts() {
        return this->attempt;
    }

    void uppAttempts() {
        this->attempt++;
    }

    void zeroAttempts() {
        this->attempt = 0;
    }
};

class Bank {
private:
    vector<BankAccount> accounts;
    unordered_map<string, size_t> names;
    int currentAccount = -1;
public:
    void saveToFile() {
        ofstream file("bank.txt", ios::out);
        if (file.is_open()) file.close();

        fstream myFile;
        myFile.open("bank.txt", ios::app);

        if (myFile.is_open()) {
            for (auto x : this->accounts) {
                myFile << x.getInfo();
            }
            myFile.close();
        }
    }

    void loadFromFIle() {
        fstream myFile;
        myFile.open("bank.txt", ios::in);

        if (myFile.is_open()) {
            string line;
            while (getline(myFile, line)) {
                int t = 0, p = 0;
                string tname, tpin;
                double tbalance;
                int tattempt;
                bool tnegative;
                int size = line.size();
                for (int i = 0; i < size; i++) {
                    if (line[i] == '{') {
                        switch (t) {
                        case 0:
                            tname = line.substr(p, i - p);
                            p = i + 1;
                            t++;
                            break;
                        case 1:
                            tpin = line.substr(p, i - p);
                            p = i + 1;
                            t++;
                            break;
                        case 2:
                            tbalance = stod(line.substr(p, i - p));
                            p = i + 1;
                            t++;
                            break;
                        case 3:
                            tattempt = stoi(line.substr(p, i - p));
                            p = i + 1;
                            t++;
                            break;
                        }
                    }
                }
                string s(1, line.back());
                (s == "0") ? tnegative = false : tnegative = true;
                BankAccount newAcc(tname, tpin, tbalance, tattempt, tnegative);
                names.insert({ tname, names.size() });
                accounts.push_back(newAcc);
            }
            myFile.close();
        }
    }

    BankAccount& getCurrent() {
        if (this->currentAccount == -1) {
            throw runtime_error("No account logged in");
        }
        return accounts[this->currentAccount];
    }

    void log_off() {
        this->currentAccount = -1;
    }
    void registerAccount(string name, int pin) {
        if (name.size() > 15 || name.size() < 4) {
            cout << "You need to enter name with minimum 4 or maximum 15 characters, please try again\n";
            return;
        }

        if (this->names.find(name) != this->names.end()) {
            cout << "Name already taken, please try again\n";
            return;
        }
        if (pin < 0 || pin > 9999) {
            cout << "You need to enter pin that is minimum number 0 or maximum number 9999, please try again\n";
            return;
        }

        BankAccount newAccount(name, pin);
        this->names.insert({ name, this->accounts.size() });
        this->accounts.push_back(newAccount);

        cout << "*********************************\n";
        cout << name << " welcome to the bank";
        cout << "\n*********************************\n";

        return;
    }
    bool loginSession(string name, int pin) {
        if (this->names.find(name) == names.end()) {
            cout << "This name does not exist, please try again\n";
            return false;
        }
        currentAccount = this->names[name];

        if (this->accounts[currentAccount].getAttempts() >= 3) {
            cout << "This account is locked please try again another time\n";
            return false;
        }

        if (!this->accounts[currentAccount].login(pin)) {
            this->accounts[currentAccount].uppAttempts();
            return false;
        }
        this->accounts[currentAccount].zeroAttempts();
        return true;
    }

    void runLoggedInSession() {
        string temp;
        while (true) {
            cout << "Choose command, if need type help: ";
            cin >> temp;

            if (temp == "help") {
                cout << "\n\n\n************************\n"
                    << "log_off\ndeposit\nwithdraw\nbalance\nnegative?\nset_negative\nset_pin\n"
                    << "************************\n";
            }

            if (temp == "log_off") {
                this->getCurrent().log_off();
                this->log_off();
                break;
            }

            if (temp == "deposit") {
                double amount;
				amount = getAmount("Please enter amount to deposit: ");
                this->getCurrent().deposit(amount);
                continue;
            }

            if (temp == "withdraw") {
                double amount;
				amount = getAmount("Please enter amount to withdraw: ");
                this->getCurrent().withdraw(amount);
                continue;
            }

            if (temp == "balance") {
                this->getCurrent().getBalance();
                continue;
            }

            if (temp == "negative?") {
                this->getCurrent().getNegative();
                continue;
            }

            if (temp == "set_negative") {
                this->getCurrent().setNegative();
            }

            if (temp == "set_pin") {
                this->getCurrent().setPin();
            }
        }
    }
};

int main() {
    Bank bank;
    bank.loadFromFIle();
    string temp;
    while (true) {
        cout << "*********************************\n";
        cout << "Options: login/register/quit";
        cout << "\n*********************************\n";
        cin >> temp;

        if (temp == "register") {
            string tname, spin; int tpin;

            cout << "Please enter your name (4-15 characters): ";
            cin >> tname;

            spin = getMaskedPin("Please enter your pin (0-9999):");
            tpin = stoi(spin);

            bank.registerAccount(tname, tpin);
            continue;
        }

        else if (temp == "login") {
            string tname, spin; int tpin;

            cout << "Please enter your name: ";
            cin >> tname;

            spin = getMaskedPin("Please enter your pin:");
            tpin = stoi(spin);

            if (bank.loginSession(tname, tpin)) {
                bank.runLoggedInSession();
            }
            continue;
        }

        else if (temp == "quit") break;

        cout << "You entered wrong command\n";
    }

    bank.saveToFile();

    return 0;
}
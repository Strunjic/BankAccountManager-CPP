# BankAccountManager-CPP
This is my first project, I hope I will improve in future

# Simple Banking System in C++
A console-based Bank Account Manager application written in C++, using SQLite for persistent storage and OpenSSL for secure PIN hashing. This program allows users to create accounts, log in, deposit, withdraw, transfer money, and manage their account settings

## Features
- User Registration: Create an account with a name and PIN
- Secure Login: PINs are hashed using SHA-256 before storage
- Deposit & Withdraw: Add or remove funds from your account
- Balance Check: View current account balance
- Transfer: Transfer money between accounts
- Negative Balance Toggle: Enable or disable the ability to go into negative balance
- PIN Management: Change your PIN securely
- Account Lockout: Accounts are locked after 3 failed login attempts
- Command Help: Simple command-based interface with help instructions

## Requirements
- C++ 17 compatible compiler
- SQLite3 library
- OpenSSL library (for SHA-256 hashing)
- Console environment supporting `_getch()` (Windows-specific)

## Database
- The program uses a SQLite database named bank.db
- Database schema:
CREATE TABLE Accounts (
    ID INTEGER PRIMARY KEY AUTOINCREMENT,
    Name TEXT UNIQUE NOT NULL,
    Pin TEXT NOT NULL,
    Balance REAL DEFAULT 1000.00,
    Negative INTEGER DEFAULT 0,
    Attempts INTEGER DEFAULT 0
);

## Installation
# Clone the repository
- git clone https://github.com/your-username/BankAccountManager-CPP.git

### Install dependencies
- SQLite3
- OpenSSL

### Compile with:
- `g++ -std=c++17 main.cpp -lsqlite3 -lssl -lcrypto -o BankManager`

### Run: 
- `./BankManager`

## Usage
- Run the program and choose one of the options: login | register | quit
- Follow prompts to manage accounts
- Use help after login to see all available commands
- Commands: log_off, deposit, withdraw, transfer, balance, negative?, set_negative, set_pin

## Security
- SHA-256 hashing via OpenSSL ensures PINs are never stored in plain text
- Accounts are locked after 3 failed login attempts to prevent brute-force attacks

## Security Notes
- PINs must be numeric between 0 and 9999.
- PINs are hashed using SHA-256
- Raw PINs are never stored
- Account lockout after 3 failed attempts
- Maximum and minimum balance limits enforced
- Username length restrictions (4-15 characters)

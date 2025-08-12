# BankAccountManager-CPP
This is my first project, I hope I will improve in future

# Simple Banking System in C++
A console-based banking system with secure PIN hashing and account management.

## Features
- Account registration with PIN hashing (SHA-256)
- Secure PIN input masking
- Deposit/withdraw functionality
- Balance inquiry
- Negative balance settings (with minimum balance requirement)
- Account locking after 3 failed attempts
- Persistent data storage
- Transfer money between accounts

## Requirements
- C++ compiler (supporting C++11 or later)
- OpenSSL library (for SHA-256 hashing)
- Console environment supporting `_getch()` (Windows-specific)

## Installation
1. Clone the repository
2. Install OpenSSL if not present
3. Compile with: `g++ main.cpp -o bank -lssl -lcrypto`
4. Run: `./bank`

## Usage
- Register new accounts
- Login with existing accounts
- Use commands: log_off, deposit, withdraw, transfer, balance, negative?, set_negative, set_pin
- Type 'help' for command list

## Security Notes
- PINs must be numeric between 0 and 9999.
- PINs are hashed using SHA-256
- Raw PINs are never stored
- Account lockout after 3 failed attempts
- Maximum and minimum balance limits enforced
- Username length restrictions (4-15 characters)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

#define MAX_NAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 20
#define MAX_TRANSACTION_HISTORY 10
#define MAX_USERS 10
#define LOGIN_SUCCESS 0
#define LOGIN_FAILED -1
#define LOGIN_LOCKED -2
#define DATA_FILE "accounts.txt"  // File to store user account data

// Define the structure for bank account
typedef struct {
    char name[MAX_NAME_LENGTH];
    int age;
    char password[MAX_PASSWORD_LENGTH];
    int account_number;
    double balance;
    char transaction_history[MAX_TRANSACTION_HISTORY][100];
    int transaction_count;
} BankAccount;

// Function declarations
void create_account(BankAccount *account);
int login(BankAccount users[], int user_count);
void display_menu();
void deposit(BankAccount *account);
void withdraw(BankAccount *account);
void check_balance(BankAccount *account);
void view_transaction_history(BankAccount *account);
void change_password(BankAccount *account);
void clear_input_buffer();
void signup(BankAccount users[], int *user_count);
void load_accounts(BankAccount users[], int *user_count);
void save_accounts(BankAccount users[], int user_count);
int validate_password(const char *password);
void forgot_password(BankAccount users[], int user_count);
void transfer_money(BankAccount users[], int user_count, int sender_index);

int main() {
    srand(time(NULL));  // Seed the random number generator for account numbers

    BankAccount users[MAX_USERS];  // Array to store multiple users (for demo purposes)
    int user_count = 0;            // Number of users created
    int logged_in_user = -1;       // Index of logged-in user (-1 means no one is logged in)
    int choice;

    // Load user data from file (if available)
    load_accounts(users, &user_count);

    while (1) {
        if (logged_in_user == -1) {
            // Display Login/Signup menu if no user is logged in
            printf("\n1. Login\n2. Sign Up\n3. Forgot Password\n4. Exit\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clear_input_buffer();

            if (choice == 1) {
                logged_in_user = login(users, user_count);
            } else if (choice == 2) {
                signup(users, &user_count);
            } else if (choice == 3) {
                forgot_password(users, user_count);
            } else {
                // Save accounts to file before exiting
                save_accounts(users, user_count);
                printf("Exiting... Goodbye!\n");
                break;
            }
        } else {
            // Menu for logged-in user
            display_menu();
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clear_input_buffer();

            switch (choice) {
                case 1:
                    deposit(&users[logged_in_user]);
                    break;
                case 2:
                    withdraw(&users[logged_in_user]);
                    break;
                case 3:
                    check_balance(&users[logged_in_user]);
                    break;
                case 4:
                    view_transaction_history(&users[logged_in_user]);
                    break;
                case 5:
                    change_password(&users[logged_in_user]);
                    break;
                case 6:
                    transfer_money(users, user_count, logged_in_user);  // Call transfer function
                    break;
                case 7:
                    printf("Logging out...\n");
                    logged_in_user = -1;  // Log the user out
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
            }
        }
    }

    return 0;
}

// Function to create an account
void create_account(BankAccount *account) {
    printf("Welcome to the Bank Account Creation\n");

    // Get user's name
    printf("Enter your name: ");
    fgets(account->name, sizeof(account->name), stdin);
    account->name[strcspn(account->name, "\n")] = '\0'; // Remove newline character from name

    // Get user's age and ensure they are 18 or older
    do {
        printf("Enter your age: ");
        scanf("%d", &account->age);
        clear_input_buffer();
        if (account->age < 18) {
            printf("You must be over 18 to create an account.\n");
        }
    } while (account->age < 18);

    // Get user's password
    printf("Create your password (max 20 characters): ");
    fgets(account->password, sizeof(account->password), stdin);
    account->password[strcspn(account->password, "\n")] = '\0'; // Remove newline from password

    // Generate a random account number
    account->account_number = rand() % 1000000000 + 100000000; // Random 9-digit number

    printf("Account created successfully!\n");
    printf("Your account number is: %d\n", account->account_number);
    account->balance = 0.0;
    account->transaction_count = 0;
}

// Function for login
int login(BankAccount users[], int user_count) {
    char name[MAX_NAME_LENGTH], password[MAX_PASSWORD_LENGTH];
    int max_attempts = 3;
    int attempts = 0;

    while (attempts < max_attempts) {
        printf("Enter your name: ");
        fgets(name, sizeof(name), stdin);
        name[strcspn(name, "\n")] = '\0';  // Remove newline

        printf("Enter your password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = '\0';  // Remove newline

        for (int i = 0; i < user_count; i++) {
            if (strcmp(name, users[i].name) == 0 && strcmp(password, users[i].password) == 0) {
                printf("Login successful!\n");
                return i;  // Return the index of the logged-in user
            }
        }

        attempts++;
        printf("Invalid username or password. You have %d attempt(s) left.\n", max_attempts - attempts);
        
        // If user fails 3 times, wait for 30 seconds before retry
        if (attempts >= max_attempts) {
            printf("Too many failed login attempts. You must wait for 30 seconds before trying again.\n");
            printf("Please wait...\n");
            sleep(30);  // Wait for 30 seconds
            printf("You can now try logging in again.\n");
        }
    }

    printf("Too many failed login attempts. Exiting...\n");
    return -1;  // Lock the user out after 3 attempts
}

// Function to display menu after successful login
void display_menu() {
    printf("\n------ Bank Menu ------\n");
    printf("1. Deposit\n");
    printf("2. Withdraw\n");
    printf("3. Check Balance\n");
    printf("4. View Transaction History\n");
    printf("5. Change Password\n");
    printf("6. Transfer Money\n");
    printf("7. Logout\n");
    printf("------------------------\n");
}

// Function to deposit money into the account
void deposit(BankAccount *account) {
    double amount;
    printf("Enter the amount to deposit: ");
    scanf("%lf", &amount);
    clear_input_buffer();

    if (amount <= 1000) {
        printf("Amount must be greater than 0.\n");
        return;
    }

    account->balance += amount;
    printf("Successfully deposited %.2f\n", amount);

    // Add to transaction history
    if (account->transaction_count < MAX_TRANSACTION_HISTORY) {
        snprintf(account->transaction_history[account->transaction_count], sizeof(account->transaction_history[account->transaction_count]),
                 "Deposited: %.2f", amount);
        account->transaction_count++;
    }
}

// Function to withdraw money from the account
void withdraw(BankAccount *account) {
    double amount;
    printf("Enter the amount to withdraw: ");
    scanf("%lf", &amount);
    clear_input_buffer();

    if (amount <= 0) {
        printf("Amount must be greater than 0.\n");
        return;
    }

    if (amount > account->balance) {
        printf("Insufficient balance.\n");
        return;
    }

    account->balance -= amount;
    printf("Successfully withdrew %.2f\n", amount);

    // Add to transaction history
    if (account->transaction_count < MAX_TRANSACTION_HISTORY) {
        snprintf(account->transaction_history[account->transaction_count], sizeof(account->transaction_history[account->transaction_count]),
                 "Withdrew: %.2f", amount);
        account->transaction_count++;
    }
}

// Function to check the current balance
void check_balance(BankAccount *account) {
    printf("Your current balance is: %.2f\n", account->balance);
}

// Function to view transaction history
void view_transaction_history(BankAccount *account) {
    if (account->transaction_count == 0) {
        printf("No transaction history available.\n");
        return;
    }

    printf("\nTransaction History:\n");
    for (int i = 0; i < account->transaction_count; i++) {
        printf("%s\n", account->transaction_history[i]);
    }
}

// Function to change password
void change_password(BankAccount *account) {
    char old_password[MAX_PASSWORD_LENGTH], new_password[MAX_PASSWORD_LENGTH];
    printf("Enter your current password: ");
    fgets(old_password, sizeof(old_password), stdin);
    old_password[strcspn(old_password, "\n")] = '\0'; // Remove newline

    if (strcmp(old_password, account->password) != 0) {
        printf("Incorrect password!\n");
        return;
    }

    printf("Enter your new password: ");
    fgets(new_password, sizeof(new_password), stdin);
    new_password[strcspn(new_password, "\n")] = '\0'; // Remove newline

    if (strlen(new_password) > 0 && strlen(new_password) < MAX_PASSWORD_LENGTH) {
        strcpy(account->password, new_password);
        printf("Password changed successfully!\n");
    } else {
        printf("Password must be between 1 and %d characters.\n", MAX_PASSWORD_LENGTH);
    }
}

// Function to clear input buffer
void clear_input_buffer() {
    while (getchar() != '\n');  // Clears the input buffer to avoid issues with fgets
}

// Function for signing up (creating a new account)
void signup(BankAccount users[], int *user_count) {
    if (*user_count >= MAX_USERS) {
        printf("User limit reached. Cannot create more accounts.\n");
        return;
    }
    printf("\nCreating a new account...\n");
    create_account(&users[*user_count]);
    (*user_count)++;
}

// Function to load user accounts from file
void load_accounts(BankAccount users[], int *user_count) {
    FILE *file = fopen(DATA_FILE, "r");
    if (!file) {
        return;  // If no file exists, we simply return (no accounts to load)
    }

    while (fscanf(file, "%49[^,],%d,%19[^,],%d,%lf\n", 
                   users[*user_count].name, &users[*user_count].age, 
                   users[*user_count].password, &users[*user_count].account_number, 
                   &users[*user_count].balance) == 5) {
        (*user_count)++;
    }

    fclose(file);
}

// Function to save user accounts to file
void save_accounts(BankAccount users[], int user_count) {
    FILE *file = fopen(DATA_FILE, "w");
    if (!file) {
        printf("Error saving data.\n");
        return;
    }

    for (int i = 0; i < user_count; i++) {
        fprintf(file, "%s,%d,%s,%d,%.2f\n", 
                users[i].name, users[i].age, users[i].password, 
                users[i].account_number, users[i].balance);
    }

    fclose(file);
}

// Function to handle forgot password (identity verification)
void forgot_password(BankAccount users[], int user_count) {
    char name[MAX_NAME_LENGTH];
    int account_number;

    printf("Forgot Password\n");
    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; // Remove newline character

    printf("Enter your account number: ");
    scanf("%d", &account_number);
    clear_input_buffer();

    // Check if the user exists with the given details
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].name, name) == 0 && users[i].account_number == account_number) {
            printf("User identity verified. Please enter a new password: ");
            fgets(users[i].password, sizeof(users[i].password), stdin);
            users[i].password[strcspn(users[i].password, "\n")] = '\0'; // Remove newline

            printf("Password successfully changed!\n");
            return;
        }
    }

    printf("No user found with the provided details.\n");
}

// Function to transfer money between accounts
void transfer_money(BankAccount users[], int user_count, int sender_index) {
    int recipient_account_number;
    double transfer_amount;
    int recipient_index = -1;

    printf("Enter the recipient's account number: ");
    scanf("%d", &recipient_account_number);
    clear_input_buffer();

    // Find recipient user by account number
    for (int i = 0; i < user_count; i++) {
        if (users[i].account_number == recipient_account_number) {
            recipient_index = i;
            break;
        }
    }

    if (recipient_index == -1) {
        printf("Recipient account number not found!\n");
        return;
    }

    printf("Enter the amount to transfer: ");
    scanf("%lf", &transfer_amount);
    clear_input_buffer();

    if (transfer_amount <= 0) {
        printf("Amount must be greater than 0.\n");
        return;
    }

    if (transfer_amount > users[sender_index].balance) {
        printf("Insufficient balance.\n");
        return;
    }

    // Perform the transfer
    users[sender_index].balance -= transfer_amount;
    users[recipient_index].balance += transfer_amount;

    // Record the transactions
    if (users[sender_index].transaction_count < MAX_TRANSACTION_HISTORY) {
        snprintf(users[sender_index].transaction_history[users[sender_index].transaction_count], sizeof(users[sender_index].transaction_history[users[sender_index].transaction_count]),
                 "Transferred: %.2f to Account %d", transfer_amount, recipient_account_number);
        users[sender_index].transaction_count++;
    }

    if (users[recipient_index].transaction_count < MAX_TRANSACTION_HISTORY) {
        snprintf(users[recipient_index].transaction_history[users[recipient_index].transaction_count], sizeof(users[recipient_index].transaction_history[users[recipient_index].transaction_count]),
                 "Received: %.2f from Account %d", transfer_amount, users[sender_index].account_number);
        users[recipient_index].transaction_count++;
    }

    printf("Transfer of %.2f to Account %d successful.\n", transfer_amount, recipient_account_number);
}

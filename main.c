#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS  50
#define MAX_TXN    100
#define USER_FILE  "users.txt"
#define TXN_FILE   "transactions.txt"
#define ADMIN_USER "admin"
#define ADMIN_PASS "admin123"

typedef struct {
    int    id;
    char   name[50];
    char   username[30];
    char   password[30];
    double balance;
    int    active;
} Account;

typedef struct {
    int    acc_id;
    char   type[10];
    double amount;
    double balance_after;
} Transaction;

Account     *accounts = NULL;
int          acc_count = 0;
Transaction  txns[MAX_TXN];
int          txn_count = 0;

void saveAccounts() {
    int i;
    FILE *f = fopen(USER_FILE, "w");
    if (!f) { printf("Error saving accounts!\n"); return; }
    fprintf(f, "%d\n", acc_count);
    for (i = 0; i < acc_count; i++)
        fprintf(f, "%d|%s|%s|%s|%.2f|%d\n",
            accounts[i].id, accounts[i].name, accounts[i].username,
            accounts[i].password, accounts[i].balance, accounts[i].active);
    fclose(f);
}

void loadAccounts() {
    int i;
    FILE *f = fopen(USER_FILE, "r");
    if (!f) return;
    fscanf(f, "%d\n", &acc_count);
    accounts = (Account *)malloc(acc_count * sizeof(Account));
    for (i = 0; i < acc_count; i++)
        fscanf(f, "%d|%49[^|]|%29[^|]|%29[^|]|%lf|%d\n",
            &accounts[i].id, accounts[i].name, accounts[i].username,
            accounts[i].password, &accounts[i].balance, &accounts[i].active);
    fclose(f);
}

void saveTxns() {
    int i;
    FILE *f = fopen(TXN_FILE, "w");
    if (!f) return;
    fprintf(f, "%d\n", txn_count);
    for (i = 0; i < txn_count; i++)
        fprintf(f, "%d|%s|%.2f|%.2f\n",
            txns[i].acc_id, txns[i].type, txns[i].amount, txns[i].balance_after);
    fclose(f);
}

void loadTxns() {
    int i;
    FILE *f = fopen(TXN_FILE, "r");
    if (!f) {
        printf("File not found, starting fresh.\n");
        return;
    }
    fscanf(f, "%d\n", &txn_count);
    for (i = 0; i < txn_count; i++)
        fscanf(f, "%d|%9[^|]|%lf|%lf\n",
            &txns[i].acc_id, txns[i].type, &txns[i].amount, &txns[i].balance_after);
    fclose(f);
}

int isUsernameUnique(char *uname) {
    int i;
    for (i = 0; i < acc_count; i++)
        if (strcmp(accounts[i].username, uname) == 0) return 0;
    return 1;
}

int findAccount(int id) {
    int i;
    for (i = 0; i < acc_count; i++)
        if (accounts[i].id == id && accounts[i].active) return i;
    return -1;
}

int findByUsername(char *uname) {
    int i;
    for (i = 0; i < acc_count; i++)
        if (strcmp(accounts[i].username, uname) == 0 && accounts[i].active) return i;
    return -1;
}

void sortAccountsByBalance() {
    int i, j;
    Account temp;
    for (i = 0; i < acc_count - 1; i++)
        for (j = 0; j < acc_count - i - 1; j++)
            if (accounts[j].balance > accounts[j+1].balance) {
                temp = accounts[j];
                accounts[j] = accounts[j+1];
                accounts[j+1] = temp;
            }
}

void createAccount() {
    if (acc_count >= MAX_USERS) { printf("Max accounts reached!\n"); return; }

    accounts = (Account *)realloc(accounts, (acc_count + 1) * sizeof(Account));
    Account *a = &accounts[acc_count];

    a->id = acc_count + 1001;
    printf("Enter name     : "); scanf(" %[^\n]", a->name);
    printf("Enter username : "); scanf("%s", a->username);

    if (!isUsernameUnique(a->username)) {
        printf("Username already exists!\n"); return;
    }

    printf("Enter password : "); scanf("%s", a->password);
    if (strlen(a->password) < 4) { printf("Password too short (min 4 chars)!\n"); return; }

    printf("Initial deposit: "); scanf("%lf", &a->balance);
    if (a->balance < 0) { printf("Balance cannot be negative!\n"); return; }

    a->active = 1;
    acc_count++;
    saveAccounts();
    printf("Account created! ID: %d\n", a->id);
}

void viewAccounts() {
    int i, found = 0;
    printf("\n%-6s %-20s %-15s %-12s\n", "ID", "Name", "Username", "Balance");
    printf("-----------------------------------------------------------\n");
    for (i = 0; i < acc_count; i++) {
        if (accounts[i].active) {
            printf("%-6d %-20s %-15s %-12.2f\n",
                accounts[i].id, accounts[i].name,
                accounts[i].username, accounts[i].balance);
            found = 1;
        }
    }
    if (!found) printf("No accounts found.\n");
}

void viewSorted() {
    sortAccountsByBalance();
    printf("\n-- Accounts sorted by balance (low to high) --\n");
    viewAccounts();
    loadAccounts();
}

void updateAccount() {
    int id, idx;
    printf("Enter account ID to update: "); scanf("%d", &id);
    idx = findAccount(id);
    if (idx == -1) { printf("Account not found!\n"); return; }

    printf("New name (current: %s): ", accounts[idx].name);
    scanf(" %[^\n]", accounts[idx].name);
    printf("New password            : "); scanf("%s", accounts[idx].password);
    if (strlen(accounts[idx].password) < 4) { printf("Password too short!\n"); return; }

    saveAccounts();
    printf("Account updated.\n");
}

void deleteAccount() {
    int id, idx;
    char confirm;
    printf("Enter account ID to delete: "); scanf("%d", &id);
    idx = findAccount(id);
    if (idx == -1) { printf("Account not found!\n"); return; }

    printf("Delete %s? (y/n): ", accounts[idx].name);
    scanf(" %c", &confirm);
    if (confirm == 'y' || confirm == 'Y') {
        accounts[idx].active = 0;
        saveAccounts();
        printf("Account deleted.\n");
    }
}

void doTransaction(int acc_id) {
    int idx = findAccount(acc_id);
    if (idx == -1) { printf("Account error!\n"); return; }

    int choice;
    double amount;
    printf("\n1. Deposit\n2. Withdraw\nChoice: "); scanf("%d", &choice);
    printf("Amount: "); scanf("%lf", &amount);

    if (amount <= 0) { printf("Invalid amount!\n"); return; }
    if (choice == 2 && accounts[idx].balance < amount) { printf("Insufficient balance!\n"); return; }
    if (txn_count >= MAX_TXN) { printf("Transaction log full!\n"); return; }

    Transaction *t = &txns[txn_count];
    t->acc_id = acc_id;
    t->amount = amount;

    if (choice == 1) {
        accounts[idx].balance += amount;
        strcpy(t->type, "Deposit");
    } else {
        accounts[idx].balance -= amount;
        strcpy(t->type, "Withdraw");
    }

    t->balance_after = accounts[idx].balance;
    txn_count++;
    saveAccounts();
    saveTxns();
    printf("Transaction successful! Balance: %.2f\n", accounts[idx].balance);
}

void miniStatement(int acc_id) {
    int i, found = 0, count = 0;
    printf("\n-- Mini Statement (last 5) --\n");
    printf("%-10s %-10s %-12s\n", "Type", "Amount", "Balance After");
    printf("----------------------------------\n");
    for (i = txn_count - 1; i >= 0 && count < 5; i--) {
        if (txns[i].acc_id == acc_id) {
            printf("%-10s %-10.2f %-12.2f\n",
                txns[i].type, txns[i].amount, txns[i].balance_after);
            found = 1;
            count++;
        }
    }
    if (!found) printf("No transactions found.\n");
}

void changePassword(int acc_id) {
    int idx = findAccount(acc_id);
    char old[30], new1[30], new2[30];

    printf("Current password: "); scanf("%s", old);
    if (strcmp(accounts[idx].password, old) != 0) { printf("Incorrect password!\n"); return; }

    printf("New password    : "); scanf("%s", new1);
    printf("Confirm password: "); scanf("%s", new2);
    if (strcmp(new1, new2) != 0) { printf("Passwords do not match!\n"); return; }
    if (strlen(new1) < 4) { printf("Too short!\n"); return; }

    strcpy(accounts[idx].password, new1);
    saveAccounts();
    printf("Password changed.\n");
}

int login(int *out_idx) {
    char uname[30], pass[30];
    printf("\nUsername: "); scanf("%s", uname);
    printf("Password: "); scanf("%s", pass);

    if (strcmp(uname, ADMIN_USER) == 0 && strcmp(pass, ADMIN_PASS) == 0) {
        printf("Welcome, Admin!\n");
        return 1;
    }

    int idx = findByUsername(uname);
    if (idx != -1 && strcmp(accounts[idx].password, pass) == 0) {
        printf("Welcome, %s!\n", accounts[idx].name);
        *out_idx = idx;
        return 2;
    }

    printf("Invalid credentials!\n");
    return 0;
}

void adminMenu() {
    int ch;
    do {
        printf("\n=== ADMIN MENU ===\n");
        printf("1. Create Account\n2. View Accounts\n3. Update Account\n");
        printf("4. Delete Account\n5. View Sorted by Balance\n0. Logout\nChoice: ");
        scanf("%d", &ch);
        switch (ch) {
            case 1: createAccount(); break;
            case 2: viewAccounts();  break;
            case 3: updateAccount(); break;
            case 4: deleteAccount(); break;
            case 5: viewSorted();    break;
            case 0: printf("Logged out.\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (ch != 0);
}

void customerMenu(int acc_id, int idx) {
    int ch;
    do {
        printf("\n=== CUSTOMER MENU === (Balance: %.2f)\n", accounts[idx].balance);
        printf("1. Deposit / Withdraw\n2. Mini Statement\n3. Change Password\n0. Logout\nChoice: ");
        scanf("%d", &ch);
        switch (ch) {
            case 1: doTransaction(acc_id);   break;
            case 2: miniStatement(acc_id);   break;
            case 3: changePassword(acc_id);  break;
            case 0: printf("Logged out.\n"); break;
            default: printf("Invalid choice!\n");
        }
        idx = findAccount(acc_id);
    } while (ch != 0);
}

int main() {
    loadAccounts();
    loadTxns();

    int ch;
    printf("=============================\n");
    printf("   FinanceFlow - Bank System  \n");
    printf("=============================\n");

    do {
        printf("\n1. Login\n0. Exit\nChoice: ");
        scanf("%d", &ch);

        if (ch == 1) {
            int idx = -1;
            int role = login(&idx);
            if (role == 1) adminMenu();
            else if (role == 2) customerMenu(accounts[idx].id, idx);
        } else if (ch != 0) {
            printf("Invalid choice!\n");
        }
    } while (ch != 0);

    printf("Thank You!\n");
    free(accounts);
    return 0;
}
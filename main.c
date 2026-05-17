#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_PRODUCTS 200
#define MAX_CART_ITEMS 100
#define MAX_SALE_RULES 50
#define INVENTORY_FILE "inventory.txt"
#define RECEIPT_FILE "receipt.txt"
#define RULES_FILE "rules.txt"
#define EXPIRED_FILE "expired_inventory.txt"
#define PENDING_ORDER_FILE "pending_order.txt"

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct
{
    int ID;
    char name[50];
    float price;
    int quantity;
    char section[30];
    Date expiryDate;
} Product;

typedef struct {
    int productID;
    int buyQty; 
    int payQty; 
} SaleRule;

typedef enum { ROLE_MANAGER, ROLE_SUPERVISOR, ROLE_CASHER, ROLE_CUSTOMER } Role;

typedef struct {
    char ID[50];
    Role role;
    int itemsSold; 
    float totalRevenue; 
} Account;


typedef struct
{
    time_t timestamp;
    char sellerID[50];
    int productID;
    int quantity;
    float total;
    char section[30]; 
} SaleRecord;

// Global Data
Product inventory[MAX_PRODUCTS];
int inventoryCount = 0;

Product expiredInventory[MAX_PRODUCTS];
int expiredCount = 0;

Product cart[MAX_CART_ITEMS];
int cartCount = 0;

Account accounts[100];
int accountCount = 0;
int currentAccountIndex = -1;

SaleRule saleRules[MAX_SALE_RULES];
int ruleCount = 0;

// Function Prototypes
void managerMenu();
void supervisorMenu();
void casherMenu();
void customerMenu();
void loadInventory(Product *inv, int *count, const char *filename);
void saveInventory(Product *inv, int count, const char *filename);
void addProduct(Product *inv, int *count);
void viewStock(Product *inv, int count, const char *title);
void sellProducts(Product *inv, int invCount, Product *cart, int *cartCount);
void addToCart(Product *inv, int invCount, Product *cart, int *cartCount);
void generateReceipt(Product *cart, int cartCount, float finalTotal);
void reviewAndCheckout(Product *inv, int invCount, Product *cart, int *cartCount);
int findProductByID(Product *inv, int count, int id);
int findProductByName(Product *inv, int count, const char *name);
void clearBuffer();
void changePrice();

void loadAccounts();
void saveAccounts();
int findAccountByID(const char *id);
void registerAccount(const char *id, Role role);
void viewReports(int autoShowPastDay);
void addAccount();
void recordSale(int productID, int quantity, float total, const char *section);

void loadRules();
void saveRules();
void manageSales();
void returnProduct();
void viewAnalytics();
void viewLastBill(); // Added this
void confirmPurchase(); // Added this
void savePendingOrder(Product *cart, int count, float total); // Added this
int isExpired(Date expiry);
Date getCurrentDate();

int main()
{
    loadInventory(inventory, &inventoryCount, INVENTORY_FILE);
    loadInventory(expiredInventory, &expiredCount, EXPIRED_FILE);
    loadAccounts();
    loadRules();

    if (findAccountByID("#001") == -1)
    {
        registerAccount("#001", ROLE_MANAGER);
    }

    char userID[50];
    while (1)
    {
        printf("\n--- WELCOME TO SUPERMARKET ---\n");
        printf("Enter your ID or 'exit' to quit: ");
        if (scanf_s("%s", userID, (unsigned)_countof(userID)) != 1)
        {
            clearBuffer();
            continue;
        }

        if (_stricmp(userID, "exit") == 0)
        {
            saveInventory(inventory, inventoryCount, INVENTORY_FILE);
            saveInventory(expiredInventory, expiredCount, EXPIRED_FILE);
            saveAccounts();
            saveRules();
            printf("Exiting... Data saved.\n");
            break;
        }

        int idx = findAccountByID(userID);
        
        if (idx == -1 && userID[0] != '#') {
            registerAccount(userID, ROLE_CUSTOMER);
            idx = accountCount - 1;
        } else if (idx == -1) {
            printf("Staff ID not found. Contact the manager.\n");
            continue;
        }

        currentAccountIndex = idx;
        switch (accounts[idx].role)
        {
        case ROLE_MANAGER: managerMenu(); break;
        case ROLE_SUPERVISOR: supervisorMenu(); break;
        case ROLE_CASHER: casherMenu(); break;
        case ROLE_CUSTOMER: customerMenu(); break;
        }
    }

    return 0;
}

void customerMenu() {
    int choice;
    do
    {
        printf("\n--- CUSTOMER MENU (%s) ---\n", accounts[currentAccountIndex].ID);
        printf("1. View Available Products\n");
        printf("2. Buy Products\n");
        printf("3. Return a Product (Only if expired before purchase)\n");
        printf("4. Logout\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1) { clearBuffer(); choice = 0; continue; }

        switch (choice)
        {
        case 1: viewStock(inventory, inventoryCount, "AVAILABLE STOCK"); break;
        case 2: sellProducts(inventory, inventoryCount, cart, &cartCount); break;
        case 3: returnProduct(); break;
        case 4: break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 4);
}

void managerMenu() {
    printf("\n--- DAILY MORNING REPORT (PAST DAY) ---");
    viewReports(1);
    int choice;
    do {
        printf("\n--- MANAGER MENU (%s) ---\n", accounts[currentAccountIndex].ID);
        printf("1. Add Account\n2. Change Price\n3. View Reports\n4. View Stock\n5. Manage Sales Rules\n6. View Section Analytics\n7. View Expired Returns\n8. Logout\nChoice: ");
        if (scanf_s("%d", &choice) != 1) { clearBuffer(); choice = 0; continue; }
        switch (choice) {
        case 1: addAccount(); break;
        case 2: changePrice(); break;
        case 3: viewReports(0); break;
        case 4: viewStock(inventory, inventoryCount, "CURRENT STOCK"); break;
        case 5: manageSales(); break;
        case 6: viewAnalytics(); break;
        case 7: viewStock(expiredInventory, expiredCount, "EXPIRED RETURNS SECTION"); break;
        case 8: break;
        }
    } while (choice != 8);
}

void supervisorMenu() {
    int choice;
    do {
        printf("\n--- SUPERVISOR MENU ---\n1. Add Product\n2. View Stock\n3. Logout\nChoice: ");
        if (scanf_s("%d", &choice) != 1) { clearBuffer(); choice = 0; continue; }
        switch (choice) {
        case 1: addProduct(inventory, &inventoryCount); break;
        case 2: viewStock(inventory, inventoryCount, "STOCK"); break;
        }
    } while (choice != 3);
}

void casherMenu() {
    int choice;
    do {
        printf("\n--- CASHER MENU ---\n1. Sell Products\n2. View Stock\n3. View Last Generated Bill\n4. Confirm Purchase & Print Bill\n5. Logout\nChoice: ");
        if (scanf_s("%d", &choice) != 1) { clearBuffer(); choice = 0; continue; }
        switch (choice) {
        case 1: sellProducts(inventory, inventoryCount, cart, &cartCount); break;
        case 2: viewStock(inventory, inventoryCount, "STOCK"); break;
        case 3: viewLastBill(); break;
        case 4: confirmPurchase(); break;
        }
    } while (choice != 5);
}

void viewLastBill() {
    FILE *f = NULL; fopen_s(&f, RECEIPT_FILE, "r");
    if (f == NULL) {
        printf("No pending or previous bills found.\n");
        return;
    }
    char line[100];
    printf("\n--- BILL ON SCREEN FOR CASHIER ---\n");
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    fclose(f);
    printf("----------------------------------\n");
    system("pause");
}

void loadInventory(Product *inv, int *count, const char *filename) {
    FILE *file = NULL; fopen_s(&file, filename, "r");
    if (file == NULL) { *count = 0; return; }
    *count = 0;
    while (*count < MAX_PRODUCTS && fscanf_s(file, "%d %s %f %d %s %d %d %d",
        &inv[*count].ID, inv[*count].name, (unsigned)_countof(inv[*count].name),
        &inv[*count].price, &inv[*count].quantity,
        inv[*count].section, (unsigned)_countof(inv[*count].section),
        &inv[*count].expiryDate.day, &inv[*count].expiryDate.month, &inv[*count].expiryDate.year) == 8) {
        (*count)++;
    }
    fclose(file);
}

void saveInventory(Product *inv, int count, const char *filename) {
    FILE *file = NULL; fopen_s(&file, filename, "w");
    if (file == NULL) return;
    for (int i = 0; i < count; i++) {
        fprintf_s(file, "%d %s %.2f %d %s %d %d %d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity, inv[i].section, inv[i].expiryDate.day, inv[i].expiryDate.month, inv[i].expiryDate.year);
    }
    fclose(file);
}

void loadAccounts() {
    FILE *file = NULL; fopen_s(&file, "accounts.txt", "r");
    if (file == NULL) { accountCount = 0; return; }
    accountCount = 0; int roleInt;
    while (accountCount < 100 && fscanf_s(file, "%s %d %d %f", accounts[accountCount].ID, (unsigned)_countof(accounts[accountCount].ID), &roleInt, &accounts[accountCount].itemsSold, &accounts[accountCount].totalRevenue) == 4) {
        accounts[accountCount].role = (Role)roleInt; accountCount++;
    }
    fclose(file);
}

void saveAccounts() {
    FILE *file = NULL; fopen_s(&file, "accounts.txt", "w");
    if (file == NULL) return;
    for (int i = 0; i < accountCount; i++) fprintf_s(file, "%s %d %d %.2f\n", accounts[i].ID, (int)accounts[i].role, accounts[i].itemsSold, accounts[i].totalRevenue);
    fclose(file);
}

int findAccountByID(const char *id) {
    for (int i = 0; i < accountCount; i++) if (strcmp(accounts[i].ID, id) == 0) return i;
    return -1;
}

void registerAccount(const char *id, Role role) {
    if (accountCount < 100) {
        strcpy_s(accounts[accountCount].ID, _countof(accounts[accountCount].ID), id);
        accounts[accountCount].role = role; accounts[accountCount].itemsSold = 0; accounts[accountCount].totalRevenue = 0.0f;
        accountCount++; saveAccounts();
    }
}

void clearBuffer() { int c; while ((c = getchar()) != '\n' && c != EOF); }

void recordSale(int productID, int quantity, float total, const char *section) {
    FILE *file = NULL; fopen_s(&file, "sales.txt", "a");
    if (file == NULL) return;
    time_t now = time(NULL);
    fprintf_s(file, "%lld %s %d %d %.2f %s\n", (long long)now, accounts[currentAccountIndex].ID, productID, quantity, total, section);
    fclose(file);
}

int findProductByID(Product *inv, int count, int id) {
    for (int i = 0; i < count; i++) if (inv[i].ID == id) return i;
    return -1;
}

int findProductByName(Product *inv, int count, const char *name) {
    for (int i = 0; i < count; i++) if (_stricmp(inv[i].name, name) == 0) return i;
    return -1;
}

Date getCurrentDate() {
    time_t t = time(NULL); struct tm tm_info; localtime_s(&tm_info, &t);
    Date d = { tm_info.tm_mday, tm_info.tm_mon + 1, tm_info.tm_year + 1900 }; return d;
}

int isExpired(Date expiry) {
    Date cur = getCurrentDate();
    if (cur.year > expiry.year) return 1;
    if (cur.year == expiry.year && cur.month > expiry.month) return 1;
    if (cur.year == expiry.year && cur.month == expiry.month && cur.day > expiry.day) return 1;
    return 0;
}

void addProduct(Product *inv, int *count) {
    if (*count >= MAX_PRODUCTS) return;
    Product n; printf("ID: "); scanf_s("%d", &n.ID);
    if (findProductByID(inv, *count, n.ID) != -1) { printf("Exists!\n"); return; }
    printf("Name: "); scanf_s("%s", n.name, (unsigned)_countof(n.name));
    printf("Price: "); scanf_s("%f", &n.price);
    printf("Qty: "); scanf_s("%d", &n.quantity);
    printf("Section: "); scanf_s("%s", n.section, (unsigned)_countof(n.section));
    printf("Expiry (DD MM YYYY): "); scanf_s("%d %d %d", &n.expiryDate.day, &n.expiryDate.month, &n.expiryDate.year);
    inv[*count] = n; (*count)++; saveInventory(inv, *count, INVENTORY_FILE);
}

void viewStock(Product *inv, int count, const char *title) {
    printf("\n--- %s ---\n%-5s %-20s %-10s %-10s %-15s %-10s\n", title, "ID", "Name", "Price", "Qty", "Section", "Expiry");
    for (int i = 0; i < count; i++) printf("%-5d %-20s %-10.2f %-10d %-15s %02d/%02d/%d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity, inv[i].section, inv[i].expiryDate.day, inv[i].expiryDate.month, inv[i].expiryDate.year);
}

void changePrice() {
    int id; float np; printf("ID: "); scanf_s("%d", &id);
    int idx = findProductByID(inventory, inventoryCount, id);
    if (idx != -1) { printf("New Price: "); scanf_s("%f", &np); inventory[idx].price = np; saveInventory(inventory, inventoryCount, INVENTORY_FILE); }
}

void addAccount() {
    char id[50]; int r; printf("New ID: "); scanf_s("%s", id, 50);
    if (findAccountByID(id) != -1) return;
    printf("Role (1:Sup, 2:Cash): "); scanf_s("%d", &r);
    registerAccount(id, (r == 1) ? ROLE_SUPERVISOR : ROLE_CASHER);
}

void viewReports(int autoDay) {
    FILE *f = NULL; fopen_s(&f, "sales.txt", "r"); if (f == NULL) return;
    time_t now = time(NULL); float dTot = 0, wTot = 0, mTot = 0; SaleRecord s;
    while (fscanf_s(f, "%lld %s %d %d %f %s", &s.timestamp, s.sellerID, 50, &s.productID, &s.quantity, &s.total, s.section, 30) == 6) {
        double diff = difftime(now, s.timestamp);
        if (diff <= 86400) dTot += s.total;
        if (diff <= 604800) wTot += s.total;
        if (diff <= 2592000) mTot += s.total;
        if (!autoDay) printf("%lld | %s | %d | %.2f | %s\n", s.timestamp, s.sellerID, s.productID, s.total, s.section);
    }
    fclose(f);
    printf("\nSummary: Day: %.2f | Week: %.2f | Month: %.2f\n", dTot, wTot, mTot);
}

void sellProducts(Product *inv, int invCount, Product *cart, int *cartCount) {
    int choice; *cartCount = 0;
    do {
        printf("\n1. View Stock\n2. Add to Cart (ID or Name)\n3. Checkout\n4. Back\nChoice: ");
        if (scanf_s("%d", &choice) != 1) { clearBuffer(); choice = 0; continue; }
        if (choice == 1) viewStock(inv, invCount, "STOCK");
        else if (choice == 2) addToCart(inv, invCount, cart, cartCount);
        else if (choice == 3 && *cartCount > 0) reviewAndCheckout(inv, invCount, cart, cartCount);
    } while (choice != 4);
}

void addToCart(Product *inv, int invCount, Product *cart, int *cartCount) {
    if (*cartCount >= MAX_CART_ITEMS) { printf("Cart is full!\n"); return; }
    char input[50]; int qty, idx = -1;
    printf("Enter Product ID or Name: "); scanf_s("%s", input, 50);
    if (input[0] >= '0' && input[0] <= '9') idx = findProductByID(inv, invCount, atoi(input));
    if (idx == -1) idx = findProductByName(inv, invCount, input);
    if (idx == -1) { printf("Not found!\n"); clearBuffer(); return; }
    printf("Quantity: "); if (scanf_s("%d", &qty) != 1) { clearBuffer(); return; }
    if (qty <= 0) { printf("Invalid quantity!\n"); return; }
    if (qty > inv[idx].quantity) { printf("Shortage! (Available: %d)\n", inv[idx].quantity); return; }
    cart[*cartCount] = inv[idx]; cart[*cartCount].quantity = qty; (*cartCount)++;
    inv[idx].quantity -= qty; printf("Added %d %s to cart.\n", qty, inv[idx].name);
    clearBuffer();
}

void reviewAndCheckout(Product *inv, int invCount, Product *cart, int *cartCount) {
    int choice;
    do {
        float finalTotal = 0;
        int totalItemsInCart = 0;
        printf("\n--- CART REVIEW ---\n%-5s %-20s %-10s %-10s %-10s\n", "ID", "Item", "Price", "Qty", "Subtotal");
        printf("------------------------------------------------------------\n");
        for (int i = 0; i < *cartCount; i++) {
            int payQty = cart[i].quantity;
            for (int j = 0; j < ruleCount; j++) {
                if (saleRules[j].productID == cart[i].ID) {
                    int sets = cart[i].quantity / saleRules[j].buyQty;
                    int remainder = cart[i].quantity % saleRules[j].buyQty;
                    payQty = (sets * saleRules[j].payQty) + remainder;
                    break;
                }
            }
            float sub = payQty * cart[i].price;
            finalTotal += sub;
            totalItemsInCart += cart[i].quantity;
            printf("%-5d %-20s %-10.2f %-10d %-10.2f\n", cart[i].ID, cart[i].name, cart[i].price, cart[i].quantity, sub);
        }
        printf("------------------------------------------------------------\n");
        printf("TOTAL TO PAY: %.2f\n", finalTotal);

        printf("\n1. Confirm Order & Proceed to Cashier\n2. Change Quantity / Remove Item\n3. Back to Shopping\nChoice: ");
        if (scanf_s("%d", &choice) != 1) { clearBuffer(); choice = 0; continue; }

        if (choice == 1) {
            savePendingOrder(cart, *cartCount, finalTotal);
            generateReceipt(cart, *cartCount, finalTotal);
            *cartCount = 0;
            saveInventory(inv, invCount, INVENTORY_FILE);
            printf("Order confirmed! Please proceed to the cashier for payment.\n");
            printf("The cashier will be able to see your bill on their screen.\n");
            system("pause");
            return;
        } else if (choice == 2) {
            int id, newQty;
            printf("Enter Item ID to change/remove: ");
            if (scanf_s("%d", &id) != 1) { clearBuffer(); continue; }
            int cIdx = -1;
            for (int i = 0; i < *cartCount; i++) if (cart[i].ID == id) { cIdx = i; break; }
            if (cIdx == -1) { printf("Item not in cart!\n"); continue; }
            
            printf("Current Qty: %d. Enter New Qty (0 to remove): ", cart[cIdx].quantity);
            if (scanf_s("%d", &newQty) != 1 || newQty < 0) { clearBuffer(); continue; }
            
            int diff = newQty - cart[cIdx].quantity;
            int iIdx = findProductByID(inv, invCount, id);
            if (diff > 0 && diff > inv[iIdx].quantity) { printf("Not enough stock available!\n"); continue; }
            
            inv[iIdx].quantity -= diff; // Restore or subtract stock
            if (newQty == 0) {
                for (int i = cIdx; i < *cartCount - 1; i++) cart[i] = cart[i+1];
                (*cartCount)--;
                printf("Item removed from cart.\n");
                if (*cartCount == 0) return;
            } else {
                cart[cIdx].quantity = newQty;
                printf("Quantity updated.\n");
            }
        }
    } while (choice != 3);
}

void generateReceipt(Product *cart, int cartCount, float finalTotal) {
    FILE *f = NULL; fopen_s(&f, RECEIPT_FILE, "w"); if (f == NULL) return;
    fprintf(f, "--- RECEIPT ---\n");
    for (int i = 0; i < cartCount; i++) {
        fprintf(f, "%-20s x%d\n", cart[i].name, cart[i].quantity);
    }
    fprintf(f, "---------------------------\n");
    fprintf(f, "TOTAL TO PAY: %.2f\n", finalTotal);
    fprintf(f, "---------------------------\n");
    fclose(f);
}

void returnProduct() {
    int id, qty; printf("Enter Product ID to return: "); 
    if (scanf_s("%d", &id) != 1) { clearBuffer(); return; }
    int idx = findProductByID(inventory, inventoryCount, id);
    if (idx == -1) { printf("Invalid product!\n"); clearBuffer(); return; }
    if (!isExpired(inventory[idx].expiryDate)) { 
        printf("Return denied: Item is not expired.\n"); 
        clearBuffer(); return; 
    }
    printf("Quantity to return: "); if (scanf_s("%d", &qty) != 1) { clearBuffer(); return; }
    if (qty <= 0) { printf("Invalid quantity!\n"); return; }
    
    if (expiredCount < MAX_PRODUCTS) {
        expiredInventory[expiredCount] = inventory[idx];
        expiredInventory[expiredCount].quantity = qty;
        expiredCount++;
        saveInventory(expiredInventory, expiredCount, EXPIRED_FILE);
        printf("Return successful. Item moved to Expiration Section.\n");
    } else {
        printf("Expiration section is full!\n");
    }
    clearBuffer();
}

void viewAnalytics() {
    FILE *f = NULL; fopen_s(&f, "sales.txt", "r"); if (f == NULL) return;
    char sections[5][30] = {"drinks", "soda", "Legumes", "Moisturizers", "Makeup"};
    int counts[5] = {0,0,0,0,0}; SaleRecord s;
    while (fscanf_s(f, "%lld %s %d %d %f %s", &s.timestamp, s.sellerID, 50, &s.productID, &s.quantity, &s.total, s.section, 30) == 6) {
        for (int i=0; i<5; i++) if (_stricmp(s.section, sections[i]) == 0) counts[i] += s.quantity;
    }
    fclose(f);
    printf("\n--- SECTION ANALYTICS ---\n");
    int maxIdx = 0;
    for (int i=0; i<5; i++) {
        printf("%s: %d items sold\n", sections[i], counts[i]);
        if (counts[i] > counts[maxIdx]) maxIdx = i;
    }
    if (counts[maxIdx] > 0) printf("\nMOST WANTED SECTION: %s (%d items sold)\n", sections[maxIdx], counts[maxIdx]);
    else printf("\nNo sales data available to determine most wanted section.\n");
}

void manageSales() {
    int choice; printf("\n1. Add Rule\n2. View\nChoice: "); scanf_s("%d", &choice);
    if (choice == 1) {
        printf("PID: "); scanf_s("%d", &saleRules[ruleCount].productID);
        printf("Buy Qty: "); scanf_s("%d", &saleRules[ruleCount].buyQty);
        printf("Pay Qty: "); scanf_s("%d", &saleRules[ruleCount].payQty);
        ruleCount++; saveRules();
    } else {
        for (int i=0; i<ruleCount; i++) printf("PID %d: Buy %d Pay %d\n", saleRules[i].productID, saleRules[i].buyQty, saleRules[i].payQty);
    }
}

void loadRules() {
    FILE *f = NULL; fopen_s(&f, RULES_FILE, "r"); if (f == NULL) return;
    ruleCount = 0;
    while (ruleCount < MAX_SALE_RULES && fscanf_s(f, "%d %d %d", &saleRules[ruleCount].productID, &saleRules[ruleCount].buyQty, &saleRules[ruleCount].payQty) == 3) ruleCount++;
    fclose(f);
}

void savePendingOrder(Product *cart, int count, float total) {
    FILE *f = NULL; fopen_s(&f, PENDING_ORDER_FILE, "w"); if (f == NULL) return;
    fprintf(f, "%d %.2f\n", count, total);
    for (int i = 0; i < count; i++) {
        fprintf(f, "%d %s %.2f %d %s\n", cart[i].ID, cart[i].name, cart[i].price, cart[i].quantity, cart[i].section);
    }
    fclose(f);
}

void confirmPurchase() {
    FILE *f = NULL; fopen_s(&f, PENDING_ORDER_FILE, "r");
    if (f == NULL) { printf("No pending orders to confirm.\n"); return; }
    
    int count; float total;
    if (fscanf_s(f, "%d %f", &count, &total) != 2) { fclose(f); return; }
    
    Product tempCart[MAX_CART_ITEMS];
    for (int i = 0; i < count; i++) {
        fscanf_s(f, "%d %s %f %d %s", &tempCart[i].ID, tempCart[i].name, (unsigned)50, &tempCart[i].price, &tempCart[i].quantity, tempCart[i].section, (unsigned)30);
    }
    fclose(f);
    
    // Process Sales
    int totalItems = 0;
    for (int i = 0; i < count; i++) {
        int payQty = tempCart[i].quantity;
        for (int j = 0; j < ruleCount; j++) {
            if (saleRules[j].productID == tempCart[i].ID) {
                int sets = tempCart[i].quantity / saleRules[j].buyQty;
                int remainder = tempCart[i].quantity % saleRules[j].buyQty;
                payQty = (sets * saleRules[j].payQty) + remainder;
                break;
            }
        }
        recordSale(tempCart[i].ID, tempCart[i].quantity, payQty * tempCart[i].price, tempCart[i].section);
        totalItems += tempCart[i].quantity;
    }
    
    // Update Cashier Account
    if (currentAccountIndex != -1) {
        accounts[currentAccountIndex].itemsSold += totalItems;
        accounts[currentAccountIndex].totalRevenue += total;
        saveAccounts();
    }
    
    // Generate Final Receipt (Same as receipt.txt but maybe marked as paid)
    generateReceipt(tempCart, count, total);
    
    // Remove pending order
    _unlink(PENDING_ORDER_FILE);
    
    printf("\n--- PURCHASE CONFIRMED & PAID ---\n");
    printf("Total Collected: %.2f\n", total);
    printf("Final Receipt Printed to 'receipt.txt'.\n");
    system("pause");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_PRODUCTS 100
#define MAX_CART_ITEMS 100
#define INVENTORY_FILE "inventory.txt"
#define RECEIPT_FILE "receipt.txt"

typedef struct
{
    int ID;
    char name[50];
    float price;
    int quantity;
} Product;

typedef enum { ROLE_MANAGER, ROLE_SUPERVISOR, ROLE_CASHER, ROLE_CUSTOMER } Role;

typedef struct {
    char ID[50];
    Role role;
    int itemsSold; // For customers, this is itemsBought
    float totalRevenue; // For customers, this is totalSpent
} Account;


typedef struct
{
    time_t timestamp;
    char sellerID[50];
    int productID;
    int quantity;
    float total;
} SaleRecord;

// Global Inventory, Cart, and Accounts
Product inventory[MAX_PRODUCTS];
int inventoryCount = 0;

Product cart[MAX_CART_ITEMS];
int cartCount = 0;

Account accounts[100];
int accountCount = 0;
int currentAccountIndex = -1; // -1 if guest/customer

// Function Prototypes
void managerMenu();
void supervisorMenu();
void casherMenu();
void loadInventory(Product *inv, int *count);
void saveInventory(Product *inv, int count);
void addProduct(Product *inv, int *count);
void viewStock(Product *inv, int count);
void sellProducts(Product *inv, int invCount, Product *cart, int *cartCount);
void addToCart(Product *inv, int invCount, Product *cart, int *cartCount);
void generateReceipt(Product *cart, int cartCount);
int findProductByID(Product *inv, int count, int id);
void clearBuffer();
void changePrice();

// New Prototypes
void loadAccounts();
void saveAccounts();
int findAccountByID(const char *id);
void registerAccount(const char *id, Role role);
void viewReports(int autoShowPastDay);
void addAccount();
void recordSale(int productID, int quantity, float total);

void customerMenu();

int main()
{
    loadInventory(inventory, &inventoryCount);
    loadAccounts();

    // Ensure manager #001 exists
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
            saveInventory(inventory, inventoryCount);
            saveAccounts();
            printf("Exiting... Data saved.\n");
            break;
        }

        int idx = findAccountByID(userID);
        
        // If ID not found and does not start with #, register as customer
        if (idx == -1 && userID[0] != '#') {
            registerAccount(userID, ROLE_CUSTOMER);
            idx = accountCount - 1;
        } else if (idx == -1) {
            printf("Staff ID not found. Please contact the manager (#001).\n");
            continue;
        }

        currentAccountIndex = idx;
        switch (accounts[idx].role)
        {
        case ROLE_MANAGER:
            managerMenu();
            break;
        case ROLE_SUPERVISOR:
            supervisorMenu();
            break;
        case ROLE_CASHER:
            casherMenu();
            break;
        case ROLE_CUSTOMER:
            customerMenu();
            break;
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
        printf("3. Logout\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1)
        {
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1:
            viewStock(inventory, inventoryCount);
            break;
        case 2:
            sellProducts(inventory, inventoryCount, cart, &cartCount);
            break;
        case 3:
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 3);
}

void loadAccounts()
{
    FILE *file = NULL;
    fopen_s(&file, "accounts.txt", "r");
    if (file == NULL)
    {
        accountCount = 0;
        return;
    }

    accountCount = 0;
    int roleInt;
    while (accountCount < 100 && fscanf_s(file, "%s %d %d %f",
                                          accounts[accountCount].ID, (unsigned)_countof(accounts[accountCount].ID),
                                          &roleInt,
                                          &accounts[accountCount].itemsSold, &accounts[accountCount].totalRevenue) == 4)
    {
        accounts[accountCount].role = (Role)roleInt;
        accountCount++;
    }
    fclose(file);
}

void saveAccounts()
{
    FILE *file = NULL;
    fopen_s(&file, "accounts.txt", "w");
    if (file == NULL)
        return;

    for (int i = 0; i < accountCount; i++)
    {
        fprintf_s(file, "%s %d %d %.2f\n", accounts[i].ID, (int)accounts[i].role, accounts[i].itemsSold, accounts[i].totalRevenue);
    }
    fclose(file);
}

void registerAccount(const char *id, Role role)
{
    if (accountCount < 100)
    {
        strcpy_s(accounts[accountCount].ID, _countof(accounts[accountCount].ID), id);
        accounts[accountCount].role = role;
        accounts[accountCount].itemsSold = 0;
        accounts[accountCount].totalRevenue = 0.0f;
        accountCount++;
        saveAccounts();
        printf("Account %s registered successfully.\n", id);
    }
    else
    {
        printf("Account database full!\n");
    }
}

void clearBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void loadInventory(Product *inv, int *count)
{
    FILE *file = NULL;
    errno_t err = fopen_s(&file, INVENTORY_FILE, "r");

    if (err != 0 || file == NULL)
    {
        printf("Inventory file not found or could not be opened. Starting with empty inventory.\n");
        *count = 0;
        return;
    }

    *count = 0;
    while (*count < MAX_PRODUCTS && fscanf_s(file, "%d %s %f %d",
                                             &inv[*count].ID, inv[*count].name, (unsigned)_countof(inv[*count].name),
                                             &inv[*count].price, &inv[*count].quantity) == 4)
    {
        (*count)++;
    }

    fclose(file);
    printf("Inventory loaded successfully. (%d items)\n", *count);
}

void saveInventory(Product *inv, int count)
{
    FILE *file = NULL;
    errno_t err = fopen_s(&file, INVENTORY_FILE, "w");

    if (err != 0 || file == NULL)
    {
        printf("Error: Could not open inventory file for writing.\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        fprintf_s(file, "%d %s %.2f %d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity);
    }

    fclose(file);
}

void recordSale(int productID, int quantity, float total)
{
    FILE *file = NULL;
    fopen_s(&file, "sales.txt", "a");
    if (file == NULL)
        return;

    time_t now = time(NULL);
    fprintf_s(file, "%lld %s %d %d %.2f\n",
              (long long)now,
              accounts[currentAccountIndex].ID,
              productID, quantity, total);
    fclose(file);
}

void managerMenu()
{
    // Auto-show past day report as requested
    printf("\n--- DAILY MORNING REPORT (PAST DAY) ---");
    viewReports(1);

    int choice;
    do
    {
        printf("\n--- MANAGER MENU (%s) ---\n", accounts[currentAccountIndex].ID);
        printf("1. Add Account (Supervisor/Casher)\n");
        printf("2. Change Product Price\n");
        printf("3. View Full Reports (Daily/Weekly/Monthly)\n");
        printf("4. View Stock\n");
        printf("5. Logout\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1)
        {
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1:
            addAccount();
            break;
        case 2:
            changePrice();
            break;
        case 3:
            viewReports(0);
            break;
        case 4:
            viewStock(inventory, inventoryCount);
            break;
        case 5:
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 5);
}

void supervisorMenu()
{
    int choice;
    do
    {
        printf("\n--- SUPERVISOR MENU (%s) ---\n", accounts[currentAccountIndex].ID);
        printf("1. Add New Product\n");
        printf("2. View Stock\n");
        printf("3. Logout\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1)
        {
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1:
            addProduct(inventory, &inventoryCount);
            break;
        case 2:
            viewStock(inventory, inventoryCount);
            break;
        case 3:
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 3);
}

void casherMenu()
{
    int choice;
    do
    {
        printf("\n--- CASHER MENU (%s) ---\n", accounts[currentAccountIndex].ID);
        printf("1. Sell Products\n");
        printf("2. View Stock\n");
        printf("3. Logout\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1)
        {
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1:
            sellProducts(inventory, inventoryCount, cart, &cartCount);
            break;
        case 2:
            viewStock(inventory, inventoryCount);
            break;
        case 3:
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 3);
}

void changePrice()
{
    int id;
    float newPrice;
    printf("Enter Product ID to change price: ");
    if (scanf_s("%d", &id) != 1)
    {
        clearBuffer();
        return;
    }

    int idx = findProductByID(inventory, inventoryCount, id);
    if (idx == -1)
    {
        printf("Product not found!\n");
        return;
    }

    printf("Current Price: %.2f. Enter New Price: ", inventory[idx].price);
    if (scanf_s("%f", &newPrice) != 1 || newPrice < 0)
    {
        printf("Invalid price! Price cannot be negative.\n");
        clearBuffer();
        return;
    }

    inventory[idx].price = newPrice;
    saveInventory(inventory, inventoryCount);
    printf("Price updated successfully.\n");
}

void addAccount()
{
    char id[50];
    int roleChoice;
    printf("Enter New Account ID: ");
    scanf_s("%s", id, (unsigned)_countof(id));

    if (findAccountByID(id) != -1)
    {
        printf("Account ID already exists!\n");
        return;
    }

    printf("Select Role (1: Supervisor, 2: Casher): ");
    if (scanf_s("%d", &roleChoice) != 1 || (roleChoice != 1 && roleChoice != 2))
    {
        printf("Invalid role selection!\n");
        clearBuffer();
        return;
    }

    Role role = (roleChoice == 1) ? ROLE_SUPERVISOR : ROLE_CASHER;
    registerAccount(id, role);
}

void viewReports(int autoShowPastDay)
{
    FILE *file = NULL;
    fopen_s(&file, "sales.txt", "r");
    if (file == NULL)
    {
        printf("\nNo sales records found.\n");
        return;
    }

    time_t now = time(NULL);
    time_t oneDay = 24 * 3600;
    time_t sevenDays = 7 * oneDay;
    time_t thirtyDays = 30 * oneDay;

    float pastDayTotal = 0, weekTotal = 0, monthTotal = 0;
    int pastDayQty = 0;

    typedef struct
    {
        char id[50];
        float revenue;
    } SellerStats;
    SellerStats sellerStats[100];
    int sellerCount = 0;

    SaleRecord s;
    if (!autoShowPastDay)
    {
        printf("\n--- FULL SALES REPORTS ---\n");
        printf("%-20s %-15s %-10s %-10s %-10s\n", "Timestamp", "Seller", "PID", "Qty", "Total");
        printf("----------------------------------------------------------------------\n");
    }

    while (fscanf_s(file, "%lld %s %d %d %f",
                    &s.timestamp, s.sellerID, (unsigned)_countof(s.sellerID),
                    &s.productID, &s.quantity, &s.total) == 5)
    {

        double diff = difftime(now, s.timestamp);

        // Past Day (Yesterday)
        if (diff <= oneDay)
        {
            pastDayTotal += s.total;
            pastDayQty += s.quantity;
        }
        // Weekly (7 days)
        if (diff <= sevenDays)
        {
            weekTotal += s.total;
        }
        // Monthly (30 days)
        if (diff <= thirtyDays)
        {
            monthTotal += s.total;
        }

        // Track seller stats for performance
        int found = -1;
        for (int i = 0; i < sellerCount; i++)
        {
            if (strcmp(sellerStats[i].id, s.sellerID) == 0)
            {
                found = i;
                break;
            }
        }
        if (found != -1)
        {
            sellerStats[found].revenue += s.total;
        }
        else if (sellerCount < 100)
        {
            strcpy_s(sellerStats[sellerCount].id, _countof(sellerStats[sellerCount].id), s.sellerID);
            sellerStats[sellerCount].revenue = s.total;
            sellerCount++;
        }

        if (!autoShowPastDay)
        {
            struct tm timeinfo;
            localtime_s(&timeinfo, &s.timestamp);
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%d/%m %H:%M", &timeinfo);
            printf("%-20s %-15s %-10d %-10d %-10.2f\n", timeStr, s.sellerID, s.productID, s.quantity, s.total);
        }
    }
    fclose(file);

    if (autoShowPastDay)
    {
        printf("\n--- YESTERDAY'S SALES SUMMARY ---");
        printf("\nTotal Revenue: %.2f", pastDayTotal);
        printf("\nTotal Quantity Sold: %d", pastDayQty);
        printf("\n----------------------------------\n");
    }
    else
    {
        printf("\n--- REVENUE SUMMARY ---");
        printf("\nPast 24 Hours: %.2f", pastDayTotal);
        printf("\nPast 7 Days:   %.2f", weekTotal);
        printf("\nPast 30 Days:  %.2f", monthTotal);

        printf("\n\n--- SELLER PERFORMANCE (ALL TIME) ---");
        printf("\n%-20s %-10s", "Seller ID", "Revenue");
        for (int i = 0; i < sellerCount; i++)
        {
            printf("\n%-20s %-10.2f", sellerStats[i].id, sellerStats[i].revenue);
        }
        printf("\n");
    }
}

void sellProducts(Product *inv, int invCount, Product *cart, int *cartCount)
{
    int choice;
    *cartCount = 0; // Reset cart for new customer
    do
    {
        printf("\n--- SALES MODE ---\n");
        printf("1. View Available Products\n");
        printf("2. Add to Cart\n");
        printf("3. Checkout & Generate Receipt\n");
        printf("4. Return to Menu\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1)
        {
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1:
            viewStock(inv, invCount);
            break;
        case 2:
            addToCart(inv, invCount, cart, cartCount);
            break;
        case 3:
            if (*cartCount > 0)
            {
                generateReceipt(cart, *cartCount);
                saveInventory(inv, invCount);
                return;
            }
            else
            {
                printf("Cart is empty!\n");
            }
            break;
        case 4:
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 4);
}

void addToCart(Product *inv, int invCount, Product *cart, int *cartCount)
{
    int id, qty;
    printf("Enter Product ID to add: ");
    if (scanf_s("%d", &id) != 1)
    {
        printf("Invalid ID!\n");
        clearBuffer();
        return;
    }

    int invIdx = findProductByID(inv, invCount, id);
    if (invIdx == -1)
    {
        printf("Product not found!\n");
        return;
    }

    printf("Enter Quantity: ");
    if (scanf_s("%d", &qty) != 1 || qty <= 0)
    {
        printf("Invalid quantity!\n");
        clearBuffer();
        return;
    }

    if (qty > inv[invIdx].quantity)
    {
        printf("Not enough stock! (Available: %d)\n", inv[invIdx].quantity);
        return;
    }

    // Check if item already in cart
    int cartIdx = -1;
    for (int i = 0; i < *cartCount; i++)
    {
        if (cart[i].ID == id)
        {
            cartIdx = i;
            break;
        }
    }

    if (cartIdx != -1)
    {
        cart[cartIdx].quantity += qty;
    }
    else
    {
        if (*cartCount >= MAX_CART_ITEMS)
        {
            printf("Cart is full!\n");
            return;
        }
        cart[*cartCount] = inv[invIdx];
        cart[*cartCount].quantity = qty;
        (*cartCount)++;
    }

    inv[invIdx].quantity -= qty;
    printf("Added %d %s to cart.\n", qty, inv[invIdx].name);
}

void generateReceipt(Product *cart, int cartCount)
{
    FILE *file = NULL;
    errno_t err = fopen_s(&file, RECEIPT_FILE, "w");

    if (err != 0 || file == NULL)
    {
        printf("Error: Could not generate receipt file.\n");
        return;
    }

    float total = 0;
    int totalItems = 0;
    printf("\n--- FORMAL RECEIPT ---\n");
    printf("%-20s %-10s %-10s %-10s\n", "Item", "Price", "Qty", "Subtotal");
    printf("----------------------------------------------------\n");
    fprintf_s(file, "--- SUPERMARKET RECEIPT ---\n");
    fprintf_s(file, "%-20s %-10s %-10s %-10s\n", "Item", "Price", "Qty", "Subtotal");
    fprintf_s(file, "----------------------------------------------------\n");

    for (int i = 0; i < cartCount; i++)
    {
        float subtotal = cart[i].price * cart[i].quantity;
        total += subtotal;
        totalItems += cart[i].quantity;
        printf("%-20s %-10.2f %-10d %-10.2f\n", cart[i].name, cart[i].price, cart[i].quantity, subtotal);
        fprintf_s(file, "%-20s %-10.2f %-10d %-10.2f\n", cart[i].name, cart[i].price, cart[i].quantity, subtotal);

        // Record each item sale
        recordSale(cart[i].ID, cart[i].quantity, subtotal);
    }

    printf("----------------------------------------------------\n");
    printf("TOTAL AMOUNT: %.2f\n", total);
    printf("----------------------------------------------------\n");
    fprintf_s(file, "----------------------------------------------------\n");
    fprintf(file, "TOTAL AMOUNT: %.2f\n", total);

    // Credit sales to manager if one is logged in
    if (currentAccountIndex != -1)
    {
        accounts[currentAccountIndex].itemsSold += totalItems;
        accounts[currentAccountIndex].totalRevenue += total;
        saveAccounts();
    }

    fclose(file);
    printf("Receipt saved to %s\n", RECEIPT_FILE);
}

int findProductByID(Product *inv, int count, int id)
{
    for (int i = 0; i < count; i++)
    {
        if (inv[i].ID == id)
            return i;
    }
    return -1;
}

int findAccountByID(const char *id)
{
    for (int i = 0; i < accountCount; i++)
    {
        if (strcmp(accounts[i].ID, id) == 0)
            return i;
    }
    return -1;
}

void addProduct(Product *inv, int *count)
{
    if (*count >= MAX_PRODUCTS)
    {
        printf("Inventory is full! Cannot add more products.\n");
        return;
    }

    Product newProduct;
    printf("Enter Product ID: ");
    if (scanf_s("%d", &newProduct.ID) != 1)
    {
        printf("Invalid ID!\n");
        clearBuffer();
        return;
    }

    if (findProductByID(inv, *count, newProduct.ID) != -1)
    {
        printf("Product ID already exists!\n");
        return;
    }

    printf("Enter Product Name: ");
    scanf_s("%s", newProduct.name, (unsigned)_countof(newProduct.name));

    printf("Enter Product Price: ");
    if (scanf_s("%f", &newProduct.price) != 1 || newProduct.price < 0)
    {
        printf("Invalid price!\n");
        clearBuffer();
        return;
    }

    printf("Enter Product Quantity: ");
    if (scanf_s("%d", &newProduct.quantity) != 1 || newProduct.quantity < 0)
    {
        printf("Invalid quantity!\n");
        clearBuffer();
        return;
    }

    inv[*count] = newProduct;
    (*count)++;
    saveInventory(inv, *count);
    printf("Product added successfully.\n");
}

void viewStock(Product *inv, int count)
{
    if (count == 0)
    {
        printf("Inventory is empty.\n");
        return;
    }

    printf("\n--- CURRENT INVENTORY ---\n");
    printf("%-5s %-20s %-10s %-10s\n", "ID", "Name", "Price", "Quantity");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < count; i++)
    {
        printf("%-5d %-20s %-10.2f %-10d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity);
    }
    printf("--------------------------------------------------\n");
}

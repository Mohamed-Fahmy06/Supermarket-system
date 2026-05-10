#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PRODUCTS 100
#define MAX_CART_ITEMS 100
#define INVENTORY_FILE "inventory.txt"
#define RECEIPT_FILE "receipt.txt"

typedef struct {
    int ID;
    char name[50];
    float price;
    int quantity;
} Product;

// Global Inventory and Cart
Product inventory[MAX_PRODUCTS];
int inventoryCount = 0;

Product cart[MAX_CART_ITEMS];
int cartCount = 0;

// Function Prototypes
void displayMenu();
void managerMode();
void customerMode();
void loadInventory(Product *inv, int *count);
void saveInventory(Product *inv, int count);
void addProduct(Product *inv, int *count);
void viewStock(Product *inv, int count);
void addToCart(Product *inv, int invCount, Product *cart, int *cartCount);
void generateReceipt(Product *cart, int cartCount);
int findProductByID(Product *inv, int count, int id);
void clearBuffer();

int main() {
    loadInventory(inventory, &inventoryCount);
    
    int choice;
    do {
        displayMenu();
        printf("Enter your choice: ");
        if (scanf_s("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1:
                managerMode();
                break;
            case 2:
                customerMode();
                break;
            case 3:
                saveInventory(inventory, inventoryCount);
                printf("Exiting... Inventory saved.\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 3);

    return 0;
}

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void loadInventory(Product *inv, int *count) {
    FILE *file = NULL;
    errno_t err = fopen_s(&file, INVENTORY_FILE, "r");
    
    if (err != 0 || file == NULL) {
        printf("Inventory file not found or could not be opened. Starting with empty inventory.\n");
        *count = 0;
        return;
    }

    *count = 0;
    // Note: fscanf_s requires buffer size for %s
    while (*count < MAX_PRODUCTS && fscanf_s(file, "%d %s %f %d", 
           &inv[*count].ID, inv[*count].name, (unsigned)_countof(inv[*count].name), 
           &inv[*count].price, &inv[*count].quantity) == 4) {
        (*count)++;
    }

    fclose(file);
    printf("Inventory loaded successfully. (%d items)\n", *count);
}

void saveInventory(Product *inv, int count) {
    FILE *file = NULL;
    errno_t err = fopen_s(&file, INVENTORY_FILE, "w");
    
    if (err != 0 || file == NULL) {
        printf("Error: Could not open inventory file for writing.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf_s(file, "%d %s %.2f %d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity);
    }

    fclose(file);
}

void displayMenu() {
    printf("\n=== SUPERMARKET MANAGEMENT SYSTEM ===\n");
    printf("1. Manager Mode\n");
    printf("2. Customer Mode\n");
    printf("3. Exit\n");
    printf("=====================================\n");
}

void managerMode() {
    int choice;
    do {
        printf("\n--- MANAGER MODE ---\n");
        printf("1. Add Product\n");
        printf("2. View Stock\n");
        printf("3. Return to Main Menu\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1: addProduct(inventory, &inventoryCount); break;
            case 2: viewStock(inventory, inventoryCount); break;
            case 3: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 3);
}

void addProduct(Product *inv, int *count) {
    if (*count >= MAX_PRODUCTS) {
        printf("Inventory full!\n");
        return;
    }

    Product p;
    printf("Enter Product ID: ");
    if (scanf_s("%d", &p.ID) != 1) {
        printf("Invalid ID!\n");
        clearBuffer();
        return;
    }
    
    if (findProductByID(inv, *count, p.ID) != -1) {
        printf("Product ID already exists!\n");
        return;
    }

    printf("Enter Name (no spaces): ");
    // scanf_s requires buffer size for %s
    scanf_s("%49s", p.name, (unsigned)_countof(p.name));
    
    printf("Enter Price: ");
    if (scanf_s("%f", &p.price) != 1 || p.price < 0) {
        printf("Invalid price!\n");
        clearBuffer();
        return;
    }

    printf("Enter Quantity: ");
    if (scanf_s("%d", &p.quantity) != 1 || p.quantity < 0) {
        printf("Invalid quantity!\n");
        clearBuffer();
        return;
    }

    inv[*count] = p;
    (*count)++;
    printf("Product added successfully.\n");
}

void viewStock(Product *inv, int count) {
    if (count == 0) {
        printf("\nInventory is empty.\n");
        return;
    }
    printf("\n%-10s %-20s %-10s %-10s\n", "ID", "Name", "Price", "Quantity");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-10d %-20s %-10.2f %-10d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity);
    }
}

void customerMode() {
    int choice;
    cartCount = 0; // Reset cart for new customer
    do {
        printf("\n--- CUSTOMER MODE ---\n");
        printf("1. View Available Products\n");
        printf("2. Add to Cart\n");
        printf("3. Checkout & Generate Receipt\n");
        printf("4. Return to Main Menu\n");
        printf("Choice: ");
        if (scanf_s("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clearBuffer();
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1: viewStock(inventory, inventoryCount); break;
            case 2: addToCart(inventory, inventoryCount, cart, &cartCount); break;
            case 3: 
                if (cartCount > 0) {
                    generateReceipt(cart, cartCount);
                    saveInventory(inventory, inventoryCount); 
                    return; 
                } else {
                    printf("Cart is empty!\n");
                }
                break;
            case 4: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 4);
}

void addToCart(Product *inv, int invCount, Product *cart, int *cartCount) {
    int id, qty;
    printf("Enter Product ID to add: ");
    if (scanf_s("%d", &id) != 1) {
        printf("Invalid ID!\n");
        clearBuffer();
        return;
    }

    int invIdx = findProductByID(inv, invCount, id);
    if (invIdx == -1) {
        printf("Product not found!\n");
        return;
    }

    printf("Enter Quantity: ");
    if (scanf_s("%d", &qty) != 1 || qty <= 0) {
        printf("Invalid quantity!\n");
        clearBuffer();
        return;
    }

    if (qty > inv[invIdx].quantity) {
        printf("Not enough stock! (Available: %d)\n", inv[invIdx].quantity);
        return;
    }

    // Check if item already in cart
    int cartIdx = -1;
    for (int i = 0; i < *cartCount; i++) {
        if (cart[i].ID == id) {
            cartIdx = i;
            break;
        }
    }

    if (cartIdx != -1) {
        cart[cartIdx].quantity += qty;
    } else {
        if (*cartCount >= MAX_CART_ITEMS) {
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

void generateReceipt(Product *cart, int cartCount) {
    FILE *file = NULL;
    errno_t err = fopen_s(&file, RECEIPT_FILE, "w");
    
    if (err != 0 || file == NULL) {
        printf("Error: Could not generate receipt file.\n");
        return;
    }

    float total = 0;
    printf("\n--- FORMAL RECEIPT ---\n");
    printf("%-20s %-10s %-10s %-10s\n", "Item", "Price", "Qty", "Subtotal");
    printf("----------------------------------------------------\n");
    fprintf_s(file, "--- SUPERMARKET RECEIPT ---\n");
    fprintf_s(file, "%-20s %-10s %-10s %-10s\n", "Item", "Price", "Qty", "Subtotal");
    fprintf_s(file, "----------------------------------------------------\n");

    for (int i = 0; i < cartCount; i++) {
        float subtotal = cart[i].price * cart[i].quantity;
        total += subtotal;
        printf("%-20s %-10.2f %-10d %-10.2f\n", cart[i].name, cart[i].price, cart[i].quantity, subtotal);
        fprintf_s(file, "%-20s %-10.2f %-10d %-10.2f\n", cart[i].name, cart[i].price, cart[i].quantity, subtotal);
    }

    printf("----------------------------------------------------\n");
    printf("TOTAL AMOUNT: %.2f\n", total);
    printf("----------------------------------------------------\n");
    fprintf_s(file, "----------------------------------------------------\n");
    fprintf(file, "TOTAL AMOUNT: %.2f\n", total);

    fclose(file);
    printf("Receipt saved to %s\n", RECEIPT_FILE);
}

int findProductByID(Product *inv, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (inv[i].ID == id) return i;
    }
    return -1;
}

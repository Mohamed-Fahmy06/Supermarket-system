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

int main() {
    loadInventory(inventory, &inventoryCount);
    
    int choice;
    do {
        displayMenu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear buffer
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

void loadInventory(Product *inv, int *count) {
    FILE *file = fopen(INVENTORY_FILE, "r");
    if (file == NULL) {
        printf("Inventory file not found. Starting with empty inventory.\n");
        *count = 0;
        return;
    }

    *count = 0;
    while (fscanf(file, "%d %s %f %d", &inv[*count].ID, inv[*count].name, &inv[*count].price, &inv[*count].quantity) == 4) {
        (*count)++;
        if (*count >= MAX_PRODUCTS) break;
    }

    fclose(file);
    printf("Inventory loaded successfully. (%d items)\n", *count);
}

void saveInventory(Product *inv, int count) {
    FILE *file = fopen(INVENTORY_FILE, "w");
    if (file == NULL) {
        printf("Error: Could not open inventory file for writing.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%d %s %.2f %d\n", inv[i].ID, inv[i].name, inv[i].price, inv[i].quantity);
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
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
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
    scanf("%d", &p.ID);
    
    // Check if ID already exists
    if (findProductByID(inv, *count, p.ID) != -1) {
        printf("Product ID already exists!\n");
        return;
    }

    printf("Enter Name: ");
    scanf("%s", p.name);
    printf("Enter Price: ");
    scanf("%f", &p.price);
    printf("Enter Quantity: ");
    scanf("%d", &p.quantity);

    if (p.quantity < 0) {
        printf("Quantity cannot be negative!\n");
        return;
    }

    inv[*count] = p;
    (*count)++;
    printf("Product added successfully.\n");
}

void viewStock(Product *inv, int count) {
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
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: viewStock(inventory, inventoryCount); break;
            case 2: addToCart(inventory, inventoryCount, cart, &cartCount); break;
            case 3: 
                if (cartCount > 0) {
                    generateReceipt(cart, cartCount);
                    saveInventory(inventory, inventoryCount); // Save changes after purchase
                    return; // Exit customer mode after checkout
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
    if (*cartCount >= MAX_CART_ITEMS) {
        printf("Cart is full!\n");
        return;
    }

    int id, qty;
    printf("Enter Product ID to add: ");
    scanf("%d", &id);

    int index = findProductByID(inv, invCount, id);
    if (index == -1) {
        printf("Product not found!\n");
        return;
    }

    printf("Enter Quantity: ");
    scanf("%d", &qty);

    if (qty <= 0) {
        printf("Invalid quantity!\n");
        return;
    }

    if (qty > inv[index].quantity) {
        printf("Not enough stock available! (Available: %d)\n", inv[index].quantity);
        return;
    }

    // Add to cart
    cart[*cartCount] = inv[index];
    cart[*cartCount].quantity = qty;
    
    // Deduct from inventory in memory
    inv[index].quantity -= qty;
    
    (*cartCount)++;
    printf("Added %d %s to cart.\n", qty, inv[index].name);
}

void generateReceipt(Product *cart, int cartCount) {
    FILE *file = fopen(RECEIPT_FILE, "w");
    if (file == NULL) {
        printf("Error: Could not generate receipt file.\n");
        return;
    }

    float total = 0;
    printf("\n--- FORMAL RECEIPT ---\n");
    printf("%-20s %-10s %-10s %-10s\n", "Item", "Price", "Qty", "Subtotal");
    printf("----------------------------------------------------\n");
    fprintf(file, "--- SUPERMARKET RECEIPT ---\n");
    fprintf(file, "%-20s %-10s %-10s %-10s\n", "Item", "Price", "Qty", "Subtotal");
    fprintf(file, "----------------------------------------------------\n");

    for (int i = 0; i < cartCount; i++) {
        float subtotal = cart[i].price * cart[i].quantity;
        total += subtotal;
        printf("%-20s %-10.2f %-10d %-10.2f\n", cart[i].name, cart[i].price, cart[i].quantity, subtotal);
        fprintf(file, "%-20s %-10.2f %-10d %-10.2f\n", cart[i].name, cart[i].price, cart[i].quantity, subtotal);
    }

    printf("----------------------------------------------------\n");
    printf("TOTAL AMOUNT: %.2f\n", total);
    printf("----------------------------------------------------\n");
    fprintf(file, "----------------------------------------------------\n");
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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

// -------------------- Structures --------------------

// Product details
struct Product {
    string id;
    string name;
    string brand;
    string type;
    double price;
    int stockQuantity;
    string dateModified;
};

// Items added to the cart
struct CartItem {
    string productId;
    string productName;
    double price;
    int quantity;
};

// Voucher details
struct Voucher {
    string code;
    double discount;
    int usesRemaining;    // 0 usesRemaining means it can no longer be used
};

// -------------------- Globals --------------------
string CURRENT_DATE;

// -------------------- Constants --------------------
const int LOW_STOCK = 8;
// Number of products shown per page
const int PAGE_SIZE = 25;

// -------------------- Function Prototypes --------------------
// File handling
void loadInventoryFile(vector<Product> &inventory);
void saveInventoryFile(const vector<Product> &inventory);
void loadVouchersFile(vector<Voucher> &vouchers);
void saveVouchersFile(const vector<Voucher> &vouchers);

// String helpers
string changeToUppercase(const string &s);
string changeToLowercase(const string &s);
void parseMultipleIDs(const string &input, vector<string> &outIds);

// Pages
void displayByPages(const vector<Product> &inventory, const vector<size_t> &indices, const string &title);

// Display and inventory actions
void displayDashboard(const vector<Product> &inventory);
void checkAndDisplayLowStock(const vector<Product> &inventory);
void displayAllProducts(const vector<Product> &inventory);
void searchProducts(vector<Product> &inventory, vector<Voucher> &vouchers);
void addNewProduct(vector<Product> &inventory);
void updateProduct(vector<Product> &inventory, size_t idx);
void deleteProducts(vector<Product> &inventory, const vector<size_t> *filter = nullptr);

// Voucher menu
void voucherManagementMenu(vector<Voucher> &vouchers);

// Order processing
void generateReceiptFile(const string &customerName, const string &orderDate, const string &customerAddress,
                         const vector<CartItem> &cart, double subtotal, const string &voucherCode,
                         double discountAmount, double finalTotal, const string &paymentMethod,
                         const string &dateProcessed);
void orderMenu(vector<Product> &inventory, vector<Voucher> &activeVouchers);

// Main menu for inventory
void inventoryMenu(vector<Product> &inventory, vector<Voucher> &vouchers);

// Date
void date(string &CURRENT_DATE);
// -------------------- main --------------------
int main() {
    vector<Product> inventory;
    vector<Voucher> activeVouchers;

    cout << "*****************************************************************\n";
    cout << "*                                                               *\n";
    cout << "*             G L A M O U R P U F F   G I R L S                 *\n";
    cout << "*                    R E T A I L   S T O R E                    *\n";
    cout << "*                                                               *\n";
    cout << "*****************************************************************\n\n\n";

    // Get date
    date(CURRENT_DATE);
    cout << "\nCurrent Date: " << CURRENT_DATE << '\n';

    // Load saved data
    loadInventoryFile(inventory);
    loadVouchersFile(activeVouchers);

    while (true) {
        displayDashboard(inventory);

        cout << "\n\n\n=================================================================\n";
        cout << "               GLAMOURPUFF GIRLS RETAIL SYSTEM                  \n";
        cout << "                         [ Main Menu ]                          \n";
        cout << "=================================================================\n\n";

        cout << " Welcome! Please select an operation from the options below:\n\n";

        cout << "    [1]  Inventory Management System\n";
        cout << "    [2]  Customer Order Processing\n";
        cout << "    [3]  Exit Program\n\n";

        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int mainChoice;
        if (!(cin >> mainChoice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n Error: Invalid selection choice!\n";
            continue;
        }
        cin.ignore();

        if (mainChoice == 1) {
            cout << "\n Successfully directed to Inventory Management!\n\n";
            inventoryMenu(inventory, activeVouchers);
        } else if (mainChoice == 2) {
            cout << "\n Successfully directed to Customer Order Processing!\n\n";
            orderMenu(inventory, activeVouchers);
        } else if (mainChoice == 3) {
            cout << " \n         /\\_/\\         \n";
            cout << "    ____/ o o \\       \n";
            cout << "  /~____  = = /     \n"  ;
            cout << "  (______)___/ ";

            cout << "\n Closing the system....\n";
            break;
        } else {
            cout << "\n Error: Invalid choice! Please enter the number that corresponds to the operation.\n\n";
        }
    }

    return 0;
}

// -------------------- Implementations --------------------

// -------------------- Utilities --------------------

// Convert string to uppercase
string changeToUppercase(const string &s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'a' && c <= 'z') {c = char(c - 32);} //32 is the difference of ('a' - 'A') ASCII
        out.push_back(c);
    }
    return out;
}

// Convert string to lowercase
string changeToLowercase(const string &s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') c = char(c + 32);
        out.push_back(c);
    }
    return out;
}

// Extract product IDs from a comma-separated input
void parseMultipleIDs(const string &input, vector<string> &outIds) {
    outIds.clear();
    size_t start = 0;
    while (start < input.size()) {
        while (start < input.size() && (input[start] == ' ' || input[start] == ',')) ++start;
        if (start >= input.size()) break;

        size_t pos = start;
        while (pos < input.size() && input[pos] != ',') ++pos;
        string id = input.substr(start, pos - start);

        size_t a = 0;
        while (a < id.size() && id[a] == ' ') ++a;

        size_t b = id.size();
        while (b > a && id[b-1] == ' ') --b;

        if (b > a) outIds.push_back(id.substr(a, b - a));
        start = pos + 1;
    }
}

// -------------------- File I/O --------------------

// Save inventory vector to text file using '|' as separator
void saveInventoryFile(const vector<Product> &inventory) {
    ofstream writeInventory("inventory.txt");
    if (!writeInventory) return;
    for (const auto &p : inventory){
        writeInventory << p.id << '|' << p.type << '|' << p.brand << '|' << p.name << '|'
            << fixed << setprecision(2) << p.price << '|' << p.stockQuantity << '|' << p.dateModified << '\n';
    }
}

// Read inventory text file and add items to vector
void loadInventoryFile(vector<Product> &inventory) {
    inventory.clear();
    ifstream readInventory("inventory.txt");

    if (!readInventory) return;
    Product p;
    string priceStr, qtyStr;

    // Each getline call reads up to the next '|' directly from the file stream.
    while (getline(readInventory, p.id,            '|') &&
           getline(readInventory, p.type,          '|') &&
           getline(readInventory, p.brand,         '|') &&
           getline(readInventory, p.name,          '|') &&
           getline(readInventory, priceStr,        '|') &&
           getline(readInventory, qtyStr,          '|') &&
           getline(readInventory, p.dateModified      )) {
        if (p.id.empty()) continue;
        try { p.price         = stod(priceStr); } catch (...) { p.price = 0.0; }
        try { p.stockQuantity = stoi(qtyStr);   } catch (...) { p.stockQuantity = 0; }
        inventory.push_back(p);
    }
}

// Save vouchers to file
void saveVouchersFile(const vector<Voucher> &vouchers) {
    ofstream writeVouchers("vouchers.txt");
    if (!writeVouchers) return;
    for (const auto &v : vouchers) {
        writeVouchers << v.code << '|'
            << v.discount << '|'
            << v.usesRemaining << '\n';
    }
}

// Read vouchers from file
void loadVouchersFile(vector<Voucher> &vouchers) {
    vouchers.clear();
    ifstream readVoucher("vouchers.txt");
    if (!readVoucher) return;
    Voucher v;
    string discountStr, usesStr;
    // Reads code up to '|', discount up to '|', then usesRemaining up to '\n'.
    while (getline(readVoucher, v.code,    '|') &&
           getline(readVoucher, discountStr,'|') &&
           getline(readVoucher, usesStr        )) {
        if (v.code.empty()) continue;
        try { v.discount      = stod(discountStr); } catch (...) { v.discount = 0.0; }
        try { v.usesRemaining = stoi(usesStr);     } catch (...) { v.usesRemaining = 0; }
        vouchers.push_back(v);
    }
}

// Displays a subset of inventory products page by page.
void displayByPages(const vector<Product> &inventory,
                                   const vector<size_t> &indices,
                                   const string &title) {
    if (indices.empty()) {
        cout << "  No products to display.\n";
        cout << "-----------------------------------------------------------------\n";
        return;
    }

    int totalProducts = static_cast<int>(indices.size());
    int totalPages    = (totalProducts + PAGE_SIZE - 1) / PAGE_SIZE;   // ceiling division
    int currentPage   = 1;

    while (true) {
        // Compute slice for this page
        int startIdx = (currentPage - 1) * PAGE_SIZE;           // inclusive
        int endIdx   = startIdx + PAGE_SIZE;                     // exclusive
        if (endIdx > totalProducts) endIdx = totalProducts;

        cout << "\n=================================================================\n";
        cout << "              [ " << title << " ]\n";
        cout << "=================================================================\n";
        cout << "  Page " << currentPage << " of " << totalPages
             << "  |  Showing products " << (startIdx + 1)
             << " - " << endIdx
             << " of " << totalProducts << " total\n";
        cout << "-----------------------------------------------------------------\n";

        for (int i = startIdx; i < endIdx; ++i) {
            const Product &p = inventory[indices[i]];
            // Display number is relative to the full result list, not just the page
            cout << "  " << setw(4) << (i + 1) << ". "
                 << "ID: " << p.id
                 << " | " << p.type
                 << " | [" << p.brand << "] " << p.name
                 << " | Price: P" << fixed << setprecision(2) << p.price
                 << " | Stock: " << p.stockQuantity << '\n';
        }

        cout << "-----------------------------------------------------------------\n";
        cout << "  Navigation:";
        if (currentPage > 1)       cout << "  [P] Previous Page";
        if (currentPage < totalPages) cout << "  [N] Next Page";
        cout << "  [Q] Quit / Back\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        string nav;
        getline(cin, nav);
        nav = changeToUppercase(nav);

        if (nav == "N" || nav == "NEXT") {
            if (currentPage < totalPages) {
                ++currentPage;
            } else {
                cout << "  You are already on the last page.\n";
            }
        } else if (nav == "P" || nav == "PREV" || nav == "PREVIOUS") {
            if (currentPage > 1) {
                --currentPage;
            } else {
                cout << "  You are already on the first page.\n";
            }
        } else if (nav == "Q" || nav == "QUIT" || nav == "BACK") {
            cout << "  Returning...\n";
            cout << "-----------------------------------------------------------------\n";
            break;
        } else {
            cout << "  Error: Invalid input. Use N, P, or Q.\n";
        }
    }
}

// -------------------- Display / Inventory ops --------------------

// Show quick stats about the current inventory
void displayDashboard(const vector<Product> &inventory) {
    cout << "\n=================================================================\n";
    cout << "                   I N V E N T O R Y   S T A T U S              \n";
    cout << "=================================================================\n";

    int totalUniqueItems       = static_cast<int>(inventory.size());
    int totalStockQuantity     = 0;
    double totalInventoryValue = 0.0;
    int lowStockAlert          = 0;

    for (const auto &p : inventory) {
        totalStockQuantity  += p.stockQuantity;
        totalInventoryValue += p.price * p.stockQuantity;
        if (p.stockQuantity < LOW_STOCK) ++lowStockAlert;
    }

    cout << "  Total Unique Products : " << totalUniqueItems << '\n';
    cout << "  Total Stock Volume    : " << totalStockQuantity << " units\n";
    cout << "  Total Asset Value     : Php" << fixed << setprecision(2) << totalInventoryValue << '\n';
    cout << "  Low Stock Alerts      : " << lowStockAlert << '\n';
    cout << "-----------------------------------------------------------------\n";

    if (lowStockAlert > 0) checkAndDisplayLowStock(inventory);
}

// Print items that have less than 8 stock
void checkAndDisplayLowStock(const vector<Product> &inventory) {
    cout << "  [ LOW STOCK ALERTS ]\n\n";
    int lowStockCount = 0;
    for (const auto &p : inventory) {
        if (p.stockQuantity < LOW_STOCK) {
            cout << "  \a! WARNING: " << p.name
                 << " is low on stock! (" << p.stockQuantity << " left)\n";
            ++lowStockCount;
        }
    }
    if (lowStockCount == 0) cout << "  All stocks are sufficient.\n";
    cout << "-----------------------------------------------------------------\n";
}

// Print a paginated list of all products
void displayAllProducts(const vector<Product> &inventory) {
    if (inventory.empty()) {
        cout << "\n=================================================================\n";
        cout << "                  [ Complete Inventory Table ]                   \n";
        cout << "=================================================================\n";
        cout << "  No products found in inventory.\n";
        cout << "-----------------------------------------------------------------\n";
        return;
    }

    // Build a full index list: 0, 1, 2, ..., N-1
    vector<size_t> allIndices;
    allIndices.reserve(inventory.size());
    for (size_t i = 0; i < inventory.size(); ++i)
        allIndices.push_back(i);

    displayByPages(inventory, allIndices, "Complete Inventory Table");
}

// Search for a product and give options to modify, delete, search again, or exit
void searchProducts(vector<Product> &inventory, vector<Voucher> &vouchers) {
    while (true) {
        cout << "\n=================================================================\n";
        cout << "                      [ Search Products ]                        \n";
        cout << "=================================================================\n";
        cout << " Enter keyword (ID, name, brand, or type): ";
        string keyword;
        getline(cin, keyword);
        string keyLower = changeToLowercase(keyword);

        // Collect matching indices
        vector<size_t> foundIndexes;
        foundIndexes.reserve(inventory.size());

        for (size_t i = 0; i < inventory.size(); ++i) {
            const Product &p = inventory[i];
            string id_lc = changeToLowercase(p.id);
            string name_lc = changeToLowercase(p.name);
            string brand_lc = changeToLowercase(p.brand);
            string type_lc = changeToLowercase(p.type);

            if (id_lc.find(keyLower)    != string::npos ||
                name_lc.find(keyLower)  != string::npos ||
                brand_lc.find(keyLower) != string::npos ||
                type_lc.find(keyLower)  != string::npos) {
                foundIndexes.push_back(i);
            }
        } //Because it uses the OR operator (||), it checks all fields.
            //If the keyword is hidden inside the ID, OR the Name, OR the Brand, OR the Type, the product qualifies.

        if (foundIndexes.empty()) {
            cout << "\n  No products matched [" << keyword << "].\n";
            cout << "-----------------------------------------------------------------\n";
            cout << "\n    [1]  Search Again\n";
            cout << "    [2]  Back to Inventory Menu\n\n";
            cout << "-----------------------------------------------------------------\n";
            cout << " Enter your choice: ";

            int noResultOpt = 0;
            while (true) {
                if (!(cin >> noResultOpt)) {
                    cin.clear(); cin.ignore(10000, '\n');
                    cout << " Error: Invalid input. Please enter 1 or 2.\n";
                    cout << " Enter your choice: ";
                    continue;
                }
                cin.ignore();
                if (noResultOpt == 1 || noResultOpt == 2) break;
                cout << " Error: Invalid input. Please enter 1 or 2.\n";
                cout << " Enter your choice: ";
            }

            if (noResultOpt == 1) continue;   // loop back to search prompt
            return;                            // back to inventory menu
        }

        cout << "\n  Found " << foundIndexes.size() << " matching product(s) for [" << keyword << "].\n";

        // Show results with pagination
        displayByPages(inventory, foundIndexes, "Search Results");

        // Post-browse action menu
        cout << "\n    [1]  Modify a Result\n";
        cout << "    [2]  Delete a Result\n";
        cout << "    [3]  Search Again\n";
        cout << "    [4]  Back to Inventory Menu\n\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int opt = 0;
        while (true) {
            if (!(cin >> opt)) {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << " Error: Invalid input. Please enter 1, 2, 3, or 4.\n";
                cout << " Enter your choice: ";
                continue;
            }
            cin.ignore();
            if (opt >= 1 && opt <= 4) break;
            cout << " Error: Invalid input. Please enter 1, 2, 3, or 4.\n";
            cout << " Enter your choice: ";
        }

        if (opt == 1) {
            // ----- Modify -----
            int sel = 0;
            while (true) {
                cout << " Enter the result number of the product to modify (1-"
                     << foundIndexes.size() << "): ";
                if (!(cin >> sel)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << " Error: Invalid input. Enter a valid number.\n";
                    continue;
                }
                cin.ignore();
                if (sel >= 1 && static_cast<size_t>(sel) <= foundIndexes.size()) break;
                cout << " Selection out of range. Try again.\n";
            }
            size_t idx = foundIndexes[sel - 1];
            updateProduct(inventory, idx);
            saveInventoryFile(inventory);
            //user can search again or exit

        } else if (opt == 2) {
            // ----- Delete (delegates to deleteProductsInteractive) -----
            deleteProducts(inventory, &foundIndexes);
            // Loop continues — user can search again or exit

        } else if (opt == 3) {
            // ----- Search Again -----
            continue;

        } else {
            // ----- Back to Inventory Menu -----
            return;
        }
    }
}
// Edit fields of a specific product
void updateProduct(vector<Product> &inventory, size_t idx) {
    if (idx >= inventory.size()) return;
    Product &p = inventory[idx];

    while (true) {
        cout << "\n=================================================================\n";
        cout << "                    [ Edit Product Details ]                     \n";
        cout << "=================================================================\n";
        cout << "  Editing: " << p.id << " | " << p.name << "\n\n";
        cout << "  Select the field you want to modify:\n\n";
        cout << "    [1]  Product ID\n";
        cout << "    [2]  Type\n";
        cout << "    [3]  Brand\n";
        cout << "    [4]  Name\n";
        cout << "    [5]  Price\n";
        cout << "    [6]  Stock Quantity\n";
        cout << "    [7]  Save and Exit\n\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << " Error: Invalid input. Try again.\n";
            continue;
        }
        cin.ignore();

        switch(choice){
        case 1: {
            string tempId;
            cout << " New Product ID    : "; getline(cin, tempId);
            bool idExists = false;
            for (size_t i = 0; i < inventory.size(); ++i) {
                if (i != idx && changeToUppercase(inventory[i].id) == changeToUppercase(tempId)) {
                    idExists = true;
                    break;
                }
            }
            if (idExists) {
                cout << " Error: A product with ID '" << tempId << "' already exists. Modification denied.\n";
            } else {
                p.id = tempId; cout << " Updated.\n";
            }
            break;
        }

        case 2: cout << " New Type          : "; getline(cin, p.type);  cout << " Updated.\n"; break;
        case 3: cout << " New Brand         : "; getline(cin, p.brand); cout << " Updated.\n"; break;
        case 4: cout << " New Name          : "; getline(cin, p.name);  cout << " Updated.\n"; break;
        case 5:
            cout << " New Price         : ";
            if (!(cin >> p.price)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid price.\n"; }
            else { cin.ignore(); cout << " Updated.\n";}
            break;
        case 6:
            cout << " New Stock Quantity: ";
            if (!(cin >> p.stockQuantity)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid quantity.\n"; }
            else { cin.ignore(); cout << " Updated.\n"; }
            break;
        case 7:
            p.dateModified = CURRENT_DATE;
            cout << "\n Changes saved. Date modified updated to " << CURRENT_DATE << ".\n";
            cout << "-----------------------------------------------------------------\n";
            return;
        default:
            cout << " Error: Invalid selection. Please try again.\n"; break;
        }
    }
}

// Delete product directly without searching
// Accepts one or more product IDs (comma-separated) and deletes them.
void deleteProducts(vector<Product> &inventory, const vector<size_t> *filter) {
    cout << "\n=================================================================\n";
    cout << "                     [ Delete Products ]                         \n";
    cout << "=================================================================\n\n";
    cout << " Enter product ID(s) to delete (comma-separated for multiple):\n";
    cout << " e.g.  P001              (single)\n";
    cout << " e.g.  P001, P002, P003  (multiple)\n\n";
    cout << "-----------------------------------------------------------------\n";
    cout << " Enter product ID(s): ";

    string delInput;
    getline(cin, delInput);
    if (delInput.empty()) { cout << " No input provided.\n"; return; }

    // Split the input into individual IDs
    vector<string> ids;
    parseMultipleIDs(delInput, ids);
    if (ids.empty()) { cout << " No valid IDs found.\n"; return; }

    // Resolve each ID to an inventory index.
    // Collect all first so we can sort descending before erasing —
    // this prevents earlier erasures from shifting later indices.
    vector<size_t> toRemoveIdx;
    toRemoveIdx.reserve(ids.size());

    for (size_t t = 0; t < ids.size(); ++t) {
        const string &targetId = ids[t];
        bool found = false;

        if (filter != NULL) {
            // For search results
            for (size_t f = 0; f < filter->size(); ++f) { //"What is the size of the vector you are pointing to?"
                size_t invIdx = (*filter)[f];
                if (inventory[invIdx].id == targetId) {
                    toRemoveIdx.push_back(invIdx);
                    found = true;
                    break;
                }
            }
        } else {
            // Full inventory
            for (size_t i = 0; i < inventory.size(); ++i) {
                if (inventory[i].id == targetId) {
                    toRemoveIdx.push_back(i);
                    found = true;
                    break;
                }
            }
        }

        if (!found) cout << " ID not found: " << targetId << '\n';
    }

    if (toRemoveIdx.empty()) { cout << " No matching products to delete.\n"; return; }

    // Sort descending so erasing does not shift remaining indices
    for (size_t i = 0; i < toRemoveIdx.size(); ++i)
        for (size_t j = i + 1; j < toRemoveIdx.size(); ++j)
            if (toRemoveIdx[j] > toRemoveIdx[i]) swap(toRemoveIdx[i], toRemoveIdx[j]);

    // Remove consecutive duplicates (after sort) in a single pass
    size_t w = 0;
    for (size_t r = 0; r < toRemoveIdx.size(); ++r)
        if (r == 0 || toRemoveIdx[r] != toRemoveIdx[r-1]) //The code checks two things to see if a number is unique
            toRemoveIdx[w++] = toRemoveIdx[r];
    toRemoveIdx.resize(w); //It slices off everything after index w-1

    // Confirm and erase each
    for (size_t k = 0; k < toRemoveIdx.size(); ++k) {
        size_t ridx = toRemoveIdx[k];
        cout << " Confirm delete [" << inventory[ridx].id << "] " << inventory[ridx].name << "? (Y/N): ";
        string conf; getline(cin, conf);
        conf = changeToUppercase(conf);
        if (conf == "Y" || conf == "YES") {
            cout << " Deleted: " << inventory[ridx].id << " - " << inventory[ridx].name << '\n';
            inventory.erase(inventory.begin() + ridx);
        } else {
            cout << " Skipped: " << inventory[ridx].id << '\n';
        }
    }

    saveInventoryFile(inventory);
}

// -------------------- Voucher management --------------------
// Menu for managing discount vouchers
void voucherManagementMenu(vector<Voucher> &vouchers) {
    while (true) {
        cout << "\n=================================================================\n";
        cout << "                  [ Voucher Management ]                         \n";
        cout << "=================================================================\n\n";
        cout << " Please type the number of the operation to start:\n\n";
        cout << "    [1]  View All Vouchers\n";
        cout << "    [2]  Add New Voucher\n";
        cout << "    [3]  Edit Existing Voucher\n";
        cout << "    [4]  Delete Voucher\n";
        cout << "    [5]  Back to Inventory Menu\n\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int ch;
        if (!(cin >> ch)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid input.\n"; continue; }
        cin.ignore();

        if (ch == 1) {
            cout << "\n=================================================================\n";
            cout << "                     [ Active Vouchers ]                         \n";
            cout << "=================================================================\n";
            if (vouchers.empty()) {
                cout << "  No vouchers currently on record.\n";
            } else {
                for (size_t i = 0; i < vouchers.size(); ++i) {
                    cout << "  " << i+1 << ". Code: " << vouchers[i].code
                         << " | Discount: " << static_cast<int>(vouchers[i].discount * 100) << "% off"
                         << " | Uses Remaining: " << vouchers[i].usesRemaining;
                    if (vouchers[i].usesRemaining == 0) cout << "  [Not Usable]";
                    cout << '\n';
                }
            }
            cout << "-----------------------------------------------------------------\n";

        } else if (ch == 2) {
            cout << "\n=================================================================\n";
            cout << "                     [ Add New Voucher ]                         \n";
            cout << "=================================================================\n";
            Voucher v; double percent; bool dup = false, sym = false;
            while (true) {
                cout << " Enter voucher code          : "; getline(cin, v.code);
                for (char c : v.code) {
		    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
		        sym = true;
		    }
                }
                for (const auto &item : vouchers) if (changeToUppercase(item.code) == changeToUppercase(v.code)) dup = true;
                if (v.code.empty() || sym || dup) { cout << " Error: Code must be non-blank, alphanumeric, and completely unique.\n"; sym = dup = false; continue; }
                break;
            }
            cout << " Enter discount percentage   : ";
            if (!(cin >> percent) || percent < 0.0 || percent > 100.0) {
                cin.clear(); cin.ignore(10000,'\n');
                cout << " Error: Invalid percentage. Must be between 0 and 100.\n";
                continue;
            }
            cin.ignore();
            v.discount = percent / 100.0;

            cout << " Enter number of uses allowed: ";
            if (!(cin >> v.usesRemaining) || v.usesRemaining < 1) {
                cin.clear(); cin.ignore(10000,'\n');
                cout << " Invalid input. Defaulting to 1 use.\n";
                v.usesRemaining = 1;
            } else {
                cin.ignore();
            }

            vouchers.push_back(v);
            saveVouchersFile(vouchers);
            cout << "\n Voucher [" << v.code << "] added with " << v.usesRemaining << " use(s).\n";
            cout << "-----------------------------------------------------------------\n";

        } else if (ch == 3) {
            cout << "\n=================================================================\n";
            cout << "                    [ Edit Existing Voucher ]                    \n";
            cout << "=================================================================\n";
            cout << " Enter voucher code to edit: ";
            string code; getline(cin, code);
            bool found = false;
            for (size_t i = 0; i < vouchers.size(); ++i) {
                if (changeToUppercase(vouchers[i].code) == changeToUppercase(code)) {
                    found = true;
                    cout << "\n  Current Details:\n";
                    cout << "  Code     : " << vouchers[i].code << '\n';
                    cout << "  Discount : " << static_cast<int>(vouchers[i].discount * 100) << "% off\n";
                    cout << "  Uses Left: " << vouchers[i].usesRemaining << "\n\n";
                    cout << "    [1]  Change voucher code\n";
                    cout << "    [2]  Change discount percentage\n";
                    cout << "    [3]  Change remaining uses\n\n";
                    cout << "-----------------------------------------------------------------\n";
                    cout << " Enter your choice: ";
                    int e;
                    if (!(cin >> e)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid input.\n"; break; }
                    cin.ignore();
                    if (e == 1) {
                        string temp; bool dup = false, sym = false;
                        while (true) {
                            cout << " New code: "; getline(cin, temp);
                            for (char c : temp) { if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) { sym = true; } }
                            for (size_t vIdx = 0; vIdx < vouchers.size(); ++vIdx) if (vIdx != i && changeToUppercase(vouchers[vIdx].code) == changeToUppercase(temp)) dup = true;
                            if (temp.empty() || sym || dup) { cout << " Error: Code must be non-blank, alphanumeric, and unique.\n"; sym = dup = false; continue; }
                            vouchers[i].code = temp; cout << " Voucher code updated.\n"; break;
                        }
                    } else if (e == 2) {
                        cout << " New discount percentage: ";
                        double percent;
                        if (!(cin >> percent) || percent < 0.0 || percent > 100.0) {
                            cin.clear(); cin.ignore(10000,'\n');
                            cout << " Error: Invalid percentage. Must be between 0 and 100.\n";
                        } else { vouchers[i].discount = percent / 100.0; cin.ignore(); cout << " Discount updated.\n"; }
                    } else if (e == 3) {
                        cout << " New remaining uses: ";
                        int newUses;
                        if (!(cin >> newUses) || newUses < 0) {
                            cin.clear(); cin.ignore(10000,'\n');
                            cout << " Error: Invalid input. No changes made.\n";
                        } else {
                            vouchers[i].usesRemaining = newUses;
                            cin.ignore();
                            cout << " Uses remaining updated to " << newUses << ".\n";
                        }
                    } else {
                        cout << " Error: Invalid choice.\n";
                    }
                    saveVouchersFile(vouchers);
                    cout << "-----------------------------------------------------------------\n";
                    break;
                }
            }
            if (!found) cout << " Error: Voucher not found.\n";

        } else if (ch == 4) {
            cout << "\n=================================================================\n";
            cout << "                      [ Delete Voucher ]                         \n";
            cout << "=================================================================\n";
            cout << " Enter voucher code to delete: ";
            string code; getline(cin, code);
            bool removed = false;
            for (size_t i = 0; i < vouchers.size(); ++i) {
                if (changeToUppercase(vouchers[i].code) == changeToUppercase(code)) {
                    cout << " Confirm delete voucher [" << vouchers[i].code << "]? (Y/N): ";
                    string conf; getline(cin, conf);
                    conf = changeToUppercase(conf);
                    if (conf == "Y" || conf == "YES") {
                        vouchers.erase(vouchers.begin() + i);
                        removed = true;
                        saveVouchersFile(vouchers);
                        cout << " Voucher successfully deleted.\n";
                    } else {
                        cout << " Deletion cancelled.\n";
                        removed = true;
                    }
                    break;
                }
            }
            if (!removed) cout << " Error: Voucher not found.\n";
            cout << "-----------------------------------------------------------------\n";

        } else if (ch == 5) {
            return;
        } else {
            cout << " Error: Invalid selection. Please enter a number from 1 to 5.\n";
        }
    }
}

// -------------------- Order processing --------------------
// Write the invoice details to a text file
void generateReceiptFile(const string &customerName, const string &orderDate, const string &customerAddress,
                         const vector<CartItem> &cart, double subtotal, const string &voucherCode,
                         double discountAmount, double finalTotal, const string &paymentMethod,
                         const string &dateProcessed) {
    ofstream createInvoice(CURRENT_DATE + "_" + customerName + "_invoice.txt");
    if (!createInvoice) { cout << " Error: Unable to write invoice file.\n"; return; }

    // Header
    createInvoice << "=================================================================\n";
    createInvoice << "                      GlamourPuff Girls                         \n";
    createInvoice << "                        Retail Store                            \n";
    createInvoice << "                       SALES INVOICE                           \n";
    createInvoice << "=================================================================\n";
    createInvoice << "  Date Processed  : " << dateProcessed << '\n';
    createInvoice << "-----------------------------------------------------------------\n";

    // Customer details
    createInvoice << "  Customer Name   : " << customerName     << '\n';
    createInvoice << "  Address         : " << customerAddress  << '\n';
    createInvoice << "  Date Ordered    : " << orderDate        << '\n';
    createInvoice << "-----------------------------------------------------------------\n";

    // Column headers for items
    createInvoice << "  " << left << setw(4)  << "QTY"
        << "  "         << setw(30) << "ITEM"
        << "  "         << setw(12) << "UNIT PRICE"
        << "  "         << "TOTAL\n";
    createInvoice << "-----------------------------------------------------------------\n";

    // Line items
    int totalItemCount = 0;
    for (const auto &item : cart) {
        double lineTotal = item.price * item.quantity;
        totalItemCount  += item.quantity;

        createInvoice << "  " << left  << setw(4)  << item.quantity
            << "  "          << setw(30) << item.productName
            << "  " << right << setw(9)  << fixed << setprecision(2) << item.price
            << "  Php "
            << setw(10) << fixed << setprecision(2) << lineTotal << '\n';
    }

    createInvoice << "-----------------------------------------------------------------\n";

    // Total item count
    createInvoice << "  Total Items Purchased : " << totalItemCount << '\n';
    createInvoice << "-----------------------------------------------------------------\n";

    // Financial summary
    createInvoice << "  " << left << setw(30) << "SUBTOTAL"
        << right << setw(14) << "P "
        << setw(10) << fixed << setprecision(2) << subtotal << '\n';

    createInvoice << "  " << left << setw(30) << ("VOUCHER: " + voucherCode)
        << right << setw(14) << "- P "
        << setw(8) << fixed << setprecision(2) << discountAmount << '\n';

    createInvoice << "-----------------------------------------------------------------\n";

    createInvoice << "  " << left << setw(30) << "TOTAL"
        << right << setw(14) << "P "
        << setw(10) << fixed << setprecision(2) << finalTotal << '\n';

    createInvoice << "-----------------------------------------------------------------\n";

    createInvoice << "  " << left << setw(30) << "MODE OF PAYMENT"
        << paymentMethod << '\n';

    createInvoice << "=================================================================\n";
}

// Handle adding to cart and checkout
void orderMenu(vector<Product> &inventory, vector<Voucher> &activeVouchers) {
    vector<CartItem> temporaryCart;
    double appliedDiscount     = 0.0;
    string appliedVoucher = "NONE";

    string customerName, orderDate, customerAddress;

    cout << "\n=================================================================\n";
    cout << "                 [ Customer Order Processing ]                   \n";
    cout << "=================================================================\n";
    cout << " Enter Customer Name     : "; getline(cin, customerName);
    cout << " Enter Date Ordered      : "; getline(cin, orderDate);
    cout << " Enter Delivery Address  : "; getline(cin, customerAddress);
    cout << "-----------------------------------------------------------------\n";

    while (true) {
        cout << "\n=================================================================\n";
        cout << "                       [ Order Actions ]                         \n";
        cout << "=================================================================\n\n";
        cout << "    [1]  Add Product to Cart\n";
        cout << "    [2]  Edit Cart\n";
        cout << "    [3]  Apply Voucher Code\n";
        cout << "    [4]  Checkout\n";
        cout << "    [5]  Cancel Order\n\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int orderAction;
        if (!(cin >> orderAction)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << " Error: Invalid input. Please try again.\n";
            continue;
        }
        cin.ignore();

        if (orderAction == 1) {
            cout << "\n Enter Product ID: ";
            string searchID;
            getline(cin, searchID);
            bool itemFound = false;
            size_t foundIndex = 0;

            for (size_t i = 0; i < inventory.size(); ++i) {
                if (changeToUppercase(inventory[i].id) == changeToUppercase(searchID)) {
                    itemFound = true;
                    foundIndex = i;
                    const Product &p = inventory[i];
                    cout << "\n-----------------------------------------------------------------\n";
                    cout << "  Item Found:\n";
                    cout << "  Name  : " << p.name  << '\n';
                    cout << "  Brand : " << p.brand << '\n';
                    cout << "  Price : P" << fixed << setprecision(2) << p.price << '\n';
                    cout << "  Stock : " << p.stockQuantity << " units\n";
                    cout << "-----------------------------------------------------------------\n";
                    break;
                }
            }
            if (!itemFound) {
                cout << " Error: Product ID not found.\n";
                continue;
            }

            int purchaseQuantity;
            if (inventory[foundIndex].stockQuantity <= 0){
                cout << "Error: This item is currently out of stock.\n";
                continue;
            } else{
            // Find how many units of this product are already in the cart
                int alreadyInCart = 0;
                for (size_t i = 0; i < temporaryCart.size(); ++i) {
                    if (temporaryCart[i].productId == inventory[foundIndex].id) {
                        alreadyInCart = temporaryCart[i].quantity;
                        break;
                    }
                }
                int availableStock = inventory[foundIndex].stockQuantity - alreadyInCart;

                while (true) {
                    cout << " Enter quantity to purchase: ";
                    if (!(cin >> purchaseQuantity)) {
                        cin.clear(); cin.ignore(10000, '\n');
                        cout << " Error: Invalid quantity. Enter a numeric value.\n";
                        continue;
                    }
                    cin.ignore();
                    if (purchaseQuantity <= 0) { cout << " Quantity must be at least 1. Try again.\n"; continue; }
                    if (purchaseQuantity > availableStock) {
                        cout << " Insufficient stock. You can add at most " << availableStock << " more unit(s).\n";
                        continue;
                    }
                    break;
                }
            // Add or merge to cart
            bool merged = false;
            for (size_t i = 0; i < temporaryCart.size(); ++i) {
                if (temporaryCart[i].productId == inventory[foundIndex].id) {
                    temporaryCart[i].quantity += purchaseQuantity;
                    merged = true;
                    break;
                }
            }
            if (!merged) {
                CartItem newItem;
                newItem.productId = inventory[foundIndex].id;
                newItem.productName = inventory[foundIndex].name;
                newItem.price = inventory[foundIndex].price;
                newItem.quantity = purchaseQuantity;
                temporaryCart.push_back(newItem);
            }
            cout << " Added to cart: " << inventory[foundIndex].name
                 << " | " << inventory[foundIndex].brand
                 << " | Qty: " << purchaseQuantity << '\n';
            } // close else block for stockQuantity > 0
        } else if (orderAction == 2) {
            if (temporaryCart.empty()) { cout << " Cart is currently empty.\n"; continue; }
            while (true) {
                cout << "\n=================================================================\n";
                cout << "                        [ Edit Cart ]                           \n";
                cout << "=================================================================\n";
                for (size_t i = 0; i < temporaryCart.size(); ++i)
                    cout << "  " << i+1 << ". " << temporaryCart[i].productName
                         << " | Qty: " << temporaryCart[i].quantity
                         << " | P" << fixed << setprecision(2) << temporaryCart[i].price << " each\n";
                cout << "\n    [1]  Change item quantity\n";
                cout << "    [2]  Remove item from cart\n";
                cout << "    [3]  Done Editing\n\n";
                cout << "-----------------------------------------------------------------\n";
                cout << " Enter your choice: ";

                int ec;
                if (!(cin >> ec)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid input.\n"; continue; }
                cin.ignore();

                if (ec == 1) {
                    int sel;
                    cout << " Enter item number to edit: ";
                    if (!(cin >> sel)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid input.\n"; continue; }
                    cin.ignore();
                    if (sel < 1 || static_cast<size_t>(sel) > temporaryCart.size()) { cout << " Selection out of range.\n"; continue; }

                    int newQty;
                    while (true) {
                        cout << " Enter new quantity: ";
                        if (!(cin >> newQty)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid input.\n"; continue; }
                        cin.ignore();
                        if (newQty <= 0) { cout << " Quantity must be at least 1.\n"; continue; }

                        string pid = temporaryCart[sel-1].productId;
                        int stock = 0;
                        for (size_t k = 0; k < inventory.size(); ++k)
                            if (inventory[k].id == pid) { stock = inventory[k].stockQuantity; break; }
                        if (newQty > stock) { cout << " Insufficient stock. Available: " << stock << ".\n"; continue; }
                        break;
                    }
                    temporaryCart[sel-1].quantity = newQty;
                    cout << " Quantity updated successfully.\n";

                } else if (ec == 2) {
                    int sel;
                    cout << " Enter item number to remove: ";
                    if (!(cin >> sel)) { cin.clear(); cin.ignore(10000,'\n'); cout << " Error: Invalid input.\n"; continue; }
                    cin.ignore();
                    if (sel < 1 || static_cast<size_t>(sel) > temporaryCart.size()) { cout << " Selection out of range.\n"; continue; }

                    cout << " Confirm remove [" << temporaryCart[sel-1].productName << "] from cart? (Y/N): ";
                    string conf; getline(cin, conf);
                    conf = changeToUppercase(conf);
                    if (conf == "Y" || conf == "YES") {
                        temporaryCart.erase(temporaryCart.begin() + (sel-1));
                        cout << " Item removed from cart.\n";
                    } else {
                        cout << " Removal cancelled.\n";
                    }
                } else if (ec == 3) {
                    break;
                } else {
                    cout << " Error: Invalid choice.\n";
                }
            }

        } else if (orderAction == 3) {
            cout << "\n Enter Promo Voucher Code: ";
            string inputCode;
            getline(cin, inputCode);
            inputCode = changeToUppercase(inputCode);
            bool voucherFound = false;

            for (size_t i = 0; i < activeVouchers.size(); ++i) {
                if (changeToUppercase(activeVouchers[i].code) == inputCode) {
                    voucherFound = true;
                    if (activeVouchers[i].usesRemaining <= 0) {
                        cout << " Alert: This voucher has no remaining uses and is no longer active.\n";
                    } else {
                        appliedVoucher = activeVouchers[i].code;
                        appliedDiscount     = activeVouchers[i].discount;
                        cout << " Promo Applied: -" << static_cast<int>(appliedDiscount * 100) << "%"
                             << " (" << activeVouchers[i].usesRemaining << " use(s) remaining before this redemption)\n";
                    }
                    break;
                }
            }
            if (!voucherFound) cout << " Error: Voucher code not found. Please check and try again.\n";

        } else if (orderAction == 4) {
            if (temporaryCart.empty()) { cout << " Error: Shopping cart is empty. Add products before checking out.\n"; continue; }

            double subtotal = 0.0;
            cout << "\n=================================================================\n";
            cout << "                      [ Checkout Review ]                        \n";
            cout << "=================================================================\n";
            for (size_t i = 0; i < temporaryCart.size(); ++i) {
                cout << "  " << i+1 << ". " << temporaryCart[i].productName
                     << " (Qty: " << temporaryCart[i].quantity << ") - P"
                     << fixed << setprecision(2) << (temporaryCart[i].price * temporaryCart[i].quantity) << '\n';
                subtotal += temporaryCart[i].price * temporaryCart[i].quantity;
            }

            double total_discount_deduction = subtotal * appliedDiscount;
            double final_total_price        = subtotal - total_discount_deduction;

            cout << "-----------------------------------------------------------------\n";
            cout << "  Subtotal        : P" << fixed << setprecision(2) << subtotal << '\n';
            cout << "  Voucher Applied : " << appliedVoucher
                 << " (-P" << fixed << setprecision(2) << total_discount_deduction << ")\n";
            cout << "  Final Total     : P" << fixed << setprecision(2) << final_total_price << '\n';
            cout << "-----------------------------------------------------------------\n";
            cout << " Confirm checkout? (Y/N): ";

            string checkout_confirm;
            getline(cin, checkout_confirm);
            checkout_confirm = changeToUppercase(checkout_confirm);

            if (checkout_confirm == "Y" || checkout_confirm == "YES") {
                cout << " Specify Mode of Payment: ";
                string payment_method;
                getline(cin, payment_method);

                // Update stock quantities
                for (size_t ci = 0; ci < temporaryCart.size(); ++ci) {
                    const CartItem &ciRef = temporaryCart[ci];
                    for (size_t p = 0; p < inventory.size(); ++p) {
                        if (inventory[p].id == ciRef.productId) {
                            inventory[p].stockQuantity -= ciRef.quantity;
                            if (inventory[p].stockQuantity < 0) inventory[p].stockQuantity = 0;
                            inventory[p].dateModified = CURRENT_DATE;
                            break;
                        }
                    }
                }

                // Reduce the number of uses left for the voucher
                if (appliedVoucher != "NONE") {
                    string usedU = changeToUppercase(appliedVoucher);
                    for (size_t v = 0; v < activeVouchers.size(); ++v) {
                        if (changeToUppercase(activeVouchers[v].code) == usedU) {
                            activeVouchers[v].usesRemaining -= 1;
                            if (activeVouchers[v].usesRemaining <= 0) {
                                cout << " Voucher [" << appliedVoucher << "] fully redeemed and removed.\n";
                                activeVouchers.erase(activeVouchers.begin() + v);
                            } else {
                                cout << " Voucher [" << appliedVoucher << "] applied! Uses left: "
                                     << activeVouchers[v].usesRemaining << "\n";
                            }
                            break;
                        }
                    }
                }

                generateReceiptFile(customerName, orderDate, customerAddress, temporaryCart,
                                    subtotal, appliedVoucher, total_discount_deduction,
                                    final_total_price, payment_method, CURRENT_DATE);

                cout << "\n Transaction complete!";
                cout << "=================================================================\n";

                appliedDiscount     = 0.0;
                appliedVoucher = "NONE";
                saveInventoryFile(inventory);
                saveVouchersFile(activeVouchers);
                return;

            } else {
                cout << " Checkout cancelled. Returning to order menu.\n";
            }

        } else if (orderAction == 5) {
            temporaryCart.clear();
            cout << " Order cancelled. Your cart has been cleared.\n";
            return;
        } else {
            cout << " Error: Invalid choice. Please enter a number from 1 to 5.\n";
        }
    }
}

// -------------------- Inventory Menu --------------------
void inventoryMenu(vector<Product> &inventory, vector<Voucher> &vouchers) {
    while (true) {
        cout << "\n=================================================================\n";
        cout << "               [ Inventory Management System ]                   \n";
        cout << "=================================================================\n\n";
        cout << " Please type the number of the operation to start:\n\n";
        cout << "    [1]  Search Products\n";
        cout << "    [2]  Add New Product\n";
        cout << "    [3]  Update Product Info\n";
        cout << "    [4]  Delete Products\n";
        cout << "    [5]  Display All Products\n";
        cout << "    [6]  Voucher Management\n";
        cout << "    [7]  Return to Main Menu\n\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int inv_action;
        if (!(cin >> inv_action)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << " Error: Invalid input. Returning to menu.\n";
            continue;
        }
        cin.ignore();

        if (inv_action == 1) {
            searchProducts(inventory, vouchers);
        } else if (inv_action == 2) {
            addNewProduct(inventory);
            saveInventoryFile(inventory);
        } else if (inv_action == 3) {
            cout << " Enter Product ID to modify: ";
            string id; getline(cin, id);
            bool found = false;
            for (size_t i = 0; i < inventory.size(); ++i) {
                if (changeToUppercase(inventory[i].id) == changeToUppercase(id)) {
                    found = true;
                    updateProduct(inventory, i);
                    saveInventoryFile(inventory);
                    break;
                }
            }
            if (!found) cout << " Error: Product ID not found.\n";
        } else if (inv_action == 4) {
            deleteProducts(inventory);
        } else if (inv_action == 5) {
            displayAllProducts(inventory);
        } else if (inv_action == 6) {
            voucherManagementMenu(vouchers);
        } else if (inv_action == 7) {
            saveInventoryFile(inventory);
            saveVouchersFile(vouchers);
            return;
        } else {
            cout << " Error: Invalid selection choice!\n";
        }
    }
}

// -------------------- Add product --------------------
// Prompt user to enter details for a new product
void addNewProduct(vector<Product> &inventory) {
    while (true) {
        cout << "\n=================================================================\n";
        cout << "                    [ Add New Product ]                          \n";
        cout << "=================================================================\n";
        cout << " Enter New Product Details:\n\n";
        Product p;
        cout << " Product ID    : "; getline(cin, p.id);

        bool idExists = false; // Check if the ID already exists
        for (const auto &item : inventory) {
            if (changeToUppercase(item.id) == changeToUppercase(p.id)) {
                idExists = true;
                break;
            }
        }

        if (idExists) {
            cout << " Error: A product with ID '" << p.id << "' already exists. Try again.\n";
            continue; // Restarts the main loop to ask for details again
        }

        cout << " Type          : "; getline(cin, p.type);
        cout << " Brand         : "; getline(cin, p.brand);
        cout << " Name          : "; getline(cin, p.name);

        while (cout << " Price         : " && (!(cin >> p.price) || p.price < 0.0)) {
            cin.clear(); cin.ignore(10000, '\n'); cout << " Error: Invalid price. Must be a non-negative number.\n";
        }
        while (cout << " Stock Quantity: " && (!(cin >> p.stockQuantity) || p.stockQuantity < 0)) {
            cin.clear(); cin.ignore(10000, '\n'); cout << " Error: Invalid quantity. Must be a non-negative number.\n";
        }
        cin.ignore(10000, '\n');

        p.dateModified = CURRENT_DATE;
        inventory.push_back(p);
        cout << "\n Product added: [" << p.id << "] " << p.name << '\n';
        cout << "-----------------------------------------------------------------\n";

        cout << "\n    [1]  Add another product\n";
        cout << "    [2]  Back to Inventory Menu\n\n";
        cout << "-----------------------------------------------------------------\n";
        cout << " Enter your choice: ";

        int opt;
        if (!(cin >> opt)) {
            cin.clear();
            cin.ignore(10000,'\n');
            cout << " Error: Invalid input. Returning to Inventory Menu.\n";
            return;
        }
        cin.ignore();

        if (opt == 1) continue;
        return;
    }
}

// Date
void date(string &CURRENT_DATE) {
    string year = "2026", month, day;
    int tempMonth = 0, tempDay = 0;

    while (true) {
        cout << "Enter current Month (MM): ";
        cin >> month;
        try { tempMonth = stoi(month); } catch (...) { tempMonth = 0; }
        if (tempMonth >= 1 && tempMonth <= 12) break;
        cout << "Error: Invalid month. Please enter a value between 01 and 12.\n";
    }

    while (true) {
        cout << "Enter current Day (DD): ";
        cin >> day;
        try { tempDay = stoi(day); } catch (...) { tempDay = 0; }
        int maxDay = (tempMonth == 2 ? 28 :
                     (tempMonth == 4 || tempMonth == 6 || tempMonth == 9 || tempMonth == 11 ? 30 : 31));

        if (tempDay >= 1 && tempDay <= maxDay) break;
        cout << "Error: Invalid day. Please enter a value between 01 and " << maxDay << ".\n";
    }

    cin.ignore(10000, '\n');
    if (tempMonth < 10 && month.size() == 1) month = "0" + month;
    if (tempDay < 10 && day.size() == 1) day = "0" + day;

    CURRENT_DATE = year + "-" + month + "-" + day;
}

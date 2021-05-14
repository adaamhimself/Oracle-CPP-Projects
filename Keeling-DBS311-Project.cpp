
#include <iostream>
#include <iomanip>
#include <sstream>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct Cart {
    int product_id = 0;
    double price = 0.0;
    int quantity = 0;
};

int mainMenu(); 
int customerLogin(Connection* conn, int customerId);
int addToCart(Connection* conn, struct Cart cart[]);
double findProduct(Connection* conn, int product_id);
void displayProducts(struct Cart cart[], int productCount);
int checkout(Connection* conn, struct Cart cart[], int customerId, int productCount);
bool isNum(string test);// helper utility
int getCustomerNumber(); // helper utility
string intToStr(int srcnum); // helper utility


int main()
{
    Environment* env = nullptr;
    Connection* conn = nullptr;
    string str;
    string usr = "dbs311_211e03"; // login
    string pass = "11614193"; // password
    string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";
    Cart sCart[5];
    int numInCart = 0;
    try
    {
        env = Environment::createEnvironment(Environment::DEFAULT);
        conn = env->createConnection(usr, pass, srv);
        cout << "Connection is Successful!" << endl << endl;
    }
    catch (SQLException& sqlExcp)
    {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    while (mainMenu() > 0)
    {
        int custID = getCustomerNumber();
        int loginStatus = customerLogin(conn, custID);
        if (loginStatus == 0) { cout << "The customer does not exist" << endl; }
        if (loginStatus)
        {
            numInCart = addToCart(conn, sCart);
            displayProducts(sCart, numInCart);
            checkout(conn, sCart, custID, numInCart);
        }
    }
    env->terminateConnection(conn);
    Environment::terminateEnvironment(env);
    cout << "Thank you --- Good bye..." << endl;
    return 0;
}


int mainMenu()
{
    int choice = -1;
    int custID = 0;
    bool running = true;
    while (running)
    {
        cout << "************* Main Menu by ********************" << endl;
        cout << "1)      Login" << endl;
        cout << "0)      Exit" << endl;
        cout << "Enter an option (0-1): ";
        cin >> choice;
        if (choice == 0 || choice == 1)
        {
            running = false;
        }
        while(choice != 0 && choice != 1)
        {
            cout << "************* Main Menu ********************" << endl;
            cout << "1)      Login" << endl;
            cout << "0)      Exit" << endl;
            cout << "You entered a wrong value. Enter an option (0-1): ";
            cin >> choice;
            if (choice == 0 || choice == 1) { running = false; }
        }

    }
    return choice;
}


int customerLogin(Connection* conn, int customerId)
{
    int id = 0;
    try {
        string findCustomerString = "BEGIN find_customer(" + to_string(customerId) + ", :1); END;";
        Statement* stmt = conn->createStatement(findCustomerString);
        stmt->registerOutParam(1, Type::OCCIINT);
        stmt->execute();
        id = stmt->getInt(1);
    }
    catch (SQLException& sqlExcp)
    {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    return id;
}

int addToCart(Connection* conn, struct Cart cart[])
{
    bool running = true;
    int productID = -1;
    int qty = 0;
    int addMore = 1;
    double price = 0.0;
    int i = 0;
    cout << "--------------Add Products to Cart--------------" << endl;
    while (i < 5 && addMore == 1)
    {
        cout << "Enter the product ID: ";
        cin >> productID;
        price = findProduct(conn, productID);
        while (price == 0.0)
        {
            cout << "The product does not exist. Please try again..." << endl;
            cout << "Enter the product ID: ";
            cin >> productID;
        }
        cout << "Product Price: " << fixed << setprecision(2) << price << endl;
        cout << "Enter the product Quantity: ";
        cin >> qty;
        cart[i].price = price;
        cart[i].product_id = productID;
        cart[i].quantity = qty;
        i++;
        cout << "Enter 1 to add more products or 0 to checkout: ";
        cin >> addMore;
    }
    return i;
}


double findProduct(Connection* conn, int product_id)
{
    double price = 0.0;
    try
    {
        string findCustomerString = "BEGIN find_product(" + to_string(product_id) + ", :1); END;";
        Statement* stmt = conn->createStatement(findCustomerString);
        stmt->registerOutParam(1, Type::OCCIDOUBLE);
        stmt->execute();
        price = stmt->getDouble(1);
    }
    catch (SQLException& sqlExcp)
    {
        cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    return price;
}


void displayProducts(struct Cart cart[], int productCount)
{

}


int checkout(Connection* conn, struct Cart cart[], int customerId, int productCount)
{
    char choice = 'x';
    bool running = true;
    int returnValue = 1;
    int newOrderNum = 0;
    while (running)
    {
        cout << "Would you like to checkout? (Y/y or N/n) ";
        cin >> choice;
        while (choice != 'Y' && choice != 'y' && choice != 'N' && choice != 'n')
        {
            cout << "Wrong input. Please try again..." << endl;
            cout << "Would you like to checkout? (Y/y or N/n) ";
            cin >> choice;
        }
        if (choice == 'N' || choice == 'n')
        {
            cout << "The order is cancelled." << endl;
            returnValue = 0;
            running = false;
        }
        else {
            
            try
            {
                string addOrderString = "BEGIN add_order(" + to_string(customerId) + ", :1); END;";
                Statement* stmt = conn->createStatement(addOrderString);
                stmt->registerOutParam(1, Type::OCCIINT);
                stmt->executeUpdate();
                newOrderNum = stmt->getInt(1);
            }
            catch (SQLException& sqlExcp)
            {
                cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
                returnValue = 0;
            }
            try
            {
                for (int i = 0; i < productCount; i++)
                {
                    string addOrderLineString = "BEGIN add_orderline(" + to_string(newOrderNum) + ", " + to_string(i + 1) + ", " + to_string(cart[i].product_id) + ", " + to_string(cart[i].quantity) + ", " + to_string(cart[i].price) + "); END;";
                    Statement* stmt = conn->createStatement(addOrderLineString);
                    stmt->executeUpdate();
                }
                cout << "The order is successfully completed." << endl;
            }
            catch (SQLException& sqlExcp)
            {
                cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
                returnValue = 0;
            }
            running = false;

        }
        
        return returnValue;
    }
}

// **************************************************************************
// * Helper utilities                                                       *
// **************************************************************************


// isNum takes a string and returns true if it is a number, false otherwise.
bool isNum(string test)
{
    bool returnvalue = false;
    for (int i = 0; i < test.length(); i++)
        if (isdigit(test[i]) == false) {
            returnvalue = false;
        }
        else {
            returnvalue = true;
        }
    return returnvalue;
}


// Input method that takes an input and verifies that it is a number. Conversion to string
// is done in order to parse the information and check for non-numeric values. Int is returned.
int getCustomerNumber()
{
    bool running = true;
    string num;
    while (running) {
        string valid;
        cout << "Enter the customer ID: ";
        cin >> num;
        if (isNum(num) == 0) {
            cout << "Not a number" << endl;
        }

        if (num == "0") {
            cout << endl;
            running = false;
        }
        if (isNum(num)) {
            running = false;
        }
    }
    int newnum = 0;
    istringstream(num) >> newnum;
    return newnum;
}


// Converts an int into a string so employeeNum can be inserted into a query statement.
string intToStr(int srcnum)
{
    stringstream temp;
    temp << srcnum;
    string num;
    temp >> num;
    return num;
}


// **************
// * Procedures *
// **************

/*

// Find Customer Procedure
CREATE OR REPLACE PROCEDURE find_customer(customer_id IN NUMBER, found OUT NUMBER) AS
    e_customerID customers.cust_no%type;
BEGIN
    SELECT cust_no 
    INTO e_customerID
    FROM customers
    WHERE cust_no = customer_id;
    found := 1;
    EXCEPTION
        WHEN no_data_found THEN 
        found := 0;
END;

// Find Product Procedure
CREATE OR REPLACE PROCEDURE find_product(product_id IN NUMBER, price OUT products.prod_sell%TYPE) AS
    e_productID products.prod_no%type;
    e_prodPrice products.prod_sell%type;
BEGIN
    SELECT prod_no, prod_sell
    INTO e_productID, e_prodPrice
    FROM products
    WHERE prod_no = product_id;
    price := e_prodPrice;
    EXCEPTION
        WHEN no_data_found THEN
        price := 0;
END;

// Add Order Procedure
CREATE OR REPLACE PROCEDURE add_order (customer_id IN NUMBER, new_order_id OUT NUMBER) AS
    new_status orders.status%type;
    new_repNo orders.rep_no%type;
BEGIN
    new_status := 'O';
    new_repNo := 36;
    SELECT MAX(order_no) 
    INTO new_order_id
    FROM orders;
    new_order_id := new_order_id + 1;
    INSERT INTO orders (order_no, cust_no, status, rep_no, order_dt)
    VALUES (new_order_id, customer_id, new_status, new_repNo, SYSDATE);
END;

// Add Orderline Procedure
CREATE OR REPLACE PROCEDURE add_orderline (orderId IN orderlines.order_no%type,
                                           itemId IN orderlines.line_no%type,
                                           productId IN orderlines.prod_no%type,
                                           quantity IN orderlines.qty%type,
                                           price IN orderlines.price%type) AS
BEGIN
    INSERT INTO orderlines (order_no, line_no, prod_no, qty, price)
    VALUES (orderId, itemId, productId, quantity, price);
END;

*/

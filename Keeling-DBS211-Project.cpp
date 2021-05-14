#define _CRT_SECURE_NO_WARNINGS
#include <iostream> 
#include <iomanip>
#include <occi.h> 
#include <ctype.h>
#include <sstream>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct Employee {
  int employeeNumber;
  char lastName[50];
  char firstName[50];
  char extension[10];
  char email[100];
  char officecode[10];
  int reportsTo;
  char jobTitle[50];
};

// Methods that require a variable of type Connection to be passed in.
int validateEmployee(Connection* conn, int employeeNumber, struct Employee* emp);
void storeValidData(Connection* conn, int employeeNumber, struct Employee* emp);
void displayEmployee(Connection* conn, struct Employee* emp);
void displayAllEmployees(Connection* conn);

// Methods that do not require a Connection type variable to be passed in.
int menu(void);
void displayMenu();
int getEmployeeNumber();
bool isNum(string test);
string intToStr(int srcnum);

int main(void) {
  // Variables needed to pass connection information to methods. A better way of doing this
  // would be to store all of this information, including username and password, into a struct.
  Connection* conn = nullptr;
  struct Employee emp[10] = { { 0 } };

  // Runs the menu.
  bool menu_run = true;
  while (menu_run) {
    int choice = menu();
    if (choice == 0) {
      menu_run = 0;
    }
    if (choice == 1) {
      int employeeNumber = getEmployeeNumber();
      int check = validateEmployee(conn, employeeNumber, emp);
      if (check >= 1) {
        cout << endl;
        storeValidData(conn, employeeNumber, emp);
        displayEmployee(conn, emp);
      }
      else if (check == 0) {
        cout << "Employee " << employeeNumber << " does not exist." << endl;
        cout << endl;
      }
    }
    if (choice == 2) {
      displayAllEmployees(conn);
    }
  }
  return 0;
}

// *************************************************
// * Methods that require Connection type variable *
// *************************************************

// Sends a select query and returns either a 0, which indicates no employee found, or
// returns the employee's id number to the main method. Verification put into it's own
// method to facilitate reusability.
int validateEmployee(Connection* conn, int employeeNumber, struct Employee* emp) {
  Environment* env = nullptr;
  string str;
  string usr = "dbs211_202a13";
  string pass = "28367423";
  string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";
  bool running = true;
  int returnValue = 0;
  while (running) {
    // Verification try block
    try {
      env = Environment::createEnvironment(Environment::DEFAULT);
      conn = env->createConnection(usr, pass, srv);
      Statement* stmt = conn->createStatement();
      string employeeNum = intToStr(employeeNumber);
      string querystring = "SELECT employeenumber FROM employees WHERE employeenumber = '" + employeeNum + "' ORDER BY employeenumber";
      ResultSet* rs = stmt->executeQuery(querystring);

      rs->next();
      int empNum = rs->getInt(1);
      returnValue = empNum;
      conn->terminateStatement(stmt);
      env->terminateConnection(conn);
      Environment::terminateEnvironment(env);
    }
    catch (SQLException& sqlExcp) {
      cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    running = false;
  }

  return returnValue;
}

// Takes an employee id, performs a query on that id's associated records, and stores
// it in the emp struct for later use.
void storeValidData(Connection* conn, int employeeNumber, struct Employee* emp) {
  Environment* env = nullptr;
  string str;
  string usr = "dbs211_202a13";
  string pass = "28367423";
  string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";
  bool running = true;
  while (running) {
    try {
      env = Environment::createEnvironment(Environment::DEFAULT);
      conn = env->createConnection(usr, pass, srv);
      Statement* stmt = conn->createStatement();
      string employeeNum = intToStr(employeeNumber);
      string querystring = "SELECT employeenumber, lastname, firstname, extension, email, officecode, reportsto, jobtitle FROM employees WHERE employeenumber = '" + employeeNum + "' ORDER BY employeenumber";
      ResultSet* rs = stmt->executeQuery(querystring);

      while (rs->next()) {
        int employeeno = rs->getInt(1);
        emp->employeeNumber = employeeno;
        string lastName = rs->getString(2);
        strcpy(emp->lastName, lastName.c_str());
        string firstName = rs->getString(3);
        strcpy(emp->firstName, firstName.c_str());
        string extension = rs->getString(4);
        strcpy(emp->extension, extension.c_str());
        string email = rs->getString(5);
        strcpy(emp->email, email.c_str());
        string officecode = rs->getString(6);
        strcpy(emp->officecode, officecode.c_str());
        int reportsto = rs->getInt(7);
        string jobtitle = rs->getString(8);
        strcpy(emp->jobTitle, jobtitle.c_str());
      }
      conn->terminateStatement(stmt);
      env->terminateConnection(conn);
      Environment::terminateEnvironment(env);
    }
    catch (SQLException& sqlExcp) {
      cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    running = false;
  }
}

// Displays a single employee's information by reading from emp and outputting
// to the display.
void displayEmployee(Connection* conn, struct Employee* emp) {
  cout << "-------------- Employee Information -------------" << endl;
  cout << "Employee Number: " << emp->employeeNumber << endl;
  cout << "Last Name: " << emp->lastName << endl;
  cout << "First Name: " << emp->firstName << endl;
  cout << "Extension: " << emp->extension << endl;
  cout << "Email: " << emp->email << endl;
  cout << "Office Code: " << emp->officecode << endl;
  cout << "Manager ID: " << emp->reportsTo << endl;
  cout << "Job Title: " << emp->jobTitle << endl << endl;
}

// Displays the information for all employees in the database along with the name of their
// manager and the office phone number. The is accomplished with a self join and an 
// inner join.
void displayAllEmployees(Connection* conn) {
  Environment* env = nullptr;
  string str;
  string usr = "dbs211_202a13";
  string pass = "28367423";
  string srv = "myoracle12c.senecacollege.ca:1521/oracle12c";
  bool running = true;
  while (running) {
    try {
      env = Environment::createEnvironment(Environment::DEFAULT);
      conn = env->createConnection(usr, pass, srv);
      Statement* stmt = conn->createStatement();

      bool running = true;
      string querystring = "SELECT a.employeenumber, a.firstname || ' ' || a.lastname employeename, a.email, c.phone, a.extension, b.firstname || ' ' || b.lastname AS manager FROM employees b RIGHT JOIN employees a ON a.reportsto = b.employeenumber INNER JOIN offices c ON c.officecode = b.officecode";
      cout << endl;

      ResultSet* rs = stmt->executeQuery(querystring);

      cout << "-----  ---------------     ---------------------------------  ----------------  ---------   -----------------" << endl;
      cout << std::setw(7) << std::left << setfill(' ') << "ID";
      cout << std::setw(20) << std::left << std::fixed << "Employee Name";
      cout << std::setw(35) << std::left << std::fixed << "Email";
      cout << std::setw(18) << std::left << std::fixed << "Phone";
      cout << std::setw(12) << std::left << std::fixed << "Extension";
      cout << std::setw(17) << std::left << std::fixed << "Manager Name" << endl;
      cout << "-----  ---------------     ---------------------------------  ----------------  ---------   -----------------" << endl;
      int employeeno = 0;
      bool check = true;
      while (rs->next()) {
        employeeno = rs->getInt(1);
        string employeename = rs->getString(2);
        string email = rs->getString(3);
        string phone = rs->getString(4);
        string extension = rs->getString(5);
        string manager = rs->getString(6);

        if (employeeno == 0) {
          check = false;
        }
        else if (employeeno != 0) {
          cout << std::setw(7) << std::left << setfill(' ') << std::fixed << employeeno;
          cout << std::setw(20) << std::left << std::fixed << employeename;
          cout << std::setw(35) << std::left << std::fixed << email;
          cout << std::setw(18) << std::left << std::fixed << phone;
          cout << std::setw(12) << std::left << std::fixed << extension;
          cout << std::setw(17) << std::left << std::fixed << manager << endl;
        }
      }
      if (check == false) {
        cout << "There is no employees' informaion to be displayed." << endl;
      }
      cout << endl;
      conn->terminateStatement(stmt);
      env->terminateConnection(conn);
      Environment::terminateEnvironment(env);
    }
    catch (SQLException& sqlExcp) {
      cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
    }
    running = false;
  }
}

// ******************
// * Helper methods *
// ******************

// Reduces clutter in the main method.
int menu(void) {
  bool running = true;
  int choice = -1;
  displayMenu();
  cout << "Enter an option (0-5): ";
  cin >> choice;
  return choice;
}

// Menu output code moved into its own method to reduce clutter.
void displayMenu() {
  cout << "********************* HR Menu *********************" << endl;
  cout << "1) Find Employee" << endl;
  cout << "2) Employees Report" << endl;
  cout << "3) Add Employee" << endl;
  cout << "4) Update Employee" << endl;
  cout << "5) Remove Employee" << endl;
  cout << "0) Exit" << endl;
}

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
int getEmployeeNumber() {
  bool running = true;
  string num;
  while (running) {
    string valid;
    cout << "Enter employee number or 0 to exit: ";
    cin >> num;
    if (isNum(num) == 0) {
      cout << endl;
      cout << "Not a number" << endl << endl;
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
string intToStr(int srcnum) {
  stringstream temp;
  temp << srcnum;
  string num;
  temp >> num;
  return num;
}
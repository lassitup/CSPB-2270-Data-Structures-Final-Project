#ifndef SUPPORT_H
#define SUPPORT_H


#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include "red_black_tree.h"


using namespace std;


/*--------------------------------------Account and Customer Node Structures ---------------------------------------------------*/


/*
    This structure holds the contents of a customer's account.
*/
struct Account_Node
{
    int account_number;
    string interest_rate;
    string account_type;
    string class_code_description;
    double current_balance;
};

/*
    This structure holds the contents / details of a Customer as well as all securities
    and accounts tied to the customer
*/
struct Customer_Node
{
    int pledge_code;
    long int tax_ID;
    string name1;
    string name2;

    //vector to hold all accounts tied to the customer
    vector<Account_Node*> accounts;

    //vector to hold all securities currently pledged to the customer
    vector<RBT_Security_Node*> pledged_to_customer;

   //calculates the total of all securities pledged to the customer
    double total_securities_pledged(vector<RBT_Security_Node*> securities_pledged)
    {   
        double total = 0;
        for(size_t i = 0; i < securities_pledged.size(); i++)
        {
            total += securities_pledged.at(i)->market_value;
        }
        return total;
    }

    //holds the sum of all securities pledged
    double total_pledged;

    //calculates the total of all accounts tied to the customer
    double total_account_balance(vector<Account_Node*> accounts)
    {
        double total = 0;
        for (size_t i = 0; i < accounts.size(); i++)
        {
            total += accounts.at(i)->current_balance;
        }
        return total;
    }
    //holds the aggregate balance of all accounts
    double total_balance;

    //calculates the current state of being over or under pledged
    double calculate_under_over(double total_pledged, double total_balance)
    {
        return total_pledged - total_balance;
    }
    
    //maintains the account balance(s) vs security values assigned - shows if over or under pledged
    double over_under; 
};


/*-------------------------------Program Build Functions (Red Black Tree and Customer Map) -------------------------------------*/

/*
    Function is called to consruct an account node. The vector parameter 
    consists of data from each customer line within the loaded csv file. 
    Data conversions are performed here
*/
Account_Node* build_account_node(const vector<string>& customer_data);

/*
    Function is called to consruct a customer node. The vector parameter 
    consists of data from each customer line within the loaded csv file. 
    Data conversions are performed here
*/
Customer_Node* build_customer_node(const vector<string>& customer_data);

//create function to read in data from security csv file
//this will eventually call the functions to build the red black tree - just using vector for right now
//need to add tests to check if file was successfully loaded

/*
    Function is called to import security data from the security source file. 
    The function will add already pledged securities to the customer listed
    on the source. If the customer is no longer in the customer map structure, 
    the security will be 'unpledged' and added to the pledge removal vector.
*/
RBT import_and_build_RBT_tree(map<int, Customer_Node*> customers, vector<RBT_Security_Node*>& pledge_removals, ifstream& security_file);

/*
    Function is called to import customer data from the customer source file. 
    For each customer in file, a customer node will be built and added to the map. 
    Any customer accounts added, where the customer is already in the map, the account
    will be added to that customers struct.
*/
map<int, Customer_Node*> load_customer_data(ifstream& customer_file);


/*---------------------------------------Security Seach / Add and Removal Functions --------------------------------------------*/


/*
    Function is called to traverse the red-black tree in search of 
    a security with the desired balance. The min and max parameters indicate the bottom
    and top of the search threshold. Any security idenfified within this range will be
    returned. If no security is found, nullptr is returned.
*/
RBT_Security_Node* find_security(RBT_Security_Node* root, double min, double max);

/*
    Function is called to perform customer pleding updates. Ultimate goal of this function is to test if enough securities can be
    pledged to a customer balance. Two methods are used 1. using the needed balance plus a threshold, and if no security is within this
    threshold, it will then search for the next security smaller than the min value. If a smaller security is found, the threshold is reset to the
    updated over_under balance and try to find a security in this range. This will continue until the balance is covered, if possible.
    and 2. balances at the needed amount and above are reviewed to see if the balance can be covered. Note that the actual search procedures 
    are performed within the increase_decrease_search function. The smaller total market value of all securities identified of the two methods 
    is used to actually pledge to a customer. Those not used are added back to the red-black tree. If at anypoint where a customer balance 
    cannot be covered (both methods return false), the function returns false. True will only be returned if, for each customer needing pledging 
    updates, all customer balances were adequately covered.
*/
bool update_customers(map<int, Customer_Node*> customers, RBT tree, vector<RBT_Security_Node*>& additions, double threshold = .5);
/*
    Function is called to perform customer pleding updates. As an alternative to the update customer function above. This function unpledges
    all securties for the entire customer map. This allows a 'redistribution' of securties as some securities appropriate for update may have
    already been pledged. This gives the program a chance to find more apprpriate market values. This function will continuously loop starting
    with a max threshold of 50% decrementing by 1% each iteration to lower the threshold until it reaches the actual balance needed for each customer.
    This function returns true only if all customers are sufficiently pledged.
*/
bool clear_all_and_repledge(RBT tree, map<int, Customer_Node*> customers, vector<RBT_Security_Node*>& additions, vector<RBT_Security_Node*>& removals);
/*
    Function is called to perform the actual over-under pledged balance testing, adding securities where possible. Within this function, find_security
    is frequently called to search for securities to cover the balance. Depending on the direction parameter, the function will search smaller securities
    or larger securities. False is smaller, True is larger.
*/
bool increase_decrease_search(RBT tree, double over_under, bool direction, vector<RBT_Security_Node*>& used_securities, double threshold);

/*
    Function is called to free memory and clear out the customer map - This would primarily be used if a new customer file is loaded
    and a new customer base is established.
*/
void clear_customers(map<int, Customer_Node*>& customers);

/*
    Function is called to remove all pledges attached to each customer. The security will be added back to the 
    red-black tree for reuse in future searches. If the unpledge parameter is set to true, the security will be added
    the unpledge_secrutiy list indicating that the security was initially pledge to the customer and needs to be officially
    unpledged. Otherwise, the security is simply added back to the tree. This is due to potentially several iterations being 
    needed and willavoid adding duplicate removals to the vector.
*/
void clear_pledges(RBT tree, map<int, Customer_Node *> customers, vector<RBT_Security_Node*>& removals, bool unpledge);

/*
    Function is called to perform initial check of customer under_over balances. If any are initially in excess of 50% 
    of the customer's aggregate account balance, it removes / unpledges them.
*/
void test_overage(map<int, Customer_Node*>& customers,vector<RBT_Security_Node*>& removals, RBT tree);

/*
    Function is called to free memory and clear all additions and removals changes included in
    these vectors.
*/
void clear_changes(vector<RBT_Security_Node*>& removals, vector<RBT_Security_Node*>& additions);


/*---------------------------------------------- Display and Export Functions --------------------------------------------------*/

/*
    Function is called to display details of customer's current state.
*/
void display_customers(map<int, Customer_Node*> customers, bool all_customers = true);

/*
    Function is called to display details of current state of changes to be made as a result of running the program.
*/
void display_changes(vector<RBT_Security_Node*> removals, vector<RBT_Security_Node*> additions);

/*
    Function is called to export details of customer's current state to a csv file.
*/
void export_customers(map<int, Customer_Node*>& customers);

/*
    Function is called to display details of current state of changes to be made to a csv file.
*/
void export_changes(vector<RBT_Security_Node*> removals, vector<RBT_Security_Node*> additions);


/*---------------------------------------------------- Utility Functions -------------------------------------------------------*/

/*
    Function is called to perform updates of a customer's total aggregate balances for securities pledge
    and accounts. Additionally, it performs an update to the over_under net pledge balance.
*/
void update_balances(Customer_Node* customer);

/*
    Function is called to prompt user to enter menu selection. The function will only accept digits 1 - 10
    as input, either in integer or decimal format. If any other number is provided, it will continuosly 
    prompt the user for an acceptable selection. If any other character is provided, the function will end
    effectively ending the program.
*/
int interface_validate();

/*
    Function copies the contents of one map to another
*/
void copy_customer_map(map<int, Customer_Node*>& customers, map<int, Customer_Node*>& customers_copy, RBT tree);

/*
    Function returns a copy node of the node passed into the function
*/
Account_Node* copy_account(Account_Node* original);

/*
    Function is called to file names from the user and open the file from with data will be imported.
    If the function fails to open the provided file name, it will continuosly prompt user to re-enter 
    the file name.
*/
void open_file(ifstream& file);

/*
    Function returns a copy of the RBT_Security_Node vector passed into the function
*/
vector<RBT_Security_Node*> copy_removals(vector<RBT_Security_Node*>& removals, RBT tree);

/*
    Function frees the memory of each pointer within a vector and then clears the vector itself
*/
void clear_vector(vector<RBT_Security_Node*>& to_clear);

/*
    Function frees removes nodes from the passed vector and adds them to the passed Red Black Tree
    The node references are not deleted, but swapped from the vector to the tree. 
*/
void restore_tree(vector<RBT_Security_Node*> to_restore, RBT tree);

/*
    Function removes securities from the tree that were added back to the tree as part of clear all and repledge. These 
    securities should no longer be in the tree when restarting a process as these are already pledged to a customer
*/
void remove_additions(map<int, Customer_Node*>& customers, RBT tree);

#endif
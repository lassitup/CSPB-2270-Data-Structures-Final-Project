#include "supporting_func_structs.h"
#include "red_black_tree.h"

using namespace std;





/*-------------------------------Program Build Functions (Red Black Tree and Customer Map) -------------------------------------*/

Account_Node *build_account_node(const vector<string> &customer_data)
{
    Account_Node *new_account = new Account_Node;
    new_account->account_number = stoi(customer_data.at(4));
    new_account->interest_rate = customer_data.at(5);
    new_account->account_type = customer_data.at(6);
    new_account->class_code_description = customer_data.at(7);
    new_account->current_balance = stod(customer_data.at(8));
    return new_account;
}

Customer_Node *build_customer_node(const vector<string> &customer_data)
{
    Customer_Node *new_customer = new Customer_Node;
    new_customer->pledge_code = stoi(customer_data.at(0));
    new_customer->tax_ID = stol(customer_data.at(1));
    new_customer->name1 = customer_data.at(2);
    new_customer->name2 = customer_data.at(3);
    new_customer->accounts.push_back(build_account_node(customer_data));
    return new_customer;
}


RBT import_and_build_RBT_tree(map<int, Customer_Node *> customers, vector<RBT_Security_Node *> &pledge_removals, ifstream& security_file)
{
    //construct new Red-Black tree object -once all securities are loaded into  the tree, the tree object is returned
    RBT security_tree;

    string security_line;

    //this assignment to security_line will 'absorb' the header line from the csv file
    getline(security_file, security_line); 


    while (getline(security_file, security_line))
    {

        stringstream security_detail_line(security_line);

        string temp_string;
        vector<string> temp_vector; //this holds each 'element' in the csv line, which will be used to create a node

        while (getline(security_detail_line, temp_string, ','))
        {
            temp_vector.push_back(temp_string);
        }

        //build out the security node
        RBT_Security_Node *next_security = security_tree.build_security_node(temp_vector);

        //check if the pledge ID (customer) is already in the map, if it is, add it to the customer
        if (customers.find(next_security->pledge_id) != customers.end())
        {
            Customer_Node *customer = customers[next_security->pledge_id];
            //add security to customer
            customer->pledged_to_customer.push_back(next_security);
            //Update all balances in customer node
            update_balances(customer);
        }
        else
        {   //if the customer is not in the map, free up the security and add to the tree
            //regardless if security was originally pledged, clear id and description and add to tree
            //makes a copy of the node to store all components into the unpledge vector, which will later be exported
            //copy allows each node in the tree to be independent of the unpledge vector. *RBTree Operations can cause nodes to 'change'
            //add the original security node to the tree
            RBT_Security_Node *security_copy = security_tree.RBT_copy_node(next_security);
            if(security_copy->pledge_id != 0)
            { //only add it to the removal list if it was previously assigned to a customer that is no longer in the map
                security_copy->change_status = "Unpledge";
                pledge_removals.push_back(security_copy);
            } 
            next_security->pledge_id = 0;
            next_security->pledge_description = "";
            security_tree.RBT_add_node(next_security);
        }
    }
    security_file.close();
    return security_tree;
}


map<int, Customer_Node *> load_customer_data(ifstream& customer_file)
{
    map<int, Customer_Node *> customers;

    string customer_line;

    //this assignment to security_line will 'absorb' the header line from the csv file
    getline(customer_file, customer_line);

    while (getline(customer_file, customer_line))
    {
        stringstream security_detail_line(customer_line);

        string temp_string;
        vector<string> temp_vector; //this holds each 'element' in the csv line, which will be used to create a node

        while (getline(security_detail_line, temp_string, ','))
        {
            temp_vector.push_back(temp_string);
        }

        int pledge_id = stoi(temp_vector.at(0));
        Customer_Node *customer;

        if (customers.find(pledge_id) == customers.end()) //check to see if customer is already in the mapping
        {
            customer = build_customer_node(temp_vector); //only build new node if not already in map

            customers[customer->pledge_code] = customer;
        }
        else
        {  //If the customer is already in the map, add account to the already created customer node
            customer = customers[pledge_id];

            Account_Node *new_account = build_account_node(temp_vector);

            customer->accounts.push_back(new_account);
        }
        //Update all balances in customer node
        update_balances(customer);
    }
    customer_file.close();
    return customers;
}


/*---------------------------------------Security Seach / Add and Removal Functions --------------------------------------------*/

RBT_Security_Node *find_security(RBT_Security_Node *root, double min, double max)
{
    RBT_Security_Node *cursor = root;

    while (cursor != nullptr)
    {
        //check to see if current security value is within range, if so, return it
        if (cursor->market_value >= min && cursor->market_value <= max)
        {
            return cursor;
        }
        else if (min < cursor->market_value)
        {
            cursor = cursor->left_child;
        }
        else
        {
            cursor = cursor->right_child;
        }
    }
    return nullptr; //if an appropriate security is not found, return null
}

bool update_customers(map<int, Customer_Node *> customers, RBT tree, vector<RBT_Security_Node*>& additions, double threshold)
{
    //array holding all customers with underpeldged balances that need to be updated 
    vector<Customer_Node *> updates_needed; 
    for (map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        //determine which customers are underpledged
        Customer_Node *current = pair->second;
        if (current->over_under < 0)
        {
            updates_needed.push_back(current);
        }
    }
    for (size_t i = 0; i < updates_needed.size(); i++)
    {
        Customer_Node *to_update = updates_needed.at(i);

        //small and large vectors to hold the securities pledge using each method (searching both smaller and larger balances)
        //the aggregate total of each vector will be compared and the smaller value will be used
        vector<RBT_Security_Node *> small;
        vector<RBT_Security_Node *> large;
        double small_sum = 0;
        double large_sum = 0;

        //perform search using the small method
        bool pledge_status_small = increase_decrease_search(tree, to_update->over_under, false, small, threshold);
        
        //temporary vector to hold the securities found during the small search method
        //the nodes herein will be used to identify which securities to remove from the tree below
        vector<RBT_Security_Node*> small_security_hold;
        //re-enter copied securities back into the tree - this ensures that they are available
        //for the large search method
        for (RBT_Security_Node *security : small)
        {
            RBT_Security_Node *copy1 = tree.RBT_copy_node(security);
            RBT_Security_Node *copy2 = tree.RBT_copy_node(security);
            copy1->pledge_id = 0;
            copy1->pledge_description = "";
            tree.RBT_add_node(copy1);
            small_security_hold.push_back(copy2);
        }
        //perform search usign the large method    
         bool pledge_status_large = increase_decrease_search(tree, to_update->over_under, true, large, threshold);
         

        if (!(pledge_status_large || pledge_status_small))
        {
            //if neither one of the methods result in the security being covered, the function returns false
            //the large method securities used (those within the 'large' vector) must be re-entered into the tree
            for(size_t unused = 0; unused < large.size(); unused++)
            {
                RBT_Security_Node *copy = tree.RBT_copy_node(large.at(unused));
                copy->pledge_id = 0;
                copy->pledge_description = "";
                tree.RBT_add_node(copy);
                delete large.at(unused);
            }
            for(size_t unused = 0; unused < small_security_hold.size(); unused++)
            {
                //small has already been replaced in the tree above
                //free the memory of nodes that were added here
                delete small_security_hold.at(unused);
            }
            for(size_t unused = 0; unused < small.size(); unused++)
            {
                //small has already been replaced in the tree above
                //free the memory of nodes that were added here
                delete small.at(unused);
            }
            return false;
        }

        for (RBT_Security_Node *security : small)
        {
            small_sum += security->market_value;
        }

        for (RBT_Security_Node *security : large)
        {
            large_sum += security->market_value;
        }
        //This is checking which path resulted in less additions value - the path with the lessor amount
        //should be used to reduce excess value pledged. Converted to int for comparison. Otherwise, when values
        //are exact, proper comparison wasn't performed since they were double values
        int small_sum_convert = small_sum;
        int large_sum_convert = large_sum;
        if (small_sum_convert < large_sum_convert || small_sum == to_update->over_under)
        {   //when small sum is smaller, assign these securities to the customers and add to the additions vector
            for (size_t add_security = 0; add_security < small.size(); add_security++)
            {
                small.at(add_security)->pledge_id = to_update->pledge_code;
                small.at(add_security)->pledge_description = to_update->name1;
                to_update->pledged_to_customer.push_back(small.at(add_security)); 
                RBT_Security_Node* copy = tree.RBT_copy_node(small.at(add_security));
                copy->change_status = "Pledge";
                additions.push_back(copy);
                update_balances(to_update);
            }
            //restore the unused large method securities to the tree and free up it's memory allocation
            for(size_t unused = 0; unused < large.size(); unused++)
            {
                RBT_Security_Node *copy = tree.RBT_copy_node(large.at(unused));
                copy->pledge_id = 0;
                copy->pledge_description = "";
                tree.RBT_add_node(copy);
                delete large.at(unused);
            }
            //need to remove the duplicate "replaced" securities so they are not added twice to the tree
            //since these were re-added to the tree above before the 'large' method was performed, they are currently
            //in the tree as well as being pledge to the customer above. This will find the security it the tree
            //and then remove it.
            for(size_t used = 0; used < small_security_hold.size(); used++)
            {
                int ticket = small_security_hold.at(used)->ticket;
                double mv = small_security_hold.at(used)->market_value;
                RBT_Security_Node* to_remove = tree.find_node(tree.get_root(), ticket, mv);
                tree.RBT_remove_node(to_remove);
                delete small_security_hold.at(used);
            }
        }
        else
        {   //if the large method results in a smaller excess amount, it's security nodes at added to the customer
            for (size_t add_security = 0; add_security < large.size(); add_security++)
            {
                large.at(add_security)->pledge_id = to_update->pledge_code;
                large.at(add_security)->pledge_description = to_update->name1;
                to_update->pledged_to_customer.push_back(large.at(add_security));
                RBT_Security_Node* copy = tree.RBT_copy_node(large.at(add_security));
                copy->change_status = "Pledge";
                additions.push_back(copy);
                update_balances(to_update);
            }
            for(size_t unused = 0; unused < small.size(); unused++)
            {
                //small has already been replaced in the tree above
                //free the memory of nodes that were added here
                delete small.at(unused);
            }
            for(size_t unused = 0; unused < small_security_hold.size(); unused++)
            {
                //small has already been replaced in the tree above
                //free the memory of nodes that were added here
                delete small_security_hold.at(unused);
            }
        }
    }
    return true;
}

bool clear_all_and_repledge(RBT tree, map<int, Customer_Node *> customers, vector<RBT_Security_Node*>& additions, vector<RBT_Security_Node*>& removals)
{
    //first step - clear all securities currently pledged to customers and add back to the tree
    //making the securities available for the new search
    clear_pledges(tree, customers, removals, true);
    //set initial threshold - gets reduced to 50% in the initial iteration below
    double threshold = .51;
    //this will only be set to true if all customers have their balances covered
    bool status = false;
    
    while(!status && threshold > 0)
    { 
        threshold -= .01;
        //when this goes below 0, set it to 0 - this will indicate a search for securities at their exact value
        if(threshold < 0) {threshold = 0;} 
        status = update_customers(customers, tree, additions, threshold);
        if(!status)
        {   
            //free memory of nodes in additions and clear out resetting for the next iteration
            for(size_t addition = 0; addition < additions.size(); addition++)
            {
                delete additions.at(addition);
            }
            additions.clear();
            //clear all securities pledged as a result of the pevious iteration
            //in preparation of the next round
            clear_pledges(tree, customers, removals, false);

        }
    }
    return status;
}

bool increase_decrease_search(RBT tree, double over_under, bool direction, vector<RBT_Security_Node *> &used_securities, double threshold)
{
    double min = over_under * -1;
    double max;
    RBT_Security_Node *smallest_mv = tree.find_minimum(tree.get_root());
    RBT_Security_Node *largest_mv = tree.find_maximum(tree.get_root());
    double temporary_over_under = over_under; //using a copy of the under_over balance to determine coverage

    if (!direction)
    {
        max = temporary_over_under * -1 * (1 + threshold);
    }
    else
    {
        //this will find the first security in the tree large enough to cover the value or the next largest one - it will continue to find smaller ones
        max = __DBL_MAX__;
    }
    while (temporary_over_under < 0)
    {
        RBT_Security_Node *security = find_security(tree.get_root(), min, max);
        if (security != nullptr)
        {   
            //if an appropriate security is found, the temporary over under is increase with the security's value
            //a copy of the security node is created and added to the vector holding the found securities
            //the security is then removed from the tree
            temporary_over_under += security->market_value;
            RBT_Security_Node *copy = tree.RBT_copy_node(security);
            used_securities.push_back(copy);
            tree.RBT_remove_node(security);

            //restablish smallest and largest securities as a result of removing nodes from the tree and rebalancing
            smallest_mv = tree.find_minimum(tree.get_root());
            largest_mv = tree.find_maximum(tree.get_root());

            if (temporary_over_under >= 0)
            {   //exit the loop/function, the balance is now covered
                break;
            }
        }
        //check if the tree is empty - exit in the loop/function and return false
        //if the over_under is still not covered at this point, it's impossible to cover
        if(tree.get_root() == nullptr)
        {
            return false;
        }
        //if the max of the threshold is smaller than the smallest available security
        //it will assigned the smallest security to the customer, as the customer must be covered
        if (max < smallest_mv->market_value)
        {
            temporary_over_under += smallest_mv->market_value;
            RBT_Security_Node *copy = tree.RBT_copy_node(smallest_mv);
            used_securities.push_back(copy);
            tree.RBT_remove_node(smallest_mv);
            //find new smallest security
            smallest_mv = tree.find_minimum(tree.get_root());
        }
        if (min > largest_mv->market_value)
        {
            temporary_over_under += largest_mv->market_value;
            RBT_Security_Node *copy = tree.RBT_copy_node(largest_mv);
            used_securities.push_back(copy);
            security = copy;
            tree.RBT_remove_node(largest_mv);
            //find new largest security
            largest_mv = tree.find_maximum(tree.get_root());
        }
        if (temporary_over_under > 0)
        { //exit the loop, the balance is now covered
            break;
        }

        if (security == nullptr && !direction)
        { //if a security wasn't found, search the entire field of securities at and below 
          //the under-over needed balance. This will beging search the tree starting at the root
          //moving 'left' down the tree.
            max = min;
            min = 0;
        }
        else if(security != nullptr && direction)
        {   //once a security is found, but the balance is not yet covered,
            //the min threshold is reset at the new temp over_under balance
            max = __DBL_MAX__;
            min = temporary_over_under * -1;
        }
        else if(security != nullptr && !direction)
        {   //once a security is found, but the balance is not yet covered,
            //the threshold is reset at the current under-over balance
            max = temporary_over_under * -1 * (1 + threshold);
            min = temporary_over_under * -1;
        }
    }
    return true;
}

void clear_customers(map<int, Customer_Node*>& customers)
{
    for (map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        //release the memory used by the pointer
        //free memory allocated to security nodes
        for(size_t i = 0; i < pair->second->pledged_to_customer.size(); i++)
        {
            delete pair->second->pledged_to_customer.at(i);
        }
        for(size_t i = 0; i < pair->second->accounts.size(); i++)
        {
            delete pair->second->accounts.at(i);
        }
        //finally free memory allocated to the customer itself
        delete pair->second;
    }
    //empty out the map
    customers.clear();
}

void clear_pledges(RBT tree, map<int, Customer_Node *> customers, vector<RBT_Security_Node*>& removals, bool unpledge)
{
    for (map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        Customer_Node *current = pair->second;
        for(size_t i = 0; i < current->pledged_to_customer.size(); i++)
        {
            if(unpledge)
            {   //it should only add the removal to the unpledged list on the original removal
                current->pledged_to_customer.at(i)->change_status = "Unpledge";
                removals.push_back(current->pledged_to_customer.at(i));
                RBT_Security_Node* copy = tree.RBT_copy_node(current->pledged_to_customer.at(i));
                tree.RBT_add_node(copy);
            }
            else
            {
                RBT_Security_Node* copy = tree.RBT_copy_node(current->pledged_to_customer.at(i));
                tree.RBT_add_node(copy);
            }
        }
        current->pledged_to_customer.clear();
        update_balances(current);
    }
}

void test_overage(map<int, Customer_Node*>& customers,vector<RBT_Security_Node*>& removals, RBT tree)
{
    for (map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        //test if the customer's overage exceeds 50% of the account balance
        //if it is, unpledge the securities from the customer and another attempt
        //will be made in the pledging function
        if(pair->second->over_under > (pair->second->total_balance * .5))
        {
            for(size_t i = 0; i < pair->second->pledged_to_customer.size(); i++)
            {
                RBT_Security_Node* unpledge = pair->second->pledged_to_customer.at(i);
                RBT_Security_Node* copy = tree.RBT_copy_node(unpledge);
                tree.RBT_add_node(copy);
                unpledge->change_status = "Unpledge";
                removals.push_back(unpledge);
            } 
            pair->second->pledged_to_customer.clear();
            update_balances(pair->second);
        }
        
    }
}

void clear_changes(vector<RBT_Security_Node*>& removals, vector<RBT_Security_Node*>& additions)
{
        for(size_t i = 0; i < removals.size(); i++)
        {
            delete removals.at(i);
        } 
        removals.clear();
        for(size_t i = 0; i < additions.size(); i++)
        {
            delete additions.at(i);
        } 
        additions.clear();
}


/*---------------------------------------------- Display and Export Functions --------------------------------------------------*/

void display_customers(map<int, Customer_Node *> customers, bool all_customers)
{

    //print out header line
    cout << endl
         << setw(9) << "Pledge ID"
         << setw(30) << "Customer Name"
         << setw(20) << "Customer Balance"
         << setw(25) << "Net Covered Balance"
         << setw(25) << "Net Covered Balance %"
         << setw(35) << "Over/Under Pledged Status" << endl;

    for (map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        Customer_Node *current = pair->second;
        double percent;
        double percentage_calc = current->over_under / current->total_balance;
        if(percentage_calc == 0){percent =100;}
        else if(current->total_balance == 0){percent = 0;}
        else{percent = percentage_calc * 100;}

        //loop through all customers in the customer map if 'all_customers' set to true, not in any particular order
        if (all_customers)
        {
            cout << fixed << showpoint << setprecision(2)
            << setw(9) << current->pledge_code
            << setw(30) << current->name1
            << setw(20) << current->total_balance
            << setw(25) << current->over_under;
            cout << fixed << setprecision(2) << setw(25) << percent << "%";
            if (current->over_under < 0)
            {
                cout << setw(35) << "Under Pledged" << endl;
            }
            else if (current->total_balance == 0)
            {
                cout << setw(35) << "No Pledges Needed" << endl;
            }
            else if (current->over_under == 0)
            {
                cout << setw(35) << "Precisely Pledged" << endl;
            }
            else
            {
                cout << setw(35) << "Over Pledged" << endl;
            }
        }
        //only print underpledged customers if all customers is set to false
        else if (!all_customers && current->over_under < 0)
        {
            cout << fixed << showpoint << setprecision(2)
                 << setw(9) << current->pledge_code
                 << setw(30) << current->name1
                 << setw(20) << current->total_balance
                 << setw(25) << current->over_under
                 << setw(30) << "Under Pledged" << endl;
        }
    }
}

void display_changes(vector<RBT_Security_Node*> removals, vector<RBT_Security_Node*> additions)
{  
        cout << endl << fixed << showpoint << setprecision(2)
        << setw(20) << "Change Satus"
        << setw(15) << "CUSIP"
        << setw(15) << "Ticket"
        << setw(15) << "Pledge ID"
        << setw(25) << "Pledge Description"
        << setw(15) << "Pledge Amount"
        << setw(15) << "Market Value"
        << setw(25) << "Security Description" << endl;
   
    for(size_t change = 0; change < removals.size(); change++)
    {
        RBT_Security_Node* next = removals.at(change);
        cout << fixed << showpoint << setprecision(2)
        << setw(20) << next->change_status
        << setw(15) << next->cusip
        << setw(15) << next->ticket
        << setw(15) << next->pledge_id
        << setw(25) << next->pledge_description
        << setw(15) << next->pledge_amount
        << setw(15) << next->market_value
        << setw(25) << next->security_description << endl;
    }

    for(size_t change = 0; change < additions.size(); change++)
    {
        RBT_Security_Node* next = additions.at(change);
        cout << fixed << showpoint << setprecision(2)
        << setw(20) << next->change_status
        << setw(15) << next->cusip
        << setw(15) << next->ticket
        << setw(15) << next->pledge_id
        << setw(25) << next->pledge_description
        << setw(15) << next->pledge_amount
        << setw(15) << next->market_value
        << setw(25) << next->security_description << endl;
    }
}

void export_customers(map<int, Customer_Node*>& customers)
{
    ofstream export_file("customer_balances_updated.csv");
    
    export_file << "Pledge ID,Tax ID Number,Name1,Name2,Number of Accounts,Total Securities Pledge Amount,";
    export_file << "Total of Customer Account Balance(s), Net Pledge Amount, Pledge Status" << endl;

    for (map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        string status;
        Customer_Node* next = pair->second;
        if (next->over_under < 0)
        {
            status = "Under Pledged";
        }
        else if (next->over_under == 0)
        {
            status = "Precisely Pledged";
        }
        else
        {
            status = "Over Pledged";
        }

       export_file << fixed << setprecision(2) 
       << next->pledge_code << ","
       << next->tax_ID << ","
       << next->name1 << ","
       << next->name2 << ","
       << next->accounts.size() << ","
       << next->total_pledged << ","
       << next->total_balance << ","
       << next->over_under << ","
       << status << endl;
       
    }
}

void export_changes(vector<RBT_Security_Node*> removals, vector<RBT_Security_Node*> additions)
{
    ofstream export_file("pledge_changes.csv");

    export_file << "Change Status,Portfolio,CUSIP,Ticket,Maturity Date,Pledge ID,Pledge Description,";
    export_file << "Pledge Amount,Par Value,Market Value,Group,Security Description" << endl;
   
    //add removals to the file
    for(size_t change = 0; change < removals.size(); change++)
    {
        RBT_Security_Node* next = removals.at(change);
        export_file << next->change_status << ","
        << next->portfolio << ","
        << next->cusip << ","
        << next->ticket << ","
        << next->maturity << ","
        << next->pledge_id << ","
        << next->pledge_description << ","
        << next->pledge_amount << ","
        << next->par_value << ","
        << next->market_value << ","
        << next->group << ","
        << next->security_description << endl;
    }

    //add a blank line inbetween releases and pledges
    export_file << endl;

    //add additions to the file
    for(size_t change = 0; change < additions.size(); change++)
    {
        RBT_Security_Node* next = additions.at(change);
        export_file << next->change_status << ","
        << next->portfolio << ","
        << next->cusip << ","
        << next->ticket << ","
        << next->maturity << ","
        << next->pledge_id << ","
        << next->pledge_description << ","
        << next->pledge_amount << ","
        << next->par_value << ","
        << next->market_value << ","
        << next->group << ","
        << next->security_description << endl;
    }
}


/*---------------------------------------------------- Utility Functions -------------------------------------------------------*/

void update_balances(Customer_Node *customer)
{
    //update total balance
    customer->total_balance = customer->total_account_balance(customer->accounts);
    //update total securities pledged
    customer->total_pledged = customer->total_securities_pledged(customer->pledged_to_customer);
    //update over/under
    customer->over_under = customer->calculate_under_over(customer->total_pledged, customer->total_balance);
}

int interface_validate()
{   
    do
    {
        // double type allows user to input int as a double value (i.e. 2.0) and still be able to proceed
        double selection;
        cin >> selection;

        string input_to_validate; 

        string extra; 
        //used to catch any extra values inputted by the user still remaining in the stream
        getline(cin, extra);

        // If user enters a character first, regardless of any excess input in the stream, the program will terminate.
        if (cin.fail()) 
        {
            //if cin enters a failed state, exit function and return control back to main where the while condition will then fail
            return -1; 
        }
        else if (extra.length() > 0)
        {
            cout << endl << "You've entered an invalid value. Please enter an integer value between 0 and 10 or \"Q\" to quit: ";
            continue;
        }
        else 
        { 

            input_to_validate = to_string(selection);
            int decimal_location = input_to_validate.find(".");

            string decimal_test = "";
            string truncated_string = "";

            for (int i = decimal_location + 1; i < input_to_validate.length(); i++)
            {
                decimal_test = decimal_test + input_to_validate[i];
            }
            for (int i = 0; i < decimal_location; i++)
            {
                truncated_string = truncated_string + input_to_validate[i];
            }

            //to_string returns only 6 digits post decimal; therefore, all digits should be 0
            if (decimal_test != "000000")
            {
                cout << endl << "You've entered an invalid value. Please enter an integer value between 0 and 10 or \"Q\" to quit: ";
                continue;
            }   

            try
                {
                int converted_int = stoi(truncated_string);
                if (converted_int < 0 || converted_int > 10)
                {
                    cout << endl << "You've entered an invalid value. Please enter an integer value between 0 and 10 or \"Q\" to quit: ";
                    continue;
                }
                return converted_int;
                } 
            catch (...) //handles all types of exceptions - main exception of concern is overflow of data type here
                {
                    cout << endl << "You've entered an invalid value. Please enter an integer value between 0 and 10 or \"Q\" to quit: ";
                    continue;          
                }       
        }
    }
    while (true);  
}

void copy_customer_map(map<int, Customer_Node*>& source_map, map<int, Customer_Node*>& target_map, RBT tree)
{
    clear_customers(target_map);
    for(map<int, Customer_Node *>::iterator pair = source_map.begin(); pair != source_map.end(); pair++)
    {
        //release the memory used by the pointer
        //need to attach the account nodes to the customer copy
        Customer_Node* customer_copy = new Customer_Node;
        for(size_t i = 0; i < pair->second->accounts.size(); i++)
        {
            Account_Node* account = pair->second->accounts.at(i);
            Account_Node* copy = copy_account(account);
            customer_copy->accounts.push_back(copy);
        }

        customer_copy->name1 = pair->second->name1;
        customer_copy->name2 = pair->second->name2;
        customer_copy->pledge_code = pair->second->pledge_code;
        customer_copy->tax_ID = pair->second->tax_ID;
        for(size_t i = 0; i < pair->second->pledged_to_customer.size(); i++)
        {
            RBT_Security_Node* node_to_copy = pair->second->pledged_to_customer.at(i);
            RBT_Security_Node* copy = tree.RBT_copy_node(node_to_copy);
            customer_copy->pledged_to_customer.push_back(copy);
        }
        target_map[pair->first] = customer_copy;
        update_balances(customer_copy);
    }
}

Account_Node* copy_account(Account_Node* original)
{
    Account_Node* copy = new Account_Node;
    copy->account_number = original->account_number;
    copy->account_type = original->account_type;
    copy->class_code_description = original->class_code_description;
    copy->current_balance = original->current_balance;
    copy->interest_rate = original->interest_rate;
    return copy;
}

void open_file(ifstream& file)
{   //if the file name entered cannot be opened, it will continuously loop until it receives a valid name, or user exits
    do 
    {   
        cout << "Enter Source File Name or \"R\" to Return to the Main Menu: ";
        string file_name;
        cin >> file_name;
        if(file_name == "R" || file_name == "r")
        {
            return;
        }
        else
        {
            file.open(file_name, ios::in); 
            if(!file.is_open())
            {
                cout << endl << "The file could not be opened. Ensure the file name is correct and re-enter." << endl << endl;
            }
        }
    
    } while(!file.is_open());
    return;
}

vector<RBT_Security_Node*> copy_removals(vector<RBT_Security_Node*>& removals, RBT tree)
{
    vector<RBT_Security_Node*> removals_copy;
    for(size_t i = 0; i < removals.size(); i++)
    {
        RBT_Security_Node* next = tree.RBT_copy_node(removals.at(i));
        next->change_status = "Unpledge";
        removals_copy.push_back(next);
    }
    return removals_copy;
}

void clear_vector(vector<RBT_Security_Node*>& to_clear)
{
    for(size_t i = 0; i < to_clear.size(); i++)
    {
        delete to_clear.at(i);
    }
    to_clear.clear();
}

void restore_tree(vector<RBT_Security_Node*> to_restore, RBT tree)
{
    for(size_t i = 0; i < to_restore.size(); i++)
    {
        RBT_Security_Node* copy = tree.RBT_copy_node(to_restore.at(i));
        tree.RBT_add_node(copy);
    }
}

void remove_additions(map<int, Customer_Node*>& customers, RBT tree)
{
    for(map<int, Customer_Node *>::iterator pair = customers.begin(); pair != customers.end(); pair++)
    {
        Customer_Node* customer = pair->second;
        for(size_t i = 0; i < customer->pledged_to_customer.size(); i++)
        {
            int ticket = customer->pledged_to_customer.at(i)->ticket;
            double mv = customer->pledged_to_customer.at(i)->market_value;
            RBT_Security_Node* security = tree.find_node(tree.get_root(), ticket, mv);
            if(security != nullptr)
            {
                tree.RBT_remove_node(security);
            }
        }
    }
}















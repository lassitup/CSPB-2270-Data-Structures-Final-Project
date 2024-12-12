#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include "red_black_tree.h"
#include "supporting_func_structs.h"

using namespace std;


int main()
{

    cout << endl << "Welcome to my Security Pledging Program!" << endl;

    //declare variable to hold the tree object
    RBT tree_root;

    //declare new map object to hold customer data and a copy variable to hold original customer data
    map<int, Customer_Node*> customers, customers_copy;

    //using vectors to store changes - all nodes have to be accessed when exporting
    vector<RBT_Security_Node*> pledge_removals, pledge_removals_copy;
    vector<RBT_Security_Node*> pledge_additions;

    
    do
    {
        cout <<  endl << "!!----- Security Pledging Menu -----!! " << endl << endl;

        cout <<
        "1. Import Customer File\n" 
        "2. Import Security File\n"
        "3. Perform Security Updates\n"
        "4. Clear All Securities and Repledge\n"
        "5. View Underpledged Customers\n"
        "6. View All Customers\n"
        "7. Export Pledge Changes and Customer Balances\n"
        "8. Display Changes Made\n\n"
        "------ Utility Functions ------\n\n"
        "9. Print Tree\n"
        "10. Test Tree - Tests Red-Black Tree Invariants\n\n";

        cout << "Please Select an Option from the List Above or 'Q' to Quit: ";

        int selection = interface_validate();
        ifstream customer_file;
        ifstream security_file;

        if(selection == 1)
        {
            cout << endl << "Import Customer File Selected" << endl << endl;
            // If the file is reloaded, clear out the data previously in the map
            if(customers.size() > 0)
            {
                clear_customers(customers);
                clear_customers(customers_copy);
            }
            open_file(customer_file);
            customers = load_customer_data(customer_file);
            cout << endl << "Customer Balances Successfully Loaded!" << endl;
        }
        else if(selection == 2)
        {
            cout << endl << "Import Security File Selected" << endl << endl;
            //if loading a second file to 'overwite' the old one, this deletes
            //the tree already entered. All previous addiitons / removals and 
            //customer pledges are cleared. 
            if(tree_root.get_root() != nullptr || pledge_additions.size() > 0)
            {
                clear_pledges(tree_root, customers, pledge_removals, false);
                clear_changes(pledge_removals, pledge_additions);
                clear_vector(pledge_removals_copy);
                tree_root.RBT_delete_tree(tree_root.get_root());
                tree_root.set_root(nullptr);
            }
            open_file(security_file);
            tree_root = import_and_build_RBT_tree(customers, pledge_removals, security_file);
            cout << endl << "Securities Successfully Loaded!" << endl;
            //make a copy of the original customer map for restoration purposes 
            //customers with net coverage over 50% of the balance has all securities unpledged and placed into the tree
            test_overage(customers, pledge_removals, tree_root);
            copy_customer_map(customers, customers_copy, tree_root);
            pledge_removals_copy = copy_removals(pledge_removals, tree_root);
            //at this point, the customer balances have any securities affilitated with them attached less the securities causing too much excess. 
            //the removal list now consists of the securities added while being inputted and the over excess securities
        }
        else if(selection == 3)
        {   
            cout << endl << "Perform Security Updates Selected" << endl << endl;
            //test if there are already additions added, if so, additions and removals are cleared/reset prior to update
            if(pledge_additions.size() > 0)
            {
                
                restore_tree(pledge_additions, tree_root);
                clear_changes(pledge_removals, pledge_additions);
                pledge_removals = copy_removals(pledge_removals_copy, tree_root);
            }
            //restore customers to its original state
            copy_customer_map(customers_copy, customers, tree_root); 
            //need restore any pledges added to additions in 4 below
            remove_additions(customers, tree_root);
            bool update_status = update_customers(customers, tree_root, pledge_additions);
            if(!update_status)
            {   //if updates failed, it automatically tries to clear all and repledge securities (redistribution of securities)
                cout << "Update Failed - Attempting Clear All Securities and Repledge.." << endl << endl; 
                //replaces the securities within additions back into the tree
                restore_tree(pledge_additions, tree_root);
                clear_changes(pledge_removals, pledge_additions);
                pledge_removals = copy_removals(pledge_removals_copy, tree_root);
                //reload customer balance to 'reset' the securities pledged to each customer - puts it back in it's original state - this should avoid duplicate removals
                copy_customer_map(customers_copy, customers, tree_root); //restore customers to  its original state
                update_status = clear_all_and_repledge(tree_root, customers, pledge_additions, pledge_removals);
            }
            if(!update_status)
            {
                cout <<  "Insufficient Securities Available!" << endl;
            }
            else
            {
                cout << "Pledging Successful!" << endl;
            }
        }
        else if(selection == 4)
        {  //test if there are already additions added, if so, additions and removals are cleared/reset prior to update
            cout << endl << "Clear All Securities and Repledge Selected" << endl << endl;
            if(pledge_additions.size() > 0)
            {
                restore_tree(pledge_additions, tree_root);
                clear_changes(pledge_removals, pledge_additions);
                pledge_removals = copy_removals(pledge_removals_copy, tree_root);
            }
            copy_customer_map(customers_copy, customers, tree_root); //restore customers to  its original state
            bool repledge_status = clear_all_and_repledge(tree_root, customers, pledge_additions, pledge_removals);
            if(!repledge_status)
            {
                cout << "Insufficient Securities Available!" << endl;
            }
            else
            {
                cout << "Pledging Successful!" << endl;
            }
        }
        else if(selection == 5)
        {
            cout << endl << "View Underpledged Customers Selected" << endl << endl;
            display_customers(customers, false);
            cout << endl;
        }
        else if(selection == 6)
        {
            cout << endl << "View All Customers Selected" << endl << endl;
            display_customers(customers);
            cout << endl;
        }
        else if(selection == 7)
        {
            cout << endl << "Export Pledge Changes and Customer Balances Selected" << endl << endl;
            export_changes(pledge_removals, pledge_additions);
            export_customers(customers);
            cout << "Changes and Customer Data Successfully Exported!" << endl;
        }
        else if(selection == 8)
        {
            cout << endl << "Display Changes Made Selected" << endl << endl;
            display_changes(pledge_removals, pledge_additions);
            cout << endl;
        }
        else if(selection == 9)
        {
            cout << endl << "Print Tree Selected" << endl << endl;
            cout << endl;
            tree_root.print_RBT_tree(tree_root.get_root());
            cout << endl;
        }
        else if(selection == 10)
        {
            cout << endl << "Test Tree Selected" << endl << endl;
            tree_root.run_RBT_tests(tree_root.get_root());
            cout << "Securities in Tree: " << tree_root.count_nodes(tree_root.get_root()) << endl << endl;
            cout << "Tree Market Value Sum:  " << fixed << setprecision(2) << tree_root.sum_nodes(tree_root.get_root()) << endl;
        }
    } while(!cin.fail());

    cout << endl << endl << "Goodbye!" << endl << endl;
    return 0;
}
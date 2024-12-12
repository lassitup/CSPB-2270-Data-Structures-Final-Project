#ifndef RBT_H__
#define RBT_H__

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cmath>


using namespace std;
#define TOTAL_SPACES 4 //used within the print_RBT_tree function


/*--------------------------------------Account and Customer Node Structures ---------------------------------------------------*/

/*
    This structure holds the contents of a security as wells as the components needed to construct the red-black tree
*/
struct RBT_Security_Node
{
    string portfolio;
    string cusip;
    int ticket;
    string maturity;
    int pledge_id; //if there is no pledge id, it will be set to 0;
    string pledge_description;
    double pledge_amount;
    double par_value;
    double market_value;
    string group;
    string security_description;

    string change_status;

    string node_color; // black or red
    RBT_Security_Node* parent = nullptr;
    RBT_Security_Node* left_child = nullptr;
    RBT_Security_Node* right_child = nullptr;
};


/* -------------------------------------------------Red-Black Tree Class--------------------------------------------------------*/


class RBT 
{
public:

    //Tree Constructor
    RBT();  

    //Tree Deconstructor
    ~RBT(); 


    /*--------------------------------- Red Black Tree Public Insert and Remove Functions --------------------------------------*/

    /*
        Function is called to initiate the addition of a security node into the red-black tree
    */
    void RBT_add_node(RBT_Security_Node* node);

    /*
        Function is called to initiate the removal of a security node from the red-black tree
    */
    void RBT_remove_node(RBT_Security_Node* node); 

 

    /*------------------------------------- Red Black Tree Public Utility Functions --------------------------------------------*/

    /*
        Function is called to consruct a security node. The vector parameter 
        consists of data from each security line within the loaded csv file. 
        Data conversions are performed here
    */
    RBT_Security_Node* build_security_node(const vector<string>& security_data);

    /*
        Function is called to return the root node of the tree.
        Returns nullptr if root is empty
    */
    RBT_Security_Node* get_root();

    /*
        Function is called to set the root of the tree as the security
        node passed in
    */
    void set_root(RBT_Security_Node* node);

    /*
        Function is called to make a copy of the node passed in.
        All details of the security are copied, with the exception
        of red-black tree data (parent, children, node color and change 
        status are not copied)
    */
    RBT_Security_Node* RBT_copy_node(RBT_Security_Node* node);

    /*
        Function is called in order to remove a pre-established red-black tree
        Beginning at the passed in node of the tree, the function performs 
        a post-order traversal to free up the memory of each security by 
        delete each node in the tree.
    */
    void RBT_delete_tree(RBT_Security_Node* root);

    /*
        Function returns the security node in the red-black tree
        with the smallest market value (the 'leftmost' node  of tree)
    */
    RBT_Security_Node* find_minimum(RBT_Security_Node* root);

    /*
        Function returns the security node in the red-black tree
        with the largest market value (the 'rightmost' node  of tree)
    */
    RBT_Security_Node* find_maximum(RBT_Security_Node* root);

    /*---------------------------------------- Red Black Tree Public Test Functions --------------------------------------------*/

   /*
        Function runs a series of checks to test if the red black tree
        invariants are appropriate. These tests include checking the 
        numeric ordering of the tree based on market value, the relationship 
        between red and black nodes as well testing the height of the tree.
    */
    void run_RBT_tests(RBT_Security_Node* root);

    /*
        Function prints a horizontal representation of the red black tree
    */
    void print_RBT_tree(RBT_Security_Node* node, int empty_space = 0);

    RBT_Security_Node* find_node(RBT_Security_Node* root, int ticket, double MV);

    double sum_nodes(RBT_Security_Node* root);

    int count_nodes(RBT_Security_Node* root);

private:

    //holds the root of the RBT object
    RBT_Security_Node** root;

    int security_node_count = 0;

    /*------------------------------------ Red Black Tree Private Insert and Remove Functions ----------------------------------*/

    void RBT_BST_Remove(RBT_Security_Node* node);

    void RBT_insert(RBT_Security_Node* new_node);

    /*------------------------------ Red Black Tree Rebalancing Private Helper Functions ---------------------------------------*/

    void RBT_Balance(RBT_Security_Node* node);
    
    RBT_Security_Node* get_grandparent_node(RBT_Security_Node* node);

    RBT_Security_Node* get_uncle_node(RBT_Security_Node* node);

    void RBT_rotate_right(RBT_Security_Node* node);

    void RBT_rotate_left(RBT_Security_Node* node);

    void RBT_set_child(RBT_Security_Node* parent, string which_child, RBT_Security_Node* child);

    void RBT_replace_child(RBT_Security_Node* parent, RBT_Security_Node* current_child, RBT_Security_Node* new_child);
    
    RBT_Security_Node* RBT_get_predecessor(RBT_Security_Node* node);

    RBT_Security_Node* RBT_get_sibling(RBT_Security_Node* node);

    bool RBT_is_nonNull_and_red(RBT_Security_Node* node);

    bool RBT_is_Null_or_black(RBT_Security_Node* node);

    bool RBT_both_children_black(RBT_Security_Node* node);

    void RBT_prepare_for_removal(RBT_Security_Node* node);

    bool RBT_try_case1(RBT_Security_Node* node);

    bool RBT_try_case2(RBT_Security_Node* node, RBT_Security_Node* sibling);

    bool RBT_try_case3(RBT_Security_Node* node, RBT_Security_Node* sibling);

    bool RBT_try_case4(RBT_Security_Node* node, RBT_Security_Node* sibling);

    bool RBT_try_case5(RBT_Security_Node* node, RBT_Security_Node* sibling);

    bool RBT_try_case6(RBT_Security_Node* node, RBT_Security_Node* sibling);


    /*--------------------------------------  Red Black Tree Private Utility Functions -----------------------------------------*/

    void extract_node_data(RBT_Security_Node* target, RBT_Security_Node* source, bool copy_color = false);


    /*--------------------------------------- Red Black Tree Private Test Functions --------------------------------------------*/

    bool test_invariants(RBT_Security_Node* root);

    int get_tree_height(RBT_Security_Node* root);

  

};



#endif
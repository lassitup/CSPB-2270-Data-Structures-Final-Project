#include "red_black_tree.h"




RBT::RBT()
{
    root = new RBT_Security_Node*;
    *root = nullptr;
}

RBT::~RBT(){}

/*------------------------------------ Red Black Tree Public Insert and Remove Functions ---------------------------------------*/

void RBT::RBT_add_node(RBT_Security_Node* node)
{
    RBT_insert(node);
    node->node_color = "red";
    RBT_Balance(node);
}

void RBT::RBT_remove_node(RBT_Security_Node* node)
{
    if(node->left_child != nullptr && node->right_child != nullptr)
    {
        RBT_Security_Node* predecessor_node = RBT_get_predecessor(node);
        RBT_Security_Node* temp_node = RBT_copy_node(predecessor_node);
        //need to create a new / temporary node to hold the the data 
        //copy node here - maybe create helper function for this
        //bst removal algorithm in book uses key to find node to remove, could just use the node itself since we may have duplicates?\
        // first one it finds would be the same one it found in the original search so shouldn't matter
        RBT_remove_node(predecessor_node);
        extract_node_data(node, temp_node);  // copy node data back
        delete temp_node; //free up temp node memory once all data extracted
        return;
    }
    if(node->node_color == "black")
    {
        RBT_prepare_for_removal(node);
    }
    //add to customer node here call copy node and store the new node in the customer this way we only need to code once
    //this is the node we want to copy over
    RBT_BST_Remove(node);
}


/*------------------------------------------ Red Black Tree Public Utility Functions -------------------------------------------*/

RBT_Security_Node* RBT::build_security_node(const vector<string>& security_data)
{
    //Security data per daily customer balance report
    RBT_Security_Node* next_security = new RBT_Security_Node;
    next_security->portfolio = security_data.at(0);
    next_security->cusip = security_data.at(1);
    next_security->ticket = stoi(security_data.at(2));
    next_security->maturity = security_data.at(3);
    if(security_data.at(4) != "")
    {
        next_security->pledge_id = stoi(security_data.at(4));
    }
    else
    {
        next_security->pledge_id = 0;
    }
    next_security->pledge_description = security_data.at(5);
    next_security->pledge_amount = stod(security_data.at(6));
    next_security->par_value = stod(security_data.at(7));
    next_security->market_value = stod(security_data.at(8));
    next_security->group = security_data.at(9);
    next_security->security_description = security_data.at(10);

    return next_security;
}


RBT_Security_Node* RBT::get_root()
{
    return *root;
}

void RBT::set_root(RBT_Security_Node* node)
{
    *root = node;
}

RBT_Security_Node* RBT::RBT_copy_node(RBT_Security_Node* node)
{
    RBT_Security_Node* temp_node = new RBT_Security_Node;
    temp_node->portfolio = node->portfolio;
    temp_node->cusip = node->cusip;
    temp_node->ticket = node->ticket;
    temp_node->maturity = node->maturity;
    temp_node->pledge_id = node->pledge_id;
    temp_node->pledge_description = node->pledge_description;
    temp_node->pledge_amount = node->pledge_amount;
    temp_node->par_value = node->par_value;
    temp_node->market_value = node->market_value;
    temp_node->group = node->group;
    temp_node->security_description = node->security_description;
    return temp_node;
}

void RBT::RBT_delete_tree(RBT_Security_Node* root)
{
    //post order traversal
    if(root == nullptr)
    {
        return;
    }
    RBT_delete_tree(root->left_child);
    RBT_delete_tree(root->right_child);
    delete root;
}

RBT_Security_Node* RBT::find_minimum(RBT_Security_Node* root)
{
    if(root == nullptr)
    {
        return nullptr;
    }
    while(root->left_child != nullptr)
    {
        root = root->left_child;
    }
    return root;
}

RBT_Security_Node* RBT::find_maximum(RBT_Security_Node* root)
{
    if(root == nullptr)
    {
        return nullptr;
    }
    while(root->right_child != nullptr)
    {
        root = root->right_child;
    }
    return root;
}


/*------------------------------------------ Red Black Tree Public Test Functions ----------------------------------------------*/

void RBT::run_RBT_tests(RBT_Security_Node* root)
{
    if(test_invariants(root))
    {
        cout << endl << "Red-Black Tree Invariants Are Correct" << endl;
    }
    int node_count = count_nodes(root);
    //compare the actual height of the tree to what it should be 
    //log based 2 of number of nodes for mininimum height
    //2 * log based 2 of number of nodes for maximum height
    int log_height_min = floor(log2(node_count + 1));
    int log_height_max = log_height_min * 2;
    int calc_height = get_tree_height(root);
    if(calc_height >= log_height_min && calc_height <= log_height_max)
    {
        cout << endl << "Red-Black Tree Height is within the Acceptable Range" << endl << endl;
    }
    else
    {
        cout << endl << "Red-Black Tree Height is Incorrect" << endl << endl;
    }
}

void RBT::print_RBT_tree(RBT_Security_Node* node, int empty_space)
{

    if(node == nullptr)
    {
        return;
    }

    empty_space += TOTAL_SPACES;

    //empty space variable increased each call to represent a tree in the terminal
    print_RBT_tree(node->right_child, empty_space);
    
    string space = "";

    for(int i = 0; i < empty_space; i++)
    {
        space += "    ";
    }

    cout << fixed << setprecision(2) <<  space << node->market_value << " " << node->node_color << endl;

    print_RBT_tree(node->left_child, empty_space);
}


/*------------------------------------- Red Black Tree Private Insert and Remove Functions -------------------------------------*/

void RBT::RBT_BST_Remove(RBT_Security_Node* node)
{

    if (*root == nullptr)
    {
        return;
    }

    //no child case
    if (node->left_child == nullptr && node->right_child == nullptr)
    {
        if (node->parent != nullptr && node->parent->left_child == node)
        {
            node->parent->left_child = nullptr;
            delete node;

        }
        else if (node->parent != nullptr && node->parent->right_child == node)
        {
            node->parent->right_child = nullptr;
            delete node; 
        }
        else
        {
            *root = nullptr;
            delete node; // simply deletes node if parent is empty / cursor is root
        }
    }
    // two children case
    else if (node->left_child != nullptr && node->right_child != nullptr)
    {
        RBT_Security_Node* successorNode = node->right_child;

        while (successorNode->left_child != nullptr)
        {
            successorNode = successorNode->left_child;
        }
        
        RBT_Security_Node* replacement = RBT_copy_node(successorNode); //temporarily store the replacement values until recursive function returns

        RBT_BST_Remove(successorNode);

        extract_node_data(node, replacement);

        delete replacement;


    }
    else if (node->right_child != nullptr)
    {
        bool color_swap = true;
        if(node->parent == nullptr)
        {
            color_swap = false;
        }
        extract_node_data(node, node->right_child, color_swap); 
        delete node->right_child; 

        node->right_child = nullptr;
    }
    else
    {
        bool color_swap = true;
        if(node->parent == nullptr)
        {
            color_swap = false;
        }
        extract_node_data(node, node->left_child, color_swap);
        delete node->left_child; 
        node->left_child = nullptr;
    }
    return;
}

void RBT::RBT_insert(RBT_Security_Node* new_node)
{
    if(*root == nullptr)
    {
        *root = new_node;
    }
    else
    {
        RBT_Security_Node* cursor = *root; //start at the root each time
        while(cursor != nullptr)
        {
            if(new_node->market_value < cursor->market_value)
            {
                if(cursor->left_child == nullptr)
                {
                    cursor->left_child = new_node;
                    new_node->parent = cursor;
                    return;
                }
                else
                {
                    cursor = cursor->left_child;        
                }
            }
            // securities of the same value go to the right
            else
            {
                if(cursor->right_child == nullptr)
                {
                    cursor->right_child = new_node;
                    new_node->parent = cursor;
                    return;
                }
                else
                {
                    cursor = cursor->right_child;
                }
            }
        }
    }
}


/*--------------------------------- Red Black Tree Rebalancing Private Helper Functions ----------------------------------------*/

void RBT::RBT_Balance(RBT_Security_Node* node)
{
    if(node->parent == nullptr)
    {
        node->node_color = "black";
        return;
    }
    if(node->parent->node_color == "black")
    {
        return;
    }


    RBT_Security_Node* parent = node->parent;
    RBT_Security_Node* grandparent = get_grandparent_node(node);
    RBT_Security_Node* uncle = get_uncle_node(node);
    if(uncle != nullptr && uncle->node_color == "red")
    {
        parent->node_color = "black";
        uncle->node_color = "black";
        grandparent->node_color = "red";
        RBT_Balance(grandparent);
        return;
    }
    if(node == parent->right_child && parent == grandparent->left_child)
    {
        RBT_rotate_left(parent);
        node = parent;
        parent = node->parent;
    }
    else if(node == parent->left_child && parent == grandparent->right_child)
    {
        RBT_rotate_right(parent);
        node = parent;
        parent = node->parent;
    }
    parent->node_color = "black";
    grandparent->node_color = "red";
    if(node == parent->left_child)
    {
        RBT_rotate_right(grandparent);
    }
    else
    {
        RBT_rotate_left(grandparent);
    }
}

RBT_Security_Node* RBT::get_grandparent_node(RBT_Security_Node* node)
{
    if(node->parent == nullptr)
    {
        return nullptr;
    }
    return node->parent->parent; 
}

RBT_Security_Node* RBT::get_uncle_node(RBT_Security_Node* node)
{
    RBT_Security_Node* grandparent = nullptr;
    if(node->parent != nullptr)
    {
        grandparent = node->parent->parent;
    }
    if(grandparent == nullptr)
    {
        return nullptr; //if there is no grandparent, there can't be an uncle
    }
    if(grandparent->left_child == node->parent)
    {
        return grandparent->right_child;
    }
    else
    {
        return grandparent->left_child;
    }
}

void RBT::RBT_rotate_right(RBT_Security_Node* node)
{
    RBT_Security_Node* left_right_child = node->left_child->right_child;
    if(node->parent != nullptr)
    {
        RBT_replace_child(node->parent, node, node->left_child);
    }
    else
    {
        //order of assigning these two shouldn't matter
        node->left_child->parent = nullptr;
        *root = node->left_child;
    }
    RBT_set_child(node->left_child, "right", node);
    RBT_set_child(node, "left", left_right_child);
}

void RBT::RBT_rotate_left(RBT_Security_Node* node)
{
    RBT_Security_Node* right_left_child = node->right_child->left_child;
    if(node->parent != nullptr)
    {
        RBT_replace_child(node->parent, node, node->right_child);
    }
    else
    {
        //order of assigning these two shouldn't matter
        node->right_child->parent = nullptr;
        *root = node->right_child;
    }
    RBT_set_child(node->right_child, "left", node);
    RBT_set_child(node, "right", right_left_child);
}

void RBT::RBT_set_child(RBT_Security_Node* parent, string which_child, RBT_Security_Node* child)
{
    if(which_child == "left")
    {
        parent->left_child = child;
    }
    else
    {
        parent->right_child = child;
    }
    if (child != nullptr)
    {
        child->parent = parent;
    }
}

void RBT::RBT_replace_child(RBT_Security_Node* parent, RBT_Security_Node* current_child, RBT_Security_Node* new_child)
{
    if(parent->left_child == current_child)
    {
        RBT_set_child(parent, "left", new_child);
    }
    else if(parent->right_child == current_child)
    {
        RBT_set_child(parent, "right", new_child);
    }
}

RBT_Security_Node* RBT::RBT_get_predecessor(RBT_Security_Node* node)
{
    node = node->left_child;
    while (node->right_child != nullptr)
    {
        node = node->right_child;
    }
    return node;
}

RBT_Security_Node* RBT::RBT_get_sibling(RBT_Security_Node* node)
{
    if(node->parent != nullptr)
    {
        if(node->parent->left_child == node)
        {
            return node->parent->right_child;
        }
        return node->parent->left_child;
    }
    return nullptr;
}

bool RBT::RBT_is_nonNull_and_red(RBT_Security_Node* node)
{
    if(node == nullptr)
    {
        return false;
    }
    return node->node_color == "red";
}

bool RBT::RBT_is_Null_or_black(RBT_Security_Node* node)
{
    if(node == nullptr)
    {
        return true;
    }
    return (node->node_color == "black");
}

bool RBT::RBT_both_children_black(RBT_Security_Node* node)
{
    if(node->left_child != nullptr && node->left_child->node_color == "red")
    {
        return false;
    }
    if(node->right_child != nullptr && node->right_child->node_color == "red")
    {
        return false;
    }
    return true;
}

void RBT::RBT_prepare_for_removal(RBT_Security_Node* node)
{
    if(RBT_try_case1(node))
    {
        return;
    }

    RBT_Security_Node* sibling = RBT_get_sibling(node);
    if(RBT_try_case2(node, sibling))
    {
        sibling = RBT_get_sibling(node);
    }
    if(RBT_try_case3(node, sibling))
    {
        return;
    }
    if(RBT_try_case4(node, sibling))
    {
        return;
    }
    if(RBT_try_case5(node, sibling))
    {
        sibling = RBT_get_sibling(node);
    }
    if(RBT_try_case6(node, sibling))
    {
        sibling = RBT_get_sibling(node);
    }
    sibling->node_color = node->parent->node_color;
    node->parent->node_color = "black";
    if(node == node->parent->left_child)
    {
        sibling->right_child->node_color = "black";
        RBT_rotate_left(node->parent);
    }
    else
    {
        sibling->left_child->node_color = "black";
        RBT_rotate_right(node->parent);
    }
}

bool RBT::RBT_try_case1(RBT_Security_Node* node)
{
    if(node->node_color == "red" || node->parent == nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool RBT::RBT_try_case2(RBT_Security_Node* node, RBT_Security_Node* sibling)
{
    if(sibling->node_color == "red")
    {
        node->parent->node_color = "red";
        sibling->node_color = "black";
        if (node == node->parent->left_child)
        {
            RBT_rotate_left(node->parent);
        }
        else
        {
            RBT_rotate_right(node->parent);
        }
        return true;
    }
    return false;
}

bool RBT::RBT_try_case3(RBT_Security_Node* node, RBT_Security_Node* sibling)
{
    if(node->parent->node_color == "black" && RBT_both_children_black(sibling))
    {
        sibling->node_color = "red";
        RBT_prepare_for_removal(node->parent);
        return true;
    }
    return false;
}

bool RBT::RBT_try_case4(RBT_Security_Node* node, RBT_Security_Node* sibling)
{
    if(node->parent->node_color == "red" && RBT_both_children_black(sibling))
    {
        node->parent->node_color = "black";
        sibling->node_color = "red";
        return true;
    }
    return false;
}

bool RBT::RBT_try_case5(RBT_Security_Node* node, RBT_Security_Node* sibling)
{
    if(RBT_is_nonNull_and_red(sibling->left_child) && RBT_is_Null_or_black(sibling->right_child) && node == node->parent->left_child)
    {
        sibling->node_color = "red";
        sibling->left_child->node_color = "black";
        RBT_rotate_right(sibling);
        return true;
    }
    return false;

}

bool RBT::RBT_try_case6(RBT_Security_Node* node, RBT_Security_Node* sibling)
{
    if(RBT_is_Null_or_black(sibling->left_child) && RBT_is_nonNull_and_red(sibling->right_child) && node == node->parent->right_child)
    {
        sibling->node_color = "red";
        sibling->right_child->node_color = "black";
        RBT_rotate_left(sibling);
        return true;
    }
    return false;
}



/*---------------------------------------  Red Black Tree Private Utility Functions --------------------------------------------*/

void RBT::extract_node_data(RBT_Security_Node* target, RBT_Security_Node* source, bool copy_color)
{   //maybe need an optional parameter to copy over color for remove function
    //function is called in multiple places, one call doesn't need the color to be carried over
    target->portfolio = source->portfolio;
    target->cusip = source->cusip;
    target->ticket = source->ticket;
    target->maturity = source->maturity;
    target->pledge_id = source->pledge_id;
    target->pledge_description = source->pledge_description;
    target->pledge_amount = source->pledge_amount;
    target->par_value = source->par_value;
    target->market_value = source->market_value;
    target->group = source->group;
    target->security_description = source->security_description;
    if(copy_color)
    {
        target->node_color = source->node_color;
    }
}

int RBT::count_nodes(RBT_Security_Node* root)
{
    int counter = 0;
    if(root == nullptr)
    {
        return 0;
    }
    counter += count_nodes(root->left_child);
    counter += count_nodes(root->right_child);
    return counter + 1;
}

/*--------------------------------------- Red Black Tree Private Test Functions ------------------------------------------------*/

bool RBT::test_invariants(RBT_Security_Node* root)
{   
    bool status = true;
    if (root == nullptr)
    {
        return true;
    }
    status = test_invariants(root->left_child);
    //test numeric ordering
    if (root->left_child!= nullptr && root->left_child->market_value >= root->market_value)
    {
        return false;
    }
    if(root->right_child != nullptr && root->right_child->market_value < root->market_value) 
    {
        return false;
    }

    //test red/black relation by testing that no red node has red children
    if (root->node_color == "red" && root->left_child != nullptr && root->left_child->node_color == "red")
    {
        return false;
    }
    if (root->node_color == "red" && root->right_child != nullptr && root->right_child->node_color == "red")
    {
        return false;
    }
    //test root is black
    if (root->parent == nullptr && root->node_color == "red")
    {
        return false;
    }
    status = test_invariants(root->right_child);

    return status;     
}

int RBT::get_tree_height(RBT_Security_Node* root)
{
    if (root == nullptr)
    {
        return -1;
    }
    int left_height = get_tree_height(root->left_child);
    int right_height = get_tree_height(root->right_child);
    return 1 + max(left_height, right_height);
}

RBT_Security_Node* RBT::find_node(RBT_Security_Node* root, int ticket, double mv)
{
    if (root == nullptr)
    {
        return nullptr;
    }
    else if (root->ticket == ticket && root->market_value == mv)
    {
        return root;
    }
    else
    {      
        if(mv < root->market_value)
        {
            return find_node(root->left_child, ticket, mv);
        }
        else
        {
            return find_node(root->right_child, ticket, mv);
        }
    }
}

double RBT::sum_nodes(RBT_Security_Node* root)
{
    {
        if (root == nullptr)
            return 0;

        return (root->market_value + sum_nodes(root->left_child) + sum_nodes(root->right_child));
    }
}
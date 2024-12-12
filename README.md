Presented by Justin Lassiter

Project: Security Pledging Application

Purpose

This repository contains my final project for CSPB 2270 Data Structure at the University of Colorado Boulder. 

Data Structure Used: 

Red-Black Tree – A self-balancing binary search tree used for the organization, storage and retrieval/search of financial security data. The securities are organized within the tree based on its market value. This allows for quick search, O(logn) time complexity, of a security that satisfies the capital amount required to cover a 
customer’s aggregate deposit balance.

Project Inspiration

With this project, my aim was to solve an issue I encounter daily in my job as a financial analyst at a regional bank. One of my daily duties is ensuring that certain customers have their account balances collateralized with financial securities owned by the bank. Currently, this process is very manual in nature. I must pull in customer balances and security values, perform comparisons between the customer’s current aggregate balances (customers can have many accounts), determine which customers need additional collateral, as well as customer’s that have a large excess of securities pledged to it. Regulators require that we do not have too much excess asset value assigned to a customer, if possible. As customer balances and security values change daily, it is very unpredictable and there are days where only 1 change is needed and days where 100+ changes are needed. As one could imagine, this is very tedious, time consuming process and is also prone to human error. 

Project Overview

With this project, I have set out to automate this process completely. The program I’ve written requires that the user load the same customer and security files that I receive daily into the program where nodes are created for each element. The customer balances are added to a map data structure and the securities are added to a red-black tree. I’ve chosen to use a red-black tree to organize the securities. By building the red-black tree based on the security market value, the self-balancing always guarantees that the search time complexity is O(logN) after insertions and removals. This allows for very quick look up of an appropriate valued security within the tree as many iterations of search are performed in the pledging update algorithms. Ultimately, the program will ensure that all customer balances are covered, if possible, and will be able to export the pledging changes needed to a csv file, which can then be provided to the safekeeping of our securities.

High level Overview of Program steps/features:

1.	Import Customer Balances 
    a.	A node is created for each customer with all relevant customer information
  	
     		 i.	Each account of the customer is established as its own node is stored within a vector in the customer node.
  	
    b.	Customer nodes are stored in a map structure for quick lookup O(1) time complexity
2.	Import Securities
    
    a.	A node is created for each security with all relevant security information
  	
    b.	Security are entered into the red-black tree structure.
  	
     		 i.	Security nodes that already pledged to customers will not be entered into the tree, but will be added to the security vector within the customer node.
  	
      		ii.	If the security is already pledged, but the customer is no longer in the map, the security gets added to the ‘pledge_removals’ vector and then get’s added to the red-black tree so that it can be made available to other customers.
  	
3.	An initial test is performed to determine if there are customers with pledged amounts causing an overage (securities pledge less account balances) more than 50% of the aggregate account balances. This is the threshold we aim to hit, if possible. For each of these customers, all securities are unpledged and added back to the tree and added to the ‘pledge_removals’ vector. By removing these securities from the customer, an opportunity is available to try to repledge securities to the customer at a smaller threshold resulting from other securities made available from other security releases or new securities purchased.

4.	There are two pledging update algorithms implemented – the first algorithm ‘update_customers’, looks only at customers that have over_under pledge excess with a negative balance (the aggregate account balance is not adequately covered) – these need additional securities pledged. Within this function and its helper function (increase_decrease_search), two search tests are performed: 
	a. 1. Search for the needed balance with a 50% threshold added to it representing a min and max value – the small method.

      i.	If a security is found in this range, the security is removed from the red-black tree and the security is added to a temporary vector holding all additions made using this first method
  	
      ii.	If a security in this range cannot be found within the red-black tree, it will then search smaller securities in the tree, update the over_under balance and keep iterating based on the new amount needed.
  	
      iii.	If the balance is covered after this process, true is returned
  	
    b.	2. The second search method (large method) looks at the balance needed (over_under), which is set to the min value and max is set to the max value of a double. This will allow the algorithm to find the first security in the red-black tree that will cover the needed balance.
  	
      i.	If a security is found, true will be return, false otherwise
  	
    c.	After these two methods return, if both result in true, indicating that it found securities to cover the balance, the aggregate market value total of the securities identified is compared and the method that returned in less security value is ultimately used to pledge to the customer and the others are restored to the red-black-tree.
  	
    d.	If, at any point in the update_pledges algorithm, the small and large methods both return false – this indicates that the update process has failed and there are not enough securities in the tree to cover the balances.
  	
      i.	If this is the case, the program automatically moves on to the second of the pledging update algorithms explained below.
  	
5.	The second of the pledge algorithms ‘clear_all_and_repledge’ will attempt a redistribution of all securities. First, all customer’s have their securities unpledged, added to ‘pledge_removals’  and they are added back to the red-black tree. At this point, the first algorithm is use then used to try to sufficiently pledge each customer.   Within ‘clear_all_and_repledge’, an initial threshold of 50% is used, but each time ‘update_pledges’ fails, it will decrement the threshold by 1% and recall the ‘update_pledges’ function. It will keep iterating until the threshold has been reduced down to 0%, indicating it’s searching for values at exactly the needed over_under value. It is possible that a customer balance grabbed a security value that was over in excess, where it could have satisfied a different customer’s balance.
    
6.	If both of these pledge algorithms fail, it prints out that there are insufficient securities available for pledging.
    
7.	Once customer balances are covered, the user can export the pledge changes to a csv file formatted for uploaded to the safekeeper of our securities. Additionally, the user can export to csv the status of customer balances for review.
    
8.	Additional elements added to the program allows the user to review the status of customers and pledge changes within the program. I’ve also implanted test functions to ensure the appropriateness of the red-black tree. These tests check the invariants of the red-black tree.
    
    a.	Root cannot be a red node.
   	
    b.	No red node can have a red child.
   	
    c.	Numeric ordering is correct, smaller market values to the left, larger and equal to the right.
   	
    d.	The height of the tree is within the acceptable range (min log¬2N max 2log2N), where N is the number of nodes in the tree.


Steps to Running the Program
	
1.	Ensure the terminal is opened to the correct folder holding the program
2.	To compile, in the terminal type:   g++ *.cpp -o main
3.	Run the program, type:  ./main
4.	Select 1 at the menu to import the customer file
i.	Type in the name of the customer balance file to be used
1.	Can use “customer_balances_demo.csv”
5.	Select 2 at the menu to import the security file
i.	Type in the name of the customer balance file to be used
1.	Can use “securities_demo.csv”
6.	Select 6 at the menu – this allows you to see that customer’s are currently under pledged and need additional collateral added.
7.	Select 10 at the menu – this runs test to show that the red-black tree invariants are appropriate.
8.	Select 3 at the menu to perform security updates.
9.	Select 6 at the menu – you can now see that the customers are now sufficiently pledged.
10.	Select 8 to see the pledges made.
11.	Select 10 at the menu – this runs test to show that the red-black tree invariants are still appropriate.
12.	Select 4 at the menu to perform clear all and repledge.
13.	Select 6 at the menu – you can now see that the customers are still now sufficiently pledged – there could potentially be changes in the % covered now.
14.	Select 8 to see the pledges made – there should be considerably more changes than in step 10 since we cleared all securities and had to pledge to all customers.
15.	Select 7 to export customer balances and changes to csv files. These files should be generated in the same folder as where the main program is located.

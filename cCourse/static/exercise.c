#include <stdio.h>

/*
 a static LOCAL variable its initialized only once and retained the value between function calls

 this means that if i want to change the variable called by a function , i have to use pointers...

*/
int sum(int num)
{
    /**
     * find sum to n numbers
     */
    static int total = 0; // persists across calls 
    total += num;         // add current number to running total (THIS IS WHERE STATIC VARIABLES comes to be USEFULL)
    return total;
}

int main()
{
    printf("%d ", sum(55)); // total 55
    printf("%d ", sum(45)); // total 100
    printf("%d ", sum(50)); // total 150
    return 0;
}
#include <stdio.h>
// for loops supply the ability to create a loop (code block that runs multiple times)

// for loops mainly require an iterator variable.
int main()
{
    // proceed with a simple structure, intialize iterator using a initial value, check if the iterarotr has reached its final value
    // and then increases the iterator if its the case

    for (int i = 0; i < 10; i++)
    {
        printf("%d\n", i);
    }
    // this block will print the numbers 0 trough 9 (10 numbers in total )
    // also can iterate arrays, like this:
    // it is important to check what initialize before/outside my code block
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int sum = 0; // HAVE TO INTIALIZE, or C initialize my varible with a random value
    for (int i = 0; i < 10; i++)
    {
        sum += array[i];
    }

    // then only print the sum of the array

    printf("%i is the sum of the last array", sum);

    return 0;
}
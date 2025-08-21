// arrays definition
#include <stdio.h>

int main()
{

    int numbersArray[5]; // declare an array of integers with 5 elements
    // Note: The array is not initialized, so it contains garbage values.

    printf("The first element of the array is: %d\n", numbersArray[0]); // prints -1460367840, because the array is not initialized and takes the min and max
    printf("The last element of the array is: %d\n", numbersArray[4]);  // prints 517994816 because the array is not initialized

    return 0;
}
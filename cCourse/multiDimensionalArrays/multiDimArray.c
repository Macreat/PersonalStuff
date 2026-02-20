#include <stdio.h>
// 2D arrays and row-major order

int main()
{
    // initialize a list of two-dimensional array.
    int matrix[2][3] = {           // save multidimensional arrays 2NX3C where 2 is ROW size 1 and 3 is COLUMNS size 2
                        {1, 2, 3}, // initializers for row indexed by 0 , then 1 ... also, i can ommit and intializes within the braces
                        {4, 5, 6}};

    int twoD[2][3] = {1, 2, 3, 4, 5, 6};
    // the same initalize wh explicit row size

    int twoDA[][3] = {           // save multidimensional arrays 2NX3C where 2 is ROW size 1 and 3 is COLUMNS size 2
                      {1, 2, 3}, // initializers for row indexed by 0 , then 1 ... also, i can ommit and intializes within the braces
                      {4, 5, 6}};

    for (int i = 0; i < 2; i++)
    { // we check every index of the array and print it all
        for (int j = 0; j < 3; j++)
            printf("%d ", matrix[i][j]);
        printf("\n");
    }
    for (int i = 0; i < 2; i++)
    { // we check every index of the array and print it all
        for (int j = 0; j < 3; j++)
            printf("%d ", twoD[i][j]);
        printf("\n");
    }

    // also, we can access to the two-d array elements, using subscripts, ie, row index and column index of the array.e.g :

    int val = twoD[0][2]; // the above statement will take the first element of the third column of the array, it means val = 3

    printf("%d", val);
}

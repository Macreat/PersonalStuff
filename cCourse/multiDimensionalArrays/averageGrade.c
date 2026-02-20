#include <stdio.h>

int main()
{
    // first declare variables
    float average;
    int i, j;
    // also declare the grades as a two dimensional array of integers
    int grades[][5] = {{3, 4, 5, 2, 4},
                       {2, 4, 5, 2, 4}};

    // proceed with the for loop (iteration statement)

    for (i = 0; i < 2; i++)
    {
        average = 0;
        for (j = 0; j < 5; j++)
        {
            average += grades[i][j]; // we calculate the average using all the notes from our TWO-D array
        }
        // now compute the average and print it
        average = average / 5;
        printf("the average marks obtained for subject %d is : %.2f\n", i, average);
    }
}
#include <stdio.h>

int main()
{
    int grades[2]; // define as two based on the zerobase concept and have three elements
    int avg;

    grades[0] = 80; // first element
    grades[1] = 85; // second element
    grades[2] = 90; // third element

    avg = (grades[0] + grades[1] + grades[2]) / 3;
    printf("Average is %d\n", avg); // apply a line jump after the print to make it more readable
    return 0;                       // allways indicate the success of the program
}
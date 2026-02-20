#include <stdio.h>
/*

array variable consists of a sequence of ten numbers

inside my while loop,  have two conditions , this change the flow of the loop (wh/ changing the printf command )


c1 : If the current number which is about to printed is less than 5, don't print it.
c2 : If the current number which is about to printed is greater than 10, don't print it and stop the loop.

*/
int main()
{
    int array[] = {1, 7, 4, 5, 9, 3, 5, 11, 6, 3, 4};
    int i = 0;

    while (i < 11) // define our while loop with the lenght of our array.
    {
        if (array[i] < 5) // we have to iterate inside our array / check C1
        {
            i++;
            continue; // on this condition ,if i doesnt iterate and use continue derivate, i get stuck in an infinite loop
        }
        else if (array[i] > 10) // check C2
        {
            break;
        }
        printf("%d\n", array[i]); // print the values that match the condition
        i++;
    }

    return 0;
}           



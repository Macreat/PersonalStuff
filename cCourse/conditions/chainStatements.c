#include <stdio.h>
#include <stdbool.h>

// now , in order to have more than two outcomes, we can "chain" multiple if statements in order to evaluate a broader context
// also implement a non equ

int main()
{
    // first declare variables evaluating some hypothetical case
    int peanutsEaten = 22;
    int peanutsInJar = 100;
    int peanutLimit = 50;

    if (peanutsInJar > 80)
    {
        if (peanutsEaten < peanutLimit)
        {
            printf("take as many peanuts as you want!\n");
        }
        else
        {
            if (peanutsEaten > peanutsInJar)
            {
                printf("you cant have anymore peanuts");
            }
            else
            {
                printf("aight, just one more peanut.\n");
            }
        }
    }
    return 0;
}

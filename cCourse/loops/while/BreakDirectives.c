#include <stdio.h>
/* EXISTS two TYPES of directives, break and continue directive.

the BREAK DIRECTIVE, HALTS/HOLD the loop after X loops, even though the condition on my while loop
doesnt changes.

this is usefull to end a LOOP given a specific condition or case.

*/
int main()
{
    int X = 5;
    int n = 0;
    while (1)
    {
        printf("%d\n", n);
        n++;
        if (n == X)
        {
            break;
        }
    }

    printf("outside of the loop");
    return 0;
}
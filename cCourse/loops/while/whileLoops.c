#include <stdio.h>
/*

while loops are less functionality, the loop continues executing while the condition inside my while block
remains true

can simulate for loops with a while loop, increasing the iterator until the while condition doesnt match anymore

ALSO , A WHILE LOOP CAN EXECUTE INFINTELY if a condition is given which always evaluates as TRUE (1)
*/
int main()
{

    int i = 0;

    while (i < 10)
    {
        i++;
        printf("%d\n", i);

        if (i == 3)
        {
            printf("waitting for iterator achieve 10\n");
        }
    }
    printf("outisde of the while loop");
    return 0;
}

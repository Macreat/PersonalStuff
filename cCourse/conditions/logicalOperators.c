#include <stdio.h>
#include <stdbool.h>

// also, we can check and evaluated two or more expressions together using logical operators.

// we check if two expresions evaluate are true together, or at least one of them. Also, we can use the NOT operator to denny statements or variables

int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    if (a < b && c > b)
    {

        printf("the smaller value is %d, also %i is the bigger\n", a, c);
    }

    if (a < b || c > b)
    {
        printf("%d is smaller than %d or %i is larger than b\n", a, b, c);
    }

    if (a != 4 || b != 4 || c != 4)
    {
        printf("doesnt match any value bigger than 4 ");
    }
    return 0;
}
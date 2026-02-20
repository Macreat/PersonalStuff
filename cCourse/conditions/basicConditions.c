#include <stdio.h>
#include <stdbool.h> // import bool librarie

// if statement : allow to check if an expression is true or false, and then execute different code according the context

int main()
{
    int target = 10;
    int x, y = (1, 2); // i CAN NOT DEFINE variables like this

    if (target < 10) // use inequality and equal operators
    {
        printf("target is below to 10");
    }
    else // use else keyword to execute code when our first statement doesnt match
    {
        printf("its okay");
        printf("\n");
    }
    if (x > y)
    {
        printf("x greater than y");
    }
    if (x < y)
    {
        printf("x smaller than y ");
    }
    return 0;
}
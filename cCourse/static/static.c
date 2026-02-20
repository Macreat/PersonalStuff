#include <stdio.h>
/*
While static variables have scope over the file containing them making them accessible only inside a given file,
global variables can be accessed outside the file too.

*/
int runner()
{
    static int count = 0; // if its not STATIC , and its global, the desire value its removed from memory, as soon as the function completes.
    count++;
    return count;
}

int main()
{
    printf("%d ", runner());
    printf("%d ", runner());
    return 0;
}
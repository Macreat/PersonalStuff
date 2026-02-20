#include <stdio.h>
#include <string.h>
int main()
{

    char *name = "Mateo";
    // we can use STRNCMP that compares between two strings, this return 0 if they are equal.

    // the arguments on this function are the two strings to be compared and the MAXIMUM comparison LENGTH.

    // proceed creating a conditinoal to evaluate the statement
    if (strncmp(name, "Mati", 5) == 0) // there is also a unsafe version called STRCMP
    {
        printf("Hi Mati ");
    }
    else
    {
        printf("who are you");
    }
}
#include <stdio.h>
#include <string.h>

int main()

{

    // strings are array of characters.

    char *name = "Mateo Almeida"; // use a pointer to a character array to define a simple string
    // this char name, only could use for reading

    char name2[] = "MateoAlmeida"; // this string can be manipulated character for character

    // in last case, the empty brackets notations, tells the compiler to calculate the sie of the array automatically.

    // so its the same that put char name[X]... WHERE x matchs the lenght of the string created +1 , where the string termination
    // is a special character (equal to 0 ) that indicates the end of the string
    int len = strlen(name2);

    int len2 = sizeof(name2); // can use STRLEN or size of to see
    // now, implement string formatting with printf and STRING length
    printf("the lenght on  %s, is %d\n", name, len); // see how i put i, to integer, s to string, d... data ¿?
    printf("the size on  %s, is %d", name, len2);    // see how i put i, to integer, s to string, d... data ¿?

    return 0;
}
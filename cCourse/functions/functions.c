#include <stdio.h>

/*

functions are straigthforward, but their power its limted for the C structure

the function receive either a fixed or variable amount of argument

also, can ONLY return one value, or RETURN NO VALUE.

ON C, the arguments are copied by VALUE to functions, this means that i cannot change the value outside of the function. FOR THIS, WE USE POINTERS.

Always have to first declare, and then define or initailize (IMPLEMENT) my function

Also, remember, that i can use header files to IMPLEMENT MODULARITY in my programs
*/
// define our function
int x(int bar);
// now implement. Nothing happens if doesnt define
int x(int bar)
{
    return bar * 2; // this function only multiplies our bar value called by the x function and return it
}

// remember that i can create a function that dont return any value using the keyword VOID

void nothing()
{
    /*write code */
    
}
int main()
{

    // we call our functions on our main function
    int a = x(2);

    printf("%i\n", a);

    // also can call the function from my printf with any other value
    printf("the value multiplied is %d", x(5));
    return 0;
}

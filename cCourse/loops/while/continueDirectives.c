#include <stdio.h>

/*
on this case, we check if 10 integer numbers are even (par) or odd (impar)
initialize variable, then inside a while loop increaste the iterator until its reach 10, and then

define a conditional inside my while loop, this conditional is in charge of tell if the remainder
(modulus operator % ) of the division between 2 is non zero or 1

if the number is even, the remainder is 0.


*/

int main()
{
    int n = 0;
    while (n < 10)
    {
        n++;

        /* check that n is odd */
        if (n % 2 == 1) // if modulus operator 2 of the number is 1 the number is odd, , then enter to the loop, and go to evaluate ANOTHEr number for the CONTINUE DIRECRIVE DEFINE
        {
            continue; /* go back to the start of the while block understand maybe as a reset direct to my while loop
        }

        /* we reach this code only if n is even */
            printf("The number %d is even.\n", n);
        }
        return 0;
    }
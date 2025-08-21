#include <stdio.h>
#include <stdbool.h>
// on this case use the stdbool.h library to use the bool type
int main()
{
    bool flag = true;
    printf("%d\n", flag); // print 1
    flag = false;         // its not necessary re define the same variable
    printf("%d\n", flag); // print 0
    return 0;
}

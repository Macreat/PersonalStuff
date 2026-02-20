// string concatenation

#include <stdio.h>
// remember include the string lib
#include <string.h>

int main()
{
    char dest[20] = "hello";
    char src[20] = "World";

    // proceed concatenating using strncat, that appends first N characters of SRC string to the DESTINATION dest, where N es min(n,lenght(src)) -> MAXIMUM NUMBER OF CHARS to BE APPENDED.
    strncat(dest, src, 3);
    printf("%s\n", dest);

    // concatening the complete sentnce
    strncat(dest, src, 10);
    printf("%s\n", dest);
}
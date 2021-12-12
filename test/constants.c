#include <stdio.h>


void omg_constants()
{
    int x = 1;
    const int * y = &x;
    const int ** ptr = &y;
    printf("ptr: %zu, *ptr: %zu, **ptr: %i\n",
           ptr, *ptr, **ptr);
}


int main()
{
    omg_constants();
}

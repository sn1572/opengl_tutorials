#include <stdio.h>
#include <stdlib.h>


void omg_constants()
{
    int x = 1;
    const int * y = &x;
    const int ** ptr = &y;
    printf("ptr: %zu, *ptr: %zu, **ptr: %i\n",
           ptr, *ptr, **ptr);
}


void const_after_change()
{
    int x = 1;
    x = 2;
    const int * y = &x;
}


int main()
{
    const_after_change();
}

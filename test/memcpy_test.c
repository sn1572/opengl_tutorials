#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../linmath/linmath.h"


int test1( vec3 * dst ){
	// Going to see what happens
	// when we allocate a vec3 on the stack
	// and then memcpy to src.
	vec3 src = {6,7,8};
	memcpy( *dst, src, 3*sizeof(float) );
	return(0);
}


int main(){
	vec3 dst2;
	test1( &dst2 );
	printf( "dst2 after copy: %1.1f,%1.1f,%1.1f\n",
		dst2[0], dst2[1], dst2[2] );
}

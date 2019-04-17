#include <stdio.h>
#include <mraa.h>

int main()
{
	printf("MRAA VERSION: %s\n", mraa_get_version());
	return 0;
}

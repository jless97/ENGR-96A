#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <mraa/aio.h>

// flag checked in while loop of main function
// initialized to be 1, set to 0 on interrupt (SIGINT)
sig_atomic_t volatile run_flag = 1;

// function to set run_flag to 0
void do_when_interrupted(int sig)
{
	if (sig == SIGINT)
		run_flag = 0;
}

int main()
{
	// variable to store the value returned from mraa_aio_read
	uint16_t value;
	// declare rotary as a mraa_aio_context variable (Analog i/o)
	mraa_aio_context rotary = mraa_aio_init(0);

	signal(SIGINT, do_when_interrupted);

	while(run_flag)
	{
		// mraa_aio_read =. read voltage provided by analog device
		// voltage read will be converted to 10 bit int
		// 10 bit int can represent 2^10(1024) discrete values
		value = mraa_aio_read(rotary);
		printf("%d\n", value);
		sleep(1);
	}
	
	mraa_aio_close(rotary);

	return 0;
}


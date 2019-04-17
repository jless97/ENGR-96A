#include <signal.h>
#include <mraa/gpio.h>

// flag checked in while loop in main function
// it is initialized to be 1, or logical true
// it will be set to 0, or logical false on receipt of SIGINT
sig_atomic_t volatile run_flag = 1;

// thus function will set the run_flag to logical false
void do_when_interrupted(int sig)
{
	// if statement will verify if the signal is SIGINT before proceeding
	if (sig == SIGINT)
		run_flag = 0;
}

int main()
{
	// declare led as mraa_gip_context variable
	mraa_gpio_context led;
	// initialize interface D3 on the Grove Base Shield for led
	led = mraa_gpio_init(3);
	// configure led GPIO interface to be an output pin
	mraa_gpio_dir(led, MRAA_GPIO_OUT);

	//when the SIGINT signal is received, call do_when_interrupted
	signal(SIGINT, do_when_interrupted);

	// execute if run_flag is 1
	// break while loop if run flag is 0
	while (run_flag) {
		// turn the LED on by setting the output to 1
		mraa_gpio_write(led, 1);
		//use sleep function so we can detect change in LED state
		sleep(1);
		// turn LED off by setting output to 0
		mraa_gpio_write(led, 0);
		sleep(1);
	}

	mraa_gpio_close(led);
	return 0;
}

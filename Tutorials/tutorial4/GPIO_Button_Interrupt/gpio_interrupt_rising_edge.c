#include <signal.h>
#include <mraa/gpio.h>

// flag checked in while loop of main function
// initialized to 1, set to 0 in interrupt_handler function
sig_atomic_t volatile run_flag = 1;

// Interrupt handler that sets run_flag to 0
void do_when_interrupted()
{
	run_flag = 0;
}

int main()
{
	//declare led and btn (button) as mraa_gpio_context variables
	mraa_gpio_context led, btn;
	// initialize led to D3 and btn to D4 on the Base Shield
	led = mraa_gpio_init(3);
	btn = mraa_gpio_init(4);

	// configure led GPIO interface to be an output pin
	mraa_gpio_dir(led, MRAA_GPIO_OUT);
	// configure btn GPIO interface to be an input pin
	mraa_gpio_dir(btn, MRAA_GPIO_IN);

	// when button is pressed, explicit interrupt of SIGINT, call handler
	mraa_gpio_isr(btn, MRAA_GPIO_EDGE_RISING, &do_when_interrupted, NULL);

	// execute if run_flag is set
	// break loop when interrupt
	while (run_flag) {
		// turn LED on by setting output to 1
		mraa_gpio_write(led, 1);
		// use sleep so I can see the LED change state
		sleep(1);
		// turn LED off by setting output to 0
		mraa_gpio_write(led, 0);
		sleep(1);
	}

	mraa_gpio_close(led);
	mraa_gpio_close(btn);

	return 0;
}

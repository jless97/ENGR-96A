#include <mraa/i2c.h>

// define the registers and addresses for the RGB backlit screen
// note the shift by 1 for the address => due to being a 7-bit address
#define RGB_ADDRESS (0xc4 >> 1)
#define REG_MODE1 0x00
#define REG_MODE2 0X01
#define REG_OUTPUT 0X08

#define REG_RED 0x04
#define REG_GREEN 0X03
#define REG_BLUE 0X02

// function that will send the byte 'dta' to the address 'addr' on the I2C bus
// 'rgb-i2c'
void rgb_command(mraa_i2c_context rgb_i2c, uint8_t addr, uint8_t dta)
{
	uint8_t to_send[2] = {addr, dta};
	mraa_i2c_write(rgb_i2c, to_send, 2);
}

// function to set the color of the RGB backlit screen
void set_RGB_color(mraa_i2c_context rgb_i2c, uint8_t r, uint8_t g, uint8_t b)
{
	rgb_command(rgb_i2c, REG_RED, r);
	rgb_command(rgb_i2c, REG_GREEN, g);
	rgb_command(rgb_i2c, REG_BLUE, b);
}

int main()
{
	mraa_i2c_context rgb_i2c;

	// initialize the mraa library
	mraa_init();

	rgb_i2c = mraa_i2c_init(0);

	// set up the i2c busses
	mraa_i2c_address(rgb_i2c, RGB_ADDRESS);
	
	// set up the rgb backlight
	rgb_command(rgb_i2c, REG_MODE1, 0);
	rgb_command(rgb_i2c, REG_OUTPUT, 0xFF);
	rgb_command(rgb_i2c, REG_MODE2, 0x20);

	// set the RGB values to 0, 0 , 255 indicating the screen should be
	// completely blue
	set_RGB_color(rgb_i2c, 50, 50, 50);

	return 0;
}	

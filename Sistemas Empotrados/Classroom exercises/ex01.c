#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"

#define DELAY 100000000
#define LEDS_ADDR XPAR_AXI_GPIO_1_BASEADDR

int main(void)
{
	init_platform();
	unsigned char	*LEDs = LEDS_ADDR;
	uint32_t		JohnsonCode = 0b00000001001101111111111011001000;

	*(LEDs + 4) = 0x00; // Outputs
	for (int i = 0; i < 32; i += 4)
	{
		*LEDs = (JohnsonCode >> i) & 0xF;
		for (int j = 0; j < DELAY; j++);
		if (i == 28)
			i = -4;
	}
	cleanup_platform();
}

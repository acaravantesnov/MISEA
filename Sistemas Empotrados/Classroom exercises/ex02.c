#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"
#include "xgpio.h"
#define DELAY 1000000

int main(void)
{
	init_platform();
	XGpio LEDs;
	XGpio BUTs;

	XGpio_Initialize(&LEDs, XPAR_AXI_GPIO_1_DEVICE_ID);
	XGpio_SetDataDirection(&LEDs, 2, 0x00);
	XGpio_Initialize(&BUTs, XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&BUTs, 1, 0xFF);

	while (1)
	{
		XGpio_DiscreteWrite(&LEDs, 2, XGpio_DiscreteRead(&BUTs, 1));
		for (int i = 0; i < DELAY; i++);
	}
	cleanup_platform();
}

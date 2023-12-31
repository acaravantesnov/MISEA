#include "platform.h"
#include "xparameters.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "xil_printf.h"
#define TmrCtrNumber 0

XTmrCtr TimerCounter;

int main(void)
{
	init_platform();
	XGpio		BUTs;
	uint32_t	TCStatus;
	int 		buttonPressed = 0;

	XTmrCtr_Initialize(&TimerCounter, XPAR_TMRCTR_0_DEVICE_ID);
	XTmrCtr_SetOptions(&TimerCounter, TmrCtrNumber, XTC_DOWN_COUNT_OPTION);
	XTmrCtr_SetResetValue(&TimerCounter, TmrCtrNumber, 5000000);
	XGpio_Initialize(&BUTs, XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&BUTs, 2, 0xFF);
	while (1)
	{
		if (((XGpio_DiscreteRead(&BUTs, 2) & 0x00000002) != 0)  && !buttonPressed)
		{
			XTmrCtr_Start(&TimerCounter, TmrCtrNumber);
			while (!XTmrCtr_IsExpired(&TimerCounter, TmrCtrNumber));
			print("Button pressed\n");
			buttonPressed = 1;
			TCStatus = XTmrCtr_GetControlStatusReg(TimerCounter.BaseAddress, TmrCtrNumber);
			XTmrCtr_SetControlStatusReg(TimerCounter.BaseAddress, TmrCtrNumber, TCStatus);
			XTmrCtr_Stop(&TimerCounter, TmrCtrNumber);
		}
		else if ((XGpio_DiscreteRead(&BUTs, 2) & 0x00000002) == 0)
			buttonPressed = 0;
	}
	cleanup_platform();
}

#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"

#define GPIO_INT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR

void Buttons_ISR(void *CallBackRef); // ISR Declaration
XGpio	Buttons;
XScuGic IntcInstance;

int ScuGic_Initialize(XScuGic *IntcInstancePtr);
int SetUpExceptions(XScuGic *XscuGicInstancePtr);

int main(void)
{
	int Status;

	// Peripheral configuration
	Status = XGpio_Initialize(&Buttons, XPAR_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&Buttons, 2, 0xFF); //inputs

	ScuGic_Initialize(&IntcInstance);

	// Configure XScuGic
	XScuGic_SetPriorityTriggerType(&IntcInstance, GPIO_INT_ID, 0xa0, 0x3);
	Status = XScuGic_Connect(&IntcInstance, GPIO_INT_ID, (Xil_InterruptHandler)Buttons_ISR, &Buttons);
	XScuGic_Enable(&IntcInstance, GPIO_INT_ID);

	// Configure GPIO Interrupts
	XGpio_InterruptEnable(&Buttons, 2);
	XGpio_InterruptGlobalEnable(&Buttons);

	SetUpExceptions(&IntcInstance);
}

// Use this function to initialize SCUGIC
int ScuGic_Initialize (XScuGic *IntcInstancePtr)
{
	int Status;
	XScuGic_Config *IntcConfig;

	// Looks up the device configuration
	IntcConfig = XScuGic_LookupConfig (XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if (IntcConfig == NULL)
		return XST_FAILURE;
	// Initializes the XScuGic structure
	Status = XScuGic_CfgInitialize (IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	return XST_SUCCESS;
}

// Use this function to configure exceptions
int SetUpExceptions (XScuGic *XScuGicInstancePtr)
{
	// Connect XScuGic default interrupt handler
	Xil_ExceptionRegisterHandler (XIL_EXCEPTION_ID_INT, // Exception ID (IRQ)
	(Xil_ExceptionHandler) XScuGic_InterruptHandler, // Generic handler
	XScuGicInstancePtr); // Pointer to XScuGic instance
	Xil_ExceptionEnable (); //Enable interrupts
	return XST_SUCCESS;
}

// Interrupt function
void Buttons_ISR(void *CallBackRef)
{
	XGpio_InterruptDisable(&Buttons, 2);
	for (int i = 0; i < 4; i++)
	{
		u32 desp = 0b1 << i;
		if ((XGpio_DiscreteRead(&Buttons, 2) & desp) == desp)
			xil_printf("Pressed button %i\n", i);
	}
	XGpio_InterruptClear(&Buttons, 2); // Clear interrupt flag
	XGpio_InterruptEnable(&Buttons, 2);
}

#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "xil_printf.h"

#define GPIO_INT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define TIMER_INT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR

XScuGic Intc; // The instance of the Interrupt Controller
XTmrCtr Tmr; // The instance of the TmrCtr device
XGpio 	BUTs;

int ScuGic_Initialize(XScuGic *IntcInstancePtr);
int SetUpExceptions(XScuGic *XscuGicInstancePtr);
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);

int TimerExpired = 0;

int main()
{
	int Status;

	XGpio_Initialize(&BUTs, XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&BUTs, 2, 0xFF);

	// Peripheral configuration: Tmr
	Status = XTmrCtr_Initialize(&Tmr, XPAR_TMRCTR_0_DEVICE_ID);
	XTmrCtr_SetOptions(&Tmr, 0, XTC_DOWN_COUNT_OPTION | XTC_INT_MODE_OPTION);
	XTmrCtr_SetResetValue(&Tmr, 0, 5000000);

	// Initialize ScuGic
	ScuGic_Initialize(&Intc);

	// Configure ScuGic interrupt sources: Tmr
	XScuGic_SetPriorityTriggerType(&Intc, TIMER_INT_ID, 0xa0, 0x3);
	Status = XScuGic_Connect(&Intc, TIMER_INT_ID, (Xil_InterruptHandler)XTmrCtr_InterruptHandler, &Tmr);
	XScuGic_Enable(&Intc, TIMER_INT_ID); // Enable Tmr interrupts

	// Configure Timer interrupt
	XTmrCtr_SetHandler(&Tmr, TimerCounterHandler, &Tmr);

	SetUpExceptions(&Intc);

	while ((XGpio_DiscreteRead(&BUTs, 2) & 0x00000002) == 0) {}
	XTmrCtr_Start(&Tmr, 0); // Start Timer 0
	while (1) ; // Main loop to be interrupted
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

// Timer ISR
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	uint32_t		TCStatus;

	if (XTmrCtr_IsExpired(&Tmr, 0))
	{
		xil_printf("Button pressed\r\n");
		TCStatus = XTmrCtr_GetControlStatusReg(Tmr.BaseAddress, TmrCtrNumber);
		XTmrCtr_SetControlStatusReg(Tmr.BaseAddress, TmrCtrNumber, TCStatus);
		XTmrCtr_Stop(&Tmr, 0);
		while ((XGpio_DiscreteRead(&BUTs, 2) & 0x00000002) == 0)
		{}
		XTmrCtr_Start(&Tmr, 0);
	}
}

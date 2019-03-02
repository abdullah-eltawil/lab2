#include "address_map_arm.h"

/*Global Variables*/
volatile int tick = 0; // increment every time the HPS timer expires
volatile unsigned char sec = 0; // increment every 100 ticks
volatile unsigned char min = 0; // increment every 60 sec
volatile unsigned char hrs = 0; // increment every 60 min

volatile int * min_sec_disp_ptr = (int *)HEX3_HEX0_BASE;	//Seven Segment Display Addresse
volatile int * hrs_disp_ptr = (int *)HEX5_HEX4_BASE;		//Seven Segment Display Addresse

/*Function Prototypes*/
//void config_GIC(void);
void config_HPS_timer(void);
void updateHex(unsigned char h, unsigned char m, unsigned char s);

/*Main Function*/
int main(void)
{
	//config_GIC(); // configure the general interrupt controller
	config_HPS_timer(); // configure the HPS timer

	while(1)
	{
		if(tick >= 100){
			tick = 0;
			++sec;
		}
		if(min >= 60){
			min = 0;
			++hrs;
		}
		if(sec >= 60){
			sec = 0;
			++min;
		}

		updateHex(hrs,min,sec);

	}
}

void updateHex(unsigned char h, unsigned char m, unsigned char s)
{
    unsigned int hexVal_lower;
    unsigned int hexVal_upper; 


	unsigned char lookUpTable[16] = {
		0x3F,
		0x06,
		0x5B,
		0x4F,
		0x66,
		0x6D,
		0x7D,
		0x07,
		0x7F,
		0x6F,
		0x77,
		0x7C,
		0x39,
		0x5E,
		0x79,
		0x71
	};

    //obtaining hex value from look up table
    hexVal_lower = ((lookUpTable[m-(m%10)])<<24) + ((lookUpTable [m%10])<<16) + ((lookUpTable[s-(s%10)])<<8) + (lookUpTable [s%10]);
    *(min_sec_disp_ptr) = hexVal_lower;

    hexVal_upper = ((lookUpTable[h-(h%10)])<<8) + (lookUpTable [h%10]);
    *(hrs_disp_ptr) = hexVal_upper;
}

/* Setup HPS Timer */
void config_HPS_timer()
{
	volatile int * HPS_timer_ptr = (int *)HPS_TIMER0_BASE; // timer base address
	*(HPS_timer_ptr + 0x2) = 0; // write to control register to stop timer
	
	/* set the timer period */
	int counter = 1000000; // period = 1/(100 MHz) x (1 x 10^6) = 0.01 sec
	*(HPS_timer_ptr) = counter; // write to timer load register
	
	/* write to control register to start timer, with interrupts */
	*(HPS_timer_ptr + 2) = 0b011; // int mask = 0, mode = 1, enable = 1
}

//Might need for Timer Interrupt??
/*
//Configure the Generic Interrupt Controller (GIC)
void config_GIC(void)
{
	int address; // used to calculate register addresses
	//configure the HPS timer interrupt 
	*((int *)0xFFFED8C4) = 0x01000000;
	*((int *)0xFFFED118) = 0x00000080;
	
	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
	// priorities
	address = MPCORE_GIC_CPUIF + ICCPMR;
	*((int *)address) = 0xFFFF;
	
	
	
	// Set CPU Interface Control Register (ICCICR). Enable signaling of
	// interrupts
	address = MPCORE_GIC_CPUIF + ICCICR;
	*((int *)address) = ENABLE;

	// Configure the Distributor Control Register (ICDDCR) to send pending
	// interrupts to CPUs
	address = MPCORE_GIC_DIST + ICDDCR;
	*((int *)address) = ENABLE;

	
}
*/

/*Interrupt Service Routine for Timer0*/
void HPS_timer_ISR()
{
	volatile int * HPS_timer_ptr = (int *)HPS_TIMER0_BASE; // HPS timer address
	++tick; 	// used by main program
	*(HPS_timer_ptr + 3); 	// Read timer end of interrupt register to
							// clear the interrupt
	return;
}
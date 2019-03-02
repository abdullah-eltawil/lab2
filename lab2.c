#include "address_map_arm.h"

/*Global Variables*/
volatile unsigned char hun = 0; // increment every 60 min
volatile unsigned char sec = 0; // increment every 100 ticks
volatile unsigned char min = 0; // increment every 60 sec

volatile int * HPS_timer_ptr = (int *)HPS_TIMER0_BASE; // timer base address
volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address

volatile int * sec_hun_disp_ptr = (int *)HEX3_HEX0_BASE;	//Seven Segment Display Address
volatile int * min_disp_ptr = (int *)HEX5_HEX4_BASE;		//Seven Segment Display Address
volatile int key_dir = 0;	//useless lol

/*Function Prototypes*/
void config_GIC(void);
void config_HPS_timer(void);
void config_KEYs(void);
void updateHex(unsigned char m, unsigned char s, unsigned char h);

/*Main Function*/
int main(void)
{
	config_GIC(); // configure the general interrupt controller
	config_HPS_timer(); // configure the HPS timer
	void config_KEYs();

	while(1)
	{
		if(hun >= 100){
			hun = 0;
			++sec;
		}
		if(sec >= 60){
			sec = 0;
			++min;
		}

		updateHex(min,sec,hun);

	}
}

void updateHex(unsigned char m, unsigned char s, unsigned char h)
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
    hexVal_lower = ((lookUpTable[s-(s%10)])<<24) + ((lookUpTable [s%10])<<16) + ((lookUpTable[s-(h%10)])<<8) + (lookUpTable [h%10]);
    *(sec_hun_disp_ptr) = hexVal_lower;

    hexVal_upper = ((lookUpTable[m-(m%10)])<<8) + (lookUpTable [m%10]);
    *(min_disp_ptr) = hexVal_upper;
}

/* setup the KEY interrupts in the FPGA */
void config_KEYs()
{
	*(KEY_ptr + 2) |= 0b011; // enable interrupts for KEY[0] & KEY[1]
}

/* Setup HPS Timer */
void config_HPS_timer()
{
	*(HPS_timer_ptr + 2) &= ~0b1; // write to control register to stop timer
	
	/* set the timer period */
	int counter = 1000000; // period = 1/(100 MHz) x (1 x 10^6) = 0.01 sec
	*(HPS_timer_ptr) = counter; // write to timer load register
	
	/* write to control register to start timer, with interrupts */
	*(HPS_timer_ptr + 2) |= 0b011; // int mask = 0, mode = 1, enable = 1
}


//Configure the Generic Interrupt Controller (GIC)
void config_GIC(void)
{
	int address; // used to calculate register addresses

	/* configure the HPS timer interrupt */
	*((int *)0xFFFED8C4) = 0x01000000;
	*((int *)0xFFFED118) = 0x00000080;

	/* configure the FPGA interval timer and KEYs interrupts */
	*((int *)0xFFFED848) = 0x00000101;
	*((int *)0xFFFED108) = 0x00000300;
	
	// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
	// priorities
	//	address = MPCORE_GIC_CPUIF + ICCPMR;
	//	*((int *)address) = 0xFFFF;
}

/*Interrupt Service Routine for Timer0*/
void HPS_timer_ISR()
{
	++hun; 	// used by main program
	*(HPS_timer_ptr + 3); 	// Read timer end of interrupt register to
							// clear the interrupt
	return;
}

void pushbutton_ISR(void)
{
	int press;
	
	press = *(KEY_ptr + 3); // read the pushbutton interrupt register
	*(KEY_ptr + 3) = press; // Clear the interrupt
	key_dir ^= 1; // Toggle key_dir value

	if(press == 0b10)
		*(HPS_timer_ptr + 2) &= ~0b01; 	// write to control register to stop timer (i.e. STOP)
	else if(press == 0b10)
		*(HPS_timer_ptr + 2) |= 0b01; 	// write to control register to start timer (i.e. START)
	
	return;
}
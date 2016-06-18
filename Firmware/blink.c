/* al1 06-2016
   Basic blink test program for samDEV_09. to avoid delay functions a Timer is used. ASF is also not used. 
   This pages helps me a lot to write this code:
   https://eewiki.net/display/microcontroller/Getting+Started+with+the+SAM+D21+Xplained+Pro+without+ASF */ 

// includes
#include "sam.h"

// prototypes
void init_TC1(void);

void init_TC1(void)
{
	//setup clock for timer/counters
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC1_TC2;	
	REG_PM_APBCMASK |= PM_APBCMASK_TC1;
	REG_TC1_CTRLA |=TC_CTRLA_PRESCALER_DIV8;	// prescaler: 8
	REG_TC1_INTENSET = TC_INTENSET_OVF;		// enable overflow interrupt
	REG_TC1_CTRLA |= TC_CTRLA_ENABLE;		// enable TC1
	while(TC1->COUNT16.STATUS.bit.SYNCBUSY==1);	// wait on sync
	NVIC_EnableIRQ(TC1_IRQn);			// enable TC1 interrupt in the nested interrupt controller
}

void TC1_Handler()
{
	REG_PORT_OUTTGL0 = PORT_PA02;		// troggle PA02
	REG_TC1_INTFLAG = TC_INTFLAG_OVF;	// reset interrupt flag - NEEDED HERE!
}

int main(void)
{
    SystemInit();
	init_TC1();
	REG_PORT_DIRSET0 = PORT_PA02;		// PA02(on board LED)-> output
    while (1);					// no CPU tasks
}

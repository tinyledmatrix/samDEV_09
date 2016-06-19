/* code to test the serial UART communiction over the USB-SERIAL birdge on SAMdev_09. No ASF used. 
   Some of the code is based on code published by Atmel.
   The UART function are not universal for all SEROM-modules, although this was my goal. In some places SERCOM1 is hard coded.
 */ 

// inlcludes
#include "sam.h"

// defines
//#define MY_BAUDREG_VALUE 64278	// 64278 (F_CPU=F_SERCOM=8MHz) -> 9600 BAUD
#define MY_BAUDREG_VALUE 55470		// 55470 (F_CPU=F_SERCOM=1MHz) -> 9600 BAUD
//==(65536*(1-16*(MY_BAUD/F_SERCOM))

// prototypes
void UART_sercom_simpleWrite(Sercom *const sercom_module, uint8_t data);
static inline void pin_set_peripheral_function(uint32_t pinmux);
uint8_t UART_sercom_init(Sercom *const sercom_module);

static inline void pin_set_peripheral_function(uint32_t pinmux)
{
	/* the variable pinmux consist of two components:
		31:16 is a pad, wich includes:
			31:21 : port information 0->PORTA, 1->PORTB
			20:16 : pin 0-31
		15:00 pin multiplex information
		there are defines for pinmux like: PINMUX_PA09D_SERCOM2_PAD1 
	*/
	uint16_t pad = pinmux >> 16;	// get pad (port+pin)
	uint8_t port = pad >> 5;		// get port
	uint8_t pin  = pad & 0x1F;		// get number of pin - no port information anymore
	
	PORT->Group[port].PINCFG[pin].bit.PMUXEN =1;
	
	/* each pinmux register is for two pins! with pin/2 you can get the index of the needed pinmux register
	   the p mux resiter is 8Bit   (7:4 odd pin; 3:0 evan bit)	*/
	// reset pinmux values.								VV shift if pin is odd (if evan:  (4*(pin & 1))==0  )
	PORT->Group[port].PMUX[pin/2].reg &= ~( 0xF << (4*(pin & 1)) );
					//  		
	// set new values
	PORT->Group[port].PMUX[pin/2].reg |=  ( (uint8_t)( (pinmux&0xFFFF) <<(4*(pin&1)) ) ); 
}

uint8_t UART_sercom_init(Sercom *const sercom_module)	// this function is still hardcoded to SERCOM0 in parts
{	
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;	// APBCMASK

	/*gclk configuration for sercom0 module*/								
	GCLK->CLKCTRL.reg |= ((uint16_t)( GCLK_CLKCTRL_GEN_GCLK0_Val|
									  GCLK_CLKCTRL_ID(SERCOM1_GCLK_ID_CORE)| //<----SERCOM0 hard coded
									  GCLK_CLKCTRL_CLKEN ));
	sercom_module->USART.CTRLA.reg |= 1;	// rest the sercom module
	
	/*wait until reset is done*/
	// (1<<15) only for SAMD20 resserved in other SAMDs, but read as zero
	while((sercom_module->USART.STATUS.reg & (1<<15)) ||
		  (sercom_module->USART.CTRLA.reg & SERCOM_USART_CTRLA_SWRST));
	
	/*USART module configuration*/
	sercom_module->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD |				// LSB first
									 SERCOM_USART_CTRLA_RXPO(0x3) |			// RX -> PAD3
									 SERCOM_USART_CTRLA_TXPO(0x1) |			// TX -> PAD2
									 SERCOM_USART_CTRLA_SAMPR(0x0)|			// 16x oversampling
									 SERCOM_USART_CTRLA_RUNSTDBY |			// run in sleep mode
									 SERCOM_USART_CTRLA_MODE_USART_INT_CLK; // using internal clock
									 
	sercom_module->USART.BAUD.reg = (uint16_t)MY_BAUDREG_VALUE;	// setting the baud rate
									 
	/* 8-bits size is selected as character size by setting the bit CHSIZE as 0,
	TXEN bit and RXEN bits are set to enable the Transmitter and receiver*/
	sercom_module->USART.CTRLB.reg =SERCOM_USART_CTRLB_CHSIZE(0x0) |
									SERCOM_USART_CTRLB_TXEN |
									SERCOM_USART_CTRLB_RXEN ;
	while(sercom_module->USART.SYNCBUSY.bit.CTRLB);
	
	NVIC_EnableIRQ(SERCOM1_IRQn);										// enable intrrupt  <---SERCOM0 hard coded
	sercom_module->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;		// RX interrupt;

	/*enable sercom0*/
	sercom_module->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;		// enable the SERCOM module
	while(sercom_module->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
	while(!(sercom_module->USART.INTFLAG.reg & 1));						//wait sercom module ready to receive data
	
// 	sercom_module->USART.DATA.reg = 0;
// 	while(!(sercom_module->USART.INTFLAG.reg & 2)); //wait until TX complete;
	
	return 0;
}

void SERCOM1_Handler()	// SERCOM0 ISR
{
	uint8_t buffer;
	buffer	= SERCOM1->USART.DATA.reg;
	while(!(SERCOM1->USART.INTFLAG.reg & 1)); // wait UART module ready to receive data
	SERCOM1->USART.DATA.reg = buffer;				// just sent that byte aback
	while(!(SERCOM1->USART.INTFLAG.reg & 2)); // wait until TX complete;
}


void UART_sercom_simpleWrite(Sercom *const sercom_module, uint8_t data)
{
	while(!(sercom_module->USART.INTFLAG.reg & 1)); //wait UART module ready to receive data
	sercom_module->USART.DATA.reg = data;
	while(!(sercom_module->USART.INTFLAG.reg & 2)); //wait until TX complete;
}

int main(void)
{
    SystemInit();
		
    pin_set_peripheral_function(PINMUX_PA24C_SERCOM1_PAD2); // SAMD09 TX
    pin_set_peripheral_function(PINMUX_PA25C_SERCOM1_PAD3);	// SAMD09 RX
    UART_sercom_init(SERCOM1);
	
    UART_sercom_simpleWrite(SERCOM1, 'H');
    UART_sercom_simpleWrite(SERCOM1, 'e');
    UART_sercom_simpleWrite(SERCOM1, 'l');
    UART_sercom_simpleWrite(SERCOM1, 'l');
    UART_sercom_simpleWrite(SERCOM1, 'o');
    UART_sercom_simpleWrite(SERCOM1, '!');
    UART_sercom_simpleWrite(SERCOM1, 0xa);
	
    while (1) ;
}

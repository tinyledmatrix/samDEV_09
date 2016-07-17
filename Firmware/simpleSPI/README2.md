some little basic files to use SERCOM0 for SPI. 

licensed under BSD 3-Clause License see also in the files

###How to use it:
in your main.c you must do some pin-pad multiplexing and call the functions. Also the 
slave select line is not handled by my library yet. You must do that in software. 
Here is one example 
```C
int main(void)
{
	uint8_t myVar;
	
	//do some other stuff
	
	// SPI setup (for pin_set_peripheral_function() see my uart library)
	pin_set_peripheral_function(PINMUX_PA14C_SERCOM0_PAD0); // mosi
	pin_set_peripheral_function(PINMUX_PA15C_SERCOM0_PAD1); // clk
	pin_set_peripheral_function(PINMUX_PA08D_SERCOM0_PAD2); // miso
	SPI_setup(3);
    PORT->Group[0].DIRSET.reg = PORT_PA02;      // pa02 as SW SS
    PORT->Group[0].OUTSET.reg = PORT_PA02;      // SS idle high
	
	//do some SPI stuff
	PORT->Group[0].OUTCLR.reg = PORT_PA02;		// SS low
    while(PORT->Group[0].OUT.reg & PORT_PA02);	// wait till SS is low
	myVar = SPI_RW_wait(0x42);
	PORT->Group[0].OUTSET.reg = PORT_PA02;		// SS high
	
	// do other stuff and some endless loop	
}
```
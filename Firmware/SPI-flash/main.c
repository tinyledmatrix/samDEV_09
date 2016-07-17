/*
 * SAMD09_SPI-Flash
 * some dirty/qick and not well commented code to read/write SPI flash. 
 *
 * Created: 07-2016
 * Author : al1
 */ 
/*
Redistribution and use in source and binary forms, with or without modification, are 
permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of 
   conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of 
   conditions and the following disclaimer in the documentation and/or other materials 
   provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used 
   to endorse or promote products derived from this software without specific prior written 
   permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "sam.h"
#include "simpleSPI.h"    
#include "uart.h"

//SPI flash commands:
#define FLASH_CMD_WRITE_ENABLE   0x06                     // set the write enable latch bit
#define FLASH_CMD_WRITE_DISABLE  0x04                     // reset the write enable latch bit
#define FLASH_CMD_READ_ID        0x9F                     // read the manufacturer ID and 2 byte device ID
#define FLASH_CMD_READ_STATUS    0x05                     // read the status register
#define FLASH_CMD_WRITE_STATUS   0x01                     // write to the status register
#define FLASH_CMD_READ_DATA      0x03 // + AD1 AD2 AD3       read n bytes until CS goes high
#define FLASH_CMD_FAST_READ_DATA 0x0b // + AD1 AD2 AD3 XX    XX:additional wait byte
#define FLASH_CMD_SECTOR_ERASE   0x20 // + AD1 AD2 AD3       Erase a sector
#define FLASH_CMD_BLOCK_ERASE    0xD8 // + AD1 AD2 AD3       Erase a block
#define FLASH_CMD_CHIP_ERASE     0x60                     // Erase the chip (alt comd: 0xC7) 
#define FLASH_CMD_PAGE_PROGRAM   0x02 // + AD1 AD3 AD3       program a page
#define FLASH_CMD_DEEP_PWR_DWN   0xB9                     // deep power down
#define FLASH_CMD_WEAKUP         0xAB                     // release from deep power down
#define FLASH_CMD_READ_E_ID      0xAB // + XX  XX  XX        read electronic ID
#define FLASH_CMD_READ_MAN_ID    0x90 // + XX  XX  ADD       read manufacturer and device ID
                              //                         ADD=0: manufacturer ID first
                              //                         ADD=1: device ID first
 /*
 Memory Organization:
 
 BLOCK | SECTOR |  ADDRESS RANGE
 ------+--------+----------------
       |   31   | 01F000 - 01FFFF
   1   |  ...   |       ...
       |   16   | 010000 - 010FFF
 ------+--------+----------------
       |   15   | 00F000 - 00FFFF
   2   |  ...   |       ...
       |    0   | 000000 - 000FFF
 
                                 
*/
 
void flash_set_SS(void)
 {
     PORT->Group[0].OUTCLR.reg = PORT_PA02;
     while(PORT->Group[0].OUT.reg & PORT_PA02);     
 }
 
void flash_reset_SS(void)
 {
     PORT->Group[0].OUTSET.reg = PORT_PA02;
 }
 
uint32_t flash_read_id(void)
{
    uint32_t ID;                    // Maufactorer ID [23:16]; device ID [15:0]
    flash_set_SS();                 // SS low
    SPI_RW_wait(FLASH_CMD_READ_ID); // read Id command
    ID  = SPI_RW_wait(0xFF)<<16;
    ID |= SPI_RW_wait(0xFF)<<8;
    ID |= SPI_RW_wait(0xFF);
    flash_reset_SS();               // SS high
    return ID;
} 

void flash_singleByteCommand(uint8_t cmd)
{
    flash_set_SS();
    SPI_RW_wait(cmd);
    flash_reset_SS();
}

uint8_t flash_read_status(void)
{
    uint8_t status;
    flash_set_SS();
    SPI_RW_wait(FLASH_CMD_READ_STATUS);
    status=SPI_RW_wait(0xFF);
    flash_reset_SS();
    return status;
}

void flash_chip_erase(void)
{
    flash_singleByteCommand(FLASH_CMD_WRITE_ENABLE);
    flash_singleByteCommand(FLASH_CMD_CHIP_ERASE);
    while (flash_read_status() & 1);                // wait for wrtie to get ready.
}

void flash_page_program(uint32_t address, uint8_t data[])
{
    flash_singleByteCommand(FLASH_CMD_WRITE_ENABLE);
    flash_set_SS();     // SS_LOW
    SPI_RW_wait(FLASH_CMD_PAGE_PROGRAM);
    SPI_RW_wait(address>>16);   // address MSB
    SPI_RW_wait(address>>8);
    SPI_RW_wait(address);
    for (uint16_t i=0; i<=255; i++)
        SPI_RW_wait(data[i]);
    flash_reset_SS();
    flash_singleByteCommand(FLASH_CMD_WRITE_DISABLE);
    while(flash_read_status() & 1);     // wati to get ready       
}

void flash_drity_read(uint32_t address, uint8_t data[], uint16_t dataLength)
{
    flash_set_SS();
    SPI_RW_wait(FLASH_CMD_READ_DATA);
    SPI_RW_wait(address>>16);   // address MSB
    SPI_RW_wait(address>>8);
    SPI_RW_wait(address);
    for (uint16_t i=0; i<dataLength; i++)
        data[i]=SPI_RW_wait(0xFF);
    flash_reset_SS();
}    

void flash_write_status(uint8_t newStatus)
{
    flash_set_SS();
    SPI_RW_wait(FLASH_CMD_WRITE_STATUS);
    SPI_RW_wait(newStatus);
    flash_reset_SS();
}

// very useful function. At least this version was written by me based on some code found somewhere on line (forgot where) 
void pin_set_peripheral_function(uint32_t pinmux)
{
    /* the variable pinmux consist of two components:
        31:16 is a pad, wich includes:
            31:21 : port information 0->PORTA, 1->PORTB
            20:16 : pin 0-31
        15:00 pin multiplex information
        there are defines for pinmux like: PINMUX_PA09D_SERCOM2_PAD1 
    */
    uint16_t pad = pinmux >> 16;    // get pad (port+pin)
    uint8_t port = pad >> 5;        // get port
    uint8_t pin  = pad & 0x1F;      // get number of pin - no port information anymore
    
    PORT->Group[port].PINCFG[pin].bit.PMUXEN =1;
    
    /* each pinmux register is for two pins! with pin/2 you can get the index of the needed pinmux register
       the p mux resiter is 8Bit   (7:4 odd pin; 3:0 evan bit)  */
    // reset pinmux values.                             VV shift if pin is odd (if evan:  (4*(pin & 1))==0  )
    PORT->Group[port].PMUX[pin/2].reg &= ~( 0xF << (4*(pin & 1)) );
                    //          
    // set new values
    PORT->Group[port].PMUX[pin/2].reg |=  ( (uint8_t)( (pinmux&0xFFFF) <<(4*(pin&1)) ) ); 
}

void printHEX(uint8_t buffer)
{
    uint8_t buffer2=(buffer & 0xF0)>>4;// high digit
    UART_sercom_simpleWrite(SERCOM1, (buffer2>9)?('A'+buffer2-10):('0' + buffer2) );
    buffer2=buffer & 0xF;              // low digit
    UART_sercom_simpleWrite(SERCOM1, (buffer2>9)?('A'+buffer2-10):('0' + buffer2) );
}

int main(void)
{
    // create test page buff
    uint8_t myPage[256];
    for (uint16_t i=0; i<=255; i++)
        myPage[i]=i;
        
    uint8_t flashReadBuffer[255];
    
    /* Initialize the SAM system */
    SystemInit();

    uint8_t buffer=0xFF;
	uint8_t buffer2=0xFF;
    uint32_t flashID=0;
	
	pin_set_peripheral_function(PINMUX_PA14C_SERCOM0_PAD0); // mosi
	pin_set_peripheral_function(PINMUX_PA15C_SERCOM0_PAD1); // clk
	pin_set_peripheral_function(PINMUX_PA08D_SERCOM0_PAD2); // miso
	SPI_setup(3);
    PORT->Group[0].DIRSET.reg = PORT_PA02;      // pa as sw ss
    PORT->Group[0].OUTSET.reg = PORT_PA02;      // SS idle high
    
	
	pinMux(PINMUX_PA24C_SERCOM1_PAD2); // SAMD09 TX
	pinMux(PINMUX_PA25C_SERCOM1_PAD3); // SAMD09 RX
	UART_sercom_init(SERCOM1);
	
	UART_sercom_simpleWrite(SERCOM1, 'H');
	UART_sercom_simpleWrite(SERCOM1, 'e');
	UART_sercom_simpleWrite(SERCOM1, 'l');
	UART_sercom_simpleWrite(SERCOM1, 'l');
	UART_sercom_simpleWrite(SERCOM1, 'o');
	UART_sercom_simpleWrite(SERCOM1, '!');
	UART_sercom_simpleWrite(SERCOM1, 0xa);
		
	buffer=SPI_RW_wait(0xAA);
	SPI_RW_wait(buffer);
	
	SPI_write_wait('!');
	SPI_write_wait('!');
    SPI_flush_RXBuffer();
    
    flashID=flash_read_id();
    buffer=flashID>>16;         // buffer= Manufacturer ID [23:16]
    flashID&=0xFFFF;            // mask device ID
    
    UART_sercom_simpleWrite(SERCOM1, 'M');
    UART_sercom_simpleWrite(SERCOM1, 'I');
    UART_sercom_simpleWrite(SERCOM1, 'D');
    UART_sercom_simpleWrite(SERCOM1, ':');
    printHEX(buffer);
    UART_sercom_simpleWrite(SERCOM1, ' ');
    UART_sercom_simpleWrite(SERCOM1, 'D');
    UART_sercom_simpleWrite(SERCOM1, 'I');
    UART_sercom_simpleWrite(SERCOM1, 'D');
    UART_sercom_simpleWrite(SERCOM1, ':');
    printHEX(flashID>>8);
    printHEX(flashID);
    UART_sercom_simpleWrite(SERCOM1, 0xa);
    
    flash_read_status();
    flash_write_status(0);
    flash_read_status();
    buffer=1;                       // for some delay I think no delay needed. But this makes the reading in the logic analyzer easier
    while(buffer++);
    buffer=1;
    while(buffer++);
    
    flash_chip_erase();
    buffer=1;
    while(buffer++);
    buffer=1;
    while(buffer++);
    
    flash_page_program(0xf000, myPage);     // write myPage to page starting at adress 0xF000
    buffer=1;
    while(buffer++);
    buffer=1;
    while(buffer++);
    
    flash_drity_read(0xf00f, flashReadBuffer, 255);
	
    while (1) 
    {
        flashID++;  //flashID just reused
// 		if (UART_sercom_getByte(SERCOM1, &buffer))	// if uart received
// 		{
// 			buffer2=SPI_RW_wait(buffer);
// 			UART_sercom_simpleWrite(SERCOM1, buffer2);
// 		}
        if (flashID%5000==0)
            PORT->Group[0].OUTTGL.reg=PORT_PA02;    // blink !SS line - on baord led if using my samDEV_09 board
    }
}

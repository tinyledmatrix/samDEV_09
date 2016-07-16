/*
 * simpleSPI.c
 *
 * see simpleSPI.h for some documentation
 *
 * Created: 15.07.2016
 * Author: al1
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
#include "simpleSPI.h"

void SPI_setup(uint8_t mode)
{
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;    // APBCMASK
	GCLK->CLKCTRL.reg |= ((uint16_t)(   GCLK_CLKCTRL_GEN_GCLK0_Val|
	GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE)|
	GCLK_CLKCTRL_CLKEN ));
	
	SERCOM0->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_SWRST;
	while(SERCOM0->SPI.SYNCBUSY.bit.SWRST);

	SERCOM0->SPI.CTRLA.reg = ( ((mode&3) <<SERCOM_SPI_CTRLA_CPHA_Pos)|
	SERCOM_SPI_CTRLA_DOPO(0)|
	SERCOM_SPI_CTRLA_DIPO(2)|
	SERCOM_SPI_CTRLA_MODE_SPI_MASTER|
	SERCOM_SPI_CTRLA_RUNSTDBY);
	
	SERCOM0->SPI.CTRLB.bit.RXEN = 1;
	SERCOM0->SPI.BAUD.reg = ((uint16_t) SERCOM_SPI_BAUD_BAUD(SPI_BAUD_VALUE));
	
	SERCOM0->SPI.CTRLA.bit.ENABLE = 1;
	while(SERCOM0->SPI.SYNCBUSY.bit.ENABLE);
}

void SPI_write_wait(uint8_t data)
{
	while(SERCOM0->SPI.INTFLAG.bit.DRE==0);			// wait for data register to get empty
    SERCOM0->SPI.DATA.reg = data;
    // there will be something new in the RX buffer now. 
}

uint8_t SPI_read_RXBuffer(uint8_t *rxData)
{
    if (SERCOM0->SPI.INTFLAG.bit.RXC)
    {
        *rxData = SERCOM0->SPI.DATA.reg;
        return 0;
    }
    return 1;       
}

void SPI_flush_RXBuffer(void)
{
    uint8_t temp;
    while(!SPI_read_RXBuffer(&temp));
}

uint8_t SPI_RW_wait(uint8_t inData)
{
	while(SERCOM0->SPI.INTFLAG.bit.DRE==0);			// wait for data register to get empty
	SERCOM0->SPI.DATA.reg = inData;
	while(SERCOM0->SPI.INTFLAG.bit.RXC==0);			// wait on receive to get ready
	return (uint8_t) SERCOM0->SPI.DATA.bit.DATA;
}

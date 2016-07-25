/*
 * simpleSPI.h
 *
 * Created: 15.07.2016
 * Author:  al1
 *
 * simple little library to use SPI on Atmels SAMD09 Cortex M0+ microcontroller. SERCOM0 
 * is used in SPI-Mode. 
 *
 * Padmapping: SERCOM0_PAD0: MOSI
 *             SERCOM0_PAD1: SCLK
 *             SERCOM0_PAD2: MISO
 *
 * licensed under BSD 3-Clause License see below
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


#ifndef SIMPLESPI_H_
#define SIMPLESPI_H_

	#include "sam.h"
    
    /*  f_SPI_CLK = f_SERCOM / ( 2* (SPI_BAUD_VALUE +1) )
        F_SPI_CLK must be smaller than or equal to 12MHz [SAMD09 datasheet p. 467]
    */
    #define SPI_BAUD_VALUE 2     // change if needed f_SPI_CLK = F_SERCOM/(2* (BAUD+1) )
                                 // =0: with f_SERCOM = 1MHz -> f_SPI_CLK = 500kHz
								 // =2: with f_SERCOM =48MHz -> f_SPI_CLK = 8MHz

	/* setup function for this little SPI library. 
       IN: uint8_t mode: SPI moder [0-3] 
    */
    void SPI_setup(uint8_t mode);

    /* SPI read and write function. inData Byte will be sent. The read data will be 
       returned
       IN:      uint8_t inData: data to be sent [0-0xFF]
       RETURN   uint8_t:        read data Byte [0-0xFF]
    */   
	uint8_t SPI_RW_wait(uint8_t inData);
    
    /* write only function. The rx-Buffer will not be read or deleted. So some data 
       could stay there
       IN: uint8_t data: data to be sent [0-0xFF]
    */
	void SPI_write_wait(uint8_t data);
    
    /* read date from rx-Buffer. no TX-action will be triggered. If the RX-buffer is empty
       !0 will get returned
       IN:      uint8_t *rxData: pointer to store rx data
       RETURN : uint8_t          0: there was something to read in rx-Buffer
                                 1: the rx-buffer was empty
    */
    uint8_t SPI_read_RXBuffer(uint8_t *rxData);
    
    /*flush the rx-buffer*/
    void SPI_flush_RXBuffer(void);
	
#endif /* SIMPLESPI_H_ */
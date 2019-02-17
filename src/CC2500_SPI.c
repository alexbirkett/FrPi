
/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Multiprotocol is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Multiprotocol.  If not, see <http://www.gnu.org/licenses/>.
 */
//-------------------------------
//-------------------------------
//CC2500 SPI routines
//-------------------------------
//-------------------------------
#include <stdio.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "CC2500_SPI.h"

#include "iface_cc2500.h"
#include "Multiprotocol.h"
#include "CC2500_SPI.h"

#define SCK_PIN  13
#define MISO_PIN 12
#define MOSI_PIN 11
#define SS_PIN   10
#define GDO2      3                     //2 main, 5 remote, 3 M16
#define GDO0     99

extern uint8_t prev_power; // unused power value
extern uint8_t protocol_flags, protocol_flags2;

//----------------------------
void CC2500_WriteReg(uint8_t address, uint8_t data) {

    //printf("spi_write_reg(0x%02X,0x%02X);\r\n", address, data);
    uint8_t tbuf[2] = {0};
    tbuf[0] = address | CC2500_WRITE_SINGLE;
    tbuf[1] = data;
    uint8_t len = 2;
    wiringPiSPIDataRW(0, tbuf, len);
    return;
}

//----------------------
void CC2500_ReadRegisterMulti(uint8_t address, uint8_t data[], uint8_t length) {
/*	//CC25_CSN_off;;
	SPI_Write(CC2500_READ_BURST | address);
	for(uint8_t i = 0; i < length; i++)
		data[i] = SPI_Read();
	//CC25_CSN_on;*/

    uint8_t rbuf[length + 1];
    rbuf[0] = address | CC2500_READ_BURST;
    wiringPiSPIDataRW (0, rbuf, length + 1) ;
    for (uint8_t i=0; i<length ;i++ )
    {
        data[i] = rbuf[i+1];
        //printf("SPI_arr_read: 0x%02X\n", pArr[i]);
    }
}

//--------------------------------------------
uint8_t CC2500_ReadReg(uint8_t address) {
    uint8_t value;
    uint8_t rbuf[2] = {0};
    rbuf[0] = address | CC2500_READ_SINGLE;
//  printf("rbuf[0] before: 0x%02X\n", rbuf[0]);
    //printf("rbuf[1] before: 0x%02X\n", rbuf[1]);
    uint8_t len = 2;
    wiringPiSPIDataRW(0, rbuf, len);

//  printf("rbuf[0] after: 0x%02X\n", rbuf[0]);
//  printf("rbuf[1] after: 0x%02X\n", rbuf[1]);

    value = rbuf[1];
    return value;
}

//------------------------
void CC2500_ReadData(uint8_t *dpbuffer, uint8_t len) {
    CC2500_ReadRegisterMulti(CC2500_3F_RXFIFO, dpbuffer, len);
}

//*********************************************
void CC2500_Strobe(uint8_t state) {
    uint8_t tbuf[1] = {0};
    tbuf[0] = state;
   // printf("SPI_data: 0x%02X\n", tbuf[0]);
    wiringPiSPIDataRW(0, tbuf, 1);
}

void CC2500_WriteRegisterMulti(uint8_t address, const uint8_t data[], uint8_t length) {
    uint8_t tbuf[length + 1];
    tbuf[0] = address | CC2500_WRITE_BURST;

    // printf("address: 0x%02X\r\n", tbuf[0]);

    for (uint8_t i = 0; i < length; i++) {
        tbuf[i + 1] = data[i];
        //printf("SPI_arr_write: 0x%02X\n", tbuf[i+1]);
    }
    wiringPiSPIDataRW(0, tbuf, length + 1);
}

void CC2500_WriteData(uint8_t *dpbuffer, uint8_t len) {
   // CC2500_Strobe(CC2500_SIDLE);
    CC2500_Strobe(CC2500_SFTX);
    CC2500_WriteRegisterMulti(CC2500_3F_TXFIFO, dpbuffer, len);
    CC2500_Strobe(CC2500_STX);
}

void CC2500_SetTxRxMode(uint8_t mode) {
    if (mode == TX_EN) {//from deviation firmware
        CC2500_WriteReg(CC2500_00_IOCFG2, 0x2F);
        CC2500_WriteReg(CC2500_02_IOCFG0, 0x2F | 0x40);
    } else if (mode == RX_EN) {
        CC2500_WriteReg(CC2500_02_IOCFG0, 0x2F);
        CC2500_WriteReg(CC2500_00_IOCFG2, 0x2F | 0x40);
    } else {
        CC2500_WriteReg(CC2500_02_IOCFG0, 0x2F);
        CC2500_WriteReg(CC2500_00_IOCFG2, 0x2F);
    }
}

//------------------------
/* void cc2500_resetChip(void)
{
	// Toggle chip select signal
	//CC25_CSN_on;
	delayMicroseconds(30);
	//CC25_CSN_off;;
	delayMicroseconds(30);
	//CC25_CSN_on;
	delayMicroseconds(45);
	CC2500_Strobe(CC2500_SRES);
	_delay_ms(100);
}
*/
uint8_t CC2500_Reset() {
    digitalWrite(SS_PIN, LOW);
    delayMicroseconds(10);
    digitalWrite(SS_PIN, HIGH);
    delayMicroseconds(40);
    CC2500_Strobe(CC2500_SRES);
    delay(1);
    return CC2500_ReadReg(CC2500_0E_FREQ1) == 0xC4;//check if reset
}

/*
 void CC2500_SetPower_Value(uint8_t power)
{
	const unsigned char patable[8]=	{
		0xC5,  // -12dbm
		0x97, // -10dbm
		0x6E, // -8dbm
		0x7F, // -6dbm
		0xA9, // -4dbm
		0xBB, // -2dbm
		0xFE, // 0dbm
		0xFF // 1.5dbm
	};
	if (power > 7)
		power = 7;
	CC2500_WriteReg(CC2500_3E_PATABLE,  patable[power]);
}
*/


void CC2500_SetPower() {
    uint8_t power = CC2500_BIND_POWER;
    if (IS_BIND_DONE)
#ifdef CC2500_ENABLE_LOW_POWER
        power=IS_POWER_FLAG_on?CC2500_HIGH_POWER:CC2500_LOW_POWER;
#else
        power = CC2500_HIGH_POWER;
#endif
    if (IS_RANGE_FLAG_on)
        power = CC2500_RANGE_POWER;
    if (prev_power != power) {
        CC2500_WriteReg(CC2500_3E_PATABLE, power);
        prev_power = power;
    }
}

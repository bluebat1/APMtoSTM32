/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
   Flymaple port by Mike McCauley
   Partly based on EEPROM.*, flash_stm* copied from AeroQuad_v3.2
   This uses n*2*2k pages of FLASH ROM to emulate an EEPROM
   This storage is retained after power down, and survives reloading of firmware
   All multi-byte accesses are reduced to single byte access so that can span EEPROM block boundaries
 */
#include <AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_STM32F4

#include <string.h>
#include "Storage.h"
#include "utility/EEPROM.h"

using namespace AP_HAL_STM32F4_NS;

extern const AP_HAL::HAL& hal;

static EEPROMClass eeprom;

STM32F4Storage::STM32F4Storage()
{}

void STM32F4Storage::init(void*)
{
		uint16_t result = eeprom.init();
		if (result != EEPROM_OK)
				hal.console->printf("STM32F4Storage::init eeprom.init failed %x\n", result);
}

uint8_t STM32F4Storage::read_byte(uint16_t loc){
		//hal.console->printf("read_byte %d\n", loc);
		uint16_t eeprom_offset = loc & 0xfff;

		// 'bytes' are packed 2 per word
		// Read existing dataword and change upper or lower byte
		uint16_t data = eeprom.read(eeprom_offset >> 1);
		if (eeprom_offset & 1)
				return data >> 8; // Odd, upper byte
		else
				return data & 0xff; // Even lower byte
}

uint16_t STM32F4Storage::read_word(uint16_t loc){
		//hal.console->printf("read_word %d\n", loc);
		uint16_t value;
		read_block(&value, loc, sizeof(value));
		return value;
}

uint32_t STM32F4Storage::read_dword(uint16_t loc){
		//hal.console->printf("read_dword %d\n", loc);
		uint32_t value;
		read_block(&value, loc, sizeof(value));
		return value;
}

void STM32F4Storage::read_block(void* dst, uint16_t src, size_t n) {
		//    hal.console->printf("read_block %d %d\n", src, n);
		// Treat as a block of bytes
		for (size_t i = 0; i < n; i++)
				((uint8_t*)dst)[i] = read_byte(src+i);
}

void STM32F4Storage::write_byte(uint16_t loc, uint8_t value)
{
		//    hal.console->printf("write_byte %d, %d\n", loc, value);
		uint16_t eeprom_offset = loc & 0xfff;

		// 'bytes' are packed 2 per word
		// Read existing data word and change upper or lower byte
		uint16_t data = eeprom.read(eeprom_offset >> 1);
		if (eeprom_offset & 1)
				data = (data & 0x00ff) | (value << 8); // Odd, upper byte
		else
				data = (data & 0xff00) | value; // Even, lower byte
		eeprom.write(eeprom_offset >> 1, data);
}

void STM32F4Storage::write_word(uint16_t loc, uint16_t value)
{
		//    hal.console->printf("write_word %d, %d\n", loc, value);
		write_block(loc, &value, sizeof(value));
}

void STM32F4Storage::write_dword(uint16_t loc, uint32_t value)
{
		//    hal.console->printf("write_dword %d, %d\n", loc, value);
		write_block(loc, &value, sizeof(value));
}

void STM32F4Storage::write_block(uint16_t loc, const void* src, size_t n)
{
		//    hal.console->printf("write_block %d, %d\n", loc, n);
		// Treat as a block of bytes
		for (size_t i = 0; i < n; i++)
				write_byte(loc+i, ((uint8_t*)src)[i]);
}

#endif

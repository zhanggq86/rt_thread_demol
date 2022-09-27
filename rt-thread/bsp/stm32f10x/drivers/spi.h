#ifndef __SPI_H__
#define __SPI_H__

#include <stm32f10x.h>

u8 spi_read_write_byte(SPI_TypeDef* spix, u8 txdata);

#endif


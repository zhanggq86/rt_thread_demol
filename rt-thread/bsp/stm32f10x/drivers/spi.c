/*
* File     :spi_flash.h
* This file si part of RT-Thread RTOS
* Change Logs:
* Date          Author          Notes
* 2022/3/13      zgq             the first version
*/
#include "spi.h"
#include "spi_flash.h"

u8 spi_read_write_byte(SPI_TypeDef* spix, u8 txdata)
{
    while(SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_TXE) == RESET){}
        SPI_I2S_SendData(spix, txdata);
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET){}
        return SPI_I2S_ReceiveData(spix);
}

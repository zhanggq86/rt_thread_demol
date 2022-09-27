/*
* File     :spi_flash.h
* This file si part of RT-Thread RTOS
* Change Logs:
* Date          Author          Notes
* 2022/3/5      zgq             the first version
*/

#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include <rtthread.h>

#define SPI1_PIN_CLK            RCC_APB2Periph_GPIOA
#define SPI1_SEL_CLK            RCC_APB2Periph_SPI1
#define SPI2_PIN_CLK            RCC_APB2Periph_GPIOB
#define SPI2_SEL_CLK            RCC_APB1Periph_SPI2
#define SPI3_PIN_CLK            RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
#define SPI3_SEL_CLK            RCC_APB1Periph_SPI3

/*SPI时钟初始化*/
#define SPI_CLK_CONFIG(xxx) {   \
    if(xxx == SPI1) {   \
    RCC_APB2PeriphClockCmd(xxx##_PIN_CLK|xxx##_SEL_CLK, ENABLE);}   \
    else RCC_APB1PeriphClockCmd(xxx##_PIN_CLK|xxx##_SEL_CLK, ENABLE);   \
}
/*SPI时钟初始化*/

/*SPI引脚初始化*/
#define SPI_PIN_CONFIG(xxx) {   \
    GPIO_InitStructure.GPIO_Pin = xxx##_SCK_PIN|xxx##_MISO_PIN|xxx##_MOSI_PIN; \
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; \
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   \
    GPIO_Init(xxx##_PORT, &GPIO_InitStructure);  \
    \
    GPIO_SetBits(xxx##_PORT,xxx##_SCK_PIN|xxx##_MISO_PIN|xxx##_MOSI_PIN);}
/*SPI引脚初始化*/

/*SPI模式初始化*/
#define SPI_MODE_CONFIG(xxx) {  \
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  \
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;   \
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   \
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; \
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    \
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   \
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;    \
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  \
    SPI_InitStructure.SPI_CRCPolynomial = 7;    \
    SPI_Init(xxx, &SPI_InitStructure);  \
    SPI_Cmd(xxx, ENABLE);}
/*SPI模式初始化*/
    
struct spi_flash_device
{
    struct rt_device                flash_device;
    struct rt_device_blk_geometry   geometry;
    struct rt_spi_device *          rt_spi_device;
    struct rt_mutex                 lock;
    void *                          user_data;
};

typedef struct spi_flash_device *rt_spi_flash_device_t;

#endif

/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include <rtthread.h>
#include "stm32f10x.h"   //这里应该消失，集中在spi中
#include "spi.h"
#include "spi_flash.h"

#define SPI1_SCK_PIN        GPIO_Pin_5
#define SPI1_MISO_PIN       GPIO_Pin_6
#define SPI1_MOSI_PIN       GPIO_Pin_7
#define SPI1_PORT           GPIOA

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

typedef struct _SPI_USER_DATA{
    SPI_TypeDef* spix;
    GPIO_TypeDef* cs_gpiox;
    uint16_t cs_gpio_pin;
}spi_user_data, *spi_user_data_t;

/**
* 开启时钟

static void rcc_congfig(const sfud_spi* spi)
{
    spi_user_data_t spi_dev = (spi_user_data_t)spi->user_data;
    
    if(spi_dev->spix == SPI1)
    {
        RCC_APB2PeriphClockCmd(SPI1_PIN_CLK|SPI1_SEL_CLK, ENABLE);
    }
}
*/

/**
* 引脚设置

static void gpio_config(const sfud_spi* spi)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    spi_user_data_t spi_dev = (spi_user_data_t)spi->user_data;
    if(spi_dev->spix == SPI1)
    {
        GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN|SPI1_MISO_PIN|SPI1_MOSI_PIN;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI1_PORT, &GPIO_InitStructure);
        
        GPIO_SetBits(SPI1_PORT,SPI1_SCK_PIN|SPI1_MISO_PIN|SPI1_MOSI_PIN);
    }
}
*/

/**
* SPI设置

static void spi_congfig(const sfud_spi* spi)
{
    SPI_InitTypeDef  SPI_InitStructure;
    
    spi_user_data_t spi_dev = (spi_user_data_t)spi->user_data;
    
    if(spi_dev->spix == SPI1)
    {
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial = 7;
        SPI_Init(SPI1, &SPI_InitStructure);
        SPI_Cmd(SPI1, ENABLE);
    }
}
*/

/**
* 锁
*/
static void spi_lock(const sfud_spi* spi)
{
    sfud_flash *sfud_dev = (sfud_flash*) (spi->user_data);
    struct spi_flash_device *rtt_dev = (struct spi_flash_device*)(sfud_dev->user_data);
    
    rt_mutex_take(&(rtt_dev->lock), RT_WAITING_FOREVER);
}

/**
* 开锁
*/
static void spi_unlock(const sfud_spi* spi)
{
    sfud_flash *sfud_dev = (sfud_flash*) (spi->user_data);
    struct spi_flash_device *rtt_dev = (struct spi_flash_device*)(sfud_dev->user_data);
    
    rt_mutex_release(&(rtt_dev->lock));
}

/**
* SPI读写接口
*/
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    //uint8_t send_data, read_data;

    spi_user_data_t spi_dev = (spi_user_data_t)spi->user_data;
    GPIO_ResetBits(spi_dev->cs_gpiox,spi_dev->cs_gpio_pin);
    
    if(write_buf != RT_Null)
    {
        if(write_size)
        {
            for(u8 i=0; i<write_size; i++)
            spi_read_write_byte(spi_dev->spix, *(write_buf+i));
        }
        else goto error;
    }
    else goto error;
    
    if(read_buf != RT_Null)
    {
        if(read_size)
        {
            for(u8 i=0; i<read_size; i++)
            spi_read_write_byte(spi_dev->spix, *(read_buf+i));
        }
    else goto error;
    }
    else goto error;

    error:
    __NOP;
    //rt_kprintf("the errcode is:%d,occur to spi read or write!");
    return result;
}
        

/**
* 读写延时
*/
static void retry_delay_100ms(void)
{
    /* 100 microsecond delay */
    rt_thread_delay(RT_TICK_PER_SECOND*10);
}


#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

static void spi_device_init(sfud_flash *flash)
{
    flash->spi.wr = spi_write_read;
    //flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
    flash->spi.lock = spi_lock;
    flash->spi.unlock = spi_unlock;
    //flash->spi.user_data = &spix;
    flash->retry.delay = retry_delay_100ms;
    flash->retry.times = 10000; //Required
}


/**
* sfud SPI初始化
*/
sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    
    SPI_CLK_CONFIG(SPI1);
    SPI_PIN_CONFIG(SPI1);
    SPI_MODE_CONFIG(SPI1);
    spi_read_write_byte(SPI1, 0xff);
    
    spi_device_init(flash);

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

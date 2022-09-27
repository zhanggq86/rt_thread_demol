/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

#include "led.h"
#include "sfud.h"
ALIGN(RT_ALIGN_SIZE)

typedef struct _can_module
{
 uint16_t sn;
    char *name;
}EC_PACK_ALIGN(2) can_module;

struct _node
{
    char *name;
    void *data;
}EC_PACK_ALIGN(2) node;


/**
* test
*/
static const uint8_t num[10] = {1,2,3,1,2,3,1,2,3,0};
char *num_A = "hello world";
char *num_B = "hello world";

/**
* 全局变量定义
*/
const sfud_flash* sfud_flash_dev = RT_Null;
static uint8_t sfud_flash_buf[512] = {0};
#define flash_read_test_addr    0x64
/**
* 线程资源
*/
static rt_uint8_t led_stack[216];
static struct rt_thread led_thread;
static rt_uint8_t flash_stack[512];
static struct rt_thread flash_thread;

/**
* 线程应用
*/
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;

    rt_hw_led_init();
        /**
    * test
    */
    rt_kprintf("the num[10] addr is : ");
    for(uint8_t i = 0; i < 10; i++)
    {
        rt_kprintf("%d ",&num[i]);
    }
    rt_kprintf("the num[10] addr is over.");
    rt_kprintf("the num_A and num_B addrs is : %d %d",(void *)num_A,(void *)num_B);

    while (1)
    {
        /* led1 on */
#ifndef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        rt_hw_led_on(0);
        rt_hw_led_off(1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        rt_kprintf("led off\r\n");
#endif
        rt_hw_led_off(0);
        rt_hw_led_on(1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

static void flash_thread_entry(void* parameter)
{
    sfud_err sfud_init_result = SFUD_SUCCESS;
    sfud_err sfud_contrl_result = SFUD_SUCCESS;
    
    sfud_init_result = sfud_init();
    sfud_flash_dev = sfud_get_device_table();
    if(sfud_init_result != SFUD_SUCCESS)
    {
        rt_kprintf("the sfud_flash_[%s] init failed! \nthe errcode is %d",
            sfud_flash_dev->name, sfud_init_result);
    }
    else
    {
        rt_kprintf("the sfud_flash_[%s] init success! \r\nthe errcode is %d",
            sfud_flash_dev->name, sfud_init_result);
    }

    while(1)
    {
        rt_thread_delay(500);
        sfud_contrl_result = sfud_read(sfud_flash_dev, flash_read_test_addr, 10, sfud_flash_buf);

        if(sfud_contrl_result != SFUD_SUCCESS)  rt_kprintf("the flash read is failed!");
        else
        {
        rt_kprintf("the flash data is:");
        for(uint8_t i; i < 10; i++)
        rt_kprintf("%d");
        rt_kprintf("\r\n");
        }
    }
}

#ifdef RT_USING_RTGUI
rt_bool_t cali_setup(void)
{
    rt_kprintf("cali setup entered\n");
    return RT_FALSE;
}

void cali_store(struct calibration_data *data)
{
    rt_kprintf("cali finished (%d, %d), (%d, %d)\n",
               data->min_x,
               data->max_x,
               data->min_y,
               data->max_y);
}
#endif /* RT_USING_RTGUI */

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
        rt_kprintf("File System initialzation failed!\n");
#endif  /* RT_USING_DFS */

#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_lcd_init();
        extern void rtgui_touch_hw_init(void);

        rt_device_t lcd;

        /* init lcd */
        rt_hw_lcd_init();

        /* init touch panel */
        rtgui_touch_hw_init();

        /* find lcd device */
        lcd = rt_device_find("lcd");

        /* set lcd device as rtgui graphic driver */
        rtgui_graphic_set_device(lcd);

#ifndef RT_USING_COMPONENTS_INIT
        /* init rtgui system server */
        rtgui_system_server_init();
#endif

        calibration_set_restore(cali_setup);
        calibration_set_after(cali_store);
        calibration_init();
    }
#endif /* #ifdef RT_USING_RTGUI */
}

/**
* 应用APP入口，线程创建
*/
int rt_application_init(void)
{
    rt_thread_t init_thread;

    rt_err_t result;

    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }

    /* init flash thread */
    result = rt_thread_init(&flash_thread,
                            "flash",
                            flash_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&flash_stack[0],
                            sizeof(flash_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&flash_thread);
    }
                            
    
#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

    return 0;
}


void pre_proc_1(void) __attribute__((constructor(101)));
void end_proc_1(void) __attribute__((destructor(101)));

void pre_proc_1(void)
{
            rt_kprintf("pre_proc_1\n");
}

void end_proc_1(void)
{
            rt_kprintf("end_proc_1\n");
}

//__ASM .global __text 
/*@}*/

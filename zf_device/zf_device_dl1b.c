/*********************************************************************************************************************
* RA8P1KFLCAC Opensourec Library 即（RA8P1KFLCAC 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
* 
* 本文件是 RA8P1KFLCAC 开源库的一部分
* 
* RA8P1KFLCAC 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
* 
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
* 
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
* 
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
* 
* 文件名称          zf_device_dl1b
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          Renesas e² studio Version: 2025-12 (25.12.0)
* 适用平台          RA8P1KFLCAC
* 店铺链接          https://seekfree.taobao.com/
* 
* 修改记录
* 日期              作者                备注
* 2026-05-19        SeekFree            first version
********************************************************************************************************************/

#include "zf_driver_soft_iic.h"
#include "zf_device_config.h"

#include "zf_device_dl1b.h"

volatile uint8 dl1b_init_flag = 0;
volatile uint8 dl1b_finsh_flag = 0;
volatile uint16 dl1b_distance_mm = 8192;

static soft_iic_info_struct dl1b_iic_struct;
#define dl1b_transfer_8bit_array(tdata, tlen, rdata, rlen) (soft_iic_transfer_8bit_array(&dl1b_iic_struct, (tdata), (tlen), (rdata), (rlen)))
#define dl1b_gpio_set_level(pin, dat) g_ioport.p_api->pinWrite(&g_ioport_ctrl, pin, dat)
#define dl1b_delay_ms(x) R_BSP_SoftwareDelay(x, BSP_DELAY_UNITS_MILLISECONDS)

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     返回以毫米为单位的范围读数
// 参数说明     void
// 返回参数     void
// 使用示例     dl1b_get_distance();
// 备注信息     在开始单次射程测量后也调用此函数
//-------------------------------------------------------------------------------------------------------------------
void dl1b_get_distance (void)
{
    if(dl1b_init_flag)
    {
        uint8 data_buffer[3];
        int16 dl1b_distance_temp = 0;

        data_buffer[0] = DL1B_GPIO__TIO_HV_STATUS >> 8;
        data_buffer[1] = DL1B_GPIO__TIO_HV_STATUS & 0xFF;
        dl1b_transfer_8bit_array(data_buffer, 2, &data_buffer[2], 1);

        if(data_buffer[2])
        {

            data_buffer[0] = DL1B_SYSTEM__INTERRUPT_CLEAR >> 8;
            data_buffer[1] = DL1B_SYSTEM__INTERRUPT_CLEAR & 0xFF;
            data_buffer[2] = 0x01;
            dl1b_transfer_8bit_array(data_buffer, 3, data_buffer, 0);// clear Interrupt

            data_buffer[0] = DL1B_RESULT__RANGE_STATUS >> 8;
            data_buffer[1] = DL1B_RESULT__RANGE_STATUS & 0xFF;
            dl1b_transfer_8bit_array(data_buffer, 2, &data_buffer[2], 1);

            if(0x89 == data_buffer[2])
            {
                data_buffer[0] = DL1B_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0 >> 8;
                data_buffer[1] = DL1B_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0 & 0xFF;
                dl1b_transfer_8bit_array(data_buffer, 2, data_buffer, 2);
                dl1b_distance_temp = data_buffer[0];
                dl1b_distance_temp = (dl1b_distance_temp << 8) | data_buffer[1];

                if(dl1b_distance_temp > 4000 || dl1b_distance_temp < 0)
                {
                    dl1b_distance_mm = 8192;
                    dl1b_finsh_flag = 0;
                }
                else
                {
                    dl1b_distance_mm = (uint16)dl1b_distance_temp;
                    dl1b_finsh_flag = 1;
                }
            }
            else
            {
                dl1b_distance_mm = 8192;
                dl1b_finsh_flag = 0;
            }
        }
        else
        {
            dl1b_distance_mm = 8192;
            dl1b_finsh_flag = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     DL1B INT 中断响应处理函数
// 参数说明     void
// 返回参数     void
// 使用示例     dl1b_int_handler();
// 备注信息     本函数需要在 DL1B_INT_PIN 对应的外部中断处理函数中调用
//-------------------------------------------------------------------------------------------------------------------
void dl1b_int_handler (external_irq_callback_args_t * arg)
{
#if DL1B_INT_ENABLE
    if(DL1B_INT_CHANNEL == arg->channel)
    {
        dl1b_get_distance();
    }
#endif
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     初始化 DL1B
// 参数说明     void
// 返回参数     uint8           1-初始化失败 0-初始化成功
// 使用示例     dl1b_init();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 dl1b_init (void)
{
    uint8   return_state    = 0;
    uint8   data_buffer[2 + sizeof(dl1b_config_file)];
    uint16  time_out_count  = 0;

    soft_iic_init(&dl1b_iic_struct, DL1B_DEV_ADDR, DL1B_SOFT_IIC_DELAY, DL1B_SCL_PIN, DL1B_SDA_PIN);

    do
    {
        dl1b_delay_ms(50);
        dl1b_gpio_set_level(DL1B_XS_PIN, 0);
        dl1b_delay_ms(10);
        dl1b_gpio_set_level(DL1B_XS_PIN, 1);
        dl1b_delay_ms(50);

        data_buffer[0] = DL1B_FIRMWARE__SYSTEM_STATUS >> 8;
        data_buffer[1] = DL1B_FIRMWARE__SYSTEM_STATUS & 0xFF;
        dl1b_transfer_8bit_array(data_buffer, 2, &data_buffer[2], 1);
        return_state = (0x01 == (data_buffer[2] & 0x01)) ? (0) : (1);
        if(1 == return_state)
        {
            break;
        }

        data_buffer[0] = DL1B_I2C_SLAVE__DEVICE_ADDRESS >> 8;
        data_buffer[1] = DL1B_I2C_SLAVE__DEVICE_ADDRESS & 0xFF;
        memcpy(&data_buffer[2], (uint8 *)dl1b_config_file, sizeof(dl1b_config_file));
        dl1b_transfer_8bit_array(data_buffer, 2 + sizeof(dl1b_config_file), data_buffer, 0);

        while(1)
        {
            data_buffer[0] = DL1B_GPIO__TIO_HV_STATUS >> 8;
            data_buffer[1] = DL1B_GPIO__TIO_HV_STATUS & 0xFF;
            dl1b_transfer_8bit_array(data_buffer, 2, &data_buffer[2], 1);
            if(0x00 == (data_buffer[2] & 0x01))
            {
                time_out_count = 0;
                break;
            }
            if(DL1B_TIMEOUT_COUNT < time_out_count ++)
            {
                return_state = 1;
                break;
            }
            dl1b_delay_ms(1);
        }

        dl1b_init_flag = 1;
    }while(0);

#if DL1B_INT_ENABLE
    g_external_irq26.p_api->open(g_external_irq26.p_ctrl, g_external_irq26.p_cfg);
    g_external_irq26.p_api->callbackSet(g_external_irq26.p_ctrl, dl1b_int_handler, NULL, NULL);
    g_external_irq26.p_api->enable(g_external_irq26.p_ctrl);
    dl1b_get_distance();
    dl1b_finsh_flag = 0;
#endif

    return return_state;
}

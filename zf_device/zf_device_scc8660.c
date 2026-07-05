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
* 文件名称          zf_device_scc8660
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

#include "zf_device_scc8660.h"

// Hardware Interface
#define SCC8660_SOFTIIC_SCL     BSP_IO_PORT_04_PIN_10
#define SCC8660_SOFTIIC_SDA     BSP_IO_PORT_04_PIN_09

#define SCC8660_DELAY_MS(x)     R_BSP_SoftwareDelay(x, BSP_DELAY_UNITS_MILLISECONDS)

vuint8 scc8660_finish_flag;
__attribute__((aligned(8))) uint16 scc8660_image[SCC8660_H][SCC8660_W];

// 需要配置到摄像头的数据 不允许在这修改参数
static int16 scc8660_set_confing_buffer[SCC8660_CONFIG_FINISH][2]=
{
    {SCC8660_INIT,              0},                                             // 摄像头开始初始化

    {SCC8660_AUTO_EXP,          SCC8660_AUTO_EXP_DEF},                          // 自动曝光
    {SCC8660_BRIGHT,            SCC8660_BRIGHT_DEF},                            // 亮度设置
    {SCC8660_FPS,               SCC8660_FPS_DEF},                               // 图像帧率
    {SCC8660_SET_COL,           SCC8660_W},                                     // 图像列数
    {SCC8660_SET_ROW,           SCC8660_H},                                     // 图像行数
    {SCC8660_PCLK_DIV,          SCC8660_PCLK_DIV_DEF},                          // PCLK分频系数
    {SCC8660_PCLK_MODE,         SCC8660_PCLK_MODE_DEF},                         // PCLK模式
    {SCC8660_COLOR_MODE,        SCC8660_COLOR_MODE_DEF},                        // 图像色彩模式
    {SCC8660_DATA_FORMAT,       SCC8660_DATA_FORMAT_DEF},                       // 输出数据格式
#if SCC8660_IS_WB_AUTO
    {SCC8660_WB_R,              0},                                             // 自动白平衡
    {SCC8660_WB_G,              0},                                             // 自动白平衡
    {SCC8660_WB_B,              0},                                             // 自动白平衡
#else   
    {SCC8660_WB_R,              SCC8660_MANUAL_WB_R},                           // 自动白平衡
    {SCC8660_WB_G,              SCC8660_MANUAL_WB_G},                           // 自动白平衡
    {SCC8660_WB_B,              SCC8660_MANUAL_WB_B},                           // 自动白平衡
#endif
};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     单独设置图像亮度
// 参数说明     data            需要设置的亮度值
// 返回参数     uint8           1-失败 0-成功
// 使用示例     scc8660_set_bright(data);                                       // 通过该函数设置的参数，不会被51单片机保存
// 备注信息     调用该函数前请先初始化摄像头配置串口
//-------------------------------------------------------------------------------------------------------------------
uint8 scc8660_set_brightness (uint16 data)
{
    return scc8660_set_brightness_sccb(data);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     单独设置白平衡
// 参数说明     data            需要设置的白平衡 可选参数为：[0,0x65-0xa0] 0：关闭手动白平衡，启用自动白平衡    其他：手动白平衡 手动白平衡时 参数范围0x65-0xa0
// 返回参数     uint8           1-失败 0-成功
// 使用示例     scc8660_set_white_balance(data);                // 调用该函数前请先初始化摄像头配置串口
// 备注信息     通过该函数设置的参数，不会被51单片机保存
//-------------------------------------------------------------------------------------------------------------------
uint8 scc8660_set_white_balance (uint16 wb_r, uint16 wb_g, uint16 wb_b)
{
    return scc8660_set_manual_wb_sccb(wb_r, wb_g, wb_b);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     对摄像头内部寄存器进行写操作
// 参数说明     addr            摄像头内部寄存器地址
// 参数说明     data            需要写入的数据
// 返回参数     uint8           1-失败 0-成功
// 使用示例     scc8660_set_reg(addr, data);                    // 调用该函数前请先初始化串口
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 scc8660_set_reg (uint8 addr, uint16 data)
{
    return scc8660_set_reg_sccb(addr, data);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      SCC8660摄像头采集完成中断函数
//  @param      NULL
//  @return     void					
//  @since      v1.0
//  Sample usage:	
//  @note       该函数由isr.c中的CSI_IRQHandler函数调用
//-------------------------------------------------------------------------------------------------------------------
void g_ceu0_user_callback (capture_callback_args_t * p_args)
{
    (void)p_args;
    scc8660_finish_flag = 1;
    g_ceu0.p_api->captureStart(g_ceu0.p_ctrl, (uint8_t *)scc8660_image);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     SCC8660 摄像头初始化
// 参数说明     void
// 返回参数     uint8           1-失败 0-成功
// 使用示例     scc8660_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 scc8660_init (void)
{
    uint8 return_state = 1;
    do
    {
        soft_iic_info_struct scc8660_iic_struct;
        soft_iic_init(&scc8660_iic_struct, 0, SCC8660_COF_IIC_DELAY, SCC8660_SOFTIIC_SCL, SCC8660_SOFTIIC_SDA);
        SCC8660_DELAY_MS(200);

        if(!scc8660_sccb_check_id(&scc8660_iic_struct))
        {
            return_state = scc8660_sccb_set_config(scc8660_set_confing_buffer);
            g_ceu0.p_api->open(g_ceu0.p_ctrl, g_ceu0.p_cfg);
            g_ceu0.p_api->captureStart(g_ceu0.p_ctrl, (uint8_t *)scc8660_image);
            return_state = 0;
            break;
        }
    }while(0);

    return return_state;
}

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
* 文件名称          zf_common_debug
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
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "zf_common_debug.h"

#include "hal_data.h"

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     debug 断言处理函数
// 参数说明     pass        判断是否触发断言
// 参数说明     *file       文件名
// 参数说明     line        目标行数
// 返回参数     void
// 使用示例     zf_assert(0);
// 备注信息     这个函数不是直接调用的 此部分不允许用户更改
//              使用 zf_commmon_debug.h 中的 zf_assert(x) 接口
//-------------------------------------------------------------------------------------------------------------------
void debug_assert_handler (uint8 pass, char *file, int line)
{
    do
    {
        if(pass)
        {
            break;
        }

        printf("Assert error in %s, line %d.\r\n", file, line);
        while(1);
    }while(0);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     debug 调试信息处理函数
// 参数说明     pass        判断是否触发断言
// 参数说明     *str        输出的信息
// 参数说明     *file       文件名
// 参数说明     line        目标行数
// 返回参数     void
// 使用示例     zf_log(0, "Log Message");
// 备注信息     这个函数不是直接调用的 此部分不允许用户更改
//              使用 zf_commmon_debug.h 中的 zf_log(x, str) 接口
//-------------------------------------------------------------------------------------------------------------------
void debug_log_handler (uint8 pass, char *str, char *file, int line)
{
    do
    {
        if(pass)
        {
            break;
        }

        printf("Log output in %s, line %d.\r\n", file, line);
        printf("Log : %s.\r\n", str);
    }while(0);
}

volatile bool debug_uart_tx_finish_flag = false;
volatile bool debug_uart_rx_finish_flag = false;
void debug_uart_callback (uart_callback_args_t * p_args)
{
    if(p_args->channel == 9)
    {
        switch(p_args->event)
        {
            case UART_EVENT_RX_COMPLETE: debug_uart_rx_finish_flag = true; break;
            case UART_EVENT_TX_COMPLETE: debug_uart_tx_finish_flag = true; break;
            default: break;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     debug 串口初始化
// 参数说明     void
// 返回参数     void
// 使用示例     debug_init();
// 备注信息     开源库示例默认调用 但默认禁用中断接收
//-------------------------------------------------------------------------------------------------------------------
void debug_init (void)
{
    g_uart9.p_api->open(g_uart9.p_ctrl, g_uart9.p_cfg);
    g_uart9.p_api->callbackSet(g_uart9.p_ctrl, debug_uart_callback, NULL, NULL);
}

// 重定向
__attribute__((weak)) int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;

    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;

    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;

    errno = EBADF;
    return -1;
}

__attribute__((weak)) int _fstat(int fd, struct stat *st)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    {
        st->st_mode = S_IFCHR;
        return 0;
    }

    errno = EBADF;
    return 0;
}

__attribute__((weak)) int _read(int fd, char *pBuffer, int size)
{
    (void)fd;
    debug_uart_rx_finish_flag = false;
    g_uart9.p_api->read(g_uart9.p_ctrl, pBuffer, 1);
    while(!debug_uart_rx_finish_flag);
    return 1;
}

__attribute__((weak)) int _write (int fd, char * pBuffer, int size)
{
    (void)fd;
    debug_uart_tx_finish_flag = false;
    g_uart9.p_api->write(g_uart9.p_ctrl, pBuffer, size);
    while(!debug_uart_tx_finish_flag);
    return size;
}

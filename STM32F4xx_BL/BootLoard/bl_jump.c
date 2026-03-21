#include "bl_jump.h"

void BL_JumpApplication(void)
{
    /* App 的初始栈指针 MSP */
    uint32_t app_start_addr         = *(volatile uint32_t*)APP_ADDR;
    /* App 的 Reset_Handler 地址（向量表第二个字节） */
    uint32_t app_reset_handler_addr = *(volatile uint32_t*)(APP_ADDR + 4U);
    /* App 的 Reset_Handler 函数指针 */
    void (*app_reset_handler)(void) = (void (*)(void))app_reset_handler_addr;

/* ----------------------- BootLoard 环境清理 ----------------------------------*/

    /* 关闭所有中断 */
    __disable_irq();
    /* 关闭 SysTick定时器 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* 关闭所有 NVIC 中断使能 */
    for(int i=0; i<8; i++)
        NVIC->ICER[i] = 0xFFFFFFFF;

    /* 关闭所有 NVIC 中断挂起标志 */
    for(int i=0; i<8; i++)
        NVIC->ICPR[i] = 0xFFFFFFFF;

/* ----------------------- 切换中断向量表 ----------------------------------*/

    /* 设置向量表偏移地址为APP的向量表 */
    SCB->VTOR = APP_ADDR;

/* ----------------------- 切换堆栈 ----------------------------------*/

    /* 设置堆栈指针 MSP 为 App 的栈顶地址 */
    __set_MSP(app_start_addr);

/* ----------------------- 跳转到 App ----------------------------------*/
	__enable_irq();
    /* 执行 APP 的 Reset_Handler，相当于 MCU 重新启动 APP */
    app_reset_handler();
}


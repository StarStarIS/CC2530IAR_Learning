#include <ioCC2530.h>
#include <string.h>

unsigned char rxTemp = 0;

// 函数声明
void Delay_Ms(unsigned int xms);
void Uart0_Init(void);
void Uart0_Send_String(unsigned char *Data, int len);

/*************************************************
 *
 * 函数名称：void Delay_Ms(unsigned int xms)
 * 功能描述：延时函数
 * 参数说明：xms：延时的时间（以 MS 为单位）
 *
 **************************************************/
void Delay_Ms(unsigned int xms)
{
    unsigned int i, j;
    for (i = xms; i > 0; i--)
        for (j = 578; j > 0; j--);
}

/**************************************************
 * 串口初始化函数
 **************************************************/
void Uart0_Init(void)
{
    CLKCONCMD &= ~0x40;      // 设置系统时钟源为32MHZ晶振
    while (CLKCONSTA & 0x40); // 等待晶振稳定
    CLKCONCMD &= ~0x47;      // 设置系统主时钟频率为32MHZ

    PERCFG = 0x00;           // 位置1 P0口
    P0SEL = 0x0c;            // P0用作串口
    P2DIR &= ~0xC0;          // P0优先作为UART0
    U0CSR |= 0x80;           // 串口设置为UART方式

    U0GCR &= 0x1F;
    U0GCR |= 8;

    U0BAUD |= 59;            // 波特率设为9600
    UTX0IF = 1;              // UART0 TX中断标志初始置位1

    U0CSR |= 0x40;           // 允许接收
    IEN0 |= 0x84;            // 开总中断，接收中断
}


/**************************************************
 * 串口发送字符串函数
 **************************************************/
void Uart0_Send_String(unsigned char *Data, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        U0DBUF = *Data++;
        while (UTX0IF == 0);
        UTX0IF = 0;
    }
}


/**************************************************
 * 主函数
 **************************************************/
void main(void)
{
    Uart0_Init();
    while (1)
    {
        if (rxTemp != 0)
        {
            Uart0_Send_String(&rxTemp, 1);
            rxTemp = 0;
        }
    }
}


/**************************************************
 * 函 数 名 : Uart0_ISR
 * 功能描述 : 中断服务函数
 * 输入参数 : NONE
 * 输出参数 : NONE
 * 返 回 值 : NONE
 **************************************************/
#pragma vector = URX0_VECTOR
__interrupt void Uart0_ISR(void)
{
    URX0IF = 0;       // 清中断标志
    rxTemp = U0DBUF;  // 读取接收到的数据
}

#include <ioCC2530.h>

#define LED1   P1_4
#define LED2   P0_1
#define LED3   P1_0
#define LED4   P1_1
#define PIN1_7 P1_7

#define LED_ON  0
#define LED_OFF 1

static unsigned char uart_cmd_step = 0;
static unsigned char uart_led_index = 0;

void Uart0_Init(void);
void Led_Init(void);
void Led_Control(unsigned char ledIndex, unsigned char ledState);
void Uart0_Parse_Command(unsigned char rxData);

void main(void)
{
    Led_Init();
    Uart0_Init();

    while (1)
    {
    }
}

void Led_Init(void)
{
    P1SEL &= ~(0x10 | 0x01 | 0x02 | 0x80);
    P1DIR |= (0x10 | 0x01 | 0x02 | 0x80);

    P0SEL &= ~0x02;
    P0DIR |= 0x02;

    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;
    LED4 = LED_OFF;
    PIN1_7 = 0;
}

void Led_Control(unsigned char ledIndex, unsigned char ledState)
{
    unsigned char ledValue;

    ledValue = (ledState == 1) ? LED_ON : LED_OFF;

    switch (ledIndex)
    {
    case 1:
        LED1 = ledValue;
        break;
    case 2:
        LED2 = ledValue;
        break;
    case 3:
        LED3 = ledValue;
        break;
    case 4:
        LED4 = ledValue;
        break;
    default:
        break;
    }
}

void Uart0_Parse_Command(unsigned char rxData)
{
    /* Command format: 11->LED1 ON, 10->LED1 OFF ... 41->LED4 ON */
    if ((rxData == '\r') || (rxData == '\n') || (rxData == ' '))
    {
        return;
    }

    if (uart_cmd_step == 0)
    {
        if ((rxData >= '1') && (rxData <= '4'))
        {
            uart_led_index = rxData - '0';
            uart_cmd_step = 1;
        }
    }
    else
    {
        if ((rxData == '0') || (rxData == '1'))
        {
            Led_Control(uart_led_index, rxData - '0');
            uart_cmd_step = 0;
        }
        else if ((rxData >= '1') && (rxData <= '4'))
        {
            uart_led_index = rxData - '0';
            uart_cmd_step = 1;
        }
        else
        {
            uart_cmd_step = 0;
        }
    }
}

void Uart0_Init(void)
{
    CLKCONCMD &= ~0x40;
    while (CLKCONSTA & 0x40);
    CLKCONCMD &= ~0x47;

    PERCFG = 0x00;
    P0SEL = 0x0C;
    P2DIR &= ~0xC0;
    U0CSR |= 0x80;

    U0GCR &= 0x1F;
    U0GCR |= 8;
    U0BAUD |= 59;
    UTX0IF = 1;

    U0CSR |= 0x40;
    IEN0 |= 0x84;
}

#pragma vector = URX0_VECTOR
__interrupt void Uart0_ISR(void)
{
    URX0IF = 0;
    Uart0_Parse_Command(U0DBUF);
}

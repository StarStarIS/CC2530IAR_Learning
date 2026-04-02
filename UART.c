/*
 * UART command usage:
 * Enter "<LED number> <value>" from the serial tool to control LED1~LED4.
 * Examples:
 * "1 0"   -> turn off LED1
 * "1 1"   -> turn on LED1 at 100% brightness
 * "1 50%" -> set LED1 brightness to 50%
 * "4 75%" -> set LED4 brightness to 75%
 * DO NOT forget the space between "<LED number>" and "<value>".
 */


#include <ioCC2530.h>

#define LED1   P1_4
#define LED2   P0_1
#define LED3   P1_0
#define LED4   P1_1
#define PIN1_7 P1_7

#define LED_ON  0
#define LED_OFF 1

#define UART_STATE_WAIT_LED   0
#define UART_STATE_WAIT_SPACE 1
#define UART_STATE_WAIT_VALUE 2
#define UART_STATE_READ_VALUE 3

static volatile unsigned char ledBrightness[4] = {0, 0, 0, 0};
static unsigned char uartState = UART_STATE_WAIT_LED;
static unsigned char uartLedIndex = 0;
static unsigned char uartValue = 0;
static unsigned char uartDigits = 0;

void Uart0_Init(void);
void Led_Init(void);
void Led_Set_Brightness(unsigned char ledIndex, unsigned char brightness);
void Led_Pwm_Task(void);
void Uart0_Reset_Command(void);
void Uart0_Parse_Command(unsigned char rxData);

void main(void)
{
    Led_Init();
    Uart0_Init();

    while (1)
    {
        Led_Pwm_Task();
    }
}

void Led_Init(void)
{
    P1SEL &= ~(0x10 | 0x01 | 0x02 | 0x80);
    P1DIR |= (0x10 | 0x01 | 0x02 | 0x80);

    P0SEL &= ~0x02;
    P0DIR |= 0x02;

    ledBrightness[0] = 0;
    ledBrightness[1] = 0;
    ledBrightness[2] = 0;
    ledBrightness[3] = 0;

    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;
    LED4 = LED_OFF;
    PIN1_7 = 0;
}

void Led_Set_Brightness(unsigned char ledIndex, unsigned char brightness)
{
    if ((ledIndex >= 1) && (ledIndex <= 4) && (brightness <= 100))
    {
        ledBrightness[ledIndex - 1] = brightness;
    }
}

void Led_Pwm_Task(void)
{
    static unsigned char pwmStep = 0;

    LED1 = (ledBrightness[0] > pwmStep) ? LED_ON : LED_OFF;
    LED2 = (ledBrightness[1] > pwmStep) ? LED_ON : LED_OFF;
    LED3 = (ledBrightness[2] > pwmStep) ? LED_ON : LED_OFF;
    LED4 = (ledBrightness[3] > pwmStep) ? LED_ON : LED_OFF;

    pwmStep++;
    if (pwmStep >= 100)
    {
        pwmStep = 0;
    }
}

void Uart0_Reset_Command(void)
{
    uartState = UART_STATE_WAIT_LED;
    uartLedIndex = 0;
    uartValue = 0;
    uartDigits = 0;
}

void Uart0_Parse_Command(unsigned char rxData)
{
    if ((rxData == '\r') || (rxData == '\n'))
    {
        if ((uartState == UART_STATE_READ_VALUE) && (uartDigits == 1))
        {
            if (uartValue == 0)
            {
                Led_Set_Brightness(uartLedIndex, 0);
            }
            else if (uartValue == 1)
            {
                Led_Set_Brightness(uartLedIndex, 100);
            }
        }

        Uart0_Reset_Command();
        return;
    }

    switch (uartState)
    {
    case UART_STATE_WAIT_LED:
        if ((rxData >= '1') && (rxData <= '4'))
        {
            uartLedIndex = rxData - '0';
            uartState = UART_STATE_WAIT_SPACE;
        }
        break;

    case UART_STATE_WAIT_SPACE:
        if (rxData == ' ')
        {
            uartState = UART_STATE_WAIT_VALUE;
        }
        else if ((rxData >= '1') && (rxData <= '4'))
        {
            uartLedIndex = rxData - '0';
        }
        else
        {
            Uart0_Reset_Command();
        }
        break;

    case UART_STATE_WAIT_VALUE:
        if (rxData == ' ')
        {
            break;
        }

        if ((rxData >= '0') && (rxData <= '9'))
        {
            uartValue = rxData - '0';
            uartDigits = 1;
            uartState = UART_STATE_READ_VALUE;

            if (uartValue == 0)
            {
                Led_Set_Brightness(uartLedIndex, 0);
            }
        }
        else
        {
            Uart0_Reset_Command();
        }
        break;

    case UART_STATE_READ_VALUE:
        if ((rxData >= '0') && (rxData <= '9'))
        {
            if (uartDigits < 3)
            {
                uartValue = (unsigned char)(uartValue * 10 + (rxData - '0'));
                uartDigits++;

                if (uartValue > 100)
                {
                    Uart0_Reset_Command();
                }
            }
            else
            {
                Uart0_Reset_Command();
            }
        }
        else if (rxData == '%')
        {
            Led_Set_Brightness(uartLedIndex, uartValue);
            Uart0_Reset_Command();
        }
        else
        {
            Uart0_Reset_Command();
        }
        break;

    default:
        Uart0_Reset_Command();
        break;
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

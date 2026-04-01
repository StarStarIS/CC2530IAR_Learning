#include <ioCC2530.h>

#define LED1 P1_4
#define LED2 P0_1
#define LED3 P1_0
#define LED4 P1_1
#define PIN1_7 P1_7
#define SW1 P1_5
#define SW2 P1_6

#define ON 0
#define OFF 1
#define DOWN 0
#define UP 1
#define KEY_NONE 0
#define KEY_SW1  1
#define KEY_SW2  2
#define MODE_STOP    0
#define MODE_FORWARD 1
#define MODE_REVERSE 2

volatile unsigned char key_flag = KEY_NONE;

void led_init(void);
void delay_ms(int num_ms);
void key_init_int(void);
void led_all_off(void);
void show_led_by_index(unsigned char led_index);

void main(void) 
{
    unsigned char run_mode = MODE_STOP;
    unsigned char led_index = 0;

    led_init();
    key_init_int();

    while(1) 
    {
        if(key_flag)
        {
            unsigned char key_value = key_flag;

            key_flag = KEY_NONE;
            delay_ms(10); // debounce delay

            if((key_value == KEY_SW1) && (SW1 == DOWN))
            {
                run_mode = MODE_FORWARD;
                led_index = 0;
            }
            else if((key_value == KEY_SW2) && (SW2 == DOWN))
            {
                run_mode = MODE_REVERSE;
                led_index = 3;
            }
        }

        if(run_mode == MODE_FORWARD)
        {
            show_led_by_index(led_index);
            delay_ms(200);

            led_index++;
            if(led_index > 3)
            {
                led_index = 0;
            }
        }
        else if(run_mode == MODE_REVERSE)
        {
            show_led_by_index(led_index);
            delay_ms(200);

            if(led_index == 0)
            {
                led_index = 3;
            }
            else
            {
                led_index--;
            }
        }
        else
        {
            led_all_off();
        }
    }
}


/*=====================================================
function: initial led i/o pin
param: none
return: none
date: 2026-03-30
author: 
=====================================================*/
void led_init(void) 
{
    P1SEL &= ~(0x10 | 0x01 | 0x02 | 0x80); // LED1, LED3, LED4, P1.7 as GPIO
    P1DIR |= (0x10 | 0x01 | 0x02 | 0x80);  // LED1, LED3, LED4, P1.7 output
    P0SEL &= ~0x02;                        // LED2 as GPIO
    P0DIR |= 0x02;                         // LED2 output

    LED1 = OFF;
    LED2 = OFF;
    LED3 = OFF;
    LED4 = OFF;
    PIN1_7 = 0;                            // keep P1.7 low
}


/*=====================================================
function: turn off all leds
param: none
return: none
date: 2026-03-31
author:
=====================================================*/
void led_all_off(void)
{
    LED1 = OFF;
    LED2 = OFF;
    LED3 = OFF;
    LED4 = OFF;
}


/*=====================================================
function: turn on led by index
param: unsigned char
return: none
date: 2026-03-31
author:
=====================================================*/
void show_led_by_index(unsigned char led_index)
{
    led_all_off();

    if(led_index == 0)
    {
        LED1 = ON;
    }
    else if(led_index == 1)
    {
        LED2 = ON;
    }
    else if(led_index == 2)
    {
        LED3 = ON;
    }
    else if(led_index == 3)
    {
        LED4 = ON;
    }
}


/*=====================================================
function: delay for ms
param: int
return: none
date: 2026-03-30
author: 
=====================================================*/
void delay_ms(int num_ms) 
{
    int i, j;
    for (i = 0; i < num_ms; i++) 
    {
        for (j = 0; j < 578; j++); // delay loop
    }
}


/*=====================================================
function: initial key interrupt
param: none
return: none
date: 2026-03-31
author: 
=====================================================*/
void key_init_int(void) 
{
    P1SEL &= ~(0x20 | 0x40); // pin function select I/O
    P1DIR &= ~(0x20 | 0x40); // direction input

    IEN2 |= 0x10; // enable interrupt

    PICTL |= 0x04; // PICTL set 0 riaise 1 down
    P1IEN |= (0x20 | 0x40); // PxIEN set
    P1IFG = 0x00;  // PxIFG set
    P1IF = 0;  // clear interrupt flag

    EA = 1; // enable global interrupt
}

#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
    if (P1IFG & 0x20) // check if key interrupt
    {
        key_flag = KEY_SW1; // set key flag
        P1IFG &= ~0x20; // clear interrupt flag
    }
    else if (P1IFG & 0x40)
    {
        key_flag = KEY_SW2; // set key flag
        P1IFG &= ~0x40; // clear interrupt flag
    }
    P1IF = 0; // clear interrupt flag
}


/*=====================================================
function: init p0_7 adc channel
param: none
return: none
date: 2026-04-01
author: 
=====================================================*/
void adc_init(void)
{
    char cfg = 0;
    P0SEL |= 0x80;
    P0DIR &= ~0x80; // P0.7 select adc channel

    APCFG |= 0x80; // set analog
    cfg = 0x80 | 0x30 | 0x07;
    ADCCON3 = cfg; // config adc

    ADCCON1 &= ~0x30;
    ADCCON1 |= 0x30;
    cfg = ADCH; // clear EOC

    // ADCIE = 1; // enable adc interrupt
    // ADCIF = 0; // clear adc flag
    // EA = 1;
    // ADCCON1 |= 0x40; // start adc
}

#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void)
{
    // readn result
    // adc flag set
    ADCIF = 0;
}
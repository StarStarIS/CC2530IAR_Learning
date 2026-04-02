#include <ioCC2530.h>

typedef unsigned char uchar;
typedef unsigned int uint;

#define DHT11_DATA               P2_0
#define PIN1_7                   P1_7
#define DHT11_PIN_MASK           0x01
#define UART0_PIN_MASK           0x0C
#define PIN1_7_MASK              0x80
#define DHT11_SAMPLE_INTERVAL_MS 1000
#define TX_BUFFER_SIZE           24

static char txBuffer[TX_BUFFER_SIZE];
static uchar dht11ByteError = 0;

void clock_init(void);
void delay_us(void);
void delay_10us(void);
void delay_ms(uint time);
void pin1_7_init(void);
void uart0_init(void);
void uart0_send_byte(uchar data);
void uart0_send_string(char *data);
void dht11_init(void);
uchar dht11_wait_for_level(uchar level, uint timeout);
uchar dht11_read_byte(void);
uchar dht11_read(uchar *humidity, uchar *temperature);
void build_sensor_message(char *buffer, uchar humidity, uchar temperature);
void build_error_message(char *buffer);

void main(void)
{
    uchar humidity = 0;
    uchar temperature = 0;

    clock_init();
    pin1_7_init();
    uart0_init();
    dht11_init();

    delay_ms(1000);

    while(1)
    {
        if(dht11_read(&humidity, &temperature))
        {
            build_sensor_message(txBuffer, humidity, temperature);
        }
        else
        {
            build_error_message(txBuffer);
        }

        uart0_send_string(txBuffer);
        delay_ms(DHT11_SAMPLE_INTERVAL_MS);
    }
}

void clock_init(void)
{
    CLKCONCMD &= ~0x40;
    while(CLKCONSTA & 0x40);
    CLKCONCMD &= ~0x47;
}

void delay_us(void)
{
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
}

void delay_10us(void)
{
    delay_us();
    delay_us();
}

void delay_ms(uint time)
{
    uint i;
    uchar j;

    for(i = 0; i < time; i++)
    {
        for(j = 0; j < 100; j++)
        {
            delay_10us();
        }
    }
}

void pin1_7_init(void)
{
    P1SEL &= ~PIN1_7_MASK;
    P1DIR |= PIN1_7_MASK;
    PIN1_7 =0;
}

void uart0_init(void)
{
    PERCFG &= ~0x01;
    P0SEL |= UART0_PIN_MASK;
    P2DIR &= ~0xC0;

    U0CSR |= 0x80;
    U0GCR = (U0GCR & ~0x1F) | 11;
    U0BAUD = 216;
    UTX0IF = 0;
}

void uart0_send_byte(uchar data)
{
    U0DBUF = data;
    while(UTX0IF == 0);
    UTX0IF = 0;
}

void uart0_send_string(char *data)
{
    while(*data != '\0')
    {
        uart0_send_byte((uchar)(*data));
        data++;
    }
}

void dht11_init(void)
{
    P2SEL &= ~DHT11_PIN_MASK;
    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;
}

uchar dht11_wait_for_level(uchar level, uint timeout)
{
    while(timeout > 0)
    {
        if((uchar)DHT11_DATA == level)
        {
            return 1;
        }

        delay_us();
        timeout--;
    }

    return 0;
}

uchar dht11_read_byte(void)
{
    uchar i;
    uchar value = 0;

    dht11ByteError = 0;

    for(i = 0; i < 8; i++)
    {
        if(!dht11_wait_for_level(1, 100))
        {
            dht11ByteError = 1;
            return 0;
        }

        delay_10us();
        delay_10us();
        delay_10us();

        value <<= 1;
        if(DHT11_DATA)
        {
            value |= 0x01;
        }

        if(!dht11_wait_for_level(0, 100))
        {
            dht11ByteError = 1;
            return 0;
        }
    }

    return value;
}

uchar dht11_read(uchar *humidity, uchar *temperature)
{
    uchar humidityHigh;
    uchar humidityLow;
    uchar temperatureHigh;
    uchar temperatureLow;
    uchar checkData;
    uchar sum;

    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 0;
    delay_ms(20);
    DHT11_DATA = 1;
    P2DIR &= ~DHT11_PIN_MASK;

    delay_10us();
    delay_10us();
    delay_10us();

    if(!dht11_wait_for_level(0, 20))
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    if(!dht11_wait_for_level(1, 100))
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    if(!dht11_wait_for_level(0, 100))
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    humidityHigh = dht11_read_byte();
    if(dht11ByteError)
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    humidityLow = dht11_read_byte();
    if(dht11ByteError)
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    temperatureHigh = dht11_read_byte();
    if(dht11ByteError)
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    temperatureLow = dht11_read_byte();
    if(dht11ByteError)
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    checkData = dht11_read_byte();
    if(dht11ByteError)
    {
        P2DIR |= DHT11_PIN_MASK;
        DHT11_DATA = 1;
        return 0;
    }

    P2DIR |= DHT11_PIN_MASK;
    DHT11_DATA = 1;

    sum = humidityHigh + humidityLow + temperatureHigh + temperatureLow;
    if(sum != checkData)
    {
        return 0;
    }

    *humidity = humidityHigh;
    *temperature = temperatureHigh;
    return 1;
}

void build_sensor_message(char *buffer, uchar humidity, uchar temperature)
{
    buffer[0] = 'T';
    buffer[1] = 'e';
    buffer[2] = 'm';
    buffer[3] = 'p';
    buffer[4] = ':';
    buffer[5] = (char)('0' + (temperature / 10));
    buffer[6] = (char)('0' + (temperature % 10));
    buffer[7] = 'C';
    buffer[8] = ' ';
    buffer[9] = 'H';
    buffer[10] = 'u';
    buffer[11] = 'm';
    buffer[12] = ':';
    buffer[13] = (char)('0' + (humidity / 10));
    buffer[14] = (char)('0' + (humidity % 10));
    buffer[15] = '%';
    buffer[16] = '\r';
    buffer[17] = '\n';
    buffer[18] = '\0';
}

void build_error_message(char *buffer)
{
    buffer[0] = 'D';
    buffer[1] = 'H';
    buffer[2] = 'T';
    buffer[3] = '1';
    buffer[4] = '1';
    buffer[5] = ' ';
    buffer[6] = 'r';
    buffer[7] = 'e';
    buffer[8] = 'a';
    buffer[9] = 'd';
    buffer[10] = ' ';
    buffer[11] = 'f';
    buffer[12] = 'a';
    buffer[13] = 'i';
    buffer[14] = 'l';
    buffer[15] = '\r';
    buffer[16] = '\n';
    buffer[17] = '\0';
}

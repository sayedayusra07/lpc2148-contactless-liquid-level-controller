/******************************************************************************
 * PROJECT: Smart Water Level Indicator with Pump Control
 * MICROCONTROLLER: NXP LPC2148 (ARM7) @ 12MHz Crystal
 ******************************************************************************/

#include <lpc214x.h>
#include <stdio.h>

/* Hardware Pin Definitions */
#define TRIG   (1<<5)
#define ECHO   (1<<4)
#define RELAY  (1<<11)

/* I2C LCD Address */
#define LCD_I2C_ADDR 0x4E

/* Function Prototypes */
void delayUS(unsigned int us);
void delayMS(unsigned int ms);
void initTimer0(void);

void i2c_init(void);
void i2c_start(void);
void i2c_write(unsigned char data);
void i2c_stop(void);

void lcd_send_nibble(unsigned char data);
void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init(void);
void lcd_string(char *str);

int main(void)
{
    /* Main state variables */
    unsigned int distance;
    unsigned int timeout;
    unsigned int water_percentage;
    unsigned int last_distance = 0xFFFF;
    unsigned int pump_state = 0;
    unsigned int last_pump_state = 0xFFFF;
    char display_buffer[17];

    /* Median filter variables */
    unsigned int pings[5];
    unsigned int valid_readings;
    unsigned int current_ping;
    unsigned int temp;
    int sample, i, j;

    /* System Power-Up Delay */
    delayMS(100);

    /* Configure GPIO Directions */
    IODIR0 |= TRIG | RELAY;
    IODIR0 &= ~ECHO;

    /* Ensure motor OFF at startup */
    IOCLR0 = RELAY;

    /* Initialize Peripherals */
    initTimer0();
    i2c_init();
    lcd_init();

    while(1)
    {
        /* Reset filter state each iteration */
        valid_readings = 0;
        current_ping = 0;

        /* ==========================================
         * 1. SENSOR POLLING (MEDIAN FILTER)
         * ========================================== */
        for(sample = 0; sample < 5; sample++)
        {
            /* Sync: Wait for ECHO to go LOW before firing */
            timeout = 0;
            while(IOPIN0 & ECHO) {
                timeout++;
                if(timeout > 100000) break;
            }

            /* Generate 10us Trigger Pulse */
            IOCLR0 = TRIG;
            delayUS(2);
            IOSET0 = TRIG;
            delayUS(10);
            IOCLR0 = TRIG;

            /* Wait for Echo HIGH with timeout */
            timeout = 0;
            while(!(IOPIN0 & ECHO)) {
                timeout++;
                if(timeout > 100000) break;
            }

            if(timeout > 100000)
                continue;

            /* Measure Echo pulse width with Hardware Timer */
            T0TCR = 0x02;
            T0TCR = 0x01;

            while(IOPIN0 & ECHO) {
                if(T0TC > 38000)
                    break;
            }

            T0TCR = 0x00;

            current_ping = T0TC / 58;

            if(current_ping > 0 && current_ping <= 50) {
                pings[valid_readings] = current_ping;
                valid_readings++;
            }

            delayMS(60);
        }

        /* ==========================================
         * 2. DATA PROCESSING (SORT & MEDIAN)
         * ========================================== */
        if(valid_readings > 0) {

            for(i = 0; i < (int)valid_readings - 1; i++) {

                for(j = 0; j < (int)valid_readings - i - 1; j++) {

                    if(pings[j] > pings[j + 1]) {
                        temp = pings[j];
                        pings[j] = pings[j + 1];
                        pings[j + 1] = temp;
                    }
                }
            }

            distance = pings[valid_readings / 2];
        }
        else {
            distance = 40;
        }

        /* ==========================================
         * 3. WATER LEVEL & MOTOR CONTROL
         * Empty >=30cm = 0%
         * Full <=10cm = 100%
         * Motor ON from 0% to 99%
         * Motor OFF only at 100%
         * ========================================== */

        if(distance >= 30) {
            water_percentage = 0;
        }
        else if(distance <= 10) {
            water_percentage = 100;
        }
        else {
            water_percentage = (30 - distance) * 5;
        }

        /* Motor Control Logic */
        if(water_percentage < 100) {
            IOSET0 = RELAY;   /* Motor ON */
            pump_state = 1;
        }
        else {
            IOCLR0 = RELAY;   /* Motor OFF */
            pump_state = 0;
        }

        /* ==========================================
         * 4. LCD UI WITH ANTI-FLICKER
         * ========================================== */
        if(distance != last_distance ||
           pump_state != last_pump_state)
        {
            lcd_cmd(0x80);
            sprintf(display_buffer,
                    "Water Lvl:%3d%%",
                    water_percentage);
            lcd_string(display_buffer);

            lcd_cmd(0xC0);

            if(pump_state == 1) {
                sprintf(display_buffer,
                        "D:%2dcm Pump:ON ",
                        distance);
            }
            else {
                sprintf(display_buffer,
                        "D:%2dcm Pump:OFF",
                        distance);
            }

            lcd_string(display_buffer);

            last_distance = distance;
            last_pump_state = pump_state;
        }

        delayMS(50);
    }
}

/* ==========================================
 * Hardware Timer Initialization
 * ========================================== */
void initTimer0(void)
{
    T0CTCR = 0x00;
    T0PR = 11;
    T0TCR = 0x02;
}

/* ==========================================
 * Hardware I2C Functions
 * ========================================== */
void i2c_init(void)
{
    PINSEL0 |= 0x00000050;
    I2C0CONCLR = 0x6C;
    I2C0CONSET = 0x40;
    I2C0SCLH = 60;
    I2C0SCLL = 60;
}

void i2c_start(void)
{
    I2C0CONSET = 0x20;

    while((I2C0CONSET & 0x08) == 0);

    I2C0CONCLR = 0x20;
}

void i2c_write(unsigned char data)
{
    I2C0DAT = data;
    I2C0CONCLR = 0x08;

    while((I2C0CONSET & 0x08) == 0);
}

void i2c_stop(void)
{
    I2C0CONSET = 0x10;
    I2C0CONCLR = 0x08;
}

/* ==========================================
 * I2C LCD Driver
 * ========================================== */
void lcd_send_nibble(unsigned char data)
{
    i2c_start();
    i2c_write(LCD_I2C_ADDR);

    i2c_write(data | 0x04);
    delayUS(50);

    i2c_write(data & ~0x04);
    delayUS(50);

    i2c_stop();
}

void lcd_cmd(unsigned char cmd)
{
    unsigned char high_nibble =
        (cmd & 0xF0) | 0x08;

    unsigned char low_nibble =
        ((cmd << 4) & 0xF0) | 0x08;

    lcd_send_nibble(high_nibble);
    lcd_send_nibble(low_nibble);

    delayMS(2);
}

void lcd_data(unsigned char data)
{
    unsigned char high_nibble =
        (data & 0xF0) | 0x09;

    unsigned char low_nibble =
        ((data << 4) & 0xF0) | 0x09;

    lcd_send_nibble(high_nibble);
    lcd_send_nibble(low_nibble);

    delayUS(50);
}

void lcd_init(void)
{
    delayMS(50);

    lcd_send_nibble(0x30 | 0x08);
    delayMS(5);

    lcd_send_nibble(0x30 | 0x08);
    delayUS(150);

    lcd_send_nibble(0x30 | 0x08);
    delayUS(150);

    lcd_send_nibble(0x20 | 0x08);
    delayMS(5);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);

    delayMS(5);
}

void lcd_string(char *str)
{
    int i = 0;

    while(str[i] != '\0') {
        lcd_data(str[i]);
        i++;
    }
}

/* ==========================================
 * Software Delays
 * ========================================== */
void delayUS(unsigned int us)
{
    unsigned int i, j;

    for(i = 0; i < us; i++) {
        for(j = 0; j < 10; j++);
    }
}

void delayMS(unsigned int ms)
{
    unsigned int i;

    for(i = 0; i < ms; i++) {
        delayUS(1000);
    }
}
/*
  E2.cpp - E2 library for Arduino
  CC-BY-SA

  Modified 02/2013 by Romain Bazile (romain.bazile@gromain.me)

  CHANGELOG
  #30/01/2013: Initial release
  #06/02/2013: Inclusion of Custom mem read and
  #             replacement of EE03_Status by Status

*/

#include"Arduino.h"
#include "E2.h"

// definitions
#define DELAY_FACTOR 50 // setup clock-frequency
#define ACK 1 // define acknowledge
#define NAK 0 // define not-acknowledge

// variables

unsigned char rh_low;
unsigned char rh_high;
unsigned char temp_low;
unsigned char temp_high;
unsigned char co2_low;
unsigned char co2_high;
unsigned char co2mean_low;
unsigned char co2mean_high;
unsigned char checksum_03;
unsigned int rh_ee03 = 0;
unsigned int temp_ee03 = 0;
unsigned int co2_ee03 = 0;
unsigned int co2mean_ee03 = 0;
unsigned int internal_pointer_addr = 0;
float rh = 0;
float temperature = 0;
float co2 = 0;
float co2mean = 0;
unsigned char information;

E2_Device::E2_Device(int pinSDA, int pinSCL)
{
    _pinSDA = pinSDA;
    _pinSCL = pinSCL;
}

/* Private Functions */
/* Arduino speed test: mean time to execute: 10µs */
void E2_Device::set_SDA(void) // set port-pin (pinSDA)
{
    pinMode(_pinSDA, OUTPUT);
    digitalWrite(_pinSDA, HIGH);
}
void E2_Device::clear_SDA(void) // clear port-pin (pinSDA)
{
    pinMode(_pinSDA, OUTPUT);
    digitalWrite(_pinSDA, LOW);
}

int E2_Device::read_SDA(void) // read pinSDA status
{
    pinMode(_pinSDA, INPUT);
    return digitalRead(_pinSDA);
}

void E2_Device::set_SCL(void) // set port-pin (pinSCL)
{
    pinMode(_pinSCL, OUTPUT);
    digitalWrite(_pinSCL, HIGH);
}
void E2_Device::clear_SCL(void) // clear port-pin (pinSCL)
{
    pinMode(_pinSCL, OUTPUT);
    digitalWrite(_pinSCL, LOW);
}

void E2_Device::E2Bus_start(void) // send Start condition to E2 Interface
{
    set_SDA();
    set_SCL();
    e2delay(30*DELAY_FACTOR);
    clear_SDA();
    e2delay(30*DELAY_FACTOR);
}
/*-------------------------------------------------------------------------*/
void E2_Device::E2Bus_stop(void) // send Stop condition to E2 Interface
{
    clear_SCL();
    e2delay(20*DELAY_FACTOR);
    clear_SDA();
    e2delay(20*DELAY_FACTOR);
    set_SCL();
    e2delay(20*DELAY_FACTOR);
    set_SDA();
    e2delay(20*DELAY_FACTOR);
}
/*-------------------------------------------------------------------------*/
void E2_Device::E2Bus_send(unsigned char value) // send Byte to E2 Interface
{
    unsigned char i;
    unsigned char maske = 0x80;
    for (i=8;i>0;i--)
    {
        clear_SCL();
        e2delay(10*DELAY_FACTOR);
        if ((value & maske) != 0)
            {set_SDA();}
        else
            {clear_SDA();}
        e2delay(20*DELAY_FACTOR);
        set_SCL();
        maske >>= 1;
        e2delay(30*DELAY_FACTOR);
        clear_SCL();
    }
    set_SDA();
}
/*-------------------------------------------------------------------------*/
unsigned char E2_Device::E2Bus_read(void) // read Byte from E2 Interface
{
    unsigned char data_in = 0x00;
    unsigned char maske = 0x80;
    for (maske=0x80;maske>0;maske >>=1)
    {
        clear_SCL();
        e2delay(30*DELAY_FACTOR);
        set_SCL();
        e2delay(15*DELAY_FACTOR);
        if (read_SDA())
            {data_in |= maske;}
        e2delay(15*DELAY_FACTOR);
        clear_SCL();
    }
    return data_in;
}
/*-------------------------------------------------------------------------*/
char E2_Device::check_ack(void) // check for acknowledge
{
    int input;
    e2delay(30*DELAY_FACTOR);
    set_SCL();
    e2delay(15*DELAY_FACTOR);
    input = read_SDA();
    e2delay(15*DELAY_FACTOR);
    if(input == 1)
        return NAK;
    else
        return ACK;
}
/*-------------------------------------------------------------------------*/
void E2_Device::send_ack(void) // send acknowledge
{
    clear_SCL();
    e2delay(15*DELAY_FACTOR);
    clear_SDA();
    e2delay(15*DELAY_FACTOR);
    set_SCL();
    e2delay(30*DELAY_FACTOR);
    clear_SCL();
    set_SDA();
}
/*-------------------------------------------------------------------------*/
void E2_Device::send_nak(void) // send NOT-acknowledge
{
    clear_SCL();
    e2delay(15*DELAY_FACTOR);
    set_SDA();
    e2delay(15*DELAY_FACTOR);
    set_SCL();
    e2delay(30*DELAY_FACTOR);
    clear_SCL();
    set_SDA();
}
/*-------------------------------------------------------------------------*/
void E2_Device::e2delay(unsigned int value) // e2delay- routine
    { delayMicroseconds(value); }
/*-------------------------------------------------------------------------*/


/* Public Functions */
float E2_Device::Temp_read(void)
{
    temperature = -300; // default value (error code)
    E2Bus_start();
    E2Bus_send(0xA1); // MV2-low request
    if (check_ack()==ACK)
    {
        temp_low = E2Bus_read();
        send_ack();
        checksum_03 = E2Bus_read();
        send_nak(); // terminate communication
        E2Bus_stop();
        if (((0xA1 + temp_low) % 256) == checksum_03) // checksum OK?
        {
            E2Bus_start();
            E2Bus_send(0xB1); // MV2-high request
            check_ack();
            temp_high = E2Bus_read();
            send_ack(); // terminate communication
            checksum_03 = E2Bus_read();
            send_nak();
            E2Bus_stop();
            if (((0xB1 + temp_high) % 256) == checksum_03) // checksum OK?
            {
                temp_ee03=temp_low+256*temp_high; //yes->calculate temperature
                temperature=((float)temp_ee03/100) - 273.15;
                // overwrite default (error) value
            }
        }
        E2Bus_stop();
    }
    return temperature;
}

float E2_Device::RH_read(void)
{
    rh = -1; // default value (error code)
    E2Bus_start();
    E2Bus_send(0x81); // MV1-low request
    if (check_ack()==ACK)
    {
        rh_low = E2Bus_read();
        send_ack();
        checksum_03 = E2Bus_read();
        send_nak(); // terminate communication
        E2Bus_stop();
        if (((0x81 + rh_low) % 256) == checksum_03) // checksum OK?
        {
            E2Bus_start();
            E2Bus_send(0x91); // MV1-high request
            check_ack();
            rh_high = E2Bus_read();
            send_ack();
            checksum_03 = E2Bus_read();
            send_nak(); // terminate communication
            E2Bus_stop();
            if (((0x91 + rh_high) % 256) == checksum_03) // checksum OK?
            {
                rh_ee03=rh_low+256*(unsigned int)rh_high;
                // yes-> calculate humidity value
                rh=(float)rh_ee03/100;
                // overwrite default (error) value
            }
        }
        E2Bus_stop();
    }
    return rh;
}

unsigned char E2_Device::Status(void)
{
    unsigned char stat_ee03;
    E2Bus_start(); // start condition for E2-Bus
    E2Bus_send(0x71); // main command for STATUS request
    if (check_ack()==ACK)
    {
        stat_ee03 = E2Bus_read(); // read status byte
        send_ack();
        checksum_03 = E2Bus_read(); // read checksum
        send_nak(); // send NAK ...
        E2Bus_stop(); // ... and stop condition to terminate
        if (((stat_ee03 + 0x71) % 256) == checksum_03) // checksum OK?
            return stat_ee03;
    }
    return 0xFF; // in error case return 0xFF
}

float E2_Device::CO2_read(void)
{
    co2 = -1; // default value (error code)
    E2Bus_start();
    E2Bus_send(0xC1); // MV3-low request
    if (check_ack()==ACK)
    {
        co2_low = E2Bus_read();
        send_ack();
        checksum_03 = E2Bus_read();
        send_nak(); // terminate communication
        E2Bus_stop();
        if (((0xC1 + co2_low) % 256) == checksum_03) // checksum OK?
        {
            E2Bus_start();
            E2Bus_send(0xD1); // MV3-high request
            check_ack();
            co2_high = E2Bus_read();
            send_ack();
            checksum_03 = E2Bus_read();
            send_nak(); // terminate communication
            E2Bus_stop();
            if (((0xD1 + co2_high) % 256) == checksum_03) // checksum OK?
            {
                co2_ee03=co2_low+256*(unsigned int)co2_high;
                // yes-> calculate CO2 value
                co2=co2_ee03;
                // overwrite default (error) value
            }
        }
        E2Bus_stop();
    }
    return co2;
}

float E2_Device::CO2mean_read(void)
{
    co2mean = -1; // default value (error code)
    E2Bus_start();
    E2Bus_send(0xE1); // MV4-low request
    if (check_ack()==ACK)
    {
        co2mean_low = E2Bus_read();
        send_ack();
        checksum_03 = E2Bus_read();
        send_nak(); // terminate communication
        E2Bus_stop();
        if (((0xE1 + co2mean_low) % 256) == checksum_03) // checksum OK?
        {
            E2Bus_start();
            E2Bus_send(0xF1); // MV4-high request
            check_ack();
            co2mean_high = E2Bus_read();
            send_ack();
            checksum_03 = E2Bus_read();
            send_nak(); // terminate communication
            E2Bus_stop();
            if (((0xF1 + co2mean_high) % 256) == checksum_03) // checksum OK?
            {
                co2mean_ee03=co2mean_low+256*(unsigned int)co2mean_high;
                // yes-> calculate CO2 value
                co2mean=co2mean_ee03;
                // overwrite default (error) value
            }
        }
        E2Bus_stop();
    }
    return co2mean;
}

unsigned char E2_Device::Custom_mem_read(void)
{
    E2Bus_start();
    E2Bus_send(0x51); // Read from custom mem request
    if (check_ack()==ACK)
    {
        information = E2Bus_read();
        send_ack();
        checksum_03 = E2Bus_read();
        send_nak(); // terminate communication
        E2Bus_stop();
        if (((0x51 + information) % 256) == checksum_03) // checksum OK?
        {
            return information;
        }
        else {
            return 'read_error';
        }
        E2Bus_stop();
    }
    if ( internal_pointer_addr < 255)
    {
        internal_pointer_addr += 1;
    }
    else {
        internal_pointer_addr = 0;
    }
}

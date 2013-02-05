/*
  E2.h - E2 library for Arduino
  CC-BY-SA

  Modified 01/2013 by Romain Bazile (romain.bazile@ubiant.com)

  CHANGELOG
  #30/01/2013: Initial release

*/
#ifndef E2_H_INCLUDED
#define E2_H_INCLUDED
#include"Arduino.h"

class E2_Device
{
private:
    int _pinSDA;
    int _pinSCL;
    char check_ack(void);
    void send_ack(void);
    void send_nak(void);
    void E2Bus_start(void); // send start condition
    void E2Bus_stop(void); // send stop condition
    void E2Bus_send(unsigned char);
    void set_SDA(void);
    void clear_SDA(void);
    int read_SDA(void);
    void set_SCL(void);
    void clear_SCL(void);
    unsigned char E2Bus_read(void); // read one byte from E2-Bus
    void e2delay(unsigned int value);
public:
    E2_Device(int pinSDA, int pinSCL);
    float RH_read(void);
    float Temp_read(void);
    unsigned char Status(void);
    float CO2_read(void);
    float CO2mean_read(void);
    unsigned char Custom_mem_read(void);

};


#endif

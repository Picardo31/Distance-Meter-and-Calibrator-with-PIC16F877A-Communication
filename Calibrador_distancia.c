//programa del transmisor
#define _XTAL_FREQ 8000000
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7
#include <xc.h>
#include <stdio.h>
// BEGIN CONFIG
#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)
//END CONFIG
int x;
int adc;
float v;
char s[20];
void ADC()
{
    //CONFIGURAR ADC
    PCFG3 = 1;
    PCFG2 = 1;
    PCFG1 = 1;
    PCFG0 = 0;
    
    ADCS2 = 0;
    ADCS1 = 0;
    ADCS0 = 1;
    //JUSTIFICAR A LA DERECHA
    ADFM = 1;
    //PRENDER MODULO
    ADON = 1;
    CHS2 = 0;
    CHS1 = 0;
    CHS0 = 0;
}

int main()
{
  
  
  ADC();
  
  TRISC6 = 1;
  TRISB = 0X01;
  SYNC = 0;         //COMUNICACION ASINCRONA
  SPEN = 1;         //HABILITACIÓN DE COM SERIAL
  TXEN = 1;         // HABILITAR TRANSMISION CONTINUA
  BRGH = 1;         // HIGH SPEED
  
  SPBRG = 16;       // 28800 BAUDIOS
   
    while(1)
    {
        __delay_us(20);
        GO_DONE = 1;
        while (GO_DONE == 1)        //Aqui se utiliza tiempo para lectura de voltaje
        {
            
        }
        adc = ADRESH;
        adc = adc<<8;
        adc = adc + ADRESL;
        v = adc*5.0/1023.0;         //Conversion a voltaje
        if (TRMT == 1)
        {
            TXREG = v;              //Envio de datos sobre el voltaje leido
        }
        __delay_ms(100);
    }
}
#define _XTAL_FREQ 8000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#include <xc.h>
#include "lcd.h"
#include <stdio.h>
#include <string.h>
#include <conio.h>

// BEGIN CONFIG
#pragma config FOSC = HS 
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON 
#pragma config LVP = OFF 
#pragma config CPD = OFF 
#pragma config WRT = OFF 
#pragma config CP = OFF
//END CONFIG

int a;      //Variable que contiene la distancia medida
char s[10]; //Variable para imprimir en LCD
int v;      //Varaible recibida del calibrador para saber los valores maximos y minimos de distancia
int v1=300; //Rango superior maximo de distancia a medir
int v2=2;   //Rango minimo de distancia a medir
int r;      //Variable de verificación si hubo perdida de voltaje o reseteo
int rr;     //Contador de reseteo POR, BOR y MCLR
int pt;     //Variable de relación a distancia minima para que comience a sonar alarma
void interrupt echo()
{
  if(RBIF == 1)                       
  {
    RBIE = 0;                         
    if(RB4 == 1)                      //Si el echo es HIGH
    TMR1ON = 1;                       //Comienza timer
    if(RB4 == 0)                      //Si el echo es LOW
    {
      TMR1ON = 0;                     //Se detiene el timer
      a = (TMR1L | (TMR1H<<8))/58.82; //Calcular distancia
    }
  }
  RBIF = 0;                           //Se pone en 0 el estado de la bandera de interrupcion en B
  RBIE = 1;                             //Se habilita de nuevo interrupciones en puerto B
  if (PIR1bits.RCIF == 1)
    {
        v = RCREG;                     //Se verifica que unho recepción de datos en el puerto RX del medidor, y se guarda los datos en v
    }
}
void ret()
{
    if (PORTBbits.RB6 == 1)             //Verificación si se activa el guardar los datos en la memoria eeprom
    {
        rr = (int)eeprom_read(0x00);    //Se lee y se guarda el registro de 0x00 en rr
        while(WR)
            continue;
        if (r == 1)
        {
            rr = rr + 1;                //Se verifica si hubo alguna activación del POR, BOR o MCLR, y se suma al contador rr un estado
        }
        r = 0;
        eeprom_write(0x00,rr);          //Se guarda el contador rr en 0x00
    }
    if (PORTBbits.RB6 == 0)
    {
        eeprom_write(0x00,0);           //Si no se activa pin para guardar los datos, se borra los datos en el registro
    }
}
void Resets()                   //Rutina de verificción de POR, BOR Y MCLR
{
    if (PCONbits.nPOR == 0)
    {
        r = 1;
    }
    if (PCONbits.nPOR == 1 && PCONbits.nBOR == 0)
    {
        r = 1;
    }
    if (PCONbits.nPOR == 1 && PCONbits.nBOR == 1)
    {
        r = 1;
    }
    PCONbits.nPOR = 1;
    PCONbits.nBOR = 1;
    ret();
    return;
}
void main(void)
{
  TRISB = 0b11110000;                   //Se coloca mascara para determinar que pin es de entrada y cual es de salida
  TRISD = 0x00;                         //Se coloca 0 en puerto D para las salidas del LCD
  GIE = 1;                              //Se activan las interrupciones globales
  RBIF = 0;                             //Se coloca la bandera de interrupciones de B en 0
  RBIE = 1;                             //Se activa las interrupciones en B
  TRISA = 0b00000000;                   //Se coloca a los puerto A como salidas
  //Receptor
  SYNC = 0;                             //COMUNICACION ASINCRONA
  SPEN = 1;                             //HABILITACIÓN DE COM SERIAL
  CREN = 1;                             // HABILITAR RECEPCIÓN CONTINUA
  BRGH = 1;                             // HIGH SPEED
  
  RCIE = 1;                             //Se habilitan las interrupciones en puerto C
  PEIE = 1;                             //Se habiliatn las interrupciones perifericas
  TRISC7 = 1;                           //Se coloca al puerto RX del medidor como entrada para recepcion de datos del calibrador
  
  SPBRG = 16;                           // 28800 BAUDIOS
  Resets();                             //Se llama a la rutina de verificacion de POR, BOR y MCLR
  Lcd_Init();                           //Se inicializa el LCD
  Lcd_Clear();                          //Se limpia la información del LCD
  T1CON = 0x10;                         //Se inicia el modulo timer
  PORTAbits.RA1 = 1;                    //Se coloca en positivo las salida de RA1 para reducir el tiempo de inicio de alarma de aproximación
  while(1)
  {
    TMR1H = 0;                          //Setear valor inical del timer
    TMR1L = 0;                          //Setear valor inical del timer
    RB0 = 1;                            //TRIGGER HIGH
    __delay_us(10);                     //10uS Delay
    RB0 = 0;                            //TRIGGER LOW 
    __delay_ms(100);                    //Se espera el ECHO
    a = (a + 1)*1.014;                   //Constante de corrección de disntacia medida al regresar de la interrupción
    v = (v*100)/5;                       //Se obtiene un valor de relación en porcentaje sobre el maximo o minimo de distancia seteada a reconocer
    if (PORTBbits.RB7 == 1)
    {
        v1 = (v*300)/100;               //Se verifica si se desea setear la distancia maxima, y se coloca el valor de la distancia de acuerdo al porcentaje recibido delcalibrador
        
    }
    else
    {
        v2 = (v*300)/100;               //Se verifica si se desea setear la distancia minima y se coloca el valor de la distancia de acuerdo al porcentaje recibido delcalibrador
        if (v == 0)
        {
            v2 = 2;                     //Si la relación da a v = 0, entonces regresar a la distancia de fabrica de v = 2 cm;
        }
    }
    if (PORTBbits.RB6 == 1)
    {
        ret();                          //Si se activa el pin RB6, se llama a la rutina de guardado o lectura de memoria eeprom
    }
    if (PORTBbits.RB5 == 1)             //Si se activa RB5, se colocan todos los valores de distancia maxima, minima y contador de reseteo, en valores de fabrica
    {
        v1 = 300;           
        v2 = 2;
        rr = 1; 
        eeprom_write(0x00,1);
    }
    pt = a - v2;                        //Se obtiene la relacion de distancia entre el valor medido y el valor minimo de distancia
    if (pt > 30)                        //La alarma comienza a sonar si la relacion pt baja de 30 cm
    {
        PORTAbits.RA1 = 1;
    }
    if (pt <= 30 && pt >= 20)           //La alarma suena si la distancia pt, esta entre menor de 30cm y mayor a 20cm
    {
        RA0 = 1;                          
        __delay_ms(1000);                 //Se envia una señal de alto segudi por un bajo, de 1000 ms, en el pinn RA0 para simular una señal modulada que haga sonar la alarma
        RA0 = 0; 
    }
    if (pt <= 20 && pt >= 10)
    {
        RA0 = 1;                          
        __delay_ms(500);                   //La señal se intensifica si la distancia pt entra en el rango [10,20] cm
        RA0 = 0; 
    }
    if (pt <= 10 && pt >= 5)
    {
        RA0 = 1;                         
        __delay_ms(100);                   //La señal se intensifica si la distancia pt entra en el rango [5,10] cm
        RA0 = 0; 
    }
    if (pt <= 5 && pt >= 1)
    {
        RA0 = 1;                          
        __delay_ms(50);                    //La señal se intensifica si la distancia pt entra en el rango [1,5] cm
        RA0 = 0; 
    }
    if(a>=v2 && a<=v1)                //Verificar si el resultado es valido o no dentro del rango de distancias maximas establecido
    {
        Lcd_Set_Cursor(1,8);
        sprintf(s,"R=%d  ",rr);       //Se imprime en el LCD el valor del conteo de reseteo
        Lcd_Write_String(s);

        Lcd_Set_Cursor(2,1);
        sprintf(s,"X=%d ",v1);         //Se imprime en el LCD el valor de la distancia maxima
        Lcd_Write_String(s);

        Lcd_Set_Cursor(2,8);
        sprintf(s,"Y=%d  ",v2);         //Se imprime en el LCD el valor de la distancia minima
        Lcd_Write_String(s);

        Lcd_Set_Cursor(2,13);
        Lcd_Write_String("[cm]");       //Se imprime en el LCD la unidad utilizada de distancia
        
        Lcd_Set_Cursor(1,1);
        sprintf(s,"D=%d  ",a);           //Se imprime en el LCD la distancia medida
        Lcd_Write_String(s);
        
        Lcd_Set_Cursor(1,13);
        Lcd_Write_String("[cm]");        //Se imprime en el LCD la unidad utilizada de distancia
    }
    else
    {
      Lcd_Set_Cursor(1,1);
      Lcd_Write_String("Fuera de Rango   ");     //Se imprime en el LCD si la distancia medida se encuentra fuera de rango
    }
  }
  return;
}
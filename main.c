#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

/*Set BAUD for your communication port*/
#define  BAUD       9600  


/*Set current ambient temperature value
for calibration. If calibration is not necessary,
you can ignore this value and leave unchanged*/
#define  CALIB_TEMP  28


/*Set your processor temperature slope.
If you don't know how to calculate slope, 
just leave it to the default value*/
#define  TEMP_SLOPE  1.22

/*Set number of calibration cycles for temperature
sensor. The higher the cycle number, the accurate 
result will be obtained. the number should not be
more than 255*/
#define  CALIB_CYCLE 100


volatile uint16_t adc_raw=0,temp=0,calib_c=0;
volatile uint16_t calib_zero_temp_mv=324;

/*transmit single character*/
void UART_tx_single(unsigned char data){
while((UCSR0A & (1<<UDRE0))==0);
UDR0=data;
}

/*transmit string*/
void UART_tx(char c[]){
for(uint8_t i=0;i<strlen(c);i++){
  UART_tx_single(c[i]);
  }
}

/*transmit single digit integer number*/
void UART_tx_single_digit(uint8_t x){
UART_tx_single(x+48);
}

/*transmit positive integer number, 9 digit max*/
void UART_print_integer(uint32_t x){
uint32_t factor=100000000;
    if(x==0){UART_tx_single_digit(0);}
    else{
        while((x/factor)==0){
    	  factor=factor/10;
        }

        while(factor!=0){
    	  int print=(x/factor)%10;
    	  UART_tx_single_digit(print);
    	  factor=factor/10;
        }
      }
}

/*transmit any number, including negative and floating points*/
void UART_print_number(double x){

	if(x<0){UART_tx_single('-');x=-x;}
    uint32_t first=x;
    uint32_t second=x*1000.0-first*1000.0;
    UART_print_integer(first);

    if(second<1){}
    else if(second<10){
    	   UART_tx_single('.');
    	   UART_tx_single('0');UART_tx_single('0');
           UART_print_integer(second);
           }
    else if(second<100){
    	   UART_tx_single('.');
		   UART_tx_single('0');
		   if(second%10==0){second=second/10;}
		   UART_print_integer(second/100);
	       }
    else if(second<1000){
    	   UART_tx_single('.');
    	   if(second%100==0){second=second/100;}
    	   if(second%10==0){second=second/10;}
		   UART_print_integer(second);
	       }
}

/*Call this function to calibrate internal temperature
sensor. This function will only work if it is called
after adc initialization*/
void calibrate_temp_sensor(void){
while(calib_c<CALIB_CYCLE);
back0:
calib_zero_temp_mv=adc_raw-(CALIB_TEMP*TEMP_SLOPE);
_delay_ms(100);
while(temp!=CALIB_TEMP){goto back0;}

back1:
calib_zero_temp_mv=adc_raw-(CALIB_TEMP*TEMP_SLOPE);
_delay_ms(100);
while(temp!=CALIB_TEMP){goto back1;}

back2:
calib_zero_temp_mv=adc_raw-(CALIB_TEMP*TEMP_SLOPE);
_delay_ms(100);
while(temp!=CALIB_TEMP){goto back2;}
}

int main(void){

/*configure UART*/
uint16_t UBRR_VAL=(((F_CPU/16)/BAUD)-1);
UBRR0H=UBRR_VAL>>8;
UBRR0L=UBRR_VAL;
UCSR0B=(1<<RXEN0)|(1<<TXEN0);
UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);

/*configure ADC*/
ADMUX |=(1<<MUX3);
ADMUX |=(1<<REFS0)|(1<<REFS1);
ADCSRA|=(1<<ADPS1)|(1<<ADPS2);
ADCSRA|=(1<<ADIE) |(1<<ADEN) |(1<<ADSC);
sei();

calibrate_temp_sensor();

while(1){
           
		   UART_tx("CPU temperature = ");
           UART_print_number(temp);
		   UART_tx("'C");
		   UART_tx("\r\n");
		   _delay_ms(500);
		   
		   
		   
		   
		   }
}


ISR(ADC_vect){

adc_raw=ADCW;
calib_c++;
if(calib_c>CALIB_CYCLE){calib_c=CALIB_CYCLE;}
temp = (adc_raw-calib_zero_temp_mv)/TEMP_SLOPE;
ADCSRA|=(1<<ADSC);

}

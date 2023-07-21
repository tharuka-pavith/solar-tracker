/*
 * Solar_Tracker.c
 *
 * Created: 7/20/2023 9:05:24 AM
 * Author : Tharuka
 */ 

#define F_CPU 16000000UL  //CPU CLK speed 8MHz
#define BAUD 9600 //BAUD rate for UART TX
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

// Define Servo angles and limits
int servoh = 0;
int servohLimitHigh = 160;
int servohLimitLow = 20;

int servov = 0;
int servovLimitHigh = 160;
int servovLimitLow = 20;

// Assigning LDR channels
int ldrtopl = 2; // top left LDR green
int ldrtopr = 1; // top right LDR yellow
int ldrbotl = 3; // bottom left LDR blue
int ldrbotr = 0; // bottom right LDR orange


void ADC_config(void);
void USART_init(void);

void display_ADC(int value);
int analogRead(int);

void transmit_USART(unsigned char data);

int get_optimum_angle(void);

int analogRead(int channel) {
	// Function to read the analog value from the specified channel
	// Implement the analog-to-digital conversion for your specific micro-controller
	// and return the 10-bit ADC value for the given channel.
	
	ADMUX = (ADMUX & 0x20) | (channel & 0x0F);
	ADCSRA |= (1 << ADSC); //start A/D conversion
	while (ADCSRA & (1 << ADSC)); //wait until conversion complete
	return ADC;
}

int main(void) {
	ADC_config();
	USART_init();
	
	int val = 0;
	while (1) {
		val = get_optimum_angle();
		display_ADC(val);
		_delay_ms(500);		

	}
	return 0;
}

void ADC_config(void)
{
	ADCSRA |= (1 << ADEN); //Enable ADC
	ADCSRA |= 0x07; //Set pre-scalar = 128
}

void USART_init(void)
{
	UBRR0H = (uint8_t) (UBRR_VALUE >> 8); //Set upper byte
	UBRR0L = (uint8_t) (UBRR_VALUE); //Set lower byte
	UCSR0B = (1 << TXEN0); //enable transmitter
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);
	//UCSR0C = (3 << UCSZ00); //set no. of data bits = 8
}

void transmit_USART(unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0))); //wait until transmit buffer is ready
	UDR0 = data; //put data to TX buffer
}

void display_ADC(int value)
{
	float voltage = 5 * value / 1000;
	
	int ones = 0;
	int tens = 0;
	int hundreds =0;
	int thousands =0;
	
	thousands = value/1000;
	hundreds = (value % 1000)/100;
	tens = (value%100)/10;
	ones = (value%10);

	
	transmit_USART('0' + thousands);
	transmit_USART('0' + hundreds);
	transmit_USART('0' + tens);
	transmit_USART('0' + ones);
	transmit_USART(0x0A);
	transmit_USART(0x0D);
	
}



int get_optimum_angle(void){
	int width = 50;
	//int optimum_angle = 0;
	
	int max_ADC = 0; //0 - 1023
	int max_index = 0; // index of the max_ADC in the array
	int ADC_array[] = {0,0,0,0} ; // [0,0,0,0]
	
	
	for (int i = 0; i < 4; i++)
	{
		ADC_array[i] = analogRead(i);
		if(ADC_array[i] > max_ADC)
		{
			max_index = i;
			max_ADC = ADC_array[i];
		}
	}
	
	
	int A1 = max_ADC;
	int A2;
	
	
	if (max_index == 0)
	{
		A2 = ADC_array[1];
		float x_bar = width * ( (A1 + 2 * A2) / (3.0 * (A1 + A2)) );
		return (15 + x_bar);
	}
	else if(max_index == 3)
	{
		A2 = ADC_array[2];
		float x_bar = width * ( (A1 + 2 * A2) / (3.0 * (A1 + A2)) );
		return (165 - x_bar);
	}
	else if (max_index == 1)
	{
		if (ADC_array[0] > ADC_array[2])
		{
			A2 = ADC_array[0];
			float x_bar = width * ( (A1 + 2 * A2) / (3.0 * (A1 + A2)) );
			return (65 - x_bar);
		}
		else
		{
			A2 = ADC_array[2];
			float x_bar = width * ( (A1 + 2 * A2) / (3.0 * (A1 + A2)) );
			return (65 + x_bar);	
		}
	}
	else
	{
		if (ADC_array[1] > ADC_array[3])
		{
			A2 = ADC_array[1];
			float x_bar = width * ( (A1 + 2 * A2) / (3.0 * (A1 + A2)) );
			return (115 - x_bar);
		}
		else
		{
			A2 = ADC_array[3];
			float x_bar = width * ( (A1 + 2 * A2) / (3.0 * (A1 + A2)) );
			return (115 + x_bar);
		}	
	}
}


/*
float get_optimum_angle(void) {
	int width = 50;
	int max_ADC = 0; // 0 - 1023
	int max_index = 0; // index of the max_ADC in the array
	int ADC_array[] = {0, 0, 0, 0}; // [0, 0, 0, 0]

	for (int i = 0; i < 4; i++) {
		ADC_array[i] = analogRead(i);
		if (ADC_array[i] > max_ADC) {
			max_index = i;
			max_ADC = ADC_array[i];
		}
	}

	int A1 = max_ADC;
	int A2;

	if (max_index == 0) {
		A2 = ADC_array[1];
		float x_bar = width * ((A1 + 2 * A2) / (3.0 * (A1 + A2)));
		return 15 + x_bar;
		} else if (max_index == 3) {
		A2 = ADC_array[2];
		float x_bar = width * ((A1 + 2 * A2) / (3.0 * (A1 + A2)));
		return 165 - x_bar;
		} else if (max_index == 1) {
		if (ADC_array[0] > ADC_array[2]) {
			A2 = ADC_array[0];
			float x_bar = width * ((A1 + 2 * A2) / (3.0 * (A1 + A2)));
			return 65 - x_bar;
			} else {
			A2 = ADC_array[2];
			float x_bar = width * ((A1 + 2 * A2) / (3.0 * (A1 + A2)));
			return 65 + x_bar;
		}
		} else {
		if (ADC_array[1] > ADC_array[3]) {
			A2 = ADC_array[1];
			float x_bar = width * ((A1 + 2 * A2) / (3.0 * (A1 + A2)));
			return 115 - x_bar;
			} else {
			A2 = ADC_array[3];
			float x_bar = width * ((A1 + 2 * A2) / (3.0 * (A1 + A2)));
			return 115 + x_bar;
		}
	}
}
*/
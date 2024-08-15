/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/08/2024 | Ejercicio 6 de l guía 1.                       |
 *
 * @author Irina E. Lauritto (irina.lauritto@ingenieri.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcdNumber)
{
	for( int i=0; i<digits; i++)
	{
		bcdNumber[i] = data%10;
		data = data/10;
	}
	return 1;
}

void BCDtoPin(gpioConf_t *vector, uint8_t dato){
	int MASK = 1;
	for(int i=0; i<sizeof(vector); i++)
		GPIOInit(vector[i].pin, vector[i].dir);
	for(int j=0; j<sizeof(vector); j++)
	{
		if((MASK&dato) != 0)
			GPIOOn(vector[j].pin);
		else
			GPIOOff(vector[j].pin);

		MASK = MASK << 1; //Corre los dígitos de la máscara un lugar a la izquierda.
	}
}
void showNumberInDisplay(uint32_t numer){
	
}
/*==================[external functions definition]==========================*/
void app_main(void){

}
/*==================[end of file]============================================*/
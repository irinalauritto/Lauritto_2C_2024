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
 * | 15/08/2024 | Ejercicio 5 de la guía 1.                      |
 *
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
// La función BCDtoPin utiliza un dato entero para modificar el vector de pines que luego nos servirá para encender un display de 7 segmentos, valiéndonos de un mux.
// La función de la máscara es comparar los valores de dicho dato. En la primer iteración comparará el dato con 0001, 
// en la segunda iteración lo comparará con 0010, así sucesivamente, hasta el 1000.
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
/*==================[external functions definition]==========================*/
void app_main(void){

	gpioConf_t vectorGPIO[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};
	uint8_t digitoBCD = 7;

	BCDtoPin(vectorGPIO, digitoBCD);

}
/*==================[end of file]============================================*/
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
 * | 15/08/2024 | Creación del ejercicio 6 de l guía 1.          |
 * | 29/08/2024 | Se actualiza ejercicio 6.						 |
 * 
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
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

/*! @fn int8_t convertToBcdArray
*	@brief Esta función escribe un dato de n digitos en un vector de tipo entero.
* 	@param 
*	@return
*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcdNumber)
{
	// en cero centena, en uno decena, en dos unidad
	for(int i=0; i<digits; i++)
	{
		bcdNumber[digits-1-i] = data%10;
		data = data/10;
	}
	return 1;
}

/*! @fn
*	@brief 
* 	@param
*	@return
*/
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

		MASK = MASK <<1 ; //Corre los dígitos de la máscara un lugar a la izquierda.
	}
}

/*! @fn
*	@brief 
* 	@param
*	@return
*/
void showNumberInDisplay(uint32_t numero, uint8_t numeroDeDigitos, gpioConf_t *pVectorPines, gpioConf_t *pVectorMUX ){
	uint8_t vectorAuxiliar[3];

	//Se separan los digitos del número y se guardan en vectorAuxiliar
	convertToBcdArray(numero, numeroDeDigitos, vectorAuxiliar);

	//Se inicializan el vector de pines del MUX
	for(uint8_t i = 0; i<numeroDeDigitos; i++)
	{
		GPIOInit(pVectorMUX[i].pin, pVectorMUX[i].dir);
	}

	//vectorAuxiliar[0] = 1;
	//vectorAuxiliar[1] = 2;
	//vectorAuxiliar[2] = 3;

	for(uint8_t i = 0; i<numeroDeDigitos; i++)
	{
		BCDtoPin(pVectorPines, vectorAuxiliar[i]);
		printf("%d", vectorAuxiliar[i]);
		printf("\n");
		GPIOOn(pVectorMUX[i].pin);
		GPIOOff(pVectorMUX[i].pin);
	}

}
/*==================[external functions definition]==========================*/

/*! @fn
*	@brief
*   @param
*	@return 
*/
void app_main(void){
	uint32_t dato = 123;
	uint8_t digitos = 3;

    gpioConf_t vectorPIN[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};
	gpioConf_t vectorMUX[3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};

	showNumberInDisplay(dato, digitos, vectorPIN, vectorMUX);

}
/*==================[end of file]============================================*/
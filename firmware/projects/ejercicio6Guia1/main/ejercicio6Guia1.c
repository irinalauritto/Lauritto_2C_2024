/*! @mainpage Guía n° 1 de Electrónica Programable. Ejercicio n°6.
 *
 * @section genDesc General Description
 *
 * 	Este programa contiene el código  principal para visualizar un 
 *  número de 3 dígitos en un display utilizando un ESP32.  
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
 * | 29/08/2024 | Se actualizan los comentarios y se genera do-  |
 * |			| -cumentación.									 |
 * 
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/

/*! @brief Inclusión de las librerías necesarias para el proyecto. */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/


/*==================[internal data definition]===============================*/
/*! @struct gpioConf_t
 *  @brief Estructura para configurar los pines GPIO.
 *  
 *  @var gpioConf_t::pin
 *  Número del pin GPIO.
 *  
 *  @var gpioConf_t::dir
 *  Dirección del GPIO: '0' para IN, '1' para OUT.
 */
typedef struct
{
	gpio_t pin;			
	io_t dir;			
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/*! @fn int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t * bcdNumber)
 *  @brief Convierte un número entero en su representación en BCD.
 *  @param data Número entero con el cual se trabajará.
 *  @param digits Cantidad de dígitos del número.
 *  @param bcdNumber Puntero al vector donde se almacenarán los dígitos del mismo.
 *  @return 1 si la conversión fue exitosa.
 */
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcdNumber)
{
	for(int i=0; i<digits; i++)
	{
		bcdNumber[digits-1-i] = data%10;
		data = data/10;
	}
	return 1;
}

/*! @fn void BCDtoPin(gpioConf_t *vector, uint8_t dato)
 *  @brief Envía un valor en BCD a un conjunto de pines.
 *  @param vector Puntero a un array de configuraciones de pines GPIO.
 *  @param dato Valor en BCD a mostrar.
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

/*! @fn void showNumberInDisplay(uint32_t numero, uint8_t numeroDeDigitos, gpioConf_t *pVectorPines, gpioConf_t *pVectorMUX)
 *  @brief Muestra un número en el display.
 *  @param numero Número entero de hasta 3 dígitos a mostrar.
 *  @param numeroDeDigitos Cantidad de dígitos a mostrar.
 *  @param pVectorPines Puntero a un array de configuraciones de pines GPIO para el display.
 *  @param pVectorMUX Puntero a un array de configuraciones de pines GPIO para el multiplexor.
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

	for(uint8_t i = 0; i<numeroDeDigitos; i++)
	{
		BCDtoPin(pVectorPines, vectorAuxiliar[i]);

		//Se muestran en la consola los valores que se hallan en vectorAuxiliar 
		printf("%d", vectorAuxiliar[i]);
		printf("\n");

		GPIOOn(pVectorMUX[i].pin);
		GPIOOff(pVectorMUX[i].pin);
	}
}
/*==================[external functions definition]==========================*/

/*! @fn void app_main(void)
 *  @brief Función principal del programa.
 *  Inicializa el sistema y muestra un número en el display utilizando los pines GPIO configurados.
 */
void app_main(void){
	uint32_t dato = 123;
	uint8_t digitos = 3;

    gpioConf_t vectorPIN[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};
	gpioConf_t vectorMUX[3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};

	showNumberInDisplay(dato, digitos, vectorPIN, vectorMUX);
}
/*==================[end of file]============================================*/
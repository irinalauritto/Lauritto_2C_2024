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
 * | 15/08/2024 |Ejercicio 4 de la guia 1.                       |
 *
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcdNumber)
{
	for( int i=0; i<digits; i++)
	{
		bcdNumber[i] = data%10;
		data = data/10;
	}
	return 1;
}


void app_main(void){
	//Definción del vector en el cual la función devolvera el número.
	uint8_t vectorNumeros[3]; 
	//Definición del dato.
	uint32_t dato = 123;
	//Definición de la variable que almacena el número de dígitos del dato.
	uint8_t digitos = 3;

	convertToBcdArray(dato, digitos, vectorNumeros);

	//Escritura de los datos en la consola:
	printf("UNIDAD: \n");
	printf("%d", vectorNumeros[0]);
	printf("\n");

	printf("DECENA: \n");
	printf("%d", vectorNumeros[1]);
	printf("\n");

	printf("CENTENA: \n");
	printf("%d", vectorNumeros[2]);
	printf("\n");



}
/*==================[end of file]============================================*/
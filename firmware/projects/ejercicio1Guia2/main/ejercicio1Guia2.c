/*! @mainpage ejercicio1Guia2
 *
 * @section genDesc General Description
 *
 * Este programa mide la distancia utilizando un sensor ultrasónico HC-SR04 y muestra
 * los resultados en una pantalla LCD y mediante el encendido de LEDs. También permite
 * detener la medición y mantener la última distancia medida usando dos teclas.
 *
 * <a href="https://docs.google.com/document/d/1KIdCYkyOgaSFXYuy92HePZLI5JyXhbb5lqL7ty_q0kM/edit">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	Trigger	 	| 	GPIO_2		|
 * | 	+5V 	 	|    +5V 		|
 * | 	 GND	 	| 	 GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/09/2024 | Se crea el documento y se comienza a realizar las actividades de la guía.	                         |
 * | 08/09/2024 | Se documenta el código.						 |
 * 
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h" 
#include "lcditse0803.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
/*! @brief Define el retardo entre mediciones de distancia (en milisegundos). */
#define RETARDO_EN_MEDICION 1000

/*! @brief Define el retardo entre la detección de teclas (en milisegundos). */
#define RETARDO_EN_DETECCION_DE_TECLAS 100

/*! @brief Define el retardo entre actualizaciones de pantalla y LEDs (en milisegundos). */
#define RETARDO_EN_MOSTRAR 500

/*==================[internal data definition]===============================*/
/*! @brief Handle para la tarea de medir distancia. */
TaskHandle_t medirTaskHandle = NULL;

/*! @brief Handle para la tarea de detectar teclas. */
TaskHandle_t detectarTeclasTaskHandle = NULL;

/*! @brief Handle para la tarea de mostrar la medición. */
TaskHandle_t mostrarMedicionTaskHandle = NULL;

/*! @brief Variable que almacena la distancia medida en centímetros. */
uint16_t distancia;

/*! @brief Bandera que indica si el sistema de medición está encendido o apagado. */
bool ENCENDIDO = 0;

/*! @brief Bandera que indica si se debe mantener el valor de la medición en pantalla. */
bool HOLD = 1; 
/*==================[internal functions declaration]=========================*/



/*!
 *  @brief Función que mide la distancia utilizando el sensor HC-SR04.
 * 
 * Esta función se ejecuta continuamente mientras el sistema esté encendido
 * y actualiza la variable global "distancia" con el valor medido en centímetros.
 * 
 * @param pvParameter Parámetro de FreeRTOS (no utilizado).
 */
static void medir(void *pvParameter){
    while(true){
        if(ENCENDIDO)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(RETARDO_EN_MEDICION/portTICK_PERIOD_MS);

    }
}

/*!
 * @brief Función que detecta la pulsación de teclas.
 * 
 * Esta función verifica el estado de las teclas cada cierto tiempo. La tecla 1 (TEC1)
 * activa o desactiva la medición ("enciende" o "apaga"), y la tecla 2 (TEC2) activa o desactiva la función "HOLD".
 * 
 * @param pvParameter Parámetro de FreeRTOS (no utilizado).
 */
static void detectarTeclas(void *pvParameter){
	uint8_t teclas;
    while(true){
        teclas  = SwitchesRead();
		printf("tecla%d\r\n", teclas);
    	switch(teclas){
    		case SWITCH_1:
    			ENCENDIDO = !ENCENDIDO;
    		break;

    		case SWITCH_2:
    			HOLD = !HOLD;
    		break;
  	  }
	
	vTaskDelay(RETARDO_EN_DETECCION_DE_TECLAS/portTICK_PERIOD_MS);

	}
}

/*!
 * @brief Función que controla el encendido y apagado de los LEDs según la distancia medida.
 * 
 * Enciende y apaga los LEDs de acuerdo a los siguientes rangos de distancia:
 * - Menor a 10 cm: Apaga todos los LEDs.
 * - Entre 10 y 20 cm: Enciende LED_1.
 * - Entre 20 y 30 cm: Enciende LED_1 y LED_2.
 * - Mayor a 30 cm: Enciende LED_1, LED_2 y LED_3.
 * 
 * @param dato Distancia medida en centímetros.
 */
void manejarLEDS(uint16_t dato){
	if(dato<=10)
	{
		LedsOffAll();
	}
	else{
		if(10<dato && dato<=20)
		{
			LedOn(LED_1);	
			LedOff(LED_2);
			LedOff(LED_3);		
		}
		else{
			if (20<dato && dato<=30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else{
				if(distancia>30)
				{
					LedOn(LED_1);
					LedOn(LED_2);
					LedOn(LED_3);
				}
			}
		}
	}
}

/*!
 * @brief Función que muestra la medición en la pantalla LCD y controla los LEDs.
 * 
 * Esta función se ejecuta periódicamente para actualizar la pantalla y los LEDs con
 * la distancia medida. Si la función "HOLD" está activa, se mantiene el último valor
 * mostrado en pantalla.
 * 
 * @param pvParameter Parámetro de FreeRTOS (no utilizado).
 */
static void mostrarMedicion(void *pvParameter){
    while(true){
        if(ENCENDIDO)
		{
			manejarLEDS(distancia);
			if(HOLD)
			{
				LcdItsE0803Write(distancia);
			}
		}
		else
		{
			LedsOffAll();
			LcdItsE0803Off();
		}
		vTaskDelay(RETARDO_EN_MOSTRAR/portTICK_PERIOD_MS);
    }
}


/*==================[external functions definition]==========================*/

/*!
 * @brief Función principal del programa.
 * 
 * Inicializa el hardware (LEDs, teclas, sensor HC-SR04 y pantalla LCD) y crea las
 * tareas de FreeRTOS para medir la distancia, detectar teclas y mostrar la medición.
 */
void app_main(void){
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	
    xTaskCreate(&medir, "Medir", 2048, NULL, 5, &medirTaskHandle);
    xTaskCreate(&detectarTeclas, "DetectarTeclas", 2048, NULL, 5, &detectarTeclasTaskHandle);
    xTaskCreate(&mostrarMedicion, "MostarMedicion", 2048, NULL, 5, &mostrarMedicionTaskHandle);
}
/*==================[end of file]============================================*/
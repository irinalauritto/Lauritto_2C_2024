/*! @mainpage Examen Parcial
 *
 * @section genDesc Descripción General:
 * 
 * 		En este proyecto se realiza el código para la implementacion de un dispositivo basado en la ESP-EDU que permita detectar eventos peligrosos para ciclistas.
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
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Irina Esmeralda Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h" 
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/*! @brief Define el retardo entre mediciones de distancia (en microsegundos). */
#define RETARDO_EN_MEDICION 500000

#define RETARDO_ALARMA_PELIGRO

#define RETARDO_ALARMA_PRECACIÓN

/*==================[internal data definition]===============================*/
/*! @brief Handle para la tarea de medir distancia. */
TaskHandle_t medirTaskHandle = NULL;

/*! @brief Variable que almacena la distancia medida en centímetros. */
uint16_t distancia;

bool ALARMA_1_SEGUNDO = false;

bool ALARMA_05_SEGUNDO = false;

typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/

/*!
 * @brief Función de interrupción para el timer que activa la tarea de medición.
 * 
 * Utiliza la función `vTaskNotifyGiveFromISR()` para notificar a la tarea de medición que debe ejecutarse.
 * 
 * @param param Parámetro de la función, no utilizado en este caso.
 */
void funcTimerMedir(void* param){
	vTaskNotifyGiveFromISR(medirTaskHandle, pdFALSE);
}
/*!
 * @brief Función que controla el encendido y apagado de los LEDs según la distancia medida.
 * 
 * Enciende y apaga los LEDs de acuerdo a los siguientes rangos de distancia:
 * - Mayor a 500 cm: Enciende LED_1.
 * - Mayor a 300 cm y menor a 500 cm: Enciende LED_1 y LED_2.
 * - Menor a 300 cm: Enciende LED_1, LED_2 y LED_3.  
 * 
 * 
 * @param dato Distancia medida en centímetros.
 */
void manejarLEDS(uint16_t dato){
	
	if(dato>500)
		{
			LedOn(LED_1);	
			LedOff(LED_2);
			LedOff(LED_3);		
		}
		else{
			if (dato>=300 && dato<=500)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else{
				if(distancia<300)
				{
					LedOn(LED_1);
					LedOn(LED_2);
					LedOn(LED_3);
				}
		}
	}
}

/*!
 * @brief Función que controla el encendido y apagado de un Buzzer según la distancia medida.
 * 
 * Enciende y apaga los el GPIO que controla la alarma con la frecuencia establecida de acuerdo a los siguientes rangos de distancia:
 * - Mayor a 300 cm y menor a 500 cm: Enciende alarma que suena cada 1 segundo.
 * - Menor a 300 cm: Enciende alarma que suena cada 0.5 segundos. 
 * 
 * @param dato Distancia medida en centímetros.
 */
void manejarAlarma(uint16_t dato){
	if(dato>500){
		ALARMA_05_SEGUNDO = false;
		ALARMA_1_SEGUNDO = false;
	}
	else{
		if (dato>=300 && dato<=500)
				{
					ALARMA_1_SEGUNDO = true;
					ALARMA_05_SEGUNDO = false;
				}
				else{
					if(distancia<300)
					{
						ALARMA_05_SEGUNDO = true;
						ALARMA_1_SEGUNDO = false;
					}
				}
	}
}


/*!
 *  @brief Función que mide la distancia utilizando el sensor HC-SR04.
 * 
 * Esta función se ejecuta continuamente mientras el sistema esté encendido
 * y actualiza la variable global "distancia" con el valor medido en centímetros.
 * 
 * @param pvParameter Parámetro de FreeRTOS.
 */
static void medir(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
		
		distancia = HcSr04ReadDistanceInCentimeters();
		manejarLEDS(distancia);
		manejarAlarma(distancia);

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
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	gpioConf_t pin = {{GPIO_20, GPIO_OUTPUT}};
	
	// Inicialización de Timers
    timer_config_t timerMedir = {
        .timer = TIMER_A,
        .period = RETARDO_EN_MEDICION,
        .func_p = funcTimerMedir,
        .param_p = NULL
    };
	TimerInit(&timerMedir);

	xTaskCreate(&medir, "Medir", 2048, NULL, 5, &medirTaskHandle);


	TimerStart(timerMedir.timer);


}
/*==================[end of file]============================================*/
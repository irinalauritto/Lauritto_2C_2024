/*! @mainpage ejercicio3Guia2
 *
 * @section genDesc General Description
 * 
 * Este programa mide la distancia utilizando un sensor ultrasónico HC-SR04 y muestra
 * los resultados en una pantalla LCD y mediante el encendido de LEDs. También permite
 * detener la medición y mantener la última distancia medida usando dos teclas físicas y
 * comandos enviados por el puerto serie desde la PC. El programa utiliza interrupciones 
 * para los temporizadores y la detección de teclas.
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
 * | 19/09/2024 | Se crea el documento y se comienza a realizar las actividades de la guía.	                         |
 * 
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/

/*!
* @brief Inclusión de librerías.
*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h" 
#include "lcditse0803.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include <string.h>
/*==================[macros and definitions]=================================*/

/*! @brief Define el retardo entre mediciones de distancia (en microsegundos). */
#define RETARDO_EN_MEDICION 100000

/*! @brief Define el retardo entre actualizaciones de pantalla y LEDs (en microsegundos). */
#define RETARDO_EN_MOSTRAR 500000

/*==================[internal data definition]===============================*/
/*! @brief Handle para la tarea de medir distancia. */
TaskHandle_t medirTaskHandle = NULL;

/*! @brief Handle para la tarea de mostrar la medición. */
TaskHandle_t mostrarMedicionTaskHandle = NULL;

/*! @brief Handle para la tarea de mostrar la medición en la PC. */
TaskHandle_t mostrarMedicionPCTaskHandle = NULL;

/*! @brief Variable que almacena la distancia medida en centímetros. */
uint16_t distancia;

/*! @brief Bandera que indica si el sistema de medición está encendido o apagado. */
bool ENCENDIDO = 0;

/*! @brief Bandera que indica si se debe mantener el valor de la medición en pantalla. */
bool HOLD = 1; 
/*==================[internal functions declaration]=========================*/

/*!
 * @brief Función que detecta la pulsación de teclas enviadas desde la PC.
 * 
 * Esta función utiliza el puerto serie para leer las teclas 'O' y 'H' enviadas desde la PC
 * - Tecla 'O' activa o desactiva la medición.
 * - Tecla 'H' activa o desactiva la función "HOLD".
 * 
 * @param param Parámetro de FreeRTOS (no utilizado).
 */
void detectarTeclasPC(void *param)
{
	uint8_t tecla;
	UartReadByte(UART_PC, &tecla);

	switch (tecla)
	{
		case 'O':
			ENCENDIDO = !ENCENDIDO;
			//Muestro la tecla enviada para validar funcionamiento
			UartSendByte(UART_PC, (char*)&tecla);
			break;
	
		case 'H':
			HOLD = !HOLD;
			//Muestro la tecla enviada para validar funcionamiento
			UartSendByte(UART_PC, (char*)&tecla);
			break;
	}
}

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
 * @brief Función de interrupción para el timer que activa la tarea de mostrar la medición.
 * 
 * Utiliza la función `vTaskNotifyGiveFromISR()` para notificar a la tarea de mostrar que debe ejecutarse.
 * 
 * @param param Parámetro de la función, no utilizado en este caso.
 */
void funcTimerMostrar(void* param){
	vTaskNotifyGiveFromISR(mostrarMedicionTaskHandle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrarMedicionPCTaskHandle, pdFALSE);
}

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
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
        if(ENCENDIDO)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
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
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
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
    }
}

/*!
 * @brief Función que envía la medición al puerto serie para ser mostrada en la PC.
 * 
 * Envía la distancia medida en formato de texto con la unidad "cm" y una nueva línea al puerto serie.
 * 
 * @param pvParameter Parámetro de FreeRTOS (no utilizado).
 */
static void mostrarMedicionPC(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
		UartSendString(UART_PC, (char*)UartItoa(distancia, 10));
		UartSendString(UART_PC, " ");
		UartSendString(UART_PC, "cm");
		UartSendString(UART_PC, "\r\n");
    }
}


/*!
 * @brief Función de interrupción para la tecla 1 (TEC1).
 * 
 * Esta función alterna la bandera `ENCENDIDO`, que controla si el sistema de medición está activado o no.
 */
void controlInterrupcionSwitch1(){
	ENCENDIDO = !ENCENDIDO;
}

/*!
 * @brief Función de interrupción para la tecla 2 (TEC2).
 * 
 * Esta función alterna la bandera `HOLD`, que controla si se debe mantener el valor de la medición en pantalla.
 */
void controlInterrupcionSwitch2(){
	HOLD = !HOLD;
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

	// Inicialización de Timers
    timer_config_t timerMedir = {
        .timer = TIMER_A,
        .period = RETARDO_EN_MEDICION,
        .func_p = funcTimerMedir,
        .param_p = NULL
    };
	TimerInit(&timerMedir);

	timer_config_t timerMostrar = {
        .timer = TIMER_B,
        .period = RETARDO_EN_MOSTRAR,
        .func_p = funcTimerMostrar,
        .param_p = NULL
    };
	TimerInit(&timerMostrar);
	
	// Inicialización del puerto serie
	serial_config_t myUart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = detectarTeclasPC,
		.param_p = NULL,
	};
	UartInit(&myUart);


	// Tareas para medición y muestra de datos
    xTaskCreate(&medir, "Medir", 2048, NULL, 5, &medirTaskHandle);
    xTaskCreate(&mostrarMedicion, "MostarMedicion", 2048, NULL, 5, &mostrarMedicionTaskHandle);
	xTaskCreate(&mostrarMedicionPC, "MostarMedicionPC", 2048, NULL, 5, &mostrarMedicionPCTaskHandle);

	// Interrupciones de teclas
	SwitchActivInt(	SWITCH_1, controlInterrupcionSwitch1, NULL);
	SwitchActivInt(	SWITCH_2, controlInterrupcionSwitch2, NULL);

	// Inicialización del conteo de timers 
    TimerStart(timerMedir.timer);
    TimerStart(timerMostrar.timer);

}
/*==================[end of file]============================================*/



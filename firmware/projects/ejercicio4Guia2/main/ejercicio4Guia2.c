/*! @mainpage ejercicio4Guia2
 *
 * @section genDesc General Description
 *
 * Este programa utiliza un conversor analógico-digital (ADC) y un conversor digital-analógico (DAC) en el ESP32
 * para leer señales de entrada analógica y generar una señal de ECG a partir de una tabla de datos almacenada.
 * Además, envía los datos de conversión ADC al puerto serie para ser visualizados.
 *
 * <a href="https://docs.google.com/document/d/1KIdCYkyOgaSFXYuy92HePZLI5JyXhbb5lqL7ty_q0kM/edit">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
  |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	+5V 	 	|    +5V 		|
 * | 	 GND	 	| 	 GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/10/2024 | Creación del documento.                        |
 *
 * @author Irina E. Lauritto (irina.lauritto@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
/*!
 * @brief Inclusión de las librerías estándar y drivers específicos para ADC, DAC, UART y Timers.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
/*! @brief Define el retardo entre conversiones del ADC (en microsegundos). */
#define RETARDO_EN_CONVERSION_AD 20000

/*! @brief Define el retardo entre conversiones del DAC (en microsegundos). */
#define RETARDO_EN_CONVERSION_DA 40000

/*! @brief Tamaño del buffer que contiene los datos de la señal ECG. */
#define BUFFER_SIZE 231

/*! @brief Handle para la tarea de la coversion AD */
TaskHandle_t conversionADTaskHandle = NULL;

/*! @brief Handle para la tarea de la coversion AD */
TaskHandle_t conversionDATaskHandle = NULL;

/*! @brief Variable para almacenar el valor de la conversión ADC. */
uint16_t datoConversionAD;


/*==================[internal data definition]===============================*/
/*!
 * @brief Buffer que contiene los datos de la señal ECG.
 *
 * Este array contiene 231 muestras de la señal ECG que se utilizarán para la conversión
 * digital-analógica (DAC) y su posterior visualización.
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
/*!
 * @brief Función de interrupción para el timer que activa la conversión ADC.
 * 
 * Esta función notifica a la tarea de conversión ADC que debe ejecutarse mediante
 * la función `vTaskNotifyGiveFromISR()`.
 *
 * @param param Parámetro de la función, no utilizado.
 */
void funcTimerConversionAD(void* param){
	vTaskNotifyGiveFromISR(conversionADTaskHandle, pdFALSE);
}

/*!
 * @brief Función de interrupción para el timer que activa la conversión DAC.
 * 
 * Notifica a la tarea de conversión DAC que debe ejecutarse utilizando la función 
 * `vTaskNotifyGiveFromISR()`.
 *
 * @param param Parámetro de la función, no utilizado.
 */
void funcTimerConversionDA(void* param){
	vTaskNotifyGiveFromISR(conversionDATaskHandle, pdFALSE);
}

/*!
 * @brief Función de conversión analógico-digital (ADC).
 *
 * Esta función se ejecuta cada vez que el timer lo notifica. Lee el valor analógico de entrada
 * en el canal CH1 y lo envía al puerto serie para ser visualizado en la PC.
 *
 * @param pvParameter Parámetro de FreeRTOS (no utilizado).
 */
static void convertirAD(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
        AnalogInputReadSingle(CH1, &datoConversionAD);
        
        UartSendString(UART_PC, ">ECG: ");
		UartSendString(UART_PC, (char*)UartItoa(datoConversionAD, 10));
		UartSendString(UART_PC, "\r\n");
    }
}

/*!
 * @brief Función de conversión digital-analógica (DAC).
 *
 * Esta función se ejecuta periódicamente para escribir los valores de la señal ECG
 * en el DAC, generando una señal analógica. Recorre el buffer de la señal ECG y lo 
 * reproduce continuamente.
 *
 * @param pvParameter Parámetro de FreeRTOS (no utilizado).
 */
static void convertirDA(void *pvParameter){
    uint8_t i = 0;
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		if(i<231)
		{
			AnalogOutputWrite(ecg[i]);
			i++;
		}
		else
		{
			i = 0; // Reinicia el índice para repetir la señal ECG
		}
    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
// Inicialización de Timer
    timer_config_t timerConversionAD = {
        .timer = TIMER_A,
        .period = RETARDO_EN_CONVERSION_AD,
        .func_p = funcTimerConversionAD,
        .param_p = NULL
    };
	TimerInit(&timerConversionAD);

	timer_config_t timerConversionDA = {
        .timer = TIMER_B,
        .period = RETARDO_EN_CONVERSION_DA,
        .func_p = funcTimerConversionDA,
        .param_p = NULL
    };
	TimerInit(&timerConversionDA);

// Tareas para medición y muestra de datos
    xTaskCreate(&convertirAD, "convertir", 2048, NULL, 5, &conversionADTaskHandle);
    xTaskCreate(&convertirDA, "convertirDA", 2048, NULL, 5, &conversionDATaskHandle);


// Inicialización del Convertidor AD
	analog_input_config_t convertidorAD = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&convertidorAD);
	AnalogOutputInit();

// Inicialización del puerto serie
	serial_config_t myUart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&myUart);

// Inicialización del conteo de timers 
    TimerStart(timerConversionAD.timer);
	TimerStart(timerConversionDA.timer);
}
/*==================[end of file]============================================*/
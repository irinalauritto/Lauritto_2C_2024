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
 * | 	GND	 		| 	GND			|
 * |    +5V			| 		+5V		|
 * |	X_Acelerometro|		CH1		|
 * |	Y_Acelerometro|		CH2		|
 * |	Z_Acelerometro|		CH3		|
 * |	Tx			  |     GPIO_16	|
 * |	Rx			| 		GPIO_17	|
 * |	Buzzer		|		GPIO_20	|
 * 
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
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
/*! @brief Define el retardo entre mediciones de distancia (en microsegundos). */
#define RETARDO_EN_MEDICION 500000

#define RETARDO_ALARMA_PELIGRO 500000

#define RETARDO_ALARMA_PRECAUCIÓN 1000000

/*==================[internal data definition]===============================*/
/*! @brief Handle para la tarea de medir distancia. */
TaskHandle_t medirTaskHandle = NULL;

/*! @brief Handle para la tarea que envia las notificaciones mediante UART. */
TaskHandle_t notificarTaskHandle = NULL;

/*! @brief Handle para la tarea que genera la alarma de precaución . */
TaskHandle_t generarAlarmaPrecaucionTaskHandle = NULL;

/*! @brief Handle para la tarea que genera la alarma de peligro . */
TaskHandle_t generarAlarmaPeligroTaskHandle = NULL;

/*! @brief Handle para la tarea que analiza caidas . */
TaskHandle_t analizarCaidaTaskHandle = NULL;

/*! @brief Variable que almacena la distancia medida en centímetros. */
uint16_t distancia;

/*! @brief Bandera para la alarma de Precaución. */
bool ALARMA_1_SEGUNDO = false;

/*! @brief Bandera para la alarma de Peligro. */
bool ALARMA_05_SEGUNDO = false;

/*! @brief Bandera para las caídas. */
bool CAIDA = true;

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
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t miGPIO = {{GPIO_20, GPIO_OUTPUT}};

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
					if(dato<300)
					{
						ALARMA_05_SEGUNDO = true;
						ALARMA_1_SEGUNDO = false;
					}
				}
	}
}

/*!
 *  @brief Función que envia notificaciones a la UART.
 * 
 * Esta función se ejecuta siempre que se realicen mediciones.
 * Envia notificaciones cuando un vehiculo se encuentra a una distancia menor a  5 metros del ciclista.
 * 
 * @param pvParameter Parámetro de FreeRTOS.
 */
static void notificar(void *pvParameter){
	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (distancia>=300 && distancia<=500)
		{
			UartSendString(UART_CONNECTOR, "Precaucion, vehículo cerca.");
		}
		
		if(distancia<300)
		{
			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca.");

		}

		if(CAIDA)
		{
			UartSendString(UART_CONNECTOR, "Caída detectada.");

		}
	}
}

/*!
 *  @brief Función que genera alarma de peligro.
 * 
 * @param pvParameter Parámetro de FreeRTOS.
 */
static void generarAlarmaPeligro(void *pvParameter)
{
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   
		if(ALARMA_05_SEGUNDO){
			GPIOToggle(miGPIO.pin);
		}
	}
}

/*!
 *  @brief Función que genera alarma de precaucion.
 * 
 * @param pvParameter Parámetro de FreeRTOS.
 */
static void generarAlarmaPrecaucion(void *pvParameter)
{
	while(true)
	{  
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   
		if(ALARMA_1_SEGUNDO){
			GPIOToggle(miGPIO.pin);
		}
	}
}

/*!
 *  @brief Función que genera analiza si el ciclista se ha caído.
 * 
 * @param pvParameter Parámetro de FreeRTOS.
 */
static void analizarCaida(void *pvParameter){
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
		uint16_t medicionX;
		uint16_t medicionY;
		uint16_t medicionZ;

		AnalogInputReadSingle(CH1, &medicionX);
		AnalogInputReadSingle(CH2, &medicionY);		
		AnalogInputReadSingle(CH3, &medicionZ);	

		//Se realiza el casteo de las variables para evitar truncamientos en cálculos siguientes. 
		//Ademas se divide por mil ya que estamos midiendo en mV y necesitamos las medidas en V.

		medicionX = (float)medicionX/1000;
		medicionY = (float)medicionY/1000;
		medicionZ =(float)medicionZ/1000;

		float GenX = (medicionX-1.65)/0.3;
		float GenY = (medicionY-1.65)/0.3;
		float GenZ = (medicionZ-1.65)/0.3;

		float GTotal = GenX+GenY+GenZ;

		if(GTotal>=4)
		{
			CAIDA = true;
		}
		else
		{
			CAIDA = false;	
		}
		  
	}
	
}
/*!
 *  @brief Función que mide la distancia utilizando el sensor HC-SR04.
 * 
 * Esta función se ejecuta continuamente mientras el sistema esté encendido
 * y actualiza la variable global "distancia" con el valor medido en centímetros.
 * Además realiza llamadas a las tareas que se encargan del análisis de caidas y de las notificaciones por puerto serie.
 * 
 * @param pvParameter Parámetro de FreeRTOS.
 */
static void medir(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
		
		distancia = HcSr04ReadDistanceInCentimeters();
		manejarLEDS(distancia);
		manejarAlarma(distancia);
		vTaskNotifyGiveFromISR(analizarCaidaTaskHandle, pdFALSE);
		vTaskNotifyGiveFromISR(notificarTaskHandle, pdFALSE);
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

	// Inicialización de Timers
    timer_config_t timerMedir = {
        .timer = TIMER_A,
        .period = RETARDO_EN_MEDICION,
        .func_p = funcTimerMedir,
        .param_p = NULL
    };
	TimerInit(&timerMedir);

    timer_config_t timerPeligro = {
        .timer = TIMER_B,
        .period = RETARDO_ALARMA_PELIGRO,
        .func_p = generarAlarmaPeligro,
        .param_p = NULL
    };
    TimerInit(&timerPeligro);

	  timer_config_t timerPrecaucion = {
        .timer = TIMER_C,
        .period = RETARDO_ALARMA_PRECAUCIÓN,
        .func_p = generarAlarmaPrecaucion,
        .param_p = NULL
    };
    TimerInit(&timerPrecaucion);

	//Inicialización UART
	serial_config_t myUart = {
		.port = UART_CONNECTOR,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL,
	};
	UartInit(&myUart);

	// Inicialización del Convertidor AD
	analog_input_config_t acelerometroX = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&acelerometroX);
	AnalogOutputInit();

	analog_input_config_t acelerometroY = {
		.input = CH2,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&acelerometroY);
	AnalogOutputInit();

	analog_input_config_t acelerometroZ = {
		.input = CH3,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&acelerometroZ);
	AnalogOutputInit();

	//Creación de Funciones
	xTaskCreate(&medir, "Medir", 2048, NULL, 5, &medirTaskHandle);
	xTaskCreate(&notificar, "Notificar", 2048, NULL, 5, &notificarTaskHandle);
	xTaskCreate(&generarAlarmaPeligro, "generarAlarmaPeligro", 2048, NULL, 5, &generarAlarmaPeligroTaskHandle);
	xTaskCreate(&generarAlarmaPrecaucion, "generarAlarmaPrecaucion", 2048, NULL, 5, &generarAlarmaPrecaucionTaskHandle);
	xTaskCreate(&analizarCaida, "analizarCaida", 2048, NULL, 5, &analizarCaidaTaskHandle);


	//Inicialización del conteo de Timers
	TimerStart(timerMedir.timer);
	TimerStart(timerPrecaucion.timer);
	TimerStart(timerPeligro.timer);
	
}
/*==================[end of file]============================================*/
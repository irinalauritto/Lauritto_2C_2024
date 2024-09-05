/*! @mainpage ejercicio1Guia2
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
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
#define RETARDO_EN_MEDICION 1000
#define RETARDO_EN_DETECCION_DE_TECLAS 100
#define RETARDO_EN_MOSTRAR 500


/*==================[internal data definition]===============================*/
// para qué son los handle
TaskHandle_t medirTaskHandle = NULL;
TaskHandle_t detectarTeclasTaskHandle = NULL;
TaskHandle_t mostrarMedicionTaskHandle = NULL;

uint16_t distancia;
bool ENCENDIDO = 0;
bool HOLD = 1; 
/*==================[internal functions declaration]=========================*/

static void medir(void *pvParameter){
    while(true){
        if(ENCENDIDO)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(RETARDO_EN_MEDICION/portTICK_PERIOD_MS);

    }
}

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
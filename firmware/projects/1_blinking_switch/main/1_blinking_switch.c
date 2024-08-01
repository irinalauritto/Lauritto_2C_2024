/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED1 500
#define CONFIG_BLINK_PERIOD_LED2 800
#define CONFIG_BLINK_PERIOD_LED3 300
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
    while(1)    {
    	teclas  = SwitchesRead();
		printf("tecla%d\r\n", teclas);
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1);
				LedOff(LED_2 | LED_3);
				vTaskDelay(CONFIG_BLINK_PERIOD_LED1 / portTICK_PERIOD_MS);
    		break;

    		case SWITCH_2:
    			LedToggle(LED_2);
				LedOff(LED_1 | LED_3);
				vTaskDelay(CONFIG_BLINK_PERIOD_LED2 / portTICK_PERIOD_MS);
    		break;

			case SWITCH_1|SWITCH_2:
    			LedToggle(LED_3);
				LedOff(LED_1 | LED_2);
				vTaskDelay(CONFIG_BLINK_PERIOD_LED3 / portTICK_PERIOD_MS);
    		break;

			//Debería apagar los LEDs, al igual que las lineas 45, 51, y 57. No lo hace.
			case (~SWITCH_1)|(~SWITCH_2):
				LedOff(LED_1 & LED_2 & LED_3);
    		break;
    	}
	    //LedToggle(LED_3);
		//vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

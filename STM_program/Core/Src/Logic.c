/*
 * Logic.c
 *
 *  Created on: Jan 13, 2026
 *      Author: robin
 */

#include "Logic_layer.h"
#include "gpio.h"

int execute_command(struct LogicInterface *cmd)
{

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	return 0;
}


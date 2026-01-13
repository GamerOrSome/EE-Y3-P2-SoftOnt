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
	char arg[128];
	for (int j = 0; j <128;j++)
	{
		arg[j] = cmd->arguments[j];
		if (cmd->arguments[j+1]=="\0") break;
	}
	printf(arg);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	return 0;
}


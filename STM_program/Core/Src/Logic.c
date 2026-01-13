/*
 * Logic.c
 *
 *  Created on: Jan 13, 2026
 *      Author: robin
 */

int execute_command(struct LogicInterface *cmd)
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	return 0;
}


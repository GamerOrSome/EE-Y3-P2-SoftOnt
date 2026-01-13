/*
 * API_func.h
 *
 *  Created on: 27 Nov 2025
 *      Author: jeremy
 */

#ifndef LOGIC_LAYER_H_
#define LOGIC_LAYER_H_

#include <string.h>

struct LogicInterface
{
    char function_name[15];
    int argument_len;
    char *arguments;
};
//functie prototype
int execute_command(struct LogicInterface* cmd);
#endif /* LOGIC_LAYER_H_ */

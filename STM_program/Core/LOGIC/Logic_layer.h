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
    char function_name[6];
    int num_arguments;
    char *arguments; 
};

#endif /* LOGIC_LAYER_H_ */

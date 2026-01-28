/**
  ******************************************************************************
  * @file           : Logic_layer.h
  * @brief          : Header voor Logic_layer.c file.
  *                   In dit bestand word de logic functie en struct gedefinieerd. 
  ******************************************************************************
  */

#ifndef LOGIC_LAYER_H_
#define LOGIC_LAYER_H_

#include <string.h>

/**
 Struct voor het opslaan van een commando.
*
* @param char         function_name[15] Naam van de functie/commando.
* @param int          arguments_len Lengte van de argumenten string.
* @param char         *arguments Pointer naar de argumenten string.
* @return             Statuscode (0 = succes, anders fout).
*
*/

struct LogicInterface
{
    char function_name[15];
    int argument_len;
    char *arguments;
};

/**
 * //configuratie doorsturen naar de parser en hieruit de juiste api's uitvoeren.
 *
 * @param struct LogicInterface* cmd Pointer naar de LogicInterface struct met commando informatie.
 * @return int Statuscode (0 = succes, anders fout).
 */

int execute_command(struct LogicInterface* cmd);
#endif /* LOGIC_LAYER_H_ */

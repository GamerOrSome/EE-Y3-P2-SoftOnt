/*
 * logic_layer.c
 *
 * Created on: 27 Nov 2025
 * Author: jeremy
 * 
 * Beschrijving: Logic layer voor script parsing en executie
 * Functie: Parset script commando's en roept corresponderende API functies aan
 */

#include "logic_layer.h"
#include "API_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Defines voor max waarden
#define MAX_ARGS 12
#define MAX_TOKEN_LEN 50
#define MAX_COLORS 15

// Enum voor kleuren (makkelijker dan string compare)
typedef enum
{
    COLOR_ZWART = 0,
    COLOR_BLAUW,
    COLOR_LICHTBLAUW,
    COLOR_GROEN,
    COLOR_LICHTGROEN,
    COLOR_CYAAN,
    COLOR_LICHTCYAAN,
    COLOR_ROOD,
    COLOR_LICHTROOD,
    COLOR_MAGENTA,
    COLOR_LICHTMAGENTA,
    COLOR_BRUIN,
    COLOR_GEEL,
    COLOR_GRIJS,
    COLOR_WIT
} COLOR_TYPE;

// Struct voor color mapping
typedef struct
{
    char name[20];
    int value;
} ColorMap;

// Global color lookup table
static const ColorMap color_table[MAX_COLORS] = 
{
    {"zwart", COLOR_ZWART},
    {"blauw", COLOR_BLAUW},
    {"lichtblauw", COLOR_LICHTBLAUW},
    {"groen", COLOR_GROEN},
    {"lichtgroen", COLOR_LICHTGROEN},
    {"cyaan", COLOR_CYAAN},
    {"lichtcyaan", COLOR_LICHTCYAAN},
    {"rood", COLOR_ROOD},
    {"lichtrood", COLOR_LICHTROOD},
    {"magenta", COLOR_MAGENTA},
    {"lichtmagenta", COLOR_LICHTMAGENTA},
    {"bruin", COLOR_BRUIN},
    {"geel", COLOR_GEEL},
    {"grijs", COLOR_GRIJS},
    {"wit", COLOR_WIT}
};

/*
 * Function: parse_color
 * Returnvalue: int (color code)
 * Arguments: char *color_str - string met kleurnaam
 * Beschrijving: Converteert kleurnaam naar kleurcode
 */
int parse_color(char *color_str)
{
    unsigned int i;
    
    // Converteer naar lowercase voor case-insensitive vergelijking
    for (i = 0; color_str[i]; i++)
        color_str[i] = tolower(color_str[i]);
    
    // Zoek kleur in lookup table
    for (i = 0; i < MAX_COLORS; i++)
    {
        if (strcmp(color_str, color_table[i].name) == 0)
            return color_table[i].value;
    }
    
    return COLOR_ZWART; // Default kleur bij niet gevonden
}

/*
 * Function: parse_script_line
 * Returnvalue: int (0 = success, error code otherwise)
 * Arguments: 
 *   - char *line: input script regel
 *   - struct LogicInterface *cmd: output parsed commando
 * Beschrijving: Parset een script regel naar LogicInterface struct
 * Geheugengebruik: Alloceert geheugen voor arguments string
 */
int parse_script_line(char *line, struct LogicInterface *cmd)
{
    char *token;
    char temp_line[256];
    int arg_count = 0;
    char args_buffer[256] = "";
    
    if (line == NULL || cmd == NULL)
        return -1;
    
    // Kopieer input string (strtok wijzigt origineel)
    strncpy(temp_line, line, sizeof(temp_line) - 1);
    temp_line[sizeof(temp_line) - 1] = '\0';
    
    // Parse eerste token (functienaam)
    token = strtok(temp_line, ",");
    if (token == NULL)
        return -1;
    
    // Trim whitespace en kopieer functienaam
    while (*token == ' ')
        token++;
    strncpy(cmd->function_name, token, 5);
    cmd->function_name[5] = '\0';
    
    // Parse argumenten
    while ((token = strtok(NULL, ",")) != NULL && arg_count < MAX_ARGS)
    {
        // Trim whitespace
        while (*token == ' ')
            token++;
        
        // Voeg argument toe aan buffer
        if (arg_count > 0)
            strcat(args_buffer, ",");
        strcat(args_buffer, token);
        arg_count++;
    }
    
    cmd->num_arguments = arg_count;
    
    // Alloceer geheugen voor arguments
    cmd->arguments = (char *)malloc(strlen(args_buffer) + 1);
    if (cmd->arguments == NULL)
        return -1;
    
    strcpy(cmd->arguments, args_buffer);
    
    return 0;
}

/*
 * Function: execute_command
 * Returnvalue: int (API return code)
 * Arguments: struct LogicInterface *cmd - geparsed commando
 * Beschrijving: Voert commando uit door juiste API functie aan te roepen
 */
int execute_command(struct LogicInterface *cmd)
{
    int result = 0;
    char *token;
    char args_copy[256];
    int int_args[MAX_ARGS];
    char str_args[3][MAX_TOKEN_LEN]; // Voor tekst, fontname, etc.
    int arg_idx = 0;
    
    if (cmd == NULL || cmd->arguments == NULL)
        return -1;
    
    // Kopieer arguments voor parsing
    strncpy(args_copy, cmd->arguments, sizeof(args_copy) - 1);
    args_copy[sizeof(args_copy) - 1] = '\0';
    
    // Parse alle argumenten
    token = strtok(args_copy, ",");
    while (token != NULL && arg_idx < MAX_ARGS)
    {
        while (*token == ' ')
            token++;
        
        // Check of het een getal is
        if (isdigit(token[0]) || token[0] == '-')
            int_args[arg_idx] = atoi(token);
        else
            strncpy(str_args[arg_idx], token, MAX_TOKEN_LEN - 1);
        
        token = strtok(NULL, ",");
        arg_idx++;
    }
    
    // String compare voor commando matching
    if (strcmp(cmd->function_name, "lijn") == 0)
    {
        // lijn, x, y, x', y', kleur, dikte
        int color = parse_color(str_args[4]);
        result = API_draw_line(int_args[0], int_args[1], int_args[2], 
                               int_args[3], color, int_args[5], 0);
    }
    else if (strcmp(cmd->function_name, "recht") == 0)
    {
        // rechthoek, x_lup, y_lup, breedte, hoogte, kleur, gevuld
        int color = parse_color(str_args[4]);
        result = API_draw_rectangle(int_args[0], int_args[1], int_args[2],
                                   int_args[3], color, int_args[5], 0, 0);
    }
    else if (strcmp(cmd->function_name, "tekst") == 0)
    {
        // tekst, x, y, kleur, tekst, fontnaam, fontgrootte, fontstijl
        int color = parse_color(str_args[2]);
        result = API_draw_text(int_args[0], int_args[1], color, str_args[3],
                              str_args[4], int_args[5], int_args[6], 0);
    }
    else if (strcmp(cmd->function_name, "bitma") == 0)
    {
        // bitmap, nr, x-lup, y-lup
        result = API_draw_bitmap(int_args[1], int_args[2], int_args[0]);
    }
    else if (strcmp(cmd->function_name, "clear") == 0)
    {
        // clearscherm, kleur
        int color = parse_color(str_args[0]);
        result = API_clearscreen(color);
    }
    else if (strcmp(cmd->function_name, "cirke") == 0)
    {
        // cirkel, x, y, radius, kleur
        int color = parse_color(str_args[3]);
        result = API_draw_circle(int_args[0], int_args[1], int_args[2], color, 0);
    }
    else
    {
        result = -1; // Onbekend commando
    }
    
    return result;
}

/*
 * Function: free_command
 * Returnvalue: void
 * Arguments: struct LogicInterface *cmd
 * Beschrijving: Maakt gealloceerd geheugen vrij
 */
void free_command(struct LogicInterface *cmd)
{
    if (cmd != NULL && cmd->arguments != NULL)
    {
        free(cmd->arguments);
        cmd->arguments = NULL;
    }
}
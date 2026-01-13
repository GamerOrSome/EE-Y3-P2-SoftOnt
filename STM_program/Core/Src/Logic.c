/*
 * logic_layer.c
 *
 * Created on: 27 Nov 2025
 * Author: jeremy
 *
 * Beschrijving: Logic layer implementatie voor script parsing en executie
 */

#include "logic_layer.h"
#include "API_func.h"
#include "stm32_ub_vga_screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// ============================================================================
// DEFINES EN CONSTANTEN
// ============================================================================

#define MAX_ARGS 12
#define MAX_TOKEN_LEN 50
#define MAX_LINE_LEN 256


// Font styles
#define FONT_NORMAAL    0
#define FONT_VET        1
#define FONT_CURSIEF    2

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

// Struct voor color mapping
typedef struct
{
    const char *name;
    int value;
} ColorMap;

// Struct voor parsed arguments (makkelijker werken)
typedef struct
{
    int int_args[MAX_ARGS];
    char str_args[MAX_ARGS][MAX_TOKEN_LEN];
    int arg_count;
} ParsedArgs;

// ============================================================================
// LOOKUP TABLES
// ============================================================================

static const ColorMap COLOR_TABLE[] =
{
    {"zwart",        VGA_COL_BLUE},
    {"blauw",        VGA_COL_BLUE},
    {"lichtblauw",   VGA_COL_LIGHTBLUE},
    {"groen",        VGA_COL_GREEN},
    {"lichtgroen",   VGA_COL_LIGHTGREEN},
    {"cyaan",        VGA_COL_CYAN},
    {"lichtcyaan",   VGA_COL_LIGHTCYAN},
    {"rood",         VGA_COL_RED},
    {"lichtrood",    VGA_COL_LIGHTRED},
    {"magenta",      VGA_COL_MAGENTA},
    {"lichtmagenta", VGA_COL_LIGHTMAGENTA},
    {"bruin",        VGA_COL_BROWN},
    {"geel",         VGA_COL_YELLOW},
    {"grijs",        VGA_COL_GRAY},
    {"wit",          VGA_COL_WHITE}
};

#define COLOR_TABLE_SIZE (sizeof(COLOR_TABLE) / sizeof(ColorMap))

// ============================================================================
// HELPER FUNCTIES
// ============================================================================
/*
 * Function: to_lowercase
 * Returnvalue: void
 * Arguments: char *str - string om te converteren
 * Beschrijving: Converteer string naar lowercase in-place
 */
static void to_lowercase(char *str)
{
    unsigned int i;

    for (i = 0; str[i] != '\0'; i++)
        str[i] = tolower((unsigned char)str[i]);
}

/*
 * Function: trim_whitespace
 * Returnvalue: char* - pointer naar getrimde string
 * Arguments: char *str - string om te trimmen
 * Beschrijving: Verwijder leading en trailing whitespace
 */
static char* trim_whitespace(char *str)
{
    char *end;

    // Trim leading whitespace
    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    // Trim trailing whitespace
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Null terminate
    *(end + 1) = '\0';

    return str;
}

/*
 * Function: parse_color
 * Returnvalue: int - kleurcode
 * Arguments: char *color_str - kleurnaam
 * Beschrijving: Converteer kleurnaam naar kleurcode
 *
 * Voorbeeld: "rood" -> COLOR_ROOD (7)
 */
int parse_color(char *color_str)
{
    unsigned int i;
    char temp[MAX_TOKEN_LEN];
    
    if (color_str == NULL)
        return VGA_COL_BLACK;

    // Kopieer naar temp buffer voor lowercase conversie
    strncpy(temp, color_str, MAX_TOKEN_LEN - 1);
    temp[MAX_TOKEN_LEN - 1] = '\0';
    to_lowercase(temp);

    // Zoek in lookup table
    for (i = 0; i < COLOR_TABLE_SIZE; i++)
    {
        if (strcmp(temp, COLOR_TABLE[i].name) == 0)
            return COLOR_TABLE[i].value;
    }

    // Default: zwart
    return VGA_COL_BLACK;
}

/*
 * Function: parse_font_style
 * Returnvalue: int - font style code
 * Arguments: char *style_str - style naam
 * Beschrijving: Converteer font style naam naar code
 */
int parse_font_style(char *style_str)
{
    char temp[MAX_TOKEN_LEN];

    if (style_str == NULL)
        return FONT_NORMAAL;

    strncpy(temp, style_str, MAX_TOKEN_LEN - 1);
    temp[MAX_TOKEN_LEN - 1] = '\0';
    to_lowercase(temp);

    if (strcmp(temp, "vet") == 0)
        return FONT_VET;
    else if (strcmp(temp, "cursief") == 0)
        return FONT_CURSIEF;
    else
        return FONT_NORMAAL;
}

/*
 * Function: parse_arguments
 * Returnvalue: int - 0 bij success, -1 bij error
 * Arguments:
 *   - char *args_str: string met alle argumenten
 *   - ParsedArgs *parsed: output struct met geparsede args
 * Beschrijving: Parse argument string naar integers en strings
 *
 * Deze functie doet het "zware werk" van argument parsing
 */
static int parse_arguments(char *args_str, ParsedArgs *parsed)
{
    char *token;
    char temp[MAX_LINE_LEN];
    int idx = 0;

    if (args_str == NULL || parsed == NULL)
        return -1;

    // Initialiseer parsed struct
    parsed->arg_count = 0;

    // Kopieer voor strtok (wijzigt origineel)
    strncpy(temp, args_str, MAX_LINE_LEN - 1);
    temp[MAX_LINE_LEN - 1] = '\0';
    
    // Parse elk argument
    token = strtok(temp, ",");
    while (token != NULL && idx < MAX_ARGS)
    {
        token = trim_whitespace(token);

        // Probeer als integer te parsen
        if (isdigit((unsigned char)token[0]) ||
            (token[0] == '-' && isdigit((unsigned char)token[1])))
        {
            parsed->int_args[idx] = atoi(token);
            parsed->str_args[idx][0] = '\0'; // Lege string
        }
        else
        {
            // Bewaar als string
            parsed->int_args[idx] = 0;
            strncpy(parsed->str_args[idx], token, MAX_TOKEN_LEN - 1);
            parsed->str_args[idx][MAX_TOKEN_LEN - 1] = '\0';
        }

        idx++;
        token = strtok(NULL, ",");
    }
    
    parsed->arg_count = idx;
    return 0;
}

// ============================================================================
// MAIN PARSER FUNCTIES
// ============================================================================

/*
 * Function: parse_script_line
 * Returnvalue: int - 0 bij success, error code anders
 * Arguments:
 *   - char *line: input script regel
 *   - struct LogicInterface *cmd: output parsed commando
 * Beschrijving: Parse een script regel naar LogicInterface struct
 *
 * Voorbeeld:
 *   Input:  "lijn, 10, 20, 100, 50, rood, 2"
 *   Output: cmd->function_name = "lijn"
 *           cmd->arguments = "10,20,100,50,rood,2"
 *           cmd->num_arguments = 7
 */
int parse_script_line(char *line, struct LogicInterface *cmd)
{
    char *comma_pos;
    char temp_line[MAX_LINE_LEN];
    char *func_name;
    char *args_str;
    int args_len;
    
    // Input validatie
    if (line == NULL || cmd == NULL)
        return -1;
    
    // Kopieer input
    strncpy(temp_line, line, MAX_LINE_LEN - 1);
    temp_line[MAX_LINE_LEN - 1] = '\0';
    
    // Zoek eerste komma
    comma_pos = strchr(temp_line, ',');
    
    if (comma_pos == NULL)
    {
        // Geen komma = commando zonder argumenten (bijv. "clearscherm")
        func_name = trim_whitespace(temp_line);
        args_str = "";
    }
    else
    {
        // Split op eerste komma
        *comma_pos = '\0';
        func_name = trim_whitespace(temp_line);
        args_str = trim_whitespace(comma_pos + 1);
    }
    
    // Converteer functienaam naar lowercase
    to_lowercase(func_name);

    // Kopieer functienaam (max 5 chars)
    strncpy(cmd->function_name, func_name, 5);
    cmd->function_name[5] = '\0';

    // Alloceer geheugen voor argumenten
    args_len = strlen(args_str);
    cmd->arguments = (char *)malloc(args_len + 1);
    
    if (cmd->arguments == NULL)
        return -1; // Memory allocation failed

    strcpy(cmd->arguments, args_str);
    
    // Tel aantal argumenten (aantal komma's + 1, tenzij leeg)
    if (args_len > 0)
    {
        char *p = args_str;
        cmd->argument_len = 1;

        while (*p != '\0')
        {
            if (*p == ',')
                cmd->argument_len++;
            p++;
        }
    }
    else
    {
        cmd->argument_len = 0;
    }
    
    return 0;
}

// ============================================================================
// COMMAND EXECUTION FUNCTIES
// ============================================================================

/*
 * Function: execute_command
 * Returnvalue: int - API return code (0 = success)
 * Arguments: struct LogicInterface *cmd
 * Beschrijving: Voert geparsed commando uit via API calls
 *
 * Dit is de "main switch" die bepaalt welke API functie aangeroepen wordt
 */
int execute_command(struct LogicInterface *cmd)
{
    ParsedArgs parsed;
    int result;
    int color;

    // Input validatie
    if (cmd == NULL)
        return -1;
    
    // Parse argumenten
    if (parse_arguments(cmd->arguments, &parsed) != 0)
        return -1;
    
    // ========================================================================
    // COMMANDO MATCHING met strcmp
    // ========================================================================
    
    // LIJN: lijn, x1, y1, x2, y2, kleur, dikte
    if (strcmp(cmd->function_name, "lijn") == 0)
    {
        if (parsed.arg_count < 6)
            return -1;
        
        color = parse_color(parsed.str_args[4]);
        result = API_draw_line(
            parsed.int_args[0],  // x1
            parsed.int_args[1],  // y1
            parsed.int_args[2],  // x2
            parsed.int_args[3],  // y2
            color,               // kleur
            parsed.int_args[5],  // dikte
            0                    // reserved
        );
    }
    
    // RECHTHOEK: rechthoek, x, y, breedte, hoogte, kleur, gevuld
    else if (strcmp(cmd->function_name, "recht") == 0)
    {
        if (parsed.arg_count < 6)
            return -1;

        color = parse_color(parsed.str_args[4]);
        result = API_draw_rectangle(
            parsed.int_args[0],  // x
            parsed.int_args[1],  // y
            parsed.int_args[2],  // breedte
            parsed.int_args[3],  // hoogte
            color,               // kleur
            parsed.int_args[5],  // gevuld (1=ja, 0=nee)
            0,                   // reserved
            0                    // reserved_2
        );
    }

    // TEKST: tekst, x, y, kleur, tekst, fontnaam, fontgrootte, fontstijl
    else if (strcmp(cmd->function_name, "tekst") == 0)
    {
        if (parsed.arg_count < 7)
            return -1;

        color = parse_color(parsed.str_args[2]);
        int fontstyle = parse_font_style(parsed.str_args[6]);

        result = API_draw_text(
            parsed.int_args[0],     // x
            parsed.int_args[1],     // y
            color,                  // kleur
            parsed.str_args[3],     // tekst
            parsed.str_args[4],     // fontnaam
            parsed.int_args[5],     // fontgrootte
            fontstyle,              // fontstijl
            0                       // reserved
        );
    }

    // BITMAP: bitmap, nr, x_lup, y_lup
    else if (strcmp(cmd->function_name, "bitmap") == 0)
    {
        if (parsed.arg_count < 3)
            return -1;

        result = API_draw_bitmap(
            parsed.int_args[1],  // x_lup
            parsed.int_args[2],  // y_lup
            parsed.int_args[0]   // bitmap nummer
        );
    }

    // CLEARSCHERM: clearscherm, kleur
    else if (strcmp(cmd->function_name, "clear") == 0)
    {
        if (parsed.arg_count < 1)
            return -1;

        color = parse_color(parsed.str_args[0]);
        result = API_clearscreen(color);
    }

    // CIRKEL: cirkel, x, y, radius, kleur
    else if (strcmp(cmd->function_name, "cirkel") == 0)
    {
        if (parsed.arg_count < 4)
            return -1;

        color = parse_color(parsed.str_args[3]);
        result = API_draw_circle(
            parsed.int_args[0],  // x
            parsed.int_args[1],  // y
            parsed.int_args[2],  // radius
            color,               // kleur
            0                    // reserved
        );
    }

    // FIGUUR: figuur, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, kleur
    else if (strcmp(cmd->function_name, "figuur") == 0)
    {
        if (parsed.arg_count < 11)
            return -1;

        color = parse_color(parsed.str_args[10]);
        result = API_draw_figure(
            parsed.int_args[0],  // x1
            parsed.int_args[1],  // y1
            parsed.int_args[2],  // x2
            parsed.int_args[3],  // y2
            parsed.int_args[4],  // x3
            parsed.int_args[5],  // y3
            parsed.int_args[6],  // x4
            parsed.int_args[7],  // y4
            parsed.int_args[8],  // x5
            parsed.int_args[9],  // y5
            color,               // kleur
            0                    // reserved
        );
    }

    // WACHT: wacht, msecs
    else if (strcmp(cmd->function_name, "wacht") == 0)
    {
        if (parsed.arg_count < 1)
            return -1;

        result = API_wait(parsed.int_args[0]);
    }

    // Onbekend commando
    else
    {
        result = -1;
    }
    
    return result;
}

/*
 * Function: free_command
 * Returnvalue: void
 * Arguments: struct LogicInterface *cmd
 * Beschrijving: Geeft dynamisch gealloceerd geheugen vrij
 *
 * BELANGRIJK: roep dit ALTIJD aan na gebruik van parse_script_line!
 */
void free_command(struct LogicInterface *cmd)
{
    if (cmd != NULL && cmd->arguments != NULL)
    {
        free(cmd->arguments);
        cmd->arguments = NULL;
    }
}

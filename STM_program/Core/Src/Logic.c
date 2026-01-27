
/*
 * logic_layer.c
 *
 * Created on: 27 Nov 2025
 * Author: jeremy , Victor
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

//defines

#define MAX_ARGS 12
#define MAX_TOKEN_LEN 50
#define MAX_LINE_LEN 256


// Font styles
#define FONT_NORMAAL    0
#define FONT_VET        1
#define FONT_CURSIEF    2

//typedefs
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


/**
 * Kleur lookup tabel
 * @param name Kleur naam als string
 * @param value Kleur code als integer'
 * @param VGA_COL_... definities uit stm32_ub_vga_screen.h
 
 * @return Kleur code of VGA_COL_BLACK als niet gevonden
 */
static const ColorMap COLOR_TABLE[] =
{
    {"zwart",        VGA_COL_BLACK},
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
/**
 * Converteer string naar lowercase
 * @param str In/uit parameter: de string die geconverteerd wordt
 */
static void to_lowercase(char *str)
{
    unsigned int i;

    for (i = 0; str[i] != '\0'; i++)
        str[i] = tolower((unsigned char)str[i]);
}

/**
 * Trim whitespace from a string
 * @param str In/uit parameter: de string die geconverteerd wordt
 * @return Pointer naar de trimmed string
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

/**
 * Converteer kleurnaam naar kleurcode
 * @param color_str Kleurnaam als string
 * @return Kleurcode of VGA_COL_BLACK als niet gevonden
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

/**
 * Converteer font style naam naar code
 * @param style_str Font style naam als string
 * @return Font style code of FONT_NORMAAL als niet gevonden
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

/**
 * Parse argumenten string naar ParsedArgs struct
 * @param args_str In parameter: de argumenten string
 * @param parsed Out parameter: de geparseerde argumenten
 * @return 0 bij succes, -1 bij fout
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


/**
 * Parse een script regel naar LogicInterface struct
 * @param line In parameter: de regel die geanalyseerd wordt
 * @param cmd Out parameter: de struct met de geanalyseerde commando's
 * @return 0 bij succes, -1 bij fout
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

    // Kopieer functienaam (max 15 chars to handle longer commands)
    strncpy(cmd->function_name, func_name, sizeof(cmd->function_name) - 1);
    cmd->function_name[sizeof(cmd->function_name) - 1] = '\0';

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

/** 
 * Executeer een commando
 * @param cmd In parameter: het commando dat uitgevoerd moet worden
 * @param parsed Geparste argumenten
 * @param result Uit parameter: de return waarde van de API functie
 * @return 0 bij succes, -1 bij fout
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
    else if (strcmp(cmd->function_name, "rechthoek") == 0)
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
    else if (strcmp(cmd->function_name, "clearscherm") == 0)
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

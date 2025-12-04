/*
 * API_func.h
 *
 *  Created on: 27 Nov 2025
 *      Author: jerem
 */

/*

TODO:
Add error check to all functions
completer functions

*/

#ifndef API_API_FUNC_H_
#define API_API_FUNC_H_

#include <stdint.h>
#include <errno.h>
#include <stm32_ub_vga_screen.h>

/**
 * Tekent input string op opgegeven linkerbovenhoek-coordinaten met een gegeven kleur, font en stijl.
 *
 * @param x_lup        X-coordinaat van de linkerbovenhoek waar de tekst begint.
 * @param y_lup        Y-coordinaat van de linkerbovenhoek waar de tekst begint.
 * @param color        Kleurcode die gebruikt wordt om de tekst te tekenen.
 * @param text         Pointer naar de NUL-terminerende tekenreeks die getekend moet worden.
 * @param fontname     Naam van het lettertype dat gebruikt moet worden (bijv. "Arial").
 * @param fontsize     Grootte van het lettertype in punten of pixels (implementatieafhankelijk).
 * @param fontstyle    Stijlflags voor het lettertype (bijv. vet, cursief; implementatieafhankelijk).
 * @param reserved     Gereserveerd voor toekomstig gebruik; momenteel genegeerd.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_draw_text(int x_lup, int y_lup, int color, char *text, char *fontname, int fontsize, int fontstyle, int reserved);

/**
 * Tekent een rechte lijn tussen twee punten met aangegeven kleur en lijngewicht.
 *
 * @param x_1          X-coordinaat van het eerste eindpunt.
 * @param y_1          Y-coordinaat van het eerste eindpunt.
 * @param x_2          X-coordinaat van het tweede eindpunt.
 * @param y_2          Y-coordinaat van het tweede eindpunt.
 * @param color        Kleurcode voor de lijn.
 * @param weight       Dikte van de lijn (in pixels of units afhankelijk van implementatie).
 * @param reserved     Gereserveerd voor toekomstig gebruik; momenteel genegeerd.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_draw_line(int x_1, int y_1, int x_2, int y_2, int color, int weight, int reserved);

/**
 * Tekent een rechthoek op de gegeven positie met breedte, hoogte en kleur. Kan gevuld of slechts omtrek zijn.
 *
 * @param x            X-coordinaat van de linkerbovenhoek van de rechthoek.
 * @param y            Y-coordinaat van de linkerbovenhoek van de rechthoek.
 * @param width        Breedte van de rechthoek in pixels.
 * @param height       Hoogte van de rechthoek in pixels.
 * @param color        Kleurcode voor de rechthoek (vulling of rand afhankelijk van 'filled').
 * @param filled       Indien niet-nul: teken een gevulde rechthoek; indien nul: teken alleen de rand.
 * @param reserved     Gereserveerd voor toekomstig gebruik; momenteel genegeerd.
 * @param reserved_2   Extra gereserveerd parameter voor toekomstig gebruik; momenteel genegeerd.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_draw_rectangle(int x, int y, int width, int height, int color, int filled, int reserved, int reserved_2);

/**
 * Tekent een eerder geladen bitmap (resource) met gegeven nummer op de opgegeven linkerbovenhoek-coördinaten.
 *
 * @param x_lup        X-coordinaat van de linkerbovenhoek waar de bitmap geplaatst wordt.
 * @param y_lup        Y-coordinaat van de linkerbovenhoek waar de bitmap geplaatst wordt.
 * @param bm_nr        Bitmapnummer/ID dat verwijst naar een geladen bitmapresource.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_draw_bitmap(int x_lup, int y_lup, int bm_nr);

/**
 * Maakt het scherm vrij en vult het met de opgegeven kleur.
 *
 * @param color        Kleurcode die gebruikt wordt om het volledige scherm mee te vullen.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_clearscreen(int color);

/**
 * Pauzeert de uitvoering gedurende het opgegeven aantal milliseconden.
 *
 * @param msecs        Aantal milliseconden om te wachten/sleepen.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_wait(int msecs);

/**
 * Herhaalt een reeks eerder uitgevoerde tekencommando's een aantal keren.
 *
 * @param nr_previous_commands  Aantal voorgaande commando's die herhaald moeten worden.
 * @param iterations            Hoe vaak de reeks herhaald moet worden.
 * @param reserved              Gereserveerd voor toekomstig gebruik; momenteel genegeerd.
 *
 * @return                      Statuscode (0 = succes, anders fout).
 */

int API_repeat_commands(int nr_previous_commands, int iterations, int reserved);

/**
 * Tekent een cirkel met middelpunt en straal in de opgegeven kleur.
 *
 * @param x            X-coordinaat van het middelpunt.
 * @param y            Y-coordinaat van het middelpunt.
 * @param radius       Straal van de cirkel in pixels.
 * @param color        Kleurcode voor de cirkel (rand of vulling afhankelijk van implementatie).
 * @param reserved     Gereserveerd voor toekomstig gebruik; momenteel genegeerd.
 *
 * @return             Statuscode (0 = succes, anders fout).
 */

int API_draw_circle(int x, int y, int radius, int color, int reserved);

/**
 * Tekent een figuur gedefinieerd door maximaal vijf punten (veelhoek/figuur) met de opgegeven kleur.
 *
 * @param x_1        X-coordinaat van het eerste punt.
 * @param y_1        Y-coordinaat van het eerste punt.
 * @param x_2        X-coordinaat van het tweede punt.
 * @param y_2        Y-coordinaat van het tweede punt.
 * @param x_3        X-coordinaat van het derde punt.
 * @param y_3        Y-coordinaat van het derde punt.
 * @param x_4        X-coordinaat van het vierde punt.
 * @param y_4        Y-coordinaat van het vierde punt.
 * @param x_5        X-coordinaat van het vijfde punt.
 * @param y_5        Y-coordinaat van het vijfde punt.
 * @param color      Kleurcode voor de figuur (rand of vulling afhankelijk van implementatie).
 * @param reserved   Gereserveerd voor toekomstig gebruik; momenteel genegeerd.
 *
 * Opmerking: punten kunnen in volgorde worden verbonden; implementatie kan bepalen of de figuur automatisch gesloten en/of gevuld wordt.
 *
 * @return         Statuscode (0 = succes, anders fout).
 */

int API_draw_figure(int x_1, int y_1, int x_2, int y_2, int x_3, int y_3, int x_4, int y_4, int x_5, int y_5, int color, int reserved);

#endif /* API_API_FUNC_H_ */

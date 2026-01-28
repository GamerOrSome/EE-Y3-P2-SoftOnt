/**
  ******************************************************************************
  * @file           : API_func.c
  * @brief          : API functies om scherm aan te sturen via ub_lib.
  ******************************************************************************
  */

//Includes

#include <API_func.h>
#include <Bitmaps.h>
#include <combined_charsets.h>

int API_draw_text(int x_lup, int y_lup, int color, char *text, char *fontname, int fontsize, int fontstyle, int text_length)
{   
    if(text_length <= 0)
    {
        text_length = strlen(text);
        if(text_length <= 0)
        {
            return -EINVAL;
        }
    }

    const uint16_t (*ascii_char_index)[4];
    const uint8_t *ascii_bitmap_data;

    // Find matching font
    const FontInfo* selected_font = NULL;

    for (int i = 0; i < NUM_FONTS; i++) {
        if (strcmp(fontname, available_fonts[i].name) == 0 && fontsize == available_fonts[i].size) {
            selected_font = &available_fonts[i];
            break;
        }
    }

    // Use default if no match found
    if (selected_font == NULL) {
        selected_font = &available_fonts[0]; // default font
    }

    ascii_char_index = selected_font->index;
    ascii_bitmap_data = selected_font->data;

    // Calculate max text dimensions
    int max_text_width = 0;
    int max_text_height = 0;

    for (int i = 0; i < text_length; i++)
    {
        uint8_t cur_char = text[i];
        if (cur_char < 32 || cur_char > 126)
        {
            break;
        }
        max_text_width += ascii_char_index[cur_char - 32][1];
        uint16_t char_height = ascii_char_index[cur_char - 32][2];
        if (char_height > max_text_height)
        {
            max_text_height = char_height;
        }
    }

    // Check if text fits on screen
    if (x_lup + max_text_width > VGA_DISPLAY_X || y_lup + max_text_height > VGA_DISPLAY_Y)
    {
        return -EINVAL;
    }

    int offset = 0;
    for (int i = 0; i < text_length; i++)
    {
        uint8_t cur_char = text[i];
        uint16_t width = ascii_char_index[cur_char - 32][1];
        uint16_t height = ascii_char_index[cur_char - 32][2];
        uint16_t char_offset = ascii_char_index[cur_char - 32][3];

        if (cur_char < 32 || cur_char > 126)
        {
            break; // Stop on unsupported characters
        }

        // Calculate baseline offset for this character
        // Align all characters to the bottom (baseline) of the max text height
        int y_baseline_offset = max_text_height - height;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                uint16_t bytes_per_row = (width + 7) / 8;
                uint8_t pixel_byte = ascii_bitmap_data[char_offset + y * bytes_per_row + (x / 8)];
                uint8_t pixel_bit = 7 - (x % 8);
                if (pixel_byte & (1 << pixel_bit))
                {
                    UB_VGA_SetPixel(x_lup + x + offset, y_lup + y + y_baseline_offset, color);
                }
            }
        }
        offset += width;
    }
    
    return 0;
}

int API_draw_line(int x_1, int y_1, int x_2, int y_2, int color, int weight, int reserved)
{
    if(x_1 < 0 || x_1 >= VGA_DISPLAY_X || x_2 < 0 || x_2 >= VGA_DISPLAY_X ||
       y_1 < 0 || y_1 >= VGA_DISPLAY_Y || y_2 < 0 || y_2 >= VGA_DISPLAY_Y ||
       weight <= 0 || color < 0 || color > 255)
    {
        return -EINVAL;
    }

    int dx = x_2 - x_1;
    int dy = y_2 - y_1;
    
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    if(steps == 0) steps = 1;

    float x_inc = (float)dx / steps;
    float y_inc = (float)dy / steps;
    
    float x = x_1;
    float y = y_1;

    int even = 0;
    if (weight % 2 == 1)
    {   
        even++;
        weight += 1;
    }
    
    for(int i = 0; i <= steps; i++)
    {
        int curr_x = (int)x;
        int curr_y = (int)y;

        for(int yw=-(weight/2); yw<(weight/2); yw++)
        {
            int oy = curr_y + yw;
            for (int xw=-(weight/2)+abs(yw)+even; xw<(weight/2)-abs(yw); xw++)
            {
                int ox = curr_x + xw;
                UB_VGA_SetPixel(ox, oy, color);
            }
        }
        x += x_inc;
        y += y_inc;
    }

    HAL_Delay(10);
    return 0;
}

int API_draw_rectangle(int x, int y, int width, int height, int color, int filled, int reserved, int reserved_2)
{
    if(x < 0 || x + width >= VGA_DISPLAY_X || y < 0 || y + height >= VGA_DISPLAY_Y ||
       width <= 0 || height <= 0 || color < 0 || color > 255)
    {
        return -EINVAL;
    }

    if (filled) {
        // Fill the rectangle
        for (int row = 0; row <= height; row++) {
            for (int col = 0; col <= width; col++) {
                UB_VGA_SetPixel(x + col, y + row, color);
            }
        }
    } else {
        // Draw outline using lines
        API_draw_line(x, y, x + width, y, color, 1, 0);                    // Top
        API_draw_line(x + width, y, x + width, y + height, color, 1, 0);   // Right
        API_draw_line(x + width, y + height, x, y + height, color, 1, 0);  // Bottom
        API_draw_line(x, y + height, x, y, color, 1, 0);                   // Left
    }
    HAL_Delay(10);
    return 0;
}

int API_draw_bitmap(int x_lup, int y_lup, int bm_nr)
{
    const uint8_t *bitmap_data;

    if(bm_nr < 0 || bm_nr >= NUM_BITMAPS)
    {
        return -EINVAL;
    }

    bitmap_data = bitmap_array[bm_nr];

    int bitmap_height = bitmap_data[1];
    int bitmap_width = bitmap_data[0];

    if (x_lup < 0 || x_lup + bitmap_width > VGA_DISPLAY_X ||
        y_lup < 0 || y_lup + bitmap_height > VGA_DISPLAY_Y)
    {
        return -EINVAL;
    }

    for (int y = 0; y < bitmap_height; y++) {
        for (int x = 0; x < bitmap_width; x++) {
            int pixel_index = y * bitmap_width + x + 2;
            uint8_t color = bitmap_data[pixel_index];
            UB_VGA_SetPixel(x_lup + x, y_lup + y, color);
        }
    }
    HAL_Delay(5);
    return 0;
}

int API_clearscreen(int color)
{
    if (color < 0 || color > 256)
    {
        return -EINVAL;
    }

    UB_VGA_FillScreen(color);
    HAL_Delay(10);
    return 0;
}

// Optional functions
int API_wait(int msecs)
{
    if (msecs < 0)
    {
        return -EINVAL;
    }
    
    HAL_Delay(msecs);
    return 0;
}

int API_repeat_commands(int nr_previous_commands, int iterations, int reserved)
{
    return -ENOTSUP;
}

int API_draw_circle(int x, int y, int radius, int color, int reserved)
{
    return -ENOTSUP;
}

int API_draw_figure(int x_1, int y_1, int x_2, int y_2, int x_3, int y_3, int x_4, int y_4, int x_5, int y_5, int color, int reserved)
{
    return -ENOTSUP;
}

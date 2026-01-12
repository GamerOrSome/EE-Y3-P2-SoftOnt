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

    uint16_t (*ascii_char_index)[4];
    uint8_t *ascii_bitmap_data;

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
    uint8_t *bitmap_data;
    switch (bm_nr)
    {
    case 1:
        bitmap_data = HU_resize_bitmap_data;
        break;
    case 2:
        bitmap_data = Wolf_bitmap_data;
        break;    
    case 3:
        bitmap_data = Skele__1__bitmap_data;
        break;
    case 4:
        bitmap_data = Skele__2__bitmap_data;
        break;
    case 5:
        bitmap_data = Skele__3__bitmap_data;
        break;
    case 6:
        bitmap_data = Skele__4__bitmap_data;
        break;
    case 7:
        bitmap_data = Skele__5__bitmap_data;
        break;
    case 8:
        bitmap_data = Skele__6__bitmap_data;
        break;
    case 9:
        bitmap_data = Skele__7__bitmap_data;
        break;
    case 10:
        bitmap_data = Skele__8__bitmap_data;
        break;
    case 11:
        bitmap_data = Skele__9__bitmap_data;
        break;
    case 12:
        bitmap_data = Skele__10__bitmap_data;
        break;
    case 13:
        bitmap_data = Skele__11__bitmap_data;
        break;
    case 14:
        bitmap_data = Skele__12__bitmap_data;
        break;
    case 15:
        bitmap_data = Skele__13__bitmap_data;
        break;
    case 16:
        bitmap_data = Skele__14__bitmap_data;
        break;
    case 17:
        bitmap_data = Dance_Skel__1__bitmap_data;
        break;
    case 18:
        bitmap_data = Dance_Skel__2__bitmap_data;
        break;
    case 19:
        bitmap_data = Dance_Skel__3__bitmap_data;
        break;
    case 20:
        bitmap_data = Dance_Skel__4__bitmap_data;
        break;
    case 21:
        bitmap_data = Dance_Skel__5__bitmap_data;
        break;
    case 22:
        bitmap_data = Dance_Skel__6__bitmap_data;
        break;
    case 23:
        bitmap_data = Dance_Skel__7__bitmap_data;
        break;
    case 24:
        bitmap_data = Dance_Skel__8__bitmap_data;
        break;
    case 25:
        bitmap_data = Dance_Skel__9__bitmap_data;
        break;
    case 26:
        bitmap_data = Dance_Skel__10__bitmap_data;
        break;
    case 27:
        bitmap_data = Dance_Skel__11__bitmap_data;
        break;
    case 28:
        bitmap_data = Dance_Skel__12__bitmap_data;
        break;
    case 29:
        bitmap_data = Dance_Skel__13__bitmap_data;
        break;
    case 30:
        bitmap_data = Dance_Skel__14__bitmap_data;
        break;
    case 31:
        bitmap_data = Dance_Skel__15__bitmap_data;
        break;
    case 32:
        bitmap_data = Dance_Skel__16__bitmap_data;
        break;
    case 33:
        bitmap_data = Dance_Skel__17__bitmap_data;
        break;
    case 34:
        bitmap_data = Dance_Skel__18__bitmap_data;
        break;
    case 35:
        bitmap_data = Dance_Skel__19__bitmap_data;
        break;
    case 36:
        bitmap_data = Dance_Skel__20__bitmap_data;
        break;
    case 37:
        bitmap_data = Dance_Skel__21__bitmap_data;
        break;
    case 38:
        bitmap_data = Dance_Skel__22__bitmap_data;
        break;
    case 39:
        bitmap_data = Dance_Skel__23__bitmap_data;
        break;
    case 40:
        bitmap_data = Dance_Skel__24__bitmap_data;
        break;
    case 41:
        bitmap_data = Dance_Skel__25__bitmap_data;
        break;
    case 42:
        bitmap_data = Dance_Skel__26__bitmap_data;
        break;
    case 43:
        bitmap_data = Dance_Skel__27__bitmap_data;
        break;
    case 44:
        bitmap_data = Dance_Skel__28__bitmap_data;
        break;
    case 45:
        bitmap_data = Dance_Skel__29__bitmap_data;
        break;
    case 46:
        bitmap_data = Dance_Skel__30__bitmap_data;
        break;
    case 47:
        bitmap_data = Dance_Skel__31__bitmap_data;
        break;
    case 48:
        bitmap_data = Dance_Skel__32__bitmap_data;
        break;
    case 49:
        bitmap_data = Dance_Skel__33__bitmap_data;
        break;
    case 50:
        bitmap_data = Dance_Skel__34__bitmap_data;
        break;
    case 51:
        bitmap_data = Dance_Skel__35__bitmap_data;
        break;
    case 52:
        bitmap_data = Dance_Skel__36__bitmap_data;
        break;
    case 53:
        bitmap_data = Dance_Skel__37__bitmap_data;
        break;
    case 54:
        bitmap_data = Dance_Skel__38__bitmap_data;
        break;
    case 55:
        bitmap_data = Dance_Skel__39__bitmap_data;
        break;
    case 56:
        bitmap_data = Dance_Skel__40__bitmap_data;
        break;
    case 57:
        bitmap_data = Dance_Skel__41__bitmap_data;
        break;
    case 58:
        bitmap_data = Dance_Skel__42__bitmap_data;
        break;
    case 59:
        bitmap_data = plank_bitmap_data;
        break;
    case 60:
        bitmap_data = Chick_bitmap_data;
        break;
    default:
        return -EINVAL;
        break;
    }

    int bitmap_height = bitmap_data[1];
    int bitmap_width = bitmap_data[0];

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
    UB_VGA_FillScreen(color);
    HAL_Delay(10);
    return 0;
}

// Optional functions
int API_wait(int msecs)
{
    return -ENOTSUP;
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

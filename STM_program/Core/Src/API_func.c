#include <API_func.h>
#include <Bitmaps.h>

int API_draw_text(int x_lup, int y_lup, int color, char *text, char *fontname, int fontsize, int fontstyle, int reserved)
{
    return -ENOTSUP;
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
    return 0;
}

int API_draw_bitmap(int x_lup, int y_lup, int bm_nr)
{
    uint8_t *bitmap_data;
    switch (bm_nr)
    {
    case 1:
        bitmap_data = hu_bitmap_data;
        break;
    case 2:
        bitmap_data = wolf_bitmap_data;
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
    return 0;
}

int API_clearscreen(int color)
{
    UB_VGA_FillScreen(color);
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

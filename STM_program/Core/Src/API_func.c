#include <API_func.h>
#include <Bitmaps.h>

int API_draw_text(int x_lup, int y_lup, int color, char *text, char *fontname, int fontsize, int fontstyle, int reserved)
{
    return -ENOTSUP;
}

int API_draw_line(int x_1, int y_1, int x_2, int y_2, int color, int weight, int reserved)
{
    int dx = x_2 - x_1;
    int dy = y_2 - y_1;
    
    int neg_x = 1;
    int neg_y = 1;

    if(dx<0)
    {
        dx = -dx;
        neg_x = -1;
    }
    if(dy<0)
    {
        dy = -dy;
        neg_y = -1;
    }


    float dir = dx/dy;
    int drawing = 1;
    float step = dir;
    int step_taken = 0;

    int x = x_1;
    int y = y_1;
    while(drawing)
    {
        if(step<1)
        {
            y = y + neg_y;
            step+=dir;
            step_taken++;
        }
        else
        {
            x = x + neg_x;
            step--;
        }

        for(int yw=0; yw<weight; yw++)
        {
            int oy = y + yw - (weight/2);
            for (int xw=0; xw<weight-yw; xw++)
            {
                int ox = x + xw - (weight/2);
                UB_VGA_SetPixel(ox, oy, color);
            }
        }
        
        if(step_taken >= dy)
        {
            drawing = 0;
        }
    }

    return 0;
}

int API_draw_rectangle(int x, int y, int width, int height, int color, int filled, int reserved, int reserved_2)
{
    return -ENOTSUP;
}

int API_draw_bitmap(int x_lup, int y_lup, int bm_nr)
{

	API_draw_line(10, 10, 100, 10,  VGA_COL_GREEN, 4, 0);

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

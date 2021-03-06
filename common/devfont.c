#include <common.h>
#include <command.h>
#include <devfont.h>

#define MAX_FONT_NUM		16

PFONT font_arrays[MAX_FONT_NUM] ;
unsigned char  font_nums = 0 ;
PFONT gpCurFont = NULL;

unsigned char is_rect_invalid(const RECT *rect)
{
    if(rect == 0)
        return 1;
        
    if((rect->x >= (rect->x+rect->width)) || (rect->y >= (rect->y+rect->height)))
        return 1;
    else
		return 0;
}

static unsigned char get_font_bit_value(unsigned char* pData, int iPos)
{
    int iCurByte;
    int iCurBit;
    unsigned char Mask;
    
    iCurByte = iPos/8;
    iCurBit = iPos%8;
    
    Mask=1<<iCurBit;
    
    if(*(pData+iCurByte) & Mask)
		return 1;
    else
		return 0;
}

int DrawPixel(unsigned short x, unsigned short y, int color)
{
    unsigned char *dest;
    unsigned short lcd_line_length = (OSD_WIDTH * OSD_BPP) / 8;
    
    if((x > OSD_WIDTH) || (y > OSD_HEIGHT)) {
    	return	-1;
    }
    
    dest = (uchar *)(FB_ADDR + y * lcd_line_length + x * (OSD_BPP / 8));
#if(OSD_BPP == OSD_COLOR24)
    *dest++ = color & 0xff;
    *dest++ = (color >> 8) & 0xff;
    *dest = (color >> 16) & 0xff;
#endif
    
    flush_cache(FB_ADDR, OSD_WIDTH * OSD_HEIGHT * OSD_BPP/8);
    return 0;
}

int DrawRect(unsigned short x, unsigned short y, unsigned short w, unsigned short h, int color)
{
    unsigned char *dest;
    unsigned short row, col;
    unsigned short lcd_line_length = (OSD_WIDTH * OSD_BPP) / 8;
    
    if((x > OSD_WIDTH) || (y > OSD_HEIGHT)) {
    	return -1;
    }
    
    if((w < 0) || (h < 0)) {
    	return -1;
    }
    
    dest = (unsigned char *)(FB_ADDR + y * lcd_line_length + x * (OSD_BPP / 8));
    for(row=0; row<h; row++) {
    	for(col=0; col<w; col++) {
#if(OSD_BPP == OSD_COLOR24)
    	    *dest++ = color & 0xff;
    	    *dest++ = (color >> 8) & 0xff;
    	    *dest++ = (color >> 16) & 0xff;
#endif
    	}
    	dest = (unsigned char *)(FB_ADDR + (y + row) * lcd_line_length + x * (OSD_BPP / 8));
    }
    flush_cache(FB_ADDR, OSD_WIDTH * OSD_HEIGHT * OSD_BPP / 8);
    return 0;
}

int DrawFont(int x, int y, int font_width, int font_height, int font_color, unsigned char *font_data)
{
    RECT draw_rect;
    unsigned int w,h;

    if((x > OSD_WIDTH) || (y > OSD_HEIGHT)) {
    	return -1;
    }

    if(font_data)
    {
    	draw_rect.x = x;
    	draw_rect.y = y;
    	draw_rect.width = font_width;
    	draw_rect.height = font_height;
    	
    	if(is_rect_invalid(&draw_rect) == 1)
    	    return -1;
    	    
    	unsigned char cBitValue = 0x80;
    	unsigned offset = 0;

		for(h = 0; h < font_height; h++){
			for(w = 0 ; w < font_width ; w++){
				if(cBitValue == 0){
					cBitValue = 0x80 ;
					offset++ ;
				}
				if (font_data[offset] & cBitValue) {
					DrawPixel(x+w, y+h, font_color);
					//printf("1");
				}
				//else {
					//printf("0");
				//}
				cBitValue = cBitValue >> 1 ;
			}
			//printf("\n");
		}
    }
    
    return 0;
}

int GetTextSize(const void *str, unsigned short cc, int *pwidth, int *pheight, int *word_length)
{
    const char *text;
    unsigned short ch;
    unsigned short iIndex = 0;
    unsigned short ch_width, width = 0;
    unsigned char *pFont;

	if(gpCurFont == NULL) {
		return;
	}
	
    if(cc == 0) {
        cc = strlen((char *)str);
    }
    if(cc <= 0 ) {
    	return -1;
    }
    
    text = (char*)str;
    
    while(iIndex < cc)
    {
    	ch = text[iIndex++] ;
    	pFont = gpCurFont->GetFontBitmap(ch, &ch_width) ;
        width += ch_width + gpCurFont->font_gaps;
    }
    *pwidth = width;
    *word_length = cc;
    *pheight = gpCurFont->font_height;
    return 0;
}

void DrawText(const void *str, unsigned short cc, unsigned short x, unsigned short y, 
              int *pwidth, int *pheight, int font_color)
{
    const char *text;
    unsigned short ch;
    unsigned short iIndex = 0;
    unsigned short ch_width;
    unsigned short draw_x;
    unsigned char *pFont;

	if(gpCurFont == NULL) {
		return;
	}
	
    if(cc == 0) {
        cc = strlen((char *)str);
    }
    if(cc <= 0 ) {
    	return;
    }
    
    text = (char*)str;
    draw_x = x;
    
    while(iIndex < cc)
    {
    	ch = text[iIndex++] ;
    	pFont = gpCurFont->GetFontBitmap(ch, &ch_width) ;
		//printf("%s: ch is 0x%x, draw x is %d, y is %d, ch_width is %d, font_height is %d\n", __FUNCTION__, ch, draw_x, y, ch_width, gpCurFont->font_height);
        DrawFont(draw_x, y, ch_width, gpCurFont->font_height, font_color, pFont);
        draw_x += ch_width + gpCurFont->font_gaps;
    }
    
    *pwidth = draw_x - x ;
    *pheight = gpCurFont->font_height;
}

void AsciiPrintf(const char * print_strings, unsigned short x, unsigned short y, int font_color)
{
    int pix_width,pix_height;
    unsigned char str_length = strlen(print_strings);

	if(gpCurFont == NULL) {
		return;
	}
    DrawText(print_strings, str_length, x, y, &pix_width,&pix_height, font_color);
}

/**
 * Set the font for future calls.
 *
 * @param font_index The font in font array to use. 
 */
void SetFont(char *font_name)
{
    unsigned char iTemp = 0;
    
    for(iTemp = 0; iTemp < font_nums; iTemp++)
    {
        if(!strcmp(font_arrays[iTemp]->font_name, font_name))
    	{
    	    gpCurFont = font_arrays[iTemp];
    	    return;
    	}
    }
    return;
}

int RegisterFont(PFONT new_font)
{
    unsigned char i = 0;
    
    for(i = 0; i < font_nums; i++)
    {
        if(font_arrays[i] == new_font)
            return 1;
    }
    font_arrays[font_nums] = new_font;
    font_nums++;
    
    if(gpCurFont == NULL)
        gpCurFont = new_font;
    return 0;
}

int UnregisterFont(PFONT del_font)
{
    unsigned char found_del_font = 0;
    unsigned char i = 0;
    
    for(i = 0; i < font_nums; i++)
    {
    	if(font_arrays[i] == del_font)
    	    found_del_font = 1;
	if(found_del_font)
	    font_arrays[i] = font_arrays[i+1];
    }
    
    if(gpCurFont == del_font) {
    	gpCurFont = font_arrays[0];
    }
    if(found_del_font)
	font_nums--;
    return 0;
}

unsigned short GetCharWidth(char ch)
{
     unsigned short ch_width;
     
     gpCurFont->GetFontBitmap(ch, &ch_width);
     return ch_width;
}

unsigned short GetCharHeight()
{
     return gpCurFont->font_height;
}

int draw_string (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned short x, y;
	int font_color;

	if (argc < 4) {
		cmd_usage(cmdtp);
		return 1;
	}

	x = simple_strtoul(argv[2], NULL, 10);
	y = simple_strtoul(argv[3], NULL, 10);
	font_color = simple_strtoul(argv[4], NULL, 16);

	AsciiPrintf(argv[1], x, y, font_color);
	return 0;
}

U_BOOT_CMD(
	drawstr,	5,	1,	draw_string,
	"Display text on the screen",
	"drawstr <text> <X offset> <Y offset> <color>\n"
	"Draw text use 'color' on ('X offset', 'Y offset')"
);



// 138paulmiller libgbak
#include "gba.h"

// Enable main display control to draw sprites
#define SPRITE_ENABLE 0x1000

// sprite image mapping 2D maps char block as 2D tiles as image 
#define SPRITE_MAP_2D 0x0

// Sprite map 1D reads row of tiles in char block as image
#define SPRITE_MAP_1D 0x40

#define MOSAIC_ENABLE 0

#define DISPLAY_BACK_BUFFER 0x10;

/* 
	VRAM (video ram) is a pointer to base memory addr of where graphics data is stored
	spans from 06000000 to 06017FFF (64 kb)
 	Size of address is 2 bytes (short) used since ports are 16 bits large
*/
#define VRAM ((volatile ushort*) 0x06000000)

// Flags to enable the background at index N. Note, these are not the bg indices
#define GBA_BG0 0x100
#define GBA_BG1 0x200
#define GBA_BG2 0x400
#define GBA_BG3 0x800

/*
  	Display control register is 4 bytes and is used to control graphic modes
	Bits	|F	  |E    |D    |C	| B	  | A	 | 9	| 8   | 7  | 6   | 5  | 4  | 3  |2   1   0
	Field	|ObjW |Win1 |Win0 |Obj  | BG3 |	BG2  | BG1  | BG0 |	FB | OM  | HB | PS | GB | Mode

    Mode: Sets video mode. (GBA_MODE0..4) 0, 1, 2 are tiled modes; 3, 4, 5 are bitmap modes.
    GB  : Is set if cartridge is a GBC game. Read-only.
    PS  : Page select. Modes 4 and 5 can use page flipping for smoother animation. This bit selects the displayed page (and allowing the other one to be drawn on without artifacts).
    HB  : Allows access to OAM in an HBlank. OAM is normally locked in VDraw. Will reduce the amount of sprite pixels rendered per line.
    OM  : Object mapping mode. Tile memory can be seen as a 32x32 matrix of tiles. 
          When sprites are composed of multiple tiles high. this bit tells whether the
          2D mapping, OM=0: - Next row of tiles are beneath the previous
          1D mapping, OM=1: - Next row of tiles are to the right
    FB  : Force a screen blank.
    BG0-BG3, Obj:  Enables rendering of the corresponding background and sprites. (BG_ENABLE, SPRITE_ENABLE)
    W0-OW:	Enables the use of windows 0, 1 and Object window, respectively. Windows can be used to mask out certain areas (like the lamp did in Zelda:LTTP)
*/
 //note 4 bytes is long addr
#define DISPLAY_CONTROL  ((volatile unsigned long*) 0x04000000)

/*
  Display status register is 2 bytes and is used to read display statuses
	F E D C | B A 9 8 | 7 6 5 4  3 2 1 0 
	T T T T | T T T T | X X Y H  V Z G W 
*/
#define DISPLAY_STATUS = ((volatile ushort*) 0x04000004)

/*
	Read-only location of y location of display.  
	GBA display height is 160 and is followed by 68 lines of vblank period. 	
 	Waiting for value to reach 160 (vblank) before updating can sync display to 60 fps (Hz). 
*/
#define SCAN_VCOUNT ((volatile ushort*) 0x04000006)

/* 
Button state register 
    Holds the bits which indicate whether each button has been pressed

	Bits    | 9 | 8 |   7  |  6 |   5  |  4    |   3   |   2    | 1 | 0 |
	Button  | L | R | DOWN | UP | LEFT | RIGHT | START | SELECT | B | A |
 */
#define BUTTONS ((volatile ushort*) 0x04000130)

// ----------------------- Begin Tiled Mode ------------------------------------
/*
	Background control registers (2 bytes) - Write Only

	Bits   |F E  | D    |C B A 9 8 |7   |6	 |5 4	| 3 2	| 1 0
	Field  |Size |Wrap  |SBB 	   |CM  |Mos | 	~ 	| CBB 	| P	
	
	Size:		Regular		Affine 	(Tile WxH)
		0 	 	32x32 		16x16
		1 		64x32 	 	32x32
		2 		32x64 	 	64x64
		3 		64x64 	  	128x128
	
	Wrap: If set, wraps background vertically and horizontally on display
	SBB : Screen Base Block.Values 0-31 base screenblock for tilemap indices
	CM	: Color Mode. 0 = 16 colors (4bpp), 1 = 256 colors (8bpp) 
	Mos : If set, enables mosaic effect
	CBB : Character Base Block(0-3) character block for tilesheet image
	P   : Determines priority of background (used for draw order 0-3)	
*/
#define BG0_CONTROL (volatile ushort*) 0x4000008
#define BG1_CONTROL (volatile ushort*) 0x400000a
#define BG2_CONTROL (volatile ushort*) 0x400000c
#define BG3_CONTROL (volatile ushort*) 0x400000e 

/*
	Background scroll registers (2 bytes) - Write Only
    Control the rendering offset for the corresponding background
*/
#define BG0_SCROLL_X (volatile short*) 0x4000010
#define BG0_SCROLL_Y (volatile short*) 0x4000012
#define BG1_SCROLL_X (volatile short*) 0x4000014
#define BG1_SCROLL_Y (volatile short*) 0x4000016
#define BG2_SCROLL_X (volatile short*) 0x4000018
#define BG2_SCROLL_Y (volatile short*) 0x400001a
#define BG3_SCROLL_X (volatile short*) 0x400001c
#define BG3_SCROLL_Y (volatile short*) 0x400001e 

/*
	Palettes are used to store all colors used by an image
	Background palette is at 05000000h 
 	Sprite palette is at 05000200h
	Each palette is 0x0200 (256) bytes
*/
#define BG_PALETTE ((volatile ushort*) 0x05000000)
#define SPRITE_PALETTE ((volatile ushort*) 0x05000200)

/*
    Address where sprite image data is stored. 
    Each value is an 2 byte index into the color palette
*/
#define SPRITE_IMAGE_BLOCK ((volatile ushort*) 0x6010000)

/*
OAM - Object Attribute Memory
    Address where the sprite instances are stored (attributes)
    Each Sprite is defined by 8 bytes (2 bytes per attribute)
OBJ Attribute 0 (R/W)

  Bit   Expl.
  0-7   Y-Coordinate           (0-255)
  8     Rotation/Scaling Flag  (0=Off, 1=On)
  When Rotation/Scaling used (Attribute 0, bit 8 set):
    9     Double-Size Flag     (0=Normal, 1=Double)
  When Rotation/Scaling not used (Attribute 0, bit 8 cleared):
    9     OBJ Disable          (0=Normal, 1=Not displayed)
  10-11 OBJ Mode  (0=Normal, 1=Semi-Transparent, 2=OBJ Window, 3=Prohibited)
  12    OBJ Mosaic             (0=Off, 1=On)
  13    Colors/Palettes        (0=16/16, 1=256/1)
  14-15 OBJ Shape              (0=Square,1=Horizontal,2=Vertical,3=Prohibited)

Caution: A very large OBJ (of 128 pixels vertically, ie. a 64 pixels OBJ in a Double Size area) located at Y>128 will be treated as at Y>-128, the OBJ is then displayed parts offscreen at the TOP of the display, it is then NOT displayed at the bottom.

OBJ Attribute 1 (R/W)

  Bit   Details.
  0-8   X-Coordinate           (0-511) 
          When Rotation/Scaling used (Attribute 0, bit 8 set):
    9-13  Rotation/Scaling Parameter Selection (0-31)
          (Selects one of the 32 Rotation/Scaling Parameters that
          can be defined in OAM, for details read next chapter.)
          When Rotation/Scaling not used (Attribute 0, bit 8 cleared):
    9-11  Not used
    12    Horizontal Flip      (0=Normal, 1=Mirrored)
    13    Vertical Flip        (0=Normal, 1=Mirrored)
    14-15 OBJ Size               (0..3, depends on OBJ Shape, see Attr 0)
          Size  Square   Horizontal  Vertical
          0     8x8      16x8        8x16
          1     16x16    32x8        8x32
          2     32x32    32x16       16x32
          3     64x64    64x32       32x64

OBJ Attribute 2 (Read/Write)
  Bit   Expl.
  0-9   Character Name          (0-1023=Tile Number)
  10-11 Priority relative to BG (0-3; 0=Highest)
  12-15 Palette Number   (0-15) (Not used in 256 color/1 palette mode)
 
*/
#define OBJ_BLOCK ((volatile ushort*) 0x7000000)

// ----------------------------- End Tiled Mode ---------------------------------//

// ----------------------------- Begin Pixel Mode ---------------------------------//

/*
    Buffer addresses to be used when double-buffering is enabled (MODE4)
*/
#define FRONT_BUFFER ((volatile ushort*)  0x06000000)
#define BACK_BUFFER ((volatile ushort*)   0x0600A000)
volatile ushort* active_buffer = FRONT_BUFFER;

// The color palette used in graphics Mode 4
#define palette = (volatile ushort*) 0x5000000;
uchar palette_count = 0; 

// ----------------------------- End Pixel Mode ---------------------------------//

/* ------------------------------  Begin DMA  ------------------------------------ 
  DMA (Direct Memory Address) control register (32- bit)

| 1F | 1E | 1D 1C | 1B | 1A | 19 | 18 17 |16 15 | 14 13 12 11 10 | F E D C B A 9 8 7 6 5 4 3 2 1 0
| En | I  | Tm    | -  | C  | R  | Sa    | Da   | -              | 		Size 
 En - enable flag
 I  - Interrupt request, raises interuupt if set
 Tm - Timing Mode. Specifies when the transfer should start.
    00: immediately
    01: at vblank
    10: at hblank
    11: at each scanline?? (unsure untested) 
 C  - chunk size: if 0 halfword(16bit) else if 1 word(32bit)
 R  - Repeats at every vblank or hblank if timing mode is set to either
 Da - Destination adjustment, address behavior after each transfer
    00: increment after each transfer (default)
    01: decrement after each transfer
    10: none; address is fixed
    11: increment the destination during the transfer, and reset it.

Sa - Source Adjustment. Works just like the two bits for the destination 
		Except 11 is not a valid opcode.
Size - the amount of data to be transfered  
*/

// flag enables DMA when written to DMA control register 
#define DMA_ENABLE      0x80000000
#define DMA_SRC_FIXED   0x01000000
#define DMA_AT_REFRESH  0x30000000
#define DMA_AT_VBLANK   0x10000000
#define DMA_AT_HBLANK   0x20000000

// DMA transfer size flags
#define DMA_16 0x00000000
#define DMA_32 0x04000000

#define DMA_CONTROL ((volatile uint*) 0x40000DC)

// DMA register for address of data's source location 
#define DMA_SOURCE ((volatile uint*) 0x40000D4)

// DMA register for address of data's destination 
#define DMA_DESTINATION ((volatile uint*) 0x40000D8)

//------------------------------  End DMA  ------------------------------------// 
// ------------------------------ Begin Timer --------------------------------//
#define TIMER_FREQ_1 0
#define TIMER_FREQ_64 1
#define TIMER_FREQ_256  2
#define TIMER_FREQ_1024 3
#define TIMER_CASCADE 0x4	//Increments timer when preceding timer overflows
#define TIMER_ENABLE 0x0080	// Enable timer

#define TIMER0		((volatile ushort*)0x04000100)
#define TIMER0_CONTROL	((volatile ushort*)0x04000102)	
#define TIMER1		((volatile ushort*)0x04000104)	
#define TIMER1_CONTROL	((volatile ushort*)0x04000106)
#define TIMER2		((volatile ushort*)0x04000108)
#define TIMER2_CONTROL	((volatile ushort*)0x0400010A)
#define TIMER3		((volatile ushort*)0x0400010C)
#define TIMER3_CONTROL	((volatile ushort*)0x0400010E)


void gba_init(uchar mode, uchar sprite_2d)
{
    switch(mode)
    {
        case 0:
            *DISPLAY_CONTROL = GBA_MODE0;
            break;   
        case 1:
            *DISPLAY_CONTROL = GBA_MODE1;
            break;
        case 2:
            *DISPLAY_CONTROL = GBA_MODE2;
            break;
        case 3:
            *DISPLAY_CONTROL = GBA_MODE3 | GBA_BG2;
            return;
        case 4:
            *DISPLAY_CONTROL = GBA_MODE4 | GBA_BG2;
            return;
    }
	
	if(sprite_2d > 0)
		*DISPLAY_CONTROL |= SPRITE_ENABLE | SPRITE_MAP_2D;
	else
		*DISPLAY_CONTROL |= SPRITE_ENABLE | SPRITE_MAP_1D;

}

/* the global interrupt enable register */
volatile ushort* INTERRUPT_ENABLE = (volatile ushort*) 0x4000208;

/* this register stores the individual interrupts we want */
#define INTERRUPT_SELECTION ((volatile ushort*) 0x4000200)

/* this registers stores which interrupts if any occured */
volatile ushort* INTERRUPT_STATE = (volatile ushort*) 0x4000202;

/* the address of the function to call when an interrupt occurs */
volatile uint* INTERRUPT_CALLBACK = (volatile uint*) 0x3007FFC;

/* this register needs a bit set to tell the hardware to send the vblank interrupt */
volatile ushort* display_interrupts = (volatile ushort*) 0x4000004;

/* the interrupts are identified by number, we only care about this one */
#define INTERRUPT_FLAG_VBLANK 0x1

void gba_dummy_callback(){}

static gba_callback vblank_callback = &gba_dummy_callback;

static void internal_gba_vblank_callback()
{
	// disable interrupts
	*INTERRUPT_ENABLE = 0;
	// save state
	ushort interrupt_state = *INTERRUPT_STATE;
	
	vblank_callback();

	// enable interrupts
	*INTERRUPT_ENABLE = 1;	
	// restore state
	*INTERRUPT_STATE = interrupt_state;
}

void gba_vblank_callback(gba_callback on_vblank)
{
	vblank_callback = on_vblank;


	*INTERRUPT_ENABLE = 0;
	*INTERRUPT_CALLBACK = (unsigned int) &internal_gba_vblank_callback;
	*INTERRUPT_SELECTION |= INTERRUPT_FLAG_VBLANK;
	*display_interrupts |= 0x08;
	*INTERRUPT_ENABLE = 1;
}

// wait for the screen to be in vblank
void gba_vsync( ) 
{
	while (*SCAN_VCOUNT >= 160) {}
	while (*SCAN_VCOUNT < 160) {}
}

void gba_wait(uint cycles, gba_callback callback)
{
	*TIMER3= -1;
	*TIMER2= -0x3000;  // 0x3000 ticks till overflow (incrementing timer 3)
	*TIMER2_CONTROL = TIMER_ENABLE | TIMER_FREQ_1024;   // we're using the 1024 cycle timer
	*TIMER3_CONTROL = TIMER_ENABLE | TIMER_CASCADE;

	uint counter_cycles = 0;
	while(1)
	{
		callback();
		counter_cycles++;
        	if(counter_cycles >= cycles)
        	{
			return;
        	}
        	*TIMER2= -0x3000;          // 0x3000 ticks till overflow
        	*TIMER2_CONTROL = TIMER_ENABLE | TIMER_FREQ_1024;
        	*TIMER3_CONTROL = TIMER_ENABLE | TIMER_CASCADE;
	}
}

void gba_wait_sec(uint sec, gba_callback callback)
{
   	 *TIMER3= -1;
	*TIMER2= -0x3000;  // 0x3000 ticks till overflow (incrementing timer 3)
	*TIMER2_CONTROL = TIMER_ENABLE | TIMER_FREQ_1024;   // we're using the 1024 cycle timer
	*TIMER3_CONTROL= TIMER_ENABLE | TIMER_CASCADE;

	uint counter_cycles = 0;
	while(1)
	{
		callback();
		if(*TIMER3 != -1)
		{
			counter_cycles++;
			if((counter_cycles / 60) >= sec)
			{
				return;
			}
			*TIMER2= -0x3000;          // 0x3000 ticks till overflow
			*TIMER2_CONTROL = TIMER_ENABLE | TIMER_FREQ_1024;
			*TIMER3_CONTROL = TIMER_ENABLE | TIMER_CASCADE;
		}
	}
}

void gba_reset()
{
    *BG0_CONTROL = 0;
    *BG1_CONTROL = 0;
    *BG2_CONTROL = 0;
    *BG3_CONTROL = 0;
    *BG0_SCROLL_X = 0;
    *BG1_SCROLL_X = 0;
    *BG2_SCROLL_X = 0;
    *BG3_SCROLL_X = 0;
    *BG0_SCROLL_Y = 0;
    *BG1_SCROLL_Y = 0;
    *BG2_SCROLL_Y = 0;
    *BG3_SCROLL_Y = 0; 

	ushort empty = 0;
	for(int i = 0; i < GBA_CHAR_BLOCK_COUNT; ++i)
		gba_fill16(gba_char_block(i), &empty, GBA_CHAR_BLOCK_SIZE);    
}

void gba_run(void(*on_draw)(), void(*on_input)())
{
	while (1)
	{
		gba_vsync();

        if(on_input) on_input();
        if(on_draw) on_draw();
	}
}

ushort gba_buttons()
{
	return *BUTTONS;
}

ushort gba_button_state(ushort buttons, ushort button)
{
    //if the buttons register anded with the button is zero, then button is down
	//return not zero if 0
	return !(buttons & button);
}

uchar gba_button_down(ushort button)
{
	return gba_button_state(gba_buttons(), button);
}

/*
 16-bit int color is bgr ranged 0-32 (5 bits each color)
    bbbbbgggggrrrrr 
*/
#define RGB2PIXEL(_r, _g, _b) ((_b & 0x1f) << 10) | ((_g & 0x1f) << 5) | (_r & 0x1f)

inline void gba_pixel(int x, int y, uchar r, uchar g, uchar b) 
{
	active_buffer[y * GBA_SCREEN_WIDTH + x] = RGB2PIXEL(r,g,b);
}

inline void gba_clear_palette()
{
    palette_count = 0;
    for(ushort i = 0; i < GBA_PALETTE_COUNT; ++i)
    {
    	BG_PALETTE[i] = 0;
    }
}

inline uchar gba_add_color(uchar r, uchar g, uchar b) 
{
    ushort color = RGB2PIXEL(r,g,b);

    BG_PALETTE[palette_count] = color;
    palette_count++;
    return palette_count - 1;
}

inline void gba_clear_screen(uchar color_index)
{
    const ushort pixel = (color_index & 0x00ff) | (color_index << 8);
    gba_vram_fill16((ushort*)active_buffer, pixel, GBA_SCREEN_SIZE >> 1);
    *DMA_CONTROL |= DMA_AT_REFRESH;
}

inline void gba_set_color(int x, int y, uchar color_index)
{
    //find the pixel index which is the regular index divided by two */
    const ushort index = ((y * GBA_SCREEN_WIDTH + x) >> 1);
    ushort pixel = active_buffer[index];
    // Join the pixels by column
    if (x & 1) //is odd 
    {
        active_buffer[index] = (color_index << 8) | (pixel & 0x00ff);
    } 
    else 
    {
        active_buffer[index] = (pixel & 0xff00) | color_index;
    }
    return;
}

inline uchar gba_color_count()
{
    return palette_count;
}

inline void gba_refresh_screen() 
{
    //Swap buffers
    if(active_buffer == FRONT_BUFFER) 
    {
        // clear back buffer bit and return back buffer pointer */
        *DISPLAY_CONTROL &= ~DISPLAY_BACK_BUFFER;
        active_buffer = BACK_BUFFER;
        return;
    } 
    
    // set back buffer bit and return front buffer */
    *DISPLAY_CONTROL |= DISPLAY_BACK_BUFFER;
    active_buffer = FRONT_BUFFER;
}

inline void gba_draw_rect(uchar x, uchar y, uchar w, uchar h, uchar color_index) 
{
    uchar cx,cy;
	const uchar cx_end = x + w;
    const uchar cy_end = y + h; 
    for (cy = y; cy < cy_end; cy++) 
	{
        for (cx = x; cx < cx_end; cx++) 
        {
            switch(cx % 10)
            {
                case 9: gba_set_color(cx++, cy, color_index);
                case 8: gba_set_color(cx++, cy, color_index);
                case 7: gba_set_color(cx++, cy, color_index);
                case 6: gba_set_color(cx++, cy, color_index);
                case 5: gba_set_color(cx++, cy, color_index);
                case 4: gba_set_color(cx++, cy, color_index);
                case 3: gba_set_color(cx++, cy, color_index);
                case 2: gba_set_color(cx++, cy, color_index);
                case 1: gba_set_color(cx++, cy, color_index);
                case 0: gba_set_color(cx, cy, color_index);
            }
        }
    }
}

// ---------------------------- Tile Mode ------------------------------------
void gba_bg_enable(uchar bg_index, ushort char_block_n, ushort screen_block_n, ushort size, ushort priority, ushort wrap)
{

	*DISPLAY_CONTROL |= (GBA_BG0 << bg_index);

	ushort control_flags = priority 
			  | (char_block_n << 2)     
			  | (MOSAIC_ENABLE << 6)  
			  | (GBA_COLOR_MODE << 7) 
			  | (screen_block_n << 8)
			  | (wrap << 13) 
			  | (size << 14);

	switch(bg_index)
	{
		case 0: *BG0_CONTROL = control_flags; 
				break;
		case 1: *BG1_CONTROL = control_flags; 
				break;
		case 2: *BG2_CONTROL = control_flags; 
				break;
		case 3: *BG3_CONTROL = control_flags; 
				break;
	}
}

void gba_bg_disable(uchar bg_index)
{
	*DISPLAY_CONTROL &= ~(GBA_BG0  << bg_index);
}

void gba_bg_reset()
{
	ushort* char_block = gba_char_block(0);
	ushort empty = 0; 
	gba_fill16(char_block, &empty, 0x2000);
}

void gba_bg_palette(const ushort* palette_data)
{
  	gba_copy16((ushort*)BG_PALETTE, palette_data, GBA_PALETTE_COUNT);
}

// Load background image data into char block n
void gba_bg_image(uint char_block_n, const uchar* image_data, uint width, uint height)
{
    ushort* char_block = gba_char_block(char_block_n);
    //divide by 2, Since we are transfer bytes, but the dma expects 16 bit values, we are counting 2 chars per short
	gba_copy16(char_block, (ushort*)image_data, (width * height) / 2);
}

// Load background image data into char block n
void gba_bg_tilemap(uint screen_block_n, const ushort* tilemap_data, uint width, uint height)
{
    ushort* screen_block = gba_screen_block(screen_block_n);
	gba_copy16(screen_block, tilemap_data, width * height);
}

void gba_bg_get_scroll(uchar bg_index, short* scroll_x, short* scroll_y)
{
	switch(bg_index)
	{
		case 0: *scroll_x = *BG0_SCROLL_X; 
				*scroll_y = *BG0_SCROLL_Y;
				break;
		case 1: *scroll_x = *BG1_SCROLL_X ; 
				*scroll_y = *BG1_SCROLL_Y;
				break;
		case 2: *scroll_x = *BG2_SCROLL_X; 
				*scroll_y = *BG2_SCROLL_Y;
				break;
		case 3: *scroll_x = *BG3_SCROLL_X; 
				*scroll_y = *BG3_SCROLL_Y;
				break;
    }
}

void gba_bg_set_scroll(uchar bg_index, short scroll_x, short scroll_y)
{
	switch(bg_index)
	{
		case 0: *BG0_SCROLL_X = scroll_x; 
				*BG0_SCROLL_Y = scroll_y;
				break;
		case 1: *BG1_SCROLL_X = scroll_x; 
				*BG1_SCROLL_Y = scroll_y;
				break;
		case 2: *BG2_SCROLL_X = scroll_x; 
				*BG2_SCROLL_Y = scroll_y;
				break;
		case 3: *BG3_SCROLL_X = scroll_x; 
				*BG3_SCROLL_Y = scroll_y;
				break;
    }
}

void gba_bg_scroll_by(uchar bg_index, short offset_x, short offset_y)
{
	switch(bg_index)
	{
		case 0: *BG0_SCROLL_X += offset_x; 
				*BG0_SCROLL_Y += offset_y;
				break;
		case 1: *BG1_SCROLL_X += offset_x; 
				*BG1_SCROLL_Y += offset_y;
				break;
		case 2: *BG2_SCROLL_X += offset_x; 
				*BG2_SCROLL_Y += offset_y;
				break;
		case 3: *BG3_SCROLL_X += offset_x; 
				*BG3_SCROLL_Y += offset_y;
				break;
    }
}

void gba_obj_palette(const ushort* palette_data)
{
    gba_copy16((ushort*)SPRITE_PALETTE, palette_data, GBA_PALETTE_COUNT);
}

void gba_obj_image(const uchar* image_data, uint width, uint height)
{
    //divide by 2, Since we are transfer bytes, but the dma expects 16 bit values, we are counting 2 chars per short
    const uint count = (width * height) / 2; 
    gba_copy16((ushort*)SPRITE_IMAGE_BLOCK, (ushort*)image_data, count);
}

// Data aligned structure for an individual object entry in the object ram 
typedef struct __attribute__((aligned (4))) obj_attr
{
	/* Attribute 0
	Bits	|15 14 |13		   |12		| 11 10	 |9 8	 | 7 6 5 4 3 2 1 0
	Field	|Shape |Color Mode |Mosaic	| Effect |Affine | Y pos

	Effect: alpha blending or masking. 
	*/
	ushort attr0;
	/* Attribute 1
	Bits	|15 14 |13		 |12		| 11 10	9 | 8 7 6 5 4 3 2 1 0
	Field	|Size  |V Flip	 |H Flip	| ~	      | X pos
	*/	
	ushort attr1;

	/* Attribute 2
	Bits	|15 14 13 12| 11 10	  | 9 8 7 6 5 4 3 2 1 0
	Field	|Palette 	| Priority| Tile Index (Frame pos ofset)	
	*/	
	ushort attr2;
	/*
	Fill not used
	*/
	ushort attr3;
} obj_attr;

/*				size
                0		1		2		3
   shape	0	8x8		16x16	32x32	64x64
            1	16x8	32x8	32x16	64x32
            2	8x16	8x32	16x32	32x64
*/

//all possible obj attributes that can be loaded into memory at a time
obj_attr obj_attrs[GBA_OBJ_COUNT];
uint obj_attr_index = 0;


int gba_obj_new(uchar size_flags, int priority)
{
    if(obj_attr_index >= GBA_OBJ_COUNT)
    {
        return GBA_OBJ_INVALID;
    }


    const uchar size_flag = (size_flags >> 4);
    const uchar shape_flag = (size_flags & 0xF);
    const uchar h_flip = 0, v_flip =0,  tile_offset =0;
    obj_attrs[obj_attr_index].attr0 =
                    (0)                          //8 bits for y value
                |(0 << 8)                     //affine
                |(0 << 10)	                  //effect
                |(0 << 12)                    //mosaic
                |(GBA_COLOR_MODE << 13)           //color mode
                |((shape_flag & 0x03) << 14); //mask 2 bits of shape

    obj_attrs[obj_attr_index].attr1 =
                        (0)                // 9 bits
                    | (0 << 9)           // not affine flag 
                    | (h_flip << 12)     // horizontal flip flag 
                    | (v_flip << 13)     // vertical flip flag 
                    | ((size_flag& 0x03) << 14); // size 

    obj_attrs[obj_attr_index].attr2 =
                        tile_offset   // tile index 
                        |   (priority << 10) //priority 
                        |   (0 << 12);          // 16 color palette	
	
	uint new_obj_attr_index = obj_attr_index;
    ++obj_attr_index;
    return new_obj_attr_index;
}

inline obj_attr* gba_obj_at(uint obj_index)
{
    if(obj_index < GBA_OBJ_COUNT)
    {
	    return &obj_attrs[obj_index];
    }
    return 0;
}

uchar gba_obj_width(uint obj_index)
{
    obj_attr* obj = gba_obj_at(obj_index);
    if(obj == 0)
    {
        return 0;
    }

    const uchar shape_flag = (obj->attr0 >> 14) & 0x03;
    const uchar size_flag = (obj->attr1 >> 14) & 0x03;

    uchar width = 0;
    switch(shape_flag)
    {
    case 0:
        switch(size_flag)
        {
        case 0: width = 8; break;
        case 1: width = 16; break;
        case 2: width = 32; break;
        case 4: width = 64; break;
        }
        break;
    case 1:
        switch(size_flag)
        {
        case 0: width = 16; break;
        case 1: width = 32; break;
        case 2: width = 32; break;
        case 4: width = 64; break;
        }
        break;
    case 2:
        switch(size_flag)
        {
        case 0: width = 8; break;
        case 1: width = 8; break;
        case 2: width = 16; break;
        case 4: width = 32; break;
        }
        break;
    }
    return width;
}

uchar gba_obj_height(uint obj_index)
{
    obj_attr* obj = gba_obj_at(obj_index);
    if(obj == 0)
    {
        return 0;
    }

    const uchar shape_flag = (obj->attr0 >> 14) & 0x03;
    const uchar size_flag = (obj->attr1 >> 14) & 0x03;
    
	uchar height = 0;
    switch(shape_flag)
    {
    case 0:
        switch(size_flag)
        {
        case 0: height = 8; break;
        case 1: height = 16; break;
        case 2: height = 32; break;
        case 4: height = 64; break;
        }
        break;
    case 1:
        switch(size_flag)
        {
        case 0: height = 8; break;
        case 1: height = 8; break;
        case 2: height = 16; break;
        case 4: height = 32; break;
        }
        break;
    case 2:
        switch(size_flag)
        {
        case 0: height = 16; break;
        case 1: height = 32; break;
        case 2: height = 32; break;
        case 4: height = 64; break;
        }
        break;
    }
    return height;
}

void gba_obj_set_pos(uint obj_index, short x, short y) 
{
    obj_attr* obj = gba_obj_at(obj_index);
    if(obj == 0)
    {
        return;
    }

    // clear lower 8 bits of attr0
    obj->attr0 &= 0xff00;
    // set the new y pos in lower 8 bits by clearing the upper bits 8 of x
    obj->attr0 |= ( y & 0xff);
    // clear lower 9 bits of attr1
    obj->attr1 &= 0xfe00;
    // set the new x pos in lower 9 bits by clearing the upper bits 7 of x
    obj->attr1 |= ( x & 0x1ff);
}

void gba_obj_get_pos(uint obj_index, short *x, short *y)
{
    obj_attr* obj = gba_obj_at(obj_index);
    if(obj == 0)
    {
        return;
    }
	*x = obj->attr1 & 0x01ff;
	*y = obj->attr0 & 0x00ff;
}

void gba_obj_set_offset(uint obj_index, short offset) 
{
    obj_attr* obj = gba_obj_at(obj_index);
    if(obj == 0)
    {
        return;
    }
    /* clear the old offset */
    obj->attr2 &= 0xfc00;
    /* apply the new one */
    obj->attr2 |= (offset & 0x03ff);
}

void gba_obj_move_by(uint obj_index, short dx, short dy) 
{
    //add by deltas
    short x, y;
    gba_obj_get_pos(obj_index, &x, &y);
    x = (x + dx);
    y = (y + dy);
    gba_obj_set_pos(obj_index, x, y);
}

void gba_obj_snap(uint obj_index) 
{
    short x, y;
    gba_obj_get_pos(obj_index, &x, &y);
    x = x >> 8; // divide by eight to pixel position
    y = y >> 8;
    gba_obj_set_pos(obj_index, x, y);
}

void gba_obj_flip(uint obj_index, char h_flip, char v_flip)
{
    obj_attr* obj = gba_obj_at(obj_index);
    if(obj == 0)
    {
        return;
    }

	//Attr1 | 0 0 V  H  |0 0 0 0 |0 0 0 0 |0 0 0 0
	// 	H	| 1 1 1  0  |1 1 1 1 |1 1 1 1 |1 1 1 1 
	// 	V	| 1 1 0  1  |1 1 1 1 |1 1 1 1 |1 1 1 1
	if(v_flip)
		obj->attr1 |=  0x2000; //set flags 
	else
		obj->attr1 &=  0xDFFF; //clear flags 
	
	if(h_flip)
		obj->attr1 |=  0x1000; //set flags 
	else
		obj->attr1 &=  0xEFFF; //clear flags 
}

void gba_obj_reset_all() 
{
    /* clear the index counter */
    obj_attr_index = 0;
    /* move all sprites offscreen to hide them */
    for(int i = 0; i < GBA_OBJ_COUNT; i++) 
	{
        obj_attrs[i].attr0 = (GBA_SCREEN_HEIGHT & 0xff);
        obj_attrs[i].attr1 = (GBA_SCREEN_WIDTH & 0x1ff);
        obj_attrs[i].attr2 = 0;
    }
    gba_obj_update_all();
}

void gba_obj_update_all() 
{
    static const int obj_attr_size = GBA_OBJ_COUNT * sizeof(obj_attr)/sizeof(short);
    // load obj attrs into object block
    gba_copy16((ushort*) OBJ_BLOCK, (ushort*)obj_attrs, obj_attr_size);
}

/* --------------------------------- Memory Utilities ----------------------------------------- */

// Returns base address of nth character block (0-3)	
ushort* gba_char_block(unsigned long block_n)
{
	//calculate distance from base vram addr by multiplying by char block size 16 kb
	return (ushort*)(0x06000000 + block_n*GBA_CHAR_BLOCK_SIZE); //0x4000 = 16 kb
}

//Returns base address of nth screen block (0-31)	
ushort* gba_screen_block(unsigned long block_n)
{
	//calculate distance from base vram addr by multiplying by screen block size 2kb
	return (ushort*)(0x06000000 + block_n*GBA_SCREEN_BLOCK_SIZE); //0x800 = 2 kb
}

//Given the char block base address return the nth block index (0-4)	
unsigned long gba_char_block_offset(ushort* block)
{
	//calculate distance from base vram addr
	return (unsigned long)(block - VRAM)/GBA_CHAR_BLOCK_SIZE;
}

//Given the screen block base address return the nth block index (0-31)
unsigned long gba_screen_block_offset(ushort* block)
{
	//calculate distance from base vram addr
	return (unsigned long)(block - VRAM)/GBA_SCREEN_BLOCK_SIZE;
}

void gba_copy16(ushort* dest, const ushort* source, ushort size) 
{
    *DMA_SOURCE = (uint) source;
    *DMA_DESTINATION = (uint) dest;
    *DMA_CONTROL = size | DMA_16 | DMA_ENABLE;
}

void gba_copy32(uint* dest, const uint* source, ushort size) 
{
    *DMA_SOURCE = (uint) source;
    *DMA_DESTINATION = (uint) dest;
    *DMA_CONTROL = size | DMA_32 | DMA_ENABLE;
}

void gba_fill16(ushort* dest, const ushort* source, ushort count)
{
    *DMA_SOURCE = (uint) source;
    *DMA_DESTINATION = (uint) dest;
    *DMA_CONTROL = count | DMA_16 | DMA_ENABLE | DMA_SRC_FIXED;  
}

void gba_fill32(uint* dest, const uint* source, ushort count)
{
    *DMA_SOURCE = (uint) source;
    *DMA_DESTINATION = (uint) dest;
    *DMA_CONTROL = count | DMA_32 | DMA_ENABLE | DMA_SRC_FIXED;
}

void gba_vram_copy16(ushort* dest, const ushort* source, ushort size) 
{
    *DMA_SOURCE = (uint) source;
    *DMA_DESTINATION = (uint) dest;
    *DMA_CONTROL = size | DMA_16 | DMA_ENABLE | DMA_AT_REFRESH;
}

void gba_vram_copy32(uint* dest, const uint* source, ushort size) 
{
    *DMA_SOURCE = (uint) source;
    *DMA_DESTINATION = (uint) dest;
    *DMA_CONTROL = size | DMA_32 | DMA_ENABLE | DMA_AT_REFRESH;
}

ushort pending_source16;
void gba_vram_fill16(ushort* dest, const ushort source, ushort count)
{
    pending_source16 = source;
    *DMA_SOURCE = (uint)((ushort*)&pending_source16);
    *DMA_DESTINATION = (uint)(dest);
    *DMA_CONTROL = (count) | DMA_16 | DMA_ENABLE | DMA_SRC_FIXED;
    *DMA_CONTROL |= DMA_AT_REFRESH;
}

uint pending_source32;
void gba_vram_fill32(uint* dest, const uint source, ushort count)
{
    pending_source32 = source;
    *DMA_SOURCE = (uint)((ushort*)&pending_source32);
    *DMA_DESTINATION = (uint)(dest);
    *DMA_CONTROL = (count) | DMA_32 | DMA_ENABLE | DMA_SRC_FIXED;
    *DMA_CONTROL |= DMA_AT_REFRESH;
}
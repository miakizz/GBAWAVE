/**************************************\
* pin8gba.h                            *
* header file for compiling game boy   *
* advance software with devkit advance *
\**************************************/


/* Copyright 2001-2003 Damian Yerrick

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   (hint: look at LessGPL.txt)

*/


/* Why the funny register names?

The register names do not match the names in the official GBA
documentation.  Because I have not signed a confidentiality agreement
with Nintendo, I try to keep the fsck away from any official-looking
documentation, so that I'm not considered "tainted" by trade secrets.

That said, I choose the names that seem most logical to me.
In addition, I do slick macro tricks with the nametables and with
registers that control timers, DMA, and backgrounds.

*/

#ifndef PIN8GBA_H
#ifdef __cplusplus
extern "C" {
#endif
#define PIN8GBA_H

/* Information based on CowBite spec
   http://www.gbadev.org/files/CowBiteSpec.txt
   and information gleaned from gbadev list
*/

#define IN_EWRAM __attribute__ ((section(".ewram")))
#define IN_IWRAM __attribute__ ((section(".iwram")))
/* and use like this:
   int array[100] IN_EWRAM;
   char temp IN_IWRAM;
*/

#define CODE_IN_IWRAM __attribute__((section(".iwram"),long_call))
/* and use like this:
   void fun(void) CODE_IN_IWRAM;

   void fun(void)
   {
     // ...
   }
*/

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;


/* put MULTIBOOT at the top of the main src file to get a ROM that
   will work on both mbv2 and flash carts
*/
#define MULTIBOOT int __gba_multiboot;


/* Generic areas of GBA memory */
#define EWRAM  ((u8 *)0x02000000)   /* 256 KB */
#define IWRAM  ((u8 *)0x03000000)   /* 32 KB, fast */
#define IORAM  ((u8 *)0x04000000)   /* 1 KB */
#define PALRAM ((u16 *)0x05000000)  /* 0x200 words */
#define VRAM   ((u16 *)0x06000000)  /* 0xc000 words */
#define ROM    ((u8 *)0x08000000)   /* up to 32 megabytes */


/* LCD mode (0x04000000)
   Corresponds to NES's $2000 and $2001

fedcba9876543210  Register 0x04000000
||||||||||||||||
|||||||||||||+++- Display mode
||||||||||||+---- GBC mode (set to 0)
|||||||||||+----- In modes 4 and 5, view second page
||||||||||+------ Unlock OAM during hblank (fewer available sprites)
|||||||||+------- 0: rowpitch for sprites is 1024 bytes;
|||||||||         1: rowpitch for sprites is spr.width tiles
||||||||+-------- Blank LCD, including background color
|||||||+--------- Show BG layer 0
||||||+---------- Show BG layer 1
|||||+----------- Show BG layer 2
||||+------------ Show BG layer 3
|||+------------- Show sprites
||+-------------- Enable window 0
|+--------------- Enable window 1
+---------------- Enable window 2

*/
#define LCDMODE (*(volatile u16 *)0x04000000)
#define LCDMODE_PAGE(x)   (((x) & 1) << 4)
#define LCDMODE_UNLOCKOAM 0x0020
#define LCDMODE_1DSPR     0x0040
#define LCDMODE_BLANK     0x0080
#define LCDMODE_BG0       0x0100
#define LCDMODE_BG1       0x0200
#define LCDMODE_BG2       0x0400
#define LCDMODE_BG3       0x0800
#define LCDMODE_SPR       0x1000
#define LCDMODE_WIN0      0x2000
#define LCDMODE_WIN1      0x4000
#define LCDMODE_SPRWIN    0x8000


/* LCD Status (0x04000004)
   Corresponds to NES's $2002, $4017, and MMC3 latches

fedcba9876543210  Register 0x04000004
||||||||  ||||||
||||||||  |||||+- 0: in refresh (160 scanlines)
||||||||  |||||   1: in vblank  (68 scanlines)
||||||||  ||||+-- 0: in refresh (1004 cycles)
||||||||  ||||    1: in hblank  (228 cycles)
||||||||  |||+--- we THINK this is set to 1 when a Y trigger int occurs
||||||||  ||+---- enable vblank interrupt source
||||||||  |+----- enable hblank interrupt source
||||||||  +------ enable Y trigger interrupt source
++++++++--------- scanline on which to trigger interrupt

*/
#define LCDSTAT          (*(volatile u16 *)0x04000004)
#define LCDSTAT_VBLANK   Ox0001
#define LCDSTAT_HBLANK   0x0002
#define LCDSTAT_VCOUNT   0x0004
#define LCDSTAT_VBLIRQ   0x0008
#define LCDSTAT_HBLIRQ   0x0010
#define LCDSTAT_VCIRQ    0x0020
#define LCDSTAT_Y(n)     ((n) << 8)


/* LCD Y position */
#define LCD_Y (*(volatile u16 *)0x04000006)  /* 0-159: draw; 160-227 vblank */


/* Background control (0x04000008-0x0400000e)
   Corresponds to NES's $2000
   This is a vector of four registers, one for each bg.

fedcba9876543210
||||||||||  ||||
||||||||||  ||++- Priority (00 closest; 11 furthest)
||||||||||  ++--- Pattern table to use (0x4000 bytes each)
|||||||||+------- Enable mosaic effect
||||||||+-------- Text: 0: 4-bit tiles; 16 palettes of 15 colors each
||||||||                1: 8-bit tiles; 256 colors
||||||||          Affine: Ignored; always treated as 1 (8-bit tiles)
|||+++++--------- Nametable to use (0x800 bytes each)
||+-------------- Affine: 0: Area outside map is transparent
||                           (map is homeomorphic to a disc)
||                        1: Area outside map wraps
||                           (map is homeomorphic to a torus)
||                Text: Ignored; always treated as 1 (map wraps)
++--------------- Size of map
                  Text: 00 32x32 tiles
                        01 64x32 tiles
                        10 32x64 tiles
                        11 64x64 tiles
                  Affine: 00 16x16 tiles
                          01 32x32 tiles
                          10 64x64 tiles
                          11 128x128 tiles

*/
#define BGCTRL ((volatile u16 *)0x04000008)
#define BGCTRL_PAT(m)    ((m) << 2)
#define BGCTRL_MOSAIC    0X0040
#define BGCTRL_16C       0x0000
#define BGCTRL_256C      0X0080
#define BGCTRL_NAME(m)   ((m) << 8)
#define BGCTRL_M7WRAP    0x2000
#define BGCTRL_H32       0x0000
#define BGCTRL_H64       0x4000
#define BGCTRL_V32       0x0000
#define BGCTRL_V64       0x8000
#define BGCTRL_M7_16     0x0000
#define BGCTRL_M7_32     0x4000
#define BGCTRL_M7_64     0x8000
#define BGCTRL_M7_128    0xc000


/* Background scroll registers for text bgs (0x04000010-0x0400001E)
   Corresponds to NES's $2005

To scroll bg1 to (3, 7), do this:
BGSCROLL[1].x = 3;
BGSCROLL[1].y = 7;

Affine backgrounds ignore these registers.

*/
struct BGPOINT
{
  u16 x, y;
};

#define BGSCROLL ((volatile struct BGPOINT *)0x04000010)



/* Background affine registers (0x04000020-0x0400003e)

There are only two of these, BGAFFINE[2] and BGAFFINE[3].

*/
struct BGAFFINEREC
{
  u16 pa;  /* map_x increment per pixel */
  u16 pb;  /* map_x increment per scanline */
  u16 pc;  /* map_y increment per pixel */
  u16 pd;  /* map_y increment per scanline */
  u32 x_origin, y_origin;
};

#define BGAFFINE ((volatile struct BGAFFINEREC *)0x04000000)




/* The joypad

The joypad is wired with resistor pullups to a positive voltage (3.3V
or 5V depending on GBA or GBC mode).  Pressing a button results in a
short to ground. This makes the joypad ACTIVE LOW, that is:

  0xffff represents "no buttons pressed" and
  0xfff6 represents "buttons A and B pressed"

It can also make an interrupt when any or all buttons in combination
are pressed.  Because of the bouncy button contacts, it will usually
trigger multiple interrupts for a given button, so in your ISR, turn
off triggered buttons in JOYIRQ until the next vblank.

*/
#define JOY (*(volatile u16 *)0x04000130)
#define JOY_A            0x0001
#define JOY_B            0x0002
#define JOY_SELECT       0x0004
#define JOY_START        0x0008
#define JOY_RIGHT        0x0010
#define JOY_LEFT         0x0020
#define JOY_UP           0x0040
#define JOY_DOWN         0x0080
#define JOY_R            0x0100
#define JOY_L            0x0200

#define JOYIRQ (*(volatile u16 *)0x04000132)
#define JOYIRQ_ANY      0x4000
#define JOYIRQ_ALL      0xc000


/* DMA

DMA is a way of transferring information from one part of memory to
another

DMA[0]  Raster effects (to BGSCROLL, BGAFFINE, PALRAM, etc)
DMA[1]  Dsound A
DMA[2]  Dsound B
DMA[3]  Typically used by memcpy()

All DMA channels can copy from RAM (0x02000000-0x07ffffff) or to RAM.
Only some can copy from ROM (0x08000000-0x0dffffff); none can read or
write 8-bit cart memory (0x0e000000-0x0e00ffff).

Ch      Fr ROM  To ROM  Max len Special trigger
DMA[0]  No      No      16384   None
DMA[1]  Yes     No      16384   Sound FIFO
DMA[2]  Yes     No      16384   Sound FIFO
DMA[3]  Yes     Yes     65536   Hblank, delayed by 2 lines (?)

The count register only copies up to 16,383 u16's or u32's; the DMA
controller ignores the high bits.

fedcba9876543210  DMA Control (0x40000BA, 0x40000C6, 0x40000D2, 0x40000DE)
|||| ||||||
|||| ||||++------ 0: inc dest after each copy
|||| ||||         1: dec dest after each copy
|||| ||||         2: leave dest unchanged
|||| ||||         3: inc dest after each copy and reset after end of transfer
|||| ||++-------- 0: inc source after each copy
|||| ||           1: dec source after each copy
|||| ||           2: leave source unchanged
|||| |+---------- Repeat transfer on every trigger
|||| +----------- 0: copy u16's; 1: copy u32's
||++------------- 0: copy now; 1: copy on vblank; 2: copy on hblank;
||                3: copy on channel's special trigger
|+--------------- Generate IRQ on completion (not verified)
+---------------- Enable this channel

*/
struct DMACHN
{
  const void *src;
  void *dst;
  u16 count;
  u16 control;
};

#define DMA ((volatile struct DMACHN *)0x040000b0)
#define DMA_DSTINC      0x0000
#define DMA_DSTDEC      0x0020
#define DMA_DSTUNCH     0x0040
#define DMA_DSTINCRESET 0x0060
#define DMA_SRCINC      0x0000
#define DMA_SRCDEC      0x0080
#define DMA_SRCUNCH     0x0100
#define DMA_REPEAT      0x0200
#define DMA_U16         0x0000
#define DMA_U32         0x0400
#define DMA_COPYNOW     0x8000
#define DMA_VBLANK      0x9000
#define DMA_HBLANK      0xA000
#define DMA_SPECIAL     0xB000
#define DMA_IRQ         0x4000

#define DMA_ENABLE      0x8000  /* deprecated */


/* The palette

The bitmap in High-color modes (3 and 5) and the palette in indexed
modes (0, 1, 2, 4, sprites) use the following format for colors:

fedcba9876543210
 |||||||||||||||
 ||||||||||+++++- Red value
 |||||+++++------ Green value
 +++++----------- Blue value

It is recommended to treat the RGB intensities as being in a range
8-31 (rather than 0-31) to allow for the darkness of the GBA screen.

*/
#define RGB(r,g,b) ((r)|(g)<<5|(b)<<10)


/* Pattern tables */

#define PATRAM(x) ((u32 *)(0x06000000 | ((x) << 14)))
#define PATRAM4(x, c) ((u32 *)(0x06000000 | (((x) << 14) + ((c) << 5)) ))
#define PATRAM8(x, c) ((u32 *)(0x06000000 | (((x) << 14) + ((c) << 6)) ))
#define SPR_VRAM(x) ((u32 *)(0x06010000 | ((x) << 5)))


/* Nametables

   They may or may not be called "nametables" in official
   Nintendo GBA documentation, but because I come from
   the NES community, and the NES community calls the
   map data areas "nametables", that's what I call them.
*/
typedef u16 NAMETABLE[32][32];

#define MAP ((NAMETABLE *)0x06000000)

/* Each nametable entry

fedcba9876543210  Text nametable entries
||||||||||||||||
||||||++++++++++- Tile number
|||||+----------- Flip tile horizontally
||||+------------ Flip tile vertically
++++------------- Palette

fedcba9876543210  Affine nametable entries
||||||||||||||||
||||||||++++++++- Tile number of even-numbered tile
++++++++--------- Tile number of odd-numbered tile

*/
/* defines to come later */


/* The sprites */

struct OAM_SPRITE
{
  u16 y;
  u16 x;
  u16 tile;
  u16 reserved;
};

#define GBA_NSPRITES 128
#define OAM ((volatile struct OAM_SPRITE *)0x07000000)


/* Sprite Y register (0x07000xx0, 0x07000xx8)

fedcba9876543210
||||||||||||||||
||||||||++++++++- Y position of nw corner of clipping rect
||||||++--------- 00: no affine; 01: affine clipped to orig. rect;
||||||            10: hide; 11: affine clipped to twice orig. rect
||||++----------- 00: opaque; 01: semitransparent; 10: object window
|||+------------- Enable mosaic
||+-------------- 0: 16-color; 1: 256-color
++--------------- 00: square; 01: wide; 10: tall

*/
#define OAM_YMASK       0x00ff
#define OAM_AFFINE      0x0100  /* set to 1 to use rot/scale */
#define OAM_HIDDEN      0x0200
#define OAM_AFFINE2X    0x0300
#define OAM_TRANSLUCENT 0x0400
#define OAM_OBJWINDOW   0x0800
#define OAM_MOSAIC      0x1000
#define OAM_16C         0x0000
#define OAM_256C        0x2000
#define OAM_SQUARE      0x0000
#define OAM_WIDE        0x4000
#define OAM_TALL        0x8000

/* Sprite X register (0x07000xx2, 0x07000xxA)

fedcba9876543210  (Sprites using affine mapping)
||||||||||||||||
|||||||+++++++++- X position of nw corner of clipping rect
||+++++---------- (If this sprite uses affine) Affine record offset
++--------------- Size of sprite (see below)

fedcba9876543210  (Sprites using direct pixel mapping)
||||   |||||||||
||||   +++++++++- X position of nw corner
|||+------------- Flip horizontally
||+-------------- Flip vertically
++--------------- Size of sprite (see below)

*/
#define OAM_XMASK       0x01ff
#define OAM_AFFINDEX(x) (((x) & 0x1f) << 9)
#define OAM_HFLIP       0x1000
#define OAM_VFLIP       0x2000
#define OAM_SIZE0       0x0000
#define OAM_SIZE1       0x4000
#define OAM_SIZE2       0x8000
#define OAM_SIZE3       0xc000

/* Size table
        square  wide    tall
size 0   8x 8   16x 8    8x16
size 1  16x16   32x 8    8x32
size 2  32x32   32x16   16x32
size 3  64x64   64x32   32x64
*/


/* Sprite name register (0x07000xx4, 0x07000xxC)

fedcba9876543210
||||||||||||||||
||||||++++++++++- Offset of tile data in 32-byte increments from
||||||            0x06010000
||||++----------- Priority (0 front; 3 back; sprites on a given layer
||||              cover background on the same layer)
++++------------- Palette number (16-color sprites only)
*/
#define OAM_TMASK       0x03ff
#define OAM_PRI(x)      (((x) & 0x03) << 10)
#define OAM_PAL(x)      (((x) & 0x0f) << 12)


struct SPRAFFINEREC
{
  u16 reserved0[3];
  u16 pa;  /* map_x increment per pixel */
  u16 reserved1[3];
  u16 pb;  /* map_x increment per scanline */
  u16 reserved2[3];
  u16 pc;  /* map_y increment per pixel */
  u16 reserved3[3];
  u16 pd;  /* map_y increment per scanline */
};

#define GBA_NSPRAFFINES 32
#define SPRAFFINE ((volatile struct SPRAFFINEREC *)0x07000000)


/* Effects *********************************************************/

/* Mosaic size (0x0400004C)

fedcba9876543210
||||||||||||||||
||||||||||||++++- BG X size minus 1
||||||||++++----- BG Y size minus 1
||||++++--------- Sprite X size minus 1
++++------------- Sprite Y size minus 1
*/
#define MOSAIC (*(volatile u16 *)0x0400004c)
#define MOSAIC_BG(x,y) ((y) << 4 | (x))
#define MOSAIC_SPR(x,y) ((y) << 12 | (x) << 8)


/* Blend mode (0x04000050)

fedcba9876543210
  ||||||||||||||
  |||||||||||||+- Blend BG0 on top
  ||||||||||||+-- Blend BG1 on top
  |||||||||||+--- Blend BG2 on top
  ||||||||||+---- Blend BG3 on top
  |||||||||+----- Blend sprites on top
  ||||||||+------ Blend sky (areas with no opaque pixels) on top
  ||||||||        (useless?)
  ||||||++------- 0: No blend; 1: Normal blend;
  ||||||          2: Fade to white; 3: Fade to black
  |||||+--------- Blend BG0 on bottom
  ||||+---------- Blend BG1 on bottom
  |||+----------- Blend BG2 on bottom
  ||+------------ Blend BG3 on bottom
  |+------------- Blend sprites on bottom
  +-------------- Blend color 0 on bottom

Make sure the "top" layers are in front of the "bottom" layers.
*/
#define BLENDMODE (*(volatile u16 *)0x04000050)
#define BLENDMODE_BG0TOP 0x0001
#define BLENDMODE_BG1TOP 0x0002
#define BLENDMODE_BG2TOP 0x0004
#define BLENDMODE_BG3TOP 0x0008
#define BLENDMODE_SPRTOP 0x0010
#define BLENDMODE_SKYTOP 0x0020
#define BLENDMODE_NONE   0x0000
#define BLENDMODE_NORMAL 0x0040
#define BLENDMODE_WHITE  0x0080
#define BLENDMODE_BLACK  0x00c0
#define BLENDMODE_BG0BOT 0x0100
#define BLENDMODE_BG1BOT 0x0200
#define BLENDMODE_BG2BOT 0x0400
#define BLENDMODE_BG3BOT 0x0800
#define BLENDMODE_SPRBOT 0x1000
#define BLENDMODE_SKYBOT 0x2000

/* Blend fraction (0x04000052)

fedcba9876543210
   |||||   |||||
   |||||   +++++- Top layer blend fraction (16ths)
   +++++--------- Bottom layer blend fraction (16ths)
*/
#define BLENDFRAC (*(volatile u16 *)0x04000052)
#define BLENDFRAC_TOP(x) (x)
#define BLENDFRAC_BOT(x) ((x) << 8)

/* Fade fraction (0x04000054)

fedcba9876543210
           |||||
           +++++- Fade fraction (16ths): 16 is pure black or
                  pure white depending on blend mode
*/
#define FADEFRAC (*(volatile u16 *)0x04000050)



/* oooh... sound...  thanks to belogic.com *************************/

/* DMG Sound Control (0x04000080)

fedcba9876543210
|||||||| ||| |||
|||||||| ||| +++- DMG left volume
|||||||| +++----- DMG right volume
|||||||+--------- Enable sqr1 on left
||||||+---------- Enable sqr2 on left
|||||+----------- Enable triangle on left
||||+------------ Enable noise on left
|||+------------- Enable sqr1 on right
||+-------------- Enable sqr2 on right
|+--------------- Enable triangle on right
+---------------- Enable noise on right

*/
#define DMGSNDCTRL         (*(volatile u16 *)0x04000080)
#define DMGSNDCTRL_LVOL(x) (x)
#define DMGSNDCTRL_RVOL(x) ((x) << 4)
#define DMGSNDCTRL_LSQR1   0x0100
#define DMGSNDCTRL_LSQR2   0x0200
#define DMGSNDCTRL_LTRI    0x0400
#define DMGSNDCTRL_LNOISE  0x0800
#define DMGSNDCTRL_RSQR1   0x1000
#define DMGSNDCTRL_RSQR2   0x2000
#define DMGSNDCTRL_RTRI    0x4000
#define DMGSNDCTRL_RNOISE  0x8000

/* Direct Sound Control (0x04000082)

fedcba9876543210
||||||||    ||||
||||||||    ||++- DMG sound output volume
||||||||    ||    (00: 25%; 01: 50%; 10: 100%)
||||||||    |+--- DSound A output volume (0: 50%; 1: 100%)
||||||||    +---- DSound B output volume (0: 50%; 1: 100%)
|||||||+--------- Enable DSound A on right
||||||+---------- Enable DSound A on left
|||||+----------- DSound A sample timer (0 or 1)
||||+------------ DSound A FIFO reset
|||+------------- Enable DSound B on right
||+-------------- Enable DSound B on left
|+--------------- DSound B sample timer (0 or 1)
+---------------- DSound B FIFO reset
*/
#define DSOUNDCTRL           (*(volatile u16 *)0x04000082)
#define DSOUNDCTRL_DMG25     0x0000
#define DSOUNDCTRL_DMG50     0x0001
#define DSOUNDCTRL_DMG100    0x0002
#define DSOUNDCTRL_A50       0x0000
#define DSOUNDCTRL_A100      0x0004
#define DSOUNDCTRL_B50       0x0000
#define DSOUNDCTRL_B100      0x0008
#define DSOUNDCTRL_AR        0x0100
#define DSOUNDCTRL_AL        0x0200
#define DSOUNDCTRL_ATIMER(x) ((x) << 10)
#define DSOUNDCTRL_ARESET    0x0400
#define DSOUNDCTRL_BR        0x1000
#define DSOUNDCTRL_BL        0x2000
#define DSOUNDCTRL_BTIMER(x) ((x) << 14)
#define DSOUNDCTRL_BRESET    0x8000

/* Sound Status (0x04000084)

Note that unlike NES's $4014, bits 0 to 3 of this register are
read-only.  They do not enable sound.

fedcba9876543210
        |   ||||
        |   |||+- Square 1 playing
        |   ||+-- Square 2 playing
        |   |+--- Triangle playing
        |   +---- Noise playing
        +-------- 0: save 10% battery power by turning off ALL sound;
                  1: play sound
*/
#define SNDSTAT        (*(volatile u16*)0x04000084)
#define SNDSTAT_SQR1   0x0001
#define SNDSTAT_SQR2   0x0002
#define SNDSTAT_TRI    0x0004
#define SNDSTAT_NOISE  0x0008
#define SNDSTAT_ENABLE 0x0080

/* Sound Bias: will not be documented.

fedcba9876543210
||    ||||||||||
||    ++++++++++- PWM bias
++--------------- Amplitude resolution
                  00: 9-bit at 32768 Hz
                  01: 8-bit at 65536 Hz (most common)
                  10: 7-bit at 131072 Hz
                  11: 6-bit at 262144 Hz

Do NOT use SNDBIAS directly.  To set the resolution, use
  SETSNDRES(1);
*/
#define SNDBIAS      (*(volatile u16 *)0x04000088)
#define SETSNDRES(x) SNDBIAS = (SNDBIAS & 0x3fff) | (x << 14)

#define DSOUND_FIFOA (*(volatile u32 *)0x040000a0)
#define DSOUND_FIFOB (*(volatile u32 *)0x040000a4)


/* Square 1 Sweep Register

fedcba9876543210
         |||||||
         ||||+++- Sweep shifts (1 fastest; 7 slowest)
         |||+---- 0: Sweep up; 1: Sweep down

Write 0x0040 into this register to disable sweep.

*/
#define SQR1SWEEP   (*(volatile u16 *)0x04000060)
#define SQR1SWEEP_OFF 0x0008


/* Square 1 Control Register
   Square 2 Control Register
   Noise Channel Control Register

fedcba9876543210
||||||||||||||||
||||||||||++++++- Sound length (1 longest; 63: shortest)
||||||||++------- Duty cycle (00: 1/8; 01: 1/4; 10: 1/2; 11: 3/4)
||||||||          (does not apply to noise)
|||||+++--------- Envelope step time (0: off; 1 fastest; 7 slowest)
||||+------------ Envelope direction (0: decrease; 1: increase)
++++------------- Volume

*/
#define SQR1CTRL    (*(volatile u16 *)0x04000062)
#define SQR2CTRL    (*(volatile u16 *)0x04000068)
#define NOISECTRL   (*(volatile u16 *)0x04000078)
#define SQR_DUTY(n) ((n) << 6)
#define SQR_VOL(n)  ((n) << 12)

/* Square 1 Frequency
   Square 2 Frequency
   Triangle Channel Frequency (0x04000074)

fedcba9876543210
||   |||||||||||
||   +++++++++++- frequency (131072 Hz/(2048-x)) (halved for tri channel)
|+--------------- 0: hold note; 1: use length
+---------------- 1: Reset channel

*/
#define SQR1FREQ      (*(volatile u16 *)0x04000064)
#define SQR2FREQ      (*(volatile u16 *)0x0400006c)
#define TRIFREQ       (*(volatile u16 *)0x04000074)
#define FREQ_HOLD  0x0000
#define FREQ_TIMED 0x4000
#define FREQ_RESET 0x8000


/* Noise Channel Frequency (0x0400007C)

fedcba9876543210
||      ||||||||
||      |||||+++- period
||      ||||+---- 0: white noise; 1: 127-step metallic
||      ++++----- octave (0 highest; 13 lowest)
|+--------------- 0: hold note; 1: use length
+---------------- 1: Reset channel

*/
#define NOISEFREQ       (*(volatile u16 *)0x0400007c)
#define NOISEFREQ_127   0x0008
#define NOISEFREQ_OCT(x) ((x) << 4)


/* Triangle Channel Control Register

fedcba9876543210
        |||
        ||+------ Bank mode (0: 2 banks of 32; 1: 1 bank of 64)
        |+------- Play this bank (and write other bank)
        +-------- Enable triangle channel
*/
#define TRICTRL         (*(volatile u16 *)0x04000070)
#define TRICTRL_2X32    0x0000
#define TRICTRL_1X64    0x0020
#define TRICTRL_BANK(x) ((x) << 6)
#define TRICTRL_ENABLE  0x0080

/* Triangle Channel Length/Volume (0x04000072)

fedcba9876543210
|||     ||||||||
|||     ++++++++- Length ((256-x)/256 seconds)
+++-------------- Volume (000: mute; 001: 100%; 010: 50%;
                          011: 25%; 100: 75%)
*/
#define TRILENVOL        (*(volatile u16 *)0x04000072)
#define TRILENVOL_LEN(x) (256 - (x))
#define TRILENVOL_MUTE   0x0000
#define TRILENVOL_25     0x6000
#define TRILENVOL_50     0x4000
#define TRILENVOL_75     0x8000
#define TRILENVOL_100    0x2000

#define TRIWAVERAM ((volatile u32 *)0x04000090)


/* Timers  (0x04000100-0x0400010E)

   The GBA has four up-counting timers.
*/
typedef struct TIMER_REC
{
  unsigned short count;
  unsigned short control;
} TIMER_REC;

/* Timer Control (0x04000102, 0x04000106, 0x0400010A, 0x0400010E)

fedcba9876543210
        ||   |||
        ||   +++- Clock divider
        ||        000: CPU/1; 001: CPU/64; 010: CPU/256; 011: CPU/1024;
        ||        100: Increment when previous timer triggers
        |+------- Generate an interrupt on trigger
        +-------- Enable
*/
#define TIMER ((volatile TIMER_REC *)0x04000100)
#define TIMER_16MHZ      0x0000
#define TIMER_262KHZ     0x0001
#define TIMER_65KHZ      0x0002
#define TIMER_16KHZ      0x0003
#define TIMER_CASCADE    0x0004
#define TIMER_IRQ        0x0040
#define TIMER_ENABLE     0x0080


/* Serial I/O control register

FEDCBA9876543210  Normal mode
 |||    |   ||||
 |||    |   ||++- 0: use external clock; 2: internal 256 kHz clock;
 |||    |   ||    3: internal 2 MHz clock
 |||    |   |+--- State of SI (other GBA's SO) (read)
 |||    |   +---- SO state during ready (0: pull low; 1: leave high)
 |||    +-------- 0: ready; 1: active
 |++------------- Communication mode (0: 8-bit; 1: 32-bit)
 +--------------- 1: Send serial interrupt on completion

FEDCBA9876543210  Multiplayer mode
 |||    ||||||||
 |||    ||||||++- Data rate (0: 9600 bps; 1: 38400 bps; 2: 57600 bps;
 |||    ||||||    3: 115200 bps)
 |||    |||||+--- SI terminal (0: parent; 1: child) (read)
 |||    ||||+---- SD terminal (0: bad connection; 1: GBAs ready)
 |||    ||||      (read)
 |||    ||++----- ID of GBA (0: master; 1-3: slave) (read)
 |||    |+------- Error occurred (1: error) (read)
 |||    +-------- 0: ready; 1: active (master write, slaves read)
 |++------------- Communication mode (2: multiplayer)
 +--------------- 1: Send serial interrupt on completion

FEDCBA9876543210  UART mode
 |||||||||||||||
 |||||||||||||++- Data rate (0: 9600 bps; 1: 38400 bps; 2: 57600 bps;
 |||||||||||||    3: 115200 bps)
 ||||||||||||+--- Wait for CTS (on SC) before sending
 |||||||||||+---- Parity control (0: even; 1: odd)
 ||||||||||+----- Send data full (read)
 |||||||||+------ Receive data empty (read)
 ||||||||+------- Error occurred (read)
 |||||||+-------- 0: 7-bit; 1: 8-bit
 ||||||+--------- Enable FIFO
 |||||+---------- Enable parity
 ||||+----------- Send
 |||+------------ Signal clear to send
 |||              (1: Pull SD (other GBA's SC) low; 0: leave high)
 |++------------- Communication mode (3: UART)
 +--------------- 1: Send serial interrupt on bits 4, 5, or 6 low

When switching modes, especially from UART, write 0x0000 first.
*/
#define SIOCNT          (*(volatile u16 *)0x04000128)
#define SIO_EXTCLOCK    0x0000
#define SIO_256KHZ      0x0002
#define SIO_2MHZ        0x0003
#define SIO_MP9600      0x2000
#define SIO_MP38400     0x2001
#define SIO_MP57600     0x2002
#define SIO_MP115200    0x2003
#define SIO_U9600       0x3000
#define SIO_U38400      0x3001
#define SIO_U57600      0x3002
#define SIO_U115200     0x3003
#define SIO_SI          0x0004
#define SIO_SO          0x0008
#define SIO_MPSD        0x0008
#define SIO_ACTIVE      0x0080
#define SIO_32BIT       0x1000
#define SIO_IRQ         0x4000
#define SIO_UWAITCTS    0x0004
#define SIO_UNOPARITY   0x0000
#define SIO_UEVENPARITY 0x0200
#define SIO_UODDPARITY  0x0208
#define SIO_USENDFULL   0x0010
#define SIO_URECVEMPTY  0x0020
#define SIO_ERROR       0x0040
#define SIO_U8BIT       0x0080
#define SIO_UFIFO       0x0100
#define SIO_USEND       0x0400
#define SIO_UCTS        0x0800


/* Serial I/O multiplayer send and receive registers

In UART mode or 8-bit normal mode, data is sent or received in
the low 8 bits of SIODATA.  In 32-bit normal mode, data is sent
or received in SIODATA32.

In multiplayer mode, each GBA places 16-bit data in SIODATA.
When the master initiates a transfer, all GBAs put their data
on the bus and listen for other GBAs' data, which pops up in
SIOMPRECV[0] (player 1) through SIOMPRECV[3] (player 4).

*/
#define SIOMPRECV ((volatile u16 *)0x04000120)
#define SIODATA   (*(volatile u16 *)0x0400012A)
#define SIODATA32 (*(volatile u32 *)0x04000120)


/* Serial I/O parallel mode register (0x04000134)

In addition to the serial transfer modes, the GBA can use its comm
lines as a 4-port ports into a bidirectional (half duplex) parallel
port.  (Some documents call the parallel mode "general purpose.")
The GBA can also act as a Joybus device connected to a GameCube, but
because nobody outside Nintendo NDA has managed to run one line of
code on a GameCube, it's not all that useful, and I'm not even
thinking about documenting it until I'm capable of testing it.

FEDCBA9876543210
||      ||||||||
||      |||||||+- SC data (parallel mode)
||      ||||||+-- SD data (parallel mode)
||      |||||+--- SI data (parallel mode)
||      ||||+---- SO data (parallel mode)
||      |||+----- SC direction (0 in; 1 out) (parallel mode)
||      ||+------ SD direction (0 in; 1 out) (parallel mode)
||      |+------- SI direction (0 in; 1 out) (parallel mode)
||      +-------- SO direction (0 in; 1 out) (parallel mode)
++--------------- Mode: 0 serial; 2 parallel; 3 GCN Joybus
*/
#define SIOPAR     (*(volatile u16 *)0x04000134)


/* Interrupt Master Enable (0x04000208)

fedcba9876543210
               |
               +- 0: disable all interrupts; 1: enable interrupts
*/
#define INTENABLE       (*(volatile u16 *)0x04000208)


/* Interrupt Mask (0x04000200)

When setting up interrupts, do the following in this order:
1. Save the old ISR.  (You'll need to restore it to reset the GBA.)
2. Install your new ISR.
3. Enable some interrupt sources.
4. Set INTMASK to allow interrupts from those sources.
5. Turn on INTENABLE.

fedcba9876543210
  ||||||||||||||
  |||||||||||||+- 1: Allow VBlank interrupts
  ||||||||||||+-- 1: Allow HBlank interrupts
  |||||||||||+--- 1: Allow VCount interrupts
  ||||||||||+---- 1: Allow TIMER[0] interrupts
  |||||||||+----- 1: Allow TIMER[1] interrupts
  ||||||||+------ 1: Allow TIMER[2] interrupts
  |||||||+------- 1: Allow TIMER[3] interrupts
  ||||||+-------- 1: Allow serial port interrupts
  |||||+--------- 1: Allow DMA[0] interrupts
  ||||+---------- 1: Allow DMA[1] interrupts
  |||+----------- 1: Allow DMA[2] interrupts
  ||+------------ 1: Allow DMA[3] interrupts
  |+------------- 1: Allow joypad press interrupts
  +-------------- 1: Allow cartridge-generated interrupts
*/
#define INTMASK         (*(volatile u16 *)0x04000200)
#define INT_VBLANK      0x0001
#define INT_HBLANK      0x0002
#define INT_VCOUNT      0x0004
#define INT_TIMER(n)    (0x0008 << (n))
#define INT_SERIAL      0x0080
#define INT_DMA(n)      (0x0100 << (n))
#define INT_JOY         0x1000
#define INT_CART        0x2000


typedef void (*ISR)(void);
#define SET_MASTER_ISR(isr) (*(u32 *)0x03007FFC = (u32)isr)
#define GET_MASTER_ISR() ((ISR)*(u32 *)0x03007FFC)

/* yeah, nasty casts, but I was up at 1 AM and couldn't think
   straight enough for type safety */


/* Interrupt Acknowledge (0x04000200)
   BIOS Interrupt Acknowledge (0x03007FF8)

When the GBA BIOS calls your ISR, this register will contain a bit set
for each interrupt that occurred.  Your ISR should do the following:
1. Turn off INTENABLE.
2. Read INTACK and store the value.
3. For each bit set in the stored value, act on the corresponding
   interrupt.  The layout of INTACK is the same as that of INTMASK.
4. Write the stored value to INTACK and or it to BIOS_INTACK.
5. Turn on INTENABLE and return.

*/
#define INTACK          (*(volatile u16 *)0x04000202)
#define BIOS_INTACK     (*(volatile u16 *)0x03007ff8)



#ifdef __cplusplus
}
#endif
#endif

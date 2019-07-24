#include <stdlib.h>
#include "pin8gba.h"
#include "gbfs.h"

extern const char _8x16_fnt[];
extern const unsigned int _8x16_fnt_len;

extern u32 fracumul(u32, u32) __attribute__((long_call));
extern s32 dv(s32, s32) __attribute__((long_call));

/* upcvt_4bit() ************************
   Converts a 1-bit font to GBA 4-bit format.
*/
void upcvt_4bit(void *dst, const u8 *src, size_t len)
{
  u32 *out = dst;

  for(; len > 0; len--)
  {
    u32 dst_bits = 0;
    u32 src_bits = *src++;
    u32 x;

    for(x = 0; x < 8; x++)
    {
      dst_bits <<= 4;
      dst_bits |= src_bits & 1;
      src_bits >>= 1;
    }
    *out++ = dst_bits;
  }
}

static void hud_wline(unsigned int y, const char *s)
{
  unsigned short *dst = MAP[31][y * 2] + 1;
  unsigned int wid_left;

  for(wid_left = 28; wid_left > 0 && *s; wid_left--)
  {
    unsigned char c0 = *s++;

    dst[0] = c0 << 1;
    dst[32] = (c0 << 1) | 1;
    ++dst;
  }
  for(; wid_left > 0; wid_left--, dst++)
  {
    dst[0] = 0x0040;
    dst[32] = 0x0041;
  }
}

static void hud_cls(void)
{
  unsigned int i;

  for(i = 0; i < 32*20; i++)
    MAP[31][0][i] = 0x0040;

  /*hud_wline(1, "GSM Player for GBA");
  hud_wline(2, "by Damian Yerrick");*/
  //I'm sorry Damian, I love your code, but I gotta do it for the aesthetic!
}

void hud_init(void)
{
  PALRAM[0] = RGB(0,0,0);
  PALRAM[1] = RGB(31,31,31);
  LCDMODE = 0;
  upcvt_4bit(VRAM, _8x16_fnt, _8x16_fnt_len);

  BGCTRL[2] = BGCTRL_NAME(31) | BGCTRL_PAT(0);

  hud_cls();
  /*hud_wline(3, "Copyright 2004");

  hud_wline(5, "For more info see");
  hud_wline(6, "TOAST-COPYRIGHT.txt");

  while(LCD_Y < 160) {}*/
  LCDMODE = 0 | LCDMODE_BG2;
}



/* base 10, 10, 6, 10 conversion */
static unsigned int hud_bcd[] =
{
  600, 60, 10, 1  
};


#undef BCD_LOOP
#define BCD_LOOP(b) if(src >= fac << b) { src -= fac << b; c += 1 << b; }

static void decimal_time(char *dst, unsigned int src)
{
  unsigned int i;

  for(i = 0; i < 4; i++)
    {
      unsigned int fac = hud_bcd[i];
      char c = '0';

      BCD_LOOP(3);
      BCD_LOOP(2);
      BCD_LOOP(1);
      BCD_LOOP(0);
      *dst++ = c;
    }
}

struct HUD_CLOCK
{
  unsigned int cycles;
  unsigned char trackno[2];
  unsigned char clock[4];
} hud_clock;

static const char clockmax[4] = {9, 9, 5, 9};

void hud_frame(int locked, unsigned int t)
{
  char line[16];
  char time_bcd[4];

  /* a fractional value for Seconds Per Byte
     1/33 frame/byte * 160 sample/frame * 924 cpu/sample / 2^24 sec/cpu
     * 2^32 fracunits = 1146880 sec/byte fracunits
   */

  t = fracumul(t, 1146880);
  if(t > 5999)
    t = 5999;
  decimal_time(time_bcd, t);

  line[0] = (locked & JOY_SELECT) ? 12 : ' ';
  line[1] = (locked & JOY_START) ? 16 : ' ';
  line[2] = ' ';
  line[3] = hud_clock.trackno[0] + '0';
  line[4] = hud_clock.trackno[1] + '0';
  line[5] = ' ';
  line[6] = ' ';
  line[7] = time_bcd[0];
  line[8] = time_bcd[1];
  line[9] = ':';
  line[10] = time_bcd[2];
  line[11] = time_bcd[3];
  line[12] = '\0';
  hud_wline(9, line);
}


/*void hud_new_song(const char *name, unsigned int trackno)
{
  int upper;

  hud_cls();
  hud_wline(4, "Now playing");
  hud_wline(5, name);
  hud_clock.cycles = 0;

  for(upper = 0; upper < 4; upper++)
    hud_clock.clock[0] = 0;
  upper = dv(trackno, 10);
  hud_clock.trackno[1] = trackno - upper * 10;
  trackno = upper;
  upper = dv(trackno, 10);
  hud_clock.trackno[0] = trackno - upper * 10;
}*/

void hud_new_song(const char *name, GBFS_FILE *fs){
	char imgName[strlen(name)+6];
	strcpy(imgName, "img");
	strcat(imgName, name);
	//while(LCD_Y >= 160);
	//while(LCD_Y < 160);
	gbfs_copy_obj(0x6000000, fs, imgName);
}

void bmp16_rect(int left, int top, int right, int bottom, u32 clr,
    void *dstBase, u32 dstPitch)
{
    int ix, iy;

    u32 width= right-left, height= bottom-top;
    u16 *dst= (u16*)(dstBase+top*dstPitch + left*2);
    dstPitch /= 2;

    // --- Draw ---
    for(iy=0; iy<height; iy++)
        for(ix=0; ix<width; ix++)
            dst[iy*dstPitch + ix]= clr;
}
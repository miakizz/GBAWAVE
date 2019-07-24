/* gsmplay.c
   Plays GSM audio through Game Boy Advance hardware.
*/

/*
 * Copyright 2004 by Damian Yerrick.
 * See the accompanying file "TOAST-COPYRIGHT.txt" for details.
 * THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "pin8gba.h"
#include "gsm.h"
#include "private.h" /* for sizeof(struct gsm_state) */

#include "gbfs.h"

void isr(void);

#if 0
#define PROFILE_WAIT_Y(y) \
	do                    \
	{                     \
	} while (LCD_Y != (y))
#define PROFILE_COLOR(r, g, b) (PALRAM[0] = RGB((r), (g), (b)))
#else
#define PROFILE_WAIT_Y(y) ((void)0)
#define PROFILE_COLOR(r, g, b) ((void)0)
#endif

static void dsound_switch_buffers(const void *src)
{
	DMA[1].control = 0;

	/* no-op to let DMA registers catch up */
	asm volatile("eor r0, r0; eor r0, r0" ::
					 : "r0");

	DMA[1].src = src;
	DMA[1].dst = (void *)0x040000a0; /* write to FIFO A address */
	DMA[1].count = 1;
	DMA[1].control = DMA_DSTUNCH | DMA_SRCINC | DMA_REPEAT | DMA_U32 |
					 DMA_SPECIAL | DMA_ENABLE;
}

void init_sound(void)
{
	TIMER[0].control = 0;
	//turn on sound circuit
	SETSNDRES(1);
	SNDSTAT = SNDSTAT_ENABLE;
	DSOUNDCTRL = 0x0b0e;
	TIMER[0].count = 0x10000 - (924 / 2);
	TIMER[0].control = TIMER_16MHZ | TIMER_ENABLE;
}

/* gsm_init() **************
   This is to gsm_create() as placement new is to new.
*/
void gsm_init(gsm r)
{
	memset((char *)r, 0, sizeof(*r));
	r->nrp = 40;
}

void wait4vbl(void)
{
	asm volatile("mov r2, #0; swi 0x05" ::
					 : "r0", "r1", "r2", "r3");
}

struct gsm_state decoder;
const GBFS_FILE *fs;
const char *src;
unsigned int src_len;

signed short out_samples[160];
signed char double_buffers[2][608] __attribute__((aligned(4)));

#if 0

static void dsound_silence(void)
{
  DMA[1].control = 0;
}

void pre_decode_run(void)
{
  const char *src_pos;
  const char *src_end;
  char *dst_pos = EWRAM;

  src = gbfs_get_obj(fs, "butterfly.pcm.gsm", &src_len);
  if(!src)
    {
      LCDMODE = 0;
      PALRAM[0] = RGB(31, 23, 0);
      while(1) {}
    }

  src_pos = src;
  src_end = src_pos + src_len;

  gsm_init(&decoder);

  while(src_pos < src_end)
    {
      unsigned int i;

      while(LCD_Y != 0 && LCD_Y != 76 && LCD_Y != 152) {}

      PALRAM[0] = RGB(0, 0, 31);
      if(gsm_decode(&decoder, src_pos, out_samples))
	{
	  LCDMODE = 0;
	  PALRAM[0] = RGB(31, 31, 0);
	  while(1) {}
	}
      src_pos += sizeof(gsm_frame);
      PALRAM[0] = RGB(0, 0, 15);
      for(i = 0; i < 160; i++)
	dst_pos[i] = out_samples[i] >> 8;
      dst_pos += 160;
      PALRAM[0] = RGB(0, 0, 0);
    }
  PALRAM[0] = RGB(0, 31, 0);

  dsound_switch_buffers(EWRAM);

}
#endif

#define CMD_START_SONG 0x0400

//void reset_gba(void) __attribute__((long_call));
void hud_init(void);
void hud_new_song(const char *name, GBFS_FILE *fs);
void hud_frame(int locked, unsigned int t);
void bmp16_rect(int left, int top, int right, int bottom, u32 clr, void *dstBase, u32 dstPitch);

void streaming_run(void)
{
	const char *src_pos = NULL;
	const char *src_end = NULL;
	unsigned int decode_pos = 160, cur_buffer = 0;
	unsigned short last_joy = 0x3ff;
	unsigned int cur_song = (unsigned int)(-1);
	int last_sample = 0;
	int locked = 0;

	while (1)
	{
		unsigned short j = (JOY & 0x3ff) ^ 0x3ff;
		unsigned short cmd = j & (~last_joy | JOY_R | JOY_L);
		signed char *dst_pos = double_buffers[cur_buffer];

		last_joy = j;

		/*      if((j & (JOY_A | JOY_B | JOY_SELECT | JOY_START))
         == (JOY_A | JOY_B | JOY_SELECT | JOY_START))
        reset_gba(); */

		if (cmd & JOY_SELECT)
			locked ^= JOY_SELECT;

		if (locked & JOY_SELECT)
			cmd = 0;

		if (cmd & JOY_START)
			locked ^= JOY_START;

		if (cmd & JOY_L)
		{
			src_pos -= 33 * 50;
			if (src_pos < src)
				cmd |= JOY_LEFT;
		}

		if (cmd & JOY_R)
		{
			src_pos += 33 * 50;
		}

		if (src_pos >= src_end)
			cmd |= JOY_RIGHT;

		if (cmd & JOY_RIGHT)
		{
			cur_song++;
			if (cur_song >= (gbfs_count_objs(fs)/2))
				cur_song = 0;
			cmd |= CMD_START_SONG;
		}

		if (cmd & JOY_LEFT)
		{
			if (cur_song == 0)
				cur_song = (gbfs_count_objs(fs)/2) - 1;
			else
				cur_song--;
			cmd |= CMD_START_SONG;
		}

		if (cmd & CMD_START_SONG)
		{
			char name[25];
			gsm_init(&decoder);
			src = gbfs_get_nth_obj(fs, cur_song, name, &src_len);
			//hud_new_song(name, cur_song + 1);
			hud_new_song(name, fs);
			if (cmd & JOY_L)
				src_pos = src + src_len - 33 * 60;
			else
				src_pos = src;
			src_end = src + src_len;
		}

		PROFILE_WAIT_Y(0);

		// PALRAM[0] = RGB(22, 0, 0);

		if (locked & JOY_START) /* if paused */
			for (j = 304 / 2; j > 0; j--)
			{
				*dst_pos++ = last_sample >> 8;
				*dst_pos++ = last_sample >> 8;
				*dst_pos++ = last_sample >> 8;
				*dst_pos++ = last_sample >> 8;
			}
		else
			for (j = 304 / 4; j > 0; j--)
			{
				int cur_sample;
				if (decode_pos >= 160)
				{
					if (src_pos < src_end)
						gsm_decode(&decoder, src_pos, out_samples);
					src_pos += sizeof(gsm_frame);
					decode_pos = 0;
				}

				/* 2:1 linear interpolation */
				cur_sample = out_samples[decode_pos++];
				*dst_pos++ = (last_sample + cur_sample) >> 9;
				*dst_pos++ = cur_sample >> 8;
				last_sample = cur_sample;

				cur_sample = out_samples[decode_pos++];
				*dst_pos++ = (last_sample + cur_sample) >> 9;
				*dst_pos++ = cur_sample >> 8;
				last_sample = cur_sample;

				cur_sample = out_samples[decode_pos++];
				*dst_pos++ = (last_sample + cur_sample) >> 9;
				*dst_pos++ = cur_sample >> 8;
				last_sample = cur_sample;

				cur_sample = out_samples[decode_pos++];
				*dst_pos++ = (last_sample + cur_sample) >> 9;
				*dst_pos++ = cur_sample >> 8;
				last_sample = cur_sample;
			}

		PROFILE_COLOR(27, 27, 27);
		wait4vbl();
		dsound_switch_buffers(double_buffers[cur_buffer]);
		PROFILE_COLOR(27, 31, 27);

		bmp16_rect(4,149, 4+(232*((src_pos-src)/((double)src_len))), 154, 0, 0x6000000, 480);
		//hud_frame(locked, src_pos - src); TODO: Add Progress bar here?
		cur_buffer = !cur_buffer;
	}
}

void copr_wait(void)
{
	unsigned int vbls = 360;

	do
	{
		wait4vbl();
	} while (--vbls);
}

int main(void)
{
	/* enable interrupts */
	SET_MASTER_ISR(isr);
	LCDSTAT = LCDSTAT_VBLIRQ;			 /* one plug to the display */
	INTMASK = INT_VBLANK | INT_TIMER(1); /* the other to the isr */
	INTENABLE = 1;						 /* and flip the switch */
	fs = find_first_gbfs_file(find_first_gbfs_file);
	if (!fs)
	{
		LCDMODE = 0;
		PALRAM[0] = RGB(31, 0, 0);
		while (1){}
	}
	LCDMODE = 0x0400 | 0x0003; //Set Screen to mode three
	//gbfs_copy_obj(0x6000000, fs, "test1");
	//hud_init();
	//copr_wait();
	init_sound();

	streaming_run();
	while(1){}
}

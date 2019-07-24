/*

Tetanus On Drugs for GBA
isr.c : interrupt service routine
Copyright (C) 2002  Damian Yerrick

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to 
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
GNU licenses can be viewed online at http://www.gnu.org/copyleft/

Visit http://www.pineight.com/ for more information.

*/

volatile int want_reset = 0;

#include "pin8gba.h"

#define INTACK          (*(volatile u16 *)0x04000202)
#define BIOS_INTACK     (*(volatile u16 *)0x03007ff8)
#define INTMASK_VBL     0x0001

#define INTENABLE       (*(volatile u16 *)0x04000208)


void dsound_vblank(void);

void isr(void)
{
  unsigned int interrupts = INTACK;

#if 0
  if(interrupts & INTMASK_VBL)
  {
    dsound_vblank();
  }
#endif
  BIOS_INTACK |= interrupts;
  INTACK = interrupts;
  INTENABLE = 1;
}

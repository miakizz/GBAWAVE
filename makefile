SONGS = gsms/*.gsm
IMAGES = images/*

ARMGCC = arm-agb-elf-gcc
ARMOBJ = arm-agb-elf-objcopy
GBAEMU = E:/gbadev/vboy/VisualBoyAdvance
TOOLS = tools/

ROM_CFLAGS = -Wall -O2 -mthumb -mthumb-interwork
IWRAM_CFLAGS = -Wall -O3 -marm -mthumb-interwork
LDFLAGS = -Wall -mthumb -mthumb-interwork

.PHONY: songs run clean

#run: gsm.gba
#	$(GBAEMU) $^

songs: gsmsongs.gbfs

gsmsongs.gbfs: $(SONGS)
#	$(TOOLS)gbfs $@ $^
	$(TOOLS)gbfs $@ gsms/*.gsm images/*
images.gbfs: $(IMAGES)
	$(TOOLS)gbfs $@ images/*

chr.s: 8x16.fnt
	$(TOOLS)bin2s $^ > $@

%.fnt: %.bmp
	$(TOOLS)bmp2tiles -W 8 -H 16 -b 1bpp $^ $@

%.o: %.c
	$(ARMGCC) $(ROM_CFLAGS) -c $^ -o $@

%.iwram.o: %.c
	$(ARMGCC) $(IWRAM_CFLAGS) -c $^ -o $@

%.ewram.o: %.c
	$(ARMGCC) $(ROM_CFLAGS) -c $^ -o $@

%.o: %.s
	$(ARMGCC) $(ROM_CFLAGS) -c $^ -o $@

%.iwram.o: %.s
	$(ARMGCC) $(IWRAM_CFLAGS) -c $^ -o $@

x.elf: gsmplay.o hud.o gsmcode.iwram.o isr.iwram.o chr.o asm.iwram.o libgbfs.o
	$(ARMGCC) $(LDFLAGS) $^ -o $@

%.bin: %.elf
	$(ARMOBJ) -O binary $^ $@
	tools/padbin 256 $@

gsm.gba: x.bin gsmsongs.gbfs
	tools/catbin $^ $@

clean:
	-rm x.bin
	-rm x.elf
	-rm *.o
	-rm gsmsongs.gbfs
	-rm chr.s

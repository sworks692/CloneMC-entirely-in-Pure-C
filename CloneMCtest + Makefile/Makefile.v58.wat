# CloneMC V58 Open Watcom build file
# Run from this project folder: wmake -f Makefile.v58.wat

CC=wcc386
LD=wlink
CFLAGS=-bt=nt -dWIN32 -d_WINDOWS -i=src -fo=project2finalalpharecreation.obj

all: CloneMC_V58_Terrain_Texture_Default.exe

CloneMC_V58_Terrain_Texture_Default.exe: project2finalalpharecreation.c
	$(CC) $(CFLAGS) project2finalalpharecreation.c
	$(LD) @clonemc_v58_nt_win.lnk

clean:
	del project2finalalpharecreation.obj
	del CloneMC_V58_Terrain_Texture_Default.exe

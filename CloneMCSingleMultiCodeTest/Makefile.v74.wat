# CloneMC V74 Open Watcom build
# Build from the project root with: wmake -f Makefile.v74.wat

EXE = CloneMC_V74_JavaTerrain_Zlib_Performance.exe
SRC = project2finalalpharecreation.c
LNK = clonemc_v74_nt_win.lnk

$(EXE): $(SRC) $(LNK)
	wcl386 -q -bt=nt -l=nt_win -fe=$(EXE) @$(LNK) $(SRC)

clean: .SYMBOLIC
	@if exist $(EXE) del $(EXE)

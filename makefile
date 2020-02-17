all:	clean bin/gba

bin/gba: bin/libBase.so bin/libDecompiler.so bin/libDebugger.so 
	@echo "[GBA] Building GBA Emulator"
	@$(MAKE) -C "GBA"
	cp GBA/bios.gba bin/
	cp GBA/emerald.gba bin/
	cp libs/lib/libSDL2.so bin/
	cp libs/lib/libSDL2_ttf-2.0.so.0 bin/

bin/libBase.so:
	@echo "[GBA] Building libBase.so"
	@$(MAKE) -C "Base"

bin/libDecompiler.so:
	@echo "[GBA] Building libDecompiler.so"
	@$(MAKE) -C "Decompiler"

bin/libDebugger.so:
	@echo "[GBA] Building libDebugger.so"
	@$(MAKE) -C "Debugger"

test:
	@echo "[GBA] Executing......."
	@cd bin && LD_LIBRARY_PATH=. ./gba

clean:
	@rm -rf bin/*
	@$(MAKE) -C "GBA" clean
	@$(MAKE) -C "Debugger" clean
	@$(MAKE) -C "Decompiler" clean
	@$(MAKE) -C "Base" clean

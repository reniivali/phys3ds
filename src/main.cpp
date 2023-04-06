#include <3ds.h>
#include <stdio.h>

#define CLEAR_COLOR 0x1E1E2EFF
#define S_WIDTH 400
#define S_HEIGHT 240

int main(int argc, char **argv) {
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	printf("\x1b[16;20HHello World!");
	printf("\x1b[30;16HPress Start to exit.");

	u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;

	while (aptMainLoop()) {
		hidScanInput();
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		u32 kUp = hidKeysUp();

		if (kDown & KEY_START) break;

		kDownOld = kDown;
		kHeldOld = kHeld;
		kUpOld = kUp;

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}
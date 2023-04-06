#include <3ds.h>
#include <citro2d.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define S_WIDTH 400
#define S_HEIGHT 240

int frame = 0;

struct object {
	float x, y;
	float w, h;
	float vx, vy;
	float r;
	float bounce;
};

bool paused = true;
float gravity = 0.75f;
object box = { 20, 20, 20, 20, 0, 0, 0, 0.9f};

int main(int argc, char **argv) {
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, NULL);

	// Create screens
	C3D_RenderTarget* top  = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	// u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;

	//colors
	u32 clrClear = C2D_Color32(0x1E, 0x1E, 0x2E, 0xFF);

	while (aptMainLoop()) {

		hidScanInput();
		u32 kDown = hidKeysDown();// u32 kHeld = hidKeysHeld(); u32 kUp = hidKeysUp();
		if (kDown & KEY_START) break;
		if (kDown & KEY_A) paused = !paused;
		if (kDown & KEY_X) box.vx += 10;
		if (kDown & KEY_Y) box.vy -= 10;

		printf("\x1b[1;0HFrame: %i", frame);
		printf("\x1b[2;0HCPU: %6.2f%% | GPU: %6.2f%%\x1b[K", C3D_GetProcessingTime()*6.0f, C3D_GetDrawingTime()*6.0f);
		printf("\x1b[3;0HCmdBuf:  %6.2f%%\x1b[K", C3D_GetCmdBufUsage()*100.0f);

		printf("\x1b[5;0HBoxVel x/y: %f/%f", box.vx, box.vy);
		printf("\x1b[6;0HBoxPos x/y: %f/%f", box.x, box.y);
		if (paused) printf("\x1b[7;0HPHYSICS PAUSED"); else printf("\x1b[7;0H              ");

		// calculate physics
		if (!paused) {
			box.vy += gravity;

			box.x += box.vx;
			box.y += box.vy;

			if (box.y + box.h > S_HEIGHT) { box.y = S_HEIGHT - box.h; box.vy = -box.vy * box.bounce; }
			if (box.y < 0) { box.y = 0; box.vy = -box.vy * box.bounce; }

			if (box.x + box.w > S_WIDTH) { box.x = S_WIDTH - box.w; box.vx = -box.vx * box.bounce; }
			if (box.x < 0) { box.x = 0; box.vx = -box.vx * box.bounce; }
		}
		// Render scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, clrClear);
		C2D_SceneBegin(top);
		C2D_DrawRectSolid(box.x, box.y, 0, box.w, box.h, C2D_Color32(0x30, 0x30, 0x30, 0xFF));

		C3D_FrameEnd(0);

		/*gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();*/

		// kDownOld = kDown; kHeldOld = kHeld; kUpOld = kUp;
		frame++;
	}

	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
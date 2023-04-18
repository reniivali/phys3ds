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
	float mass;
};

struct predict {
	float x, y;
	float vx, vy;
};

struct predictions {
	predict pr[10];
};

struct crosshair {
	float x, y;
};

struct coord {
	float x, y;
};

crosshair cross = { S_WIDTH / 2, S_HEIGHT / 2 };

void renderCrosshair() {
	C2D_DrawRectSolid(cross.x - 10, cross.y - 2, 0, 20, 4, C2D_Color32(0x50, 0x50, 0x50, 0xFF));
	C2D_DrawRectSolid(cross.x - 2, cross.y - 10, 0, 4, 20, C2D_Color32(0x50, 0x50, 0x50, 0xFF));
}

bool paused = true;
bool showPaths = false;
float gravity = 0.75f;
float crossSens = 3.0f;
float expForce = 20.0f;
float friction = 0.95f;

int boxes = 9;
object box[9] = {
	{20, 20, 20, 20, 0, 0, 0, 0.75f, 10},
	{50, 20, 30, 30, 0, 0, 0, 0.75f, 45},
	{90, 20, 20, 20, 0, 0, 0, 0.75f, 10},
	{120, 20, 30, 30, 0, 0, 0, 0.75f, 45},
	{160, 20, 20, 20, 0, 0, 0, 0.75f, 10},
	{190, 20, 30, 30, 0, 0, 0, 0.75f, 45},
	{230, 20, 20, 20, 0, 0, 0, 0.75f, 10},
	{260, 20, 30, 30, 0, 0, 0, 0.75f, 45},
	{300, 20, 20, 20, 0, 0, 0, 0.75f, 10}//,
	/*{20, 100, 30, 30, 0, 0, 0, 0.9f, 45, 0.95f},
	{50, 20, 20, 20, 0, 0, 0, 0.9f, 10, 0.95f},
	{90, 20, 30, 30, 0, 0, 0, 0.9f, 45, 0.95f},
	{120, 20, 20, 20, 0, 0, 0, 0.9f, 10, 0.95f},
	{160, 20, 30, 30, 0, 0, 0, 0.9f, 45, 0.95f},
	{190, 20, 20, 20, 0, 0, 0, 0.9f, 10, 0.95f},
	{230, 20, 30, 30, 0, 0, 0, 0.9f, 45, 0.95f},
	{260, 20, 20, 20, 0, 0, 0, 0.9f, 10, 0.95f},
	{300, 20, 30, 30, 0, 0, 0, 0.9f, 45, 0.95f}*/
};

predictions pred[9];



static void drawGradientRect(float x, float y, float w, float h, float p, u32 color, int r1, int g1, int b1, int r2, int g2, int b2, int opacity) {
	C2D_DrawRectangle(x, y, 0, w, h, C2D_Color32(r1, g1, b1, opacity), C2D_Color32((r1*w/(w+h) + r2*h/(w+h)), (g1*w/(w+h) + g2*h/(w+h)), (b1*w/(w+h) + b2*h/(w+h)), opacity), C2D_Color32((r1*h/(w+h) + r2*w/(w+h)), (g1*h/(w+h) + g2*w/(w+h)), (b1*h/(w+h) + b2*w/(w+h)), opacity), C2D_Color32(r2, g2, b2, opacity));
	C2D_DrawRectSolid(x + p, y + p, 0, w - p * 2, h - p * 2, color);
}

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

		//fill predictions array with positions of boxes over next 10 frames
		for (int i = 0; i < boxes; i++) {
			for (int j = 0; j < 10; j++) {
				if (j == 0) {
					pred[i].pr[0].vy = box[i].vy;
					pred[i].pr[0].vx = box[i].vx;
					pred[i].pr[0].x = box[i].x + box[i].vx;
					pred[i].pr[0].y = box[i].y + pred[i].pr[0].vy;
					pred[i].pr[j].vy += gravity;
					if (pred[i].pr[0].vx != 0) pred[i].pr[0].vx *= friction;
					if (pred[i].pr[0].y < 0) {
						pred[i].pr[0].vy = -pred[i].pr[0].vy * box[i].bounce;
						pred[i].pr[0].y = 0;
					} else if (pred[i].pr[0].y > S_HEIGHT - box[i].h) {
						pred[i].pr[0].vy = -pred[i].pr[0].vy * box[i].bounce;
						pred[i].pr[0].y = S_HEIGHT - box[i].h;
					}
					if (pred[i].pr[0].x < 0) {
						pred[i].pr[0].vx = -pred[i].pr[0].vx * box[i].bounce;
						pred[i].pr[0].x = 0;
					} else if (pred[i].pr[0].x > S_WIDTH - box[i].w) {
						pred[i].pr[0].vx = -pred[i].pr[0].vx * box[i].bounce;
						pred[i].pr[0].x = S_WIDTH - box[i].w;
					}
				} else {
					pred[i].pr[j].vy = pred[i].pr[j-1].vy;
					pred[i].pr[j].vx = pred[i].pr[j-1].vx;
					pred[i].pr[j].x = pred[i].pr[j-1].x + pred[i].pr[j].vx;
					pred[i].pr[j].y = pred[i].pr[j-1].y + pred[i].pr[j].vy;
					pred[i].pr[j].vy += gravity;
					if (pred[i].pr[j].vx != 0) pred[i].pr[j].vx *= friction;
					if (pred[i].pr[j].y < 0) {
						pred[i].pr[j].vy = -pred[i].pr[j].vy * box[i].bounce;
						pred[i].pr[j].y = 0;
					} else if (pred[i].pr[j].y > S_HEIGHT - box[i].h) {
						pred[i].pr[j].vy = -pred[i].pr[j].vy * box[i].bounce;
						pred[i].pr[j].y = S_HEIGHT - box[i].h;
					}
					if (pred[i].pr[j].x < 0) {
						pred[i].pr[j].vx = -pred[i].pr[j].vx * box[i].bounce;
						pred[i].pr[j].x = 0;
					} else if (pred[i].pr[j].x > S_WIDTH - box[i].w) {
						pred[i].pr[j].vx = -pred[i].pr[j].vx * box[i].bounce;
						pred[i].pr[j].x = S_WIDTH - box[i].w;
					}
				}
			}
		}

		hidScanInput();
		u32 kDown = hidKeysDown(); u32 kHeld = hidKeysHeld();// u32 kUp = hidKeysUp();
		if (kDown & KEY_START) paused = !paused;
		if (kDown & KEY_START && kHeld & KEY_R) break;

		printf("\x1b[1;0HFrame: %i", frame);
		printf("\x1b[2;0HCPU: %6.2f%% | GPU: %6.2f%%\x1b[K", C3D_GetProcessingTime()*6.0f, C3D_GetDrawingTime()*6.0f);
		printf("\x1b[3;0HCmdBuf:  %6.2f%%\x1b[K", C3D_GetCmdBufUsage()*100.0f);
		printf("\x1b[4;0HGravity: %f", gravity);
		printf("\x1b[5;0HFriction: %f", friction);
		printf("\x1b[6;0HexpForce: %f", expForce);
		if (paused) printf("\x1b[7;0HPHYSICS PAUSED"); else printf("\x1b[7;0H              ");

		// move crosshair
		circlePosition pos;
		hidCircleRead(&pos);
		if (pos.dx > 40 || pos.dx < -40) cross.x += (pos.dx / 154) * crossSens;
		if (pos.dy > 40 || pos.dy < -40) cross.y -= (pos.dy / 154) * crossSens;
		if (cross.x > S_WIDTH) cross.x = S_WIDTH;
		if (cross.x < 0) cross.x = 0;
		if (cross.y > S_HEIGHT) cross.y = S_HEIGHT;
		if (cross.y < 0) cross.y = 0;

		if (kHeld & KEY_DUP) cross.y -= 1;
		if (kHeld & KEY_DDOWN) cross.y += 1;
		if (kHeld & KEY_DLEFT) cross.x -= 1;
		if (kHeld & KEY_DRIGHT) cross.x += 1;

		if (kDown & KEY_L) showPaths = !showPaths;

		if (kHeld & KEY_ZL) friction -= 0.01f;
		if (kHeld & KEY_ZR) friction += 0.01f;

		if (kHeld & KEY_A) gravity += 0.01f;
		if (kHeld & KEY_B) gravity -= 0.01f;

		if (kHeld & KEY_X) expForce += 0.01f;
		if (kHeld & KEY_Y) expForce -= 0.01f;

		if (kDown & KEY_SELECT) {
			for (int i = 0; i < boxes; i++) {
				box[i].vx = 0;
				box[i].vy = 0;
			}
		}

		// calculate physics
		if (!paused) {
			for (int i = 0; i < boxes; i++) {
				box[i].x += box[i].vx;
				box[i].y += box[i].vy;

				box[i].vy += gravity;
				if (box[i].vx != 0) box[i].vx *= friction;

				if (kDown & KEY_R) {
					float difx = box[i].x - cross.x;
					float dify = box[i].y - cross.y;

					float dist = sqrt(difx * difx + dify * dify);
					box[i].vx += ((difx / dist) * expForce) / dist * dist;
					box[i].vy += ((dify / dist) * expForce) / dist * dist;
				}
				for (int j = 0; j < 9; j++) {
					pred[i].pr[j] = pred[i].pr[j + 1];
				}
				// add new prediction
				pred[i].pr[9].vy = pred[i].pr[8].vy + gravity;
				pred[i].pr[9].vx = pred[i].pr[8].vx;
				pred[i].pr[9].x = pred[i].pr[8].x + pred[i].pr[9].vx;
				pred[i].pr[9].y = pred[i].pr[8].y + pred[i].pr[9].vy;
				if (pred[i].pr[9].vx != 0) pred[i].pr[9].vx *= friction;
				if (pred[i].pr[9].y + box[i].h > S_HEIGHT) {
					pred[i].pr[9].y = S_HEIGHT - box[i].h;
					pred[i].pr[9].vy = -pred[i].pr[9].vy * box[i].bounce;
				} else if (pred[i].pr[9].y < 0) {
					pred[i].pr[9].y = 0;
					pred[i].pr[9].vy = -pred[i].pr[9].vy * box[i].bounce;
				}
				if (pred[i].pr[9].x + box[i].w > S_WIDTH) {
					pred[i].pr[9].x = S_WIDTH - box[i].w;
					pred[i].pr[9].vx = -pred[i].pr[9].vx * box[i].bounce;
				} else if (pred[i].pr[9].x < 0) {
					pred[i].pr[9].x = 0;
					pred[i].pr[9].vx = -pred[i].pr[9].vx * box[i].bounce;
				}
			}
		}

		for (int i = 0; i < boxes; i++) {
			// OFFSCREEN COLLISION - KEEP AT BOTTOM
			if (box[i].y + box[i].h > S_HEIGHT) {
				box[i].y = S_HEIGHT - box[i].h;
				if (box[i].vy > 5)
					box[i].vy = -box[i].vy * box[i].bounce;
				else
					box[i].vy = 0;
			}
			if (box[i].y < 0) {
				box[i].y = 0;
				box[i].vy = -box[i].vy * box[i].bounce;
			}
			if (box[i].x + box[i].w > S_WIDTH) {
				box[i].x = S_WIDTH - box[i].w;
				box[i].vx = -box[i].vx * box[i].bounce;
			}
			if (box[i].x < 0) {
				box[i].x = 0;
				box[i].vx = -box[i].vx * box[i].bounce;
			}
			// detect collision with other boxes
			for (int j = 0; j < boxes; j++) {
				if (i == j) continue;
				if (box[i].x + box[i].w > box[j].x && box[i].x < box[j].x + box[j].w && box[i].y + box[i].h > box[j].y && box[i].y < box[j].y + box[j].h) {
					// collision detected
					// calculate collision vector
					float difx = abs(box[i].x - box[j].x);
					float dify = abs(box[i].y - box[j].y);

					coord center1 = { box[i].x + box[i].w / 2, box[i].y + box[i].h / 2 };
					coord center2 = { box[j].x + box[j].w / 2, box[j].y + box[j].h / 2 };

					float cDiffX = abs(center1.x - center2.x);
					float cDiffY = abs(center1.y - center2.y);

					float dist = sqrt(difx * difx + dify * dify);
					float nx = difx / dist;
					float ny = dify / dist;
					//move boxes away from each other
					float pen = (box[i].w / 2 + box[j].w / 2) - dist;
					box[i].x += nx * pen;
					box[i].y += ny * pen;
					box[j].x -= nx * pen;
					box[j].y -= ny * pen;

					if (cDiffX > cDiffY) {
						// collision on x axis
						if (box[i].x < box[j].x) {
							// box i is left of box j
							box[i].x = box[j].x - box[i].w;
						} else {
							// box i is right of box j
							box[i].x = box[j].x + box[j].w;
						}
						box[i].vx = -box[i].vx * box[i].bounce;
						box[j].vx = -box[j].vx * box[j].bounce;
					} else {
						// collision on y axis
						if (box[i].y < box[j].y) {
							// box i is above box j
							box[i].y = box[j].y - box[i].h;
						} else {
							// box i is below box j
							box[i].y = box[j].y + box[j].h;
						}
						box[i].vy = -box[i].vy * box[i].bounce;
						box[j].vy = -box[j].vy * box[j].bounce;
					}
				}
			}
		}

		// Render scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, clrClear);
		C2D_SceneBegin(top);

		for (int i = 0; i < boxes; i++) {
			drawGradientRect(box[i].x, box[i].y, box[i].w, box[i].h, 3, C2D_Color32(0x18, 0x18, 0x28, 0xDD), 0xFA, 0xB3, 0x87, 0xF5, 0xC2, 0xE7, 0xFF);
			if (showPaths) {
				for (int j = 0; j < 10; j++) {
					C2D_DrawLine(
						pred[i].pr[j].x + box[i].w / 2,
						pred[i].pr[j].y + box[i].h / 2,
						C2D_Color32(0x00, 0xFF, 0x00, 0xFF),
						pred[i].pr[j].x + box[i].w / 2 + pred[i].pr[j].vx,
						pred[i].pr[j].y + box[i].h / 2 + pred[i].pr[j].vy,
						C2D_Color32(0x00, 0xFF, 0x00, 0xFF),
						2.0f,
						0
					);
				}
			}
		}

		renderCrosshair();

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
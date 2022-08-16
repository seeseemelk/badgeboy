#ifndef _PPU

#define _PPU


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Gameboy Emulator: Pixel Processing Unit
 *
 * Resources:
 *
 * > General
 * https://www.youtube.com/watch?v=HyzD8pNlpwI
 *
 * > LCD
 * http://www.codeslinger.co.uk/pages/projects/gameboy/lcd.html
 *
 * > OpenGL Textures
 * https://learnopengl.com/Getting-started/Textures
 */


#define SCREEN_MULTIPLIER 2

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144


extern unsigned char joypad_state;

void init_gui();
void render_frame();
void screen_cyan();
void screen_yellow();

void ppu(int cycles);


#ifdef __cplusplus
}
#endif

#endif

#include <Arduino.h>
//#include <SPI.h>
#include <FS.h>
//#include <SD_MMC.h>
#include <SD.h>

#include "emulator.h"
#include "ppu.h"
#include "cpu.h"
#include "keys.h"
#include "timer.h"
#include "memory.h"
#include "cartridge.h"

#define CARTRIDGE_SIZE 0x200000

//spi = SPI(2, baudrate=40000000, polarity=1)
//gc.collect()  # Precaution before instantiating framebuf
//pcs = Pin(5, Pin.OUT)
//pdc = Pin(33, Pin.OUT)
//prst = Pin(32, Pin.OUT)
//ssd = SSD(spi, height=240, width=240, cs=pcs, dc=pdc, rst=prst)


const int FRAME_MAX_CYCLES = 69905;
//const int FRAME_MAX_CYCLES = 1;
unsigned long emulation_time = 0;

void setup() {
  Serial.begin(115200);

  // Initialise keys
  pinMode(KEY_A, INPUT_PULLUP);
  pinMode(KEY_B, INPUT_PULLUP);
  pinMode(KEY_UP, INPUT_PULLUP);
  pinMode(KEY_LEFT, INPUT_PULLUP);
  pinMode(KEY_DOWN, INPUT_PULLUP);
  pinMode(KEY_RIGHT, INPUT_PULLUP);
  pinMode(KEY_START, INPUT_PULLUP);
  pinMode(KEY_SELECT, INPUT_PULLUP);

  // Initialise chip select pins
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(5, HIGH);

  //pinMode(SS, OUTPUT);
  //digitalWrite(4, HIGH);

  Serial.println("Init SD");
  if (!SD.begin(4)) {
    Serial.println("Failed to init SD");
  } else {
    Serial.println("SD initialised");
  }

  if (SD.cardType() == CARD_NONE) {
    Serial.println("No SD card detected");
  } else {
    Serial.println("SD card was detected");
  }

  //digitalWrite(4, LOW);

  puts("Initialising memory");
  init_memory();
  puts("Loading cartridge");

  File file = SD.open("/tetris.gb", "rb");
  if (!file) {
    puts("Could not find file");
  } else {
    puts("Reading cartridge");
    size_t size = 0;
    size_t read;
    do {
      read = file.read(rom + size, CARTRIDGE_SIZE - size);
      size += read;
    } while (read != 0);
    printf("Read a cartridge of %d bytes\n", size);
    file.close();
    insert_cartridge_now();
  }

  digitalWrite(5, LOW);
  puts("Initialising display");
  init_gui();

  puts("Loading roms");
  load_roms();
  *joyp |= 0xF;
  *tdiv = 0;
  puts("Ready");

  while (1)
  {
    if (digitalRead(KEY_UP) == 0) {
      screen_cyan();
    } else {
      screen_yellow();
    }
  }
}

static void process_input() {
  //*joyp &= 0xF0;
  *joyp |= 0x0F;
  if ((*joyp & 0x10) != 0) {
    if (digitalRead(KEY_RIGHT) == 0) {
      *joyp &= ~0x01;
    }
    if (digitalRead(KEY_LEFT) == 0) {
      *joyp &= ~0x02;
    }
    if (digitalRead(KEY_UP) == 0) {
      *joyp &= ~0x04;
    }
    if (digitalRead(KEY_DOWN) == 0) {
      *joyp &= ~0x08;
    }
  }
  if ((*joyp & 0x20) != 0) {
    if (digitalRead(KEY_A) == 0) {
      *joyp &= ~0x01;
    }
    if (digitalRead(KEY_B) == 0) {
      *joyp &= ~0x02;
    }
    if (digitalRead(KEY_SELECT) == 0) {
      *joyp &= ~0x04;
    }
    if (digitalRead(KEY_START) == 0) {
      *joyp &= ~0x08;
    }
  }
}

void loop() {
  puts("In loop");
  unsigned int cycles_this_frame = 0;

  while (cycles_this_frame < FRAME_MAX_CYCLES) {
      //puts("Executing cycle");
      int cycles = cpu();


      ppu(cycles); /* The Pixel Processing Unit receives the
      //              * the amount of cycles run by the processor
      //              * in order to keep it in sync with the processor.
      //              */

      timer(cycles); /* Same thing as the PPU goes for timers */

      process_input();

      cycles_this_frame += cycles;
  }

  render_frame();

  emulation_time += cycles_this_frame;
}
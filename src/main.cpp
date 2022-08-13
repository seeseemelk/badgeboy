#include <Arduino.h>
//#include <SPI.h>
#include <FS.h>
//#include <SD_MMC.h>
#include <SD.h>

#include "emulator.h"
#include "ppu.h"
#include "cpu.h"
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
  Serial.begin(9600);

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

  File file = SD.open("/cartridge.gb", "rb");
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

      //process_input();

      cycles_this_frame += cycles;
  }

  render_frame();

  emulation_time += cycles_this_frame;
}
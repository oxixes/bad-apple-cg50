#include <fxcg/display.h>
#include <fxcg/keyboard.h>
#include <fxcg/rtc.h>
#include <fxcg/system.h>
#include <fxcg/misc.h>
#include <fxcg/file.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fastlz.h"

#define CREATEMODE_FILE 1
#define CREATEMODE_FOLDER 5
#define READ 0
#define READ_SHARE 1
#define WRITE 2
#define READWRITE 3
#define READWRITE_SHARE 4

#define FILE_PATH "\\\\fls0\\video.bin"

unsigned short pFile[sizeof(FILE_PATH)*2];
int hFile;

void clearDisplay() {
  unsigned short *p = GetVRAMAddress();
  for (int i = 0; i < 216 * 384; i++) {
    *p++ = 65535;
  }
  Bdisp_PutDisp_DD();
}

bool wait(int ticks, int ms) {
  int keyboard_row;
  int keyboard_column;
  bool shouldExit = false;
  do {
    GetKeyWait_OS(&keyboard_column, &keyboard_row, KEYWAIT_HALTOFF_TIMEROFF, 0, 0, NULL);

    if (keyboard_column == 4 && keyboard_row == 9) { // MENU Key
      shouldExit = true;
      break;
    }
  } while (RTC_Elapsed_ms(ticks, ms) == 0);
  if (shouldExit) return true;
  return false;
}

int whichElementAreWeIn(unsigned short* reps, int val) {
  int previousValue = 0;
  int i = 0;
  while (1) {
    if (val >= (reps[i] + previousValue)) {
      previousValue = reps[i] + previousValue;
      i++;
      continue;
    }

    return i;
  }
}

void drawMainMenu(unsigned short frameCount, unsigned short width, unsigned short height, unsigned short fps, unsigned char scale, char realTime, char frameCounter) {
  unsigned char frameCountStr[5];
  itoa(frameCount, frameCountStr);
  locate_OS(1,1);
  Print_OS((char*) frameCountStr, 0, 0);
  locate_OS(7, 1);
  Print_OS("frames.",0,0);

  unsigned char wStr[3];
  unsigned char hStr[3];
  itoa(width, wStr);
  itoa(height, hStr);

  locate_OS(1, 2);
  Print_OS((char*) wStr, 0, 0);
  locate_OS(4, 2);
  Print_OS("x", 0, 0);
  locate_OS(6, 2);
  Print_OS((char*) hStr, 0, 0);

  unsigned char scaleStr[2];
  unsigned char fpsStr[2];
  itoa(scale, scaleStr);
  itoa(fps, fpsStr);

  locate_OS(1, 3);
  Print_OS("Scale:",0,0);
  locate_OS(8, 3);
  Print_OS((char*) scaleStr,0,0);
  locate_OS(10, 3);
  Print_OS(", FPS:",0,0);
  locate_OS(17, 3);
  Print_OS((char*) fpsStr,0,0);

  locate_OS(1, 5);
  Print_OS("F1: Real-Time",0,0);

  if (realTime) {
    locate_OS(18, 5);
    Print_OS("Yes",0,0);
  } else {
    locate_OS(18, 5);
    Print_OS("No ",0,0);
  }

  locate_OS(1, 6);
  Print_OS("F2: Frame count",0,0);

  if (frameCounter) {
    locate_OS(18, 6);
    Print_OS("Yes",0,0);
  } else {
    locate_OS(18, 6);
    Print_OS("No ",0,0);
  }

  locate_OS(1, 8);
  Print_OS("EXE: Play",0,0);

  Bdisp_PutDisp_DD();
}

int main(void) {
    clearDisplay();

    Bfile_StrToName_ncpy(pFile, (char*) FILE_PATH, sizeof(FILE_PATH));
    hFile = Bfile_OpenFile_OS(pFile, READ, 0);

    if (hFile < 0) {
      locate_OS(1, 1);
      Print_OS("Error reading video.",0,0);
      while (1) GetKey(NULL);
    }

    unsigned short frameCount = 0;
    Bfile_ReadFile_OS(hFile, &frameCount, 2, 0);
    unsigned short width = 0;
    Bfile_ReadFile_OS(hFile, &width, 2, -1);
    unsigned short height = 0;
    Bfile_ReadFile_OS(hFile, &height, 2, -1);
    unsigned short fps = 0;
    Bfile_ReadFile_OS(hFile, &fps, 2, -1);
    unsigned char scale = 0;
    Bfile_ReadFile_OS(hFile, &scale, 1, -1);

    if (width * scale > 384 || height * scale > 216 || fps > 60 || scale > 99) {
      locate_OS(1, 1);
      Print_OS("Invalid video.",0,0);
      Bfile_FindClose(hFile);
      while (1) GetKey(NULL);
    }

    char realTime = 1;
    char frameCounter = 1;

    while (1) {
        int key = 0;
        drawMainMenu(frameCount, width, height, fps, scale, realTime, frameCounter);
        GetKey(&key);

        if (key == 0x7539) {
          realTime = !realTime;
        }

        if (key == 0x753A) {
          frameCounter = !frameCounter;
        }

        if (key == 0x7534 || key == 0x000D) {
          break;
        }
    }

    unsigned int currentByte = 9;

    int startTicks = RTC_GetTicks();
    for (int f = 0; f < frameCount; f++) {
      if (realTime) {
        unsigned int frameWeShouldBeIn = (RTC_GetTicks() - startTicks) * ((double)1/128) * fps;
        int framesToSkip = frameWeShouldBeIn - f;
        if (framesToSkip > 0) {
          if (f + framesToSkip > frameCount) {
            break;
          }

          for (int i = 0; i < framesToSkip; i++) {
            currentByte += 2;
            unsigned short frameRepLength;
            Bfile_ReadFile_OS(hFile, &frameRepLength, 2, currentByte);
            currentByte += 3 + frameRepLength;
          }
          f += framesToSkip;
        }
      }

      int ticks = RTC_GetTicks();

      unsigned short originalFrameRepLength;
      Bfile_ReadFile_OS(hFile, &originalFrameRepLength, 2, currentByte);
      currentByte += 2;

      unsigned short compressedFrameRepLength;
      Bfile_ReadFile_OS(hFile, &compressedFrameRepLength, 2, -1);
      currentByte += 2;

      unsigned short frameReps[originalFrameRepLength / 2];
      unsigned char compressedFrameReps[compressedFrameRepLength];
      Bfile_ReadFile_OS(hFile, compressedFrameReps, compressedFrameRepLength, -1);
      currentByte += compressedFrameRepLength;

      if (originalFrameRepLength <= 16) {
        for (int i = 0; i < originalFrameRepLength / 2; i++) {
          frameReps[i] = 0;
          frameReps[i] |= (compressedFrameReps[i * 2] << 8);
          frameReps[i] |= (compressedFrameReps[(i * 2) + 1]);
        }
      } else {
        fastlz_decompress(compressedFrameReps, compressedFrameRepLength, frameReps, originalFrameRepLength);
      }

      unsigned char actualColor;
      Bfile_ReadFile_OS(hFile, &actualColor, 1, -1);
      currentByte += 1;

      unsigned char frameBoolValues[originalFrameRepLength / 2];
      for (int i = 0; i < originalFrameRepLength / 2; i++) {
        frameBoolValues[i] = actualColor;
        if (actualColor) {
          actualColor = 0;
        } else {
          actualColor = 1;
        }
      }

      unsigned short *p = GetVRAMAddress();
      int pixCounter = 0;

      for (int i = 0; i < 216 * 384; i++) {
        int x = i % 384;
        int y = i / 384;

        if (x >= width * scale || y >= height * scale) {
          *p++ = 65535;
          continue;
        }

        if (y % scale != 0 && x == 0) {
          pixCounter -= width;
        }

        char color = frameBoolValues[whichElementAreWeIn(frameReps, pixCounter)];
        for (int j = 0; j < scale; j++) {
          if (color) {
            *p++ = 65535;
          } else {
            *p++ = 0;
          }
        }
        i += scale - 1;

        pixCounter++;
      }

      if (frameCounter) {
        unsigned char frame[5];
        itoa(f, frame);

        int x = 320;
        int y = 170;
        PrintMini(&x, &y, (char*)frame, 0, 0xFFFFFFFF, 0, 0, COLOR_BLACK, COLOR_WHITE, 1, 0);
      }

      Bdisp_PutDisp_DD();

      if (wait(ticks, 1000 / fps)) break;
    }

    Bfile_FindClose(hFile);

    clearDisplay();
    locate_OS(1, 4);
    Print_OS("Go to another app and",0,0); //21
    locate_OS(8, 5);
    Print_OS("reenter",0,0);

    while(1) {
      GetKey(NULL);
    }

    return 0;
}

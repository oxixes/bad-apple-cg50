#include "fastlz.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <climits>

// https://stackoverflow.com/a/4956493
template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Video data not provided.";
    return 1;
  }

  std::ifstream dataFile(argv[1], std::ios::out | std::ios::binary);
  if (!dataFile) {
    std::cout << "Error reading video data.";
    return 1;
  }

  struct stat dataStat;
  stat(argv[1], &dataStat);

  int num = 1;
  char switchEndianness = 0;
  if (*(char *)&num == 1) {
    switchEndianness = 1;
  }

  unsigned char* outputBuffer = new unsigned char[dataStat.st_size * 2];

  unsigned char* buffer = new unsigned char[dataStat.st_size];
  dataFile.read((char*) buffer, dataStat.st_size);

  memcpy(outputBuffer, buffer, 9);

  unsigned short frameCount = 0;
  memcpy(&frameCount, buffer, 2);
  if (switchEndianness) frameCount = swap_endian<unsigned short>(frameCount);

  std::cout << "Original size: " << dataStat.st_size << " bytes\n";
  std::cout << "Frame count: " << frameCount << "\n";

  int currentByte = 9;
  int currentOutputByte = 9;

  for (int f = 0; f < frameCount; f++) {
    unsigned short repLen = 0;
    memcpy(&repLen, buffer + currentByte, 2);
    if (switchEndianness) repLen = swap_endian<unsigned short>(repLen);
    currentByte += 2;

    unsigned char* repBuffer = new unsigned char[repLen * 2];
    memcpy(repBuffer, buffer + currentByte, repLen * 2);
    currentByte += repLen * 2;

    unsigned char* outputRepBuffer = new unsigned char[repLen * 8]; // 4 times bigger to make sure it's higher than 66 bytes if it's compressed.
    unsigned short outputRepBufferLength = 0;
    if (repLen * 2 <= 16) {
      outputRepBufferLength = repLen * 2;
      memcpy(outputRepBuffer, repBuffer, repLen * 2);
    } else {
      outputRepBufferLength = fastlz_compress_level(2, repBuffer, repLen * 2, outputRepBuffer);
    }

    unsigned short originalRepBufferLength = repLen * 2;
    if (switchEndianness) outputRepBufferLength = swap_endian<unsigned short>(outputRepBufferLength);
    if (switchEndianness) originalRepBufferLength = swap_endian<unsigned short>(originalRepBufferLength);

    memcpy(outputBuffer + currentOutputByte, &originalRepBufferLength, 2);
    currentOutputByte += 2;
    memcpy(outputBuffer + currentOutputByte, &outputRepBufferLength, 2);
    currentOutputByte += 2;

    if (switchEndianness) outputRepBufferLength = swap_endian<unsigned short>(outputRepBufferLength);
    memcpy(outputBuffer + currentOutputByte, outputRepBuffer, outputRepBufferLength);
    currentOutputByte += outputRepBufferLength;

    memcpy(outputBuffer + currentOutputByte, buffer + currentByte, 1);
    currentByte += 1;
    currentOutputByte += 1;

    delete[] outputRepBuffer;
    delete[] repBuffer;
  }

  std::ofstream outputFile("data.c.bin", std::ios::out | std::ios::binary);
  outputFile.write((char*) outputBuffer, currentOutputByte);

  outputFile.close();
  dataFile.close();

  delete[] buffer;
  delete[] outputBuffer;
  dataFile.close();

  return 0;
}

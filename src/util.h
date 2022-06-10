#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
using namespace std;

void step(const uint8_t motorPins[4], uint8_t step, boolean halfStep);
String toString(int number);

boolean deleteFile(int id);
int getFirstDispoId();
int getLastId();
String file_md5(File &f);
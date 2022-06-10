#pragma once
#include <Arduino.h>

class MusicEvent; // Forward declaration

void synchronizedBufferPush(MusicEvent* event);
MusicEvent* synchronizedBufferPop();
bool synchronizedBufferIsEmpty();
int synchronizedBufferSize();
void synchronizedBufferReset();
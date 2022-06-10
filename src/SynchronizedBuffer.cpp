#include <Arduino.h>
#include <SynchronizedBuffer.h>
#include <MusicTrack.h>

const uint8_t bufferSize = 30;
MusicEvent* musicBuffer = (MusicEvent *) malloc(sizeof(MusicEvent) * bufferSize);

volatile uint8_t head = 0;
volatile uint8_t tail = 0;

void synchronizedBufferPush(MusicEvent* value)
{
    memcpy(&musicBuffer[head], value, sizeof(MusicEvent));
    head = (head + 1) % bufferSize;
}

MusicEvent *synchronizedBufferPop()
{
    if (head == tail)
    {
        return NULL;
    }
    MusicEvent *value = &musicBuffer[tail];
    tail = (tail + 1) % bufferSize;
    return value;
}

boolean synchronizedBufferIsEmpty()
{
    return head == tail;
}

int synchronizedBufferSize()
{
    return (head - tail + bufferSize) % bufferSize;
}

void synchronizedBufferReset()
{
    head = 0;
    tail = 0;
}
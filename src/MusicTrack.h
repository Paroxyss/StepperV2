#pragma once
#include <Arduino.h>
#include <config.h>
#include <MusicMotor.h>
#include <SPIFFS.h>
#include <InternalEvent.h>
#include <SynchronizedBuffer.h>

class MusicEvent
{
public:
    byte state;
    byte note;
    byte channel;
    int delta;
    uint8_t motorId;
    MusicEvent(byte state, byte note, byte channel, int delta);
    void setMotorId(uint8_t motorId);
};

class MusicTrack
{
public:
    File file;
    byte tempo;
    char name[16];
    int timeDivision;
    bool playing;
    uint32_t idPlaying;
    bool opened = false;
    double beatToUsFactor;
    unsigned long nextEventDelta = 0;

    int transpose = 0;

    QueueHandle_t *queueIn;
    QueueHandle_t *queueOut;

    InternalEvent *lastEvent;

    uint8_t motorRotation = 0;

    MusicTrack();
    MusicTrack(QueueHandle_t *queueIn, QueueHandle_t *queueOut);
    
    void request(uint32_t id);
    MusicEvent getNextEvent();
    void handleQueue();
    void tick(unsigned long delta);
    void setPlaying(bool playing);
    uint8_t getFreeMotor(byte note, byte channel);
    uint8_t getMotor(byte note, byte channel);
    int getTrueNote(int note);
};
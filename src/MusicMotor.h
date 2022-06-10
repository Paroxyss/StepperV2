#pragma once

class MusicMotor {
public:
    uint8_t actualStep = 0;
    int actualChannel = -1;
    int actualNote = -1;
    int actualDelay = 0;
    unsigned long noteSomme = 0;
    unsigned long noteTotalTime = 0;

    boolean published = true;

    const uint8_t *pins;
    byte id;
    MusicMotor(){};
    MusicMotor(const uint8_t pinsIn[], const uint8_t motorId);
    void doStep();
    void setNote(int note, int channel);
    void tick(unsigned long delta);
    void reset();
};
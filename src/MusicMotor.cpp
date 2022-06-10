#include <Arduino.h>
#include <config.h>
#include <util.h>
#include <MusicMotor.h>
#include <SynchronizedBuffer.h>
#include <MusicTrack.h>

// This class describes a stepper motor that can play a note by it's midi frequency.

MusicMotor::MusicMotor(const uint8_t *pinsIn, uint8_t motorId)
{
	id = motorId;

	pins = pinsIn;
	actualNote = -1;
	for (int i = 0; i < 4; i++)
	{
		pinMode(pins[i], OUTPUT);
	}
}
void MusicMotor::doStep()
{
	step(pins, actualStep, true);
	actualStep = (actualStep + 1) % 8;
}
void MusicMotor::setNote(int note, int channel)
{
	published = false;
	actualNote = note;
	actualChannel = channel;
	actualDelay = delay_notes[note];
	noteSomme = 0;
	noteTotalTime = 0;
}
void MusicMotor::tick(unsigned long delta)
{
	if (actualNote == -1)
		return;
	noteSomme += delta;
	noteTotalTime += delta;
	// if not published and notetotaltime > 2000, publish
	if (!published && noteTotalTime > 2000)
	{
		published = true;
		MusicEvent event = MusicEvent(1, actualNote, actualChannel, 0);
		event.setMotorId(id);
		synchronizedBufferPush(&event);
	}
	if (noteSomme >= actualDelay)
	{
		noteSomme -= actualDelay;
		doStep();
	}
}
void MusicMotor::reset()
{
	actualStep = 0;
	actualNote = -1;
	actualChannel = -1;
	actualDelay = 0;
	noteSomme = 0;
	noteTotalTime = 0;

	MusicEvent event = MusicEvent(0, 0, 0, 0);
	event.setMotorId(id);
	synchronizedBufferPush(&event);
}

#pragma once
#include <Arduino.h>
#include <MusicMotor.h>

#define MAX_EVENTS_IN_QUEUE 10
#define NUMBER_OF_NOTE_TO_CALC 128

#define MOTOR_PUBLISHER_PORT 82

#define MOTOR_NUMBER 4
#define MOTOR_ENABLE_PIN 25
#define MAX_NOTE_LENGTH_IN_MS 20000 // 20 seconds

#define ACCELERATION_DELAY_MAX 1136    // Délai (en µs) en dessous du quel le moteur fera une accélération, plus grand => accélération pour des notes plus graves, 1136 est le délai pour un La 440Hz
#define ACCELERATION_FACTOR 1.25       // Facteur d'accélération, plus grand => accélération plus forte
#define ACCELERATION_MAX_ITERATIONS 60 // Nombre maximal d'itérations d'accélération (un trop grand nombre est inutile)

#define NOTE_TIMEOUT 5000000       //(actuellement 10 sec) Timeout d'une note (en µs)
#define MOTOT_ALIM_TIMEOUT 1000000 // Timeout de l'alimentation des moteurs (en µs)
#define ENABLE_PIN 21

const int states[8][4] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 0}, {1, 0, 1, 0}, {0, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 1, 0, 1}};
const uint8_t motorsPins[4][4] = {
	{12, 14, 27, 26},
	{25, 33, 32, 35},
	{4, 0, 2, 15},
	{10, 5, 17, 16},
};

extern MusicMotor motors[MOTOR_NUMBER];

extern unsigned long *delay_notes;
extern void generateNotes();
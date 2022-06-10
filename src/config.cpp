#include <config.h>
#include <Arduino.h>

MusicMotor motors[MOTOR_NUMBER];

unsigned long * delay_notes;
double getMidiNote(int m) { return pow(2, (double)(m - 69) / (double)12) * (double)440; }

void generateNotes(){
    if(delay_notes) free(delay_notes);
    delay_notes = (unsigned long *)malloc(sizeof(unsigned long) * NUMBER_OF_NOTE_TO_CALC);
    
    for (int i = 0; i < NUMBER_OF_NOTE_TO_CALC; i++)
	{
		delay_notes[i] = double((double)1000000 / (double)getMidiNote(i) / (double)2);
	}
}
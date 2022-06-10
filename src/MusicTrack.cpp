#include <Arduino.h>
#include <config.h>
#include <MusicMotor.h>
#include <SPIFFS.h>
#include <MusicTrack.h>
#include <util.h>
#include <SynchronizedBuffer.h>

MusicEvent::MusicEvent(byte state, byte note, byte channel, int delta)
{
    this->state = state;
    this->note = note;
    this->channel = channel;
    this->delta = delta;
}
void MusicEvent::setMotorId(uint8_t motorId)
{
    this->motorId = motorId;
}

MusicTrack::MusicTrack() {}
MusicTrack::MusicTrack(QueueHandle_t *queueIn, QueueHandle_t *queueOut)
{
    this->queueIn = queueIn;
    this->queueOut = queueOut;
}
void MusicTrack::request(uint32_t id)
{
    String filePath = String("/") + toString(id);
    Serial.print("Request ");
    Serial.println(filePath);
    if (!SPIFFS.exists(filePath))
    {
        Serial.println("File does not exits");
        return;
    }
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        motors[i].reset();
    }
    file = SPIFFS.open(filePath, "r");
    file.readBytes(name, 16);
    name[15] = '\0';

    file.readBytes((char *)&tempo, 1);
    if (tempo == 0)
    {
        Serial.println("Tempo is 0");
        return;
    }
    file.readBytes((char *)&timeDivision, 4);
    beatToUsFactor = ((float)1000000) / ((float)(float(tempo) / 60.0)) / (float)timeDivision;

    idPlaying = id;
    setPlaying(true);
}
void MusicTrack::handleQueue()
{
    if (xQueueReceive(*this->queueIn, &this->lastEvent, 0) == pdTRUE)
    {
        int data = lastEvent->getData();
        switch (lastEvent->type)
        {
        case Event::PAUSE:
            setPlaying(false);
            break;
        case Event::RESUME:
            setPlaying(true);
            break;
        case Event::REQUEST:
            request(data);
            break;
        case Event::TRANSPOSE:
        {
            // set each motor to the new transpose
            int transposeDelta = data - transpose;
            for (int i = 0; i < MOTOR_NUMBER; i++)
            {
                if (motors[i].actualNote != -1)
                {
                    int note = motors[i].actualNote;
                    int channel = motors[i].actualChannel;
                    int newNote = note + transposeDelta;
                    if(note < 0)
                    {
                        newNote = 0;
                    }
                    if(note > NUMBER_OF_NOTE_TO_CALC)
                    {
                        newNote = NUMBER_OF_NOTE_TO_CALC;
                    }
                    motors[i].setNote(newNote, channel);
                }
            }
            transpose = data;
            break;
        }
        default:
            break;
        }
        lastEvent = NULL;
    }
}
MusicEvent MusicTrack::getNextEvent()
{
    byte state;
    byte note;
    byte channel;
    int delta;
    file.readBytes((char *)&state, sizeof(state));
    file.readBytes((char *)&channel, sizeof(channel));
    file.readBytes((char *)&note, sizeof(note));
    file.readBytes((char *)&delta, sizeof(delta));

    return MusicEvent(state, getTrueNote(note), channel, delta);
}

int MusicTrack::getTrueNote(int note)
{
    if (transpose != 0)
    {
        if (transpose * -1 > note)
        {
            note = 0;
        }
        else if (transpose + note > NUMBER_OF_NOTE_TO_CALC - 1)
        {
            note = NUMBER_OF_NOTE_TO_CALC - 1;
        }
        else
        {
            note += transpose;
        }
    }
    return note;
}

void MusicTrack::tick(unsigned long delta)
{
    this->handleQueue();
    if (!playing)
        return;
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        motors[i].tick(delta);
    }
    if (delta >= nextEventDelta)
    {
        int iterations = 0;
        while (delta >= nextEventDelta && iterations < 5)
        {
            iterations++; // Pour être sûr de ne pas ralentir les moteurs
            if (file.position() >= file.size())
            {
                Serial.println("end");
                setPlaying(false);
                for (int i = 0; i < MOTOR_NUMBER; i++)
                {
                    motors[i].reset();
                }
                return;
            }
            MusicEvent event = getNextEvent();
            delta -= nextEventDelta;
            if (delta < 0)
            {
                delta = 0;
            }
            nextEventDelta = event.delta * beatToUsFactor + delta;
            if (event.state == 0)
            {
                short motorId = getMotor(event.note, event.channel);
                if (motorId != 255)
                {
                    motors[motorId].reset();
                }
            }
            else if (event.state == 1)
            {
                short motorId = getFreeMotor(event.note, event.channel);
                motors[motorId].setNote(event.note, event.channel);
            }
        }
    }
    else
    {
        nextEventDelta -= delta;
    }
}
uint8_t MusicTrack::getFreeMotor(byte note, byte channel)
{
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        uint8_t motor = (i + motorRotation) % MOTOR_NUMBER;
        if (motors[motor].actualNote == -1)
        {
            motorRotation = (motorRotation + 1) % MOTOR_NUMBER;
            return motor;
        }
    }
    // check if two motors have the same note
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        uint8_t motor = (i + motorRotation) % MOTOR_NUMBER;
        if (motors[motor].actualNote == note)
        {
            motorRotation = (motorRotation + 1) % MOTOR_NUMBER;
            return motor;
        }
    }
    // check if a motors have the same channel as the note
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        uint8_t motor = (i + motorRotation) % MOTOR_NUMBER;
        if (motors[motor].actualChannel == channel)
        {
            motorRotation = (motorRotation + 1) % MOTOR_NUMBER;
            return motor;
        }
    }
    // get the motor who played the note the longest
    int max = 0;
    int maxIndex = 0;
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        if (motors[i].noteTotalTime > max)
        {
            max = motors[i].noteTotalTime;
            maxIndex = i;
        }
    }
    return maxIndex;
}
uint8_t MusicTrack::getMotor(byte note, byte channel)
{
    for (int i = 0; i < MOTOR_NUMBER; i++)
    {
        if (motors[i].actualNote == note && motors[i].actualChannel == channel)
        {
            return i;
        }
    }
    return -1;
}
void MusicTrack::setPlaying(bool playing)
{
    digitalWrite(ENABLE_PIN, playing ? HIGH : LOW);
    this->playing = playing;
}
#include <Arduino.h>
#include <FakeTrack.h>
#include <InternalEvent.h>

FakeTrack::FakeTrack(QueueHandle_t *queue)
{
    this->queue = queue;
    this->lastEvent = NULL;
}
void FakeTrack::resume()
{
    Serial.println("Resuming-Fake");
}
void FakeTrack::pause()
{
    Serial.println("Pausing-Fake");
}
void FakeTrack::tick()
{
    
    if (xQueueReceive(*this->queue, &this->lastEvent, 0) == pdTRUE){
        if(lastEvent->type == Event::PAUSE){
            this->pause();
        }
        else if(lastEvent->type == Event::RESUME){
            this->resume();
        }
        lastEvent = NULL;
    }
}
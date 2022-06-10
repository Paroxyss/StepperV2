#include <Arduino.h>
#include <InternalEvent.h>

class FakeTrack
{
public:
    QueueHandle_t *queue;
    InternalEvent *lastEvent;
    FakeTrack(){};
    FakeTrack(QueueHandle_t *queue);
    void resume();
    void pause();
    void tick();
};
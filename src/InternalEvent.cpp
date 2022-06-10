#include <Arduino.h>
#include <InternalEvent.h>

InternalEvent::InternalEvent(Event type, uint32_t dataIn) {
    this->type = type;
    this->data = dataIn;
}
int InternalEvent::getData(){
    return this->data;
}

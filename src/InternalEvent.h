#pragma once

enum Event {
    REQUEST,
    STOP,
    PAUSE,
    RESUME,
    TRACKSTATE,
    TRANSPOSE
};

class InternalEvent {
public:
    InternalEvent(Event type, uint32_t dataIn = 0);
    Event type;
    int data;
    int getData();
};


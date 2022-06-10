#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <SynchronizedBuffer.h>
#include <MusicTrack.h>
#pragma once

#define INPUT_DOC_SIZE 100

class WebSocketManager
{
public:
    WebSocketsServer webSocket = WebSocketsServer(81);

    xQueueHandle *queueIn;
    xQueueHandle *queueOut;
    
    boolean sendMotor = false;
    
    WebSocketManager(){};
    WebSocketManager(xQueueHandle *queueIn, xQueueHandle *queueOut);
    void setup();
    void tick();
    void handle(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
    void handleWsJsonMessage(StaticJsonDocument<INPUT_DOC_SIZE> message, uint8_t num);
};
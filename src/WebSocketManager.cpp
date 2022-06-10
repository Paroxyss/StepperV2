#include <WebSocketsServer.h>
#include <WebSocketManager.h>
#include <ArduinoJson.h>
#include <InternalEvent.h>
#include <SPIFFS.h>
#include <util.h>
#include <FileUploadHandler.h>

WebSocketManager::WebSocketManager(xQueueHandle *queueIn, xQueueHandle *queueOut)
{
    this->queueIn = queueIn;
    this->queueOut = queueOut;
}

void WebSocketManager::setup()
{
    webSocket.begin();
    webSocket.onEvent([=](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      { handle(num, type, payload, length); });
}
void WebSocketManager::tick()
{
    webSocket.loop();
    tickFileSocket();
    // if motor broadcast is enabled, send motor data
    int size = synchronizedBufferSize();
    if (sendMotor && size > 0)
    {
        short i = 0;
        uint8_t *totalPayload = (uint8_t *)malloc(4 * size);
        for (i = 0; i < size; i++)
        {
            MusicEvent *musicEvent = synchronizedBufferPop();
            uint8_t payload[4] = {musicEvent->motorId, musicEvent->state, musicEvent->note, musicEvent->channel};
            memcpy(&totalPayload[i * 4], payload, 4);
        }
        webSocket.broadcastBIN(totalPayload, 4 * size);
        free(totalPayload);
    }
}

void WebSocketManager::handle(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        webSocket.sendTXT(num, "{\"type\":\"connected\"}");
        Serial.printf("[%u] Connected\n", num);
        break;
    }
    case WStype_TEXT:
    {
        if (payload[0] == '{')
        {
            StaticJsonDocument<INPUT_DOC_SIZE> doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
            handleWsJsonMessage(doc, num);
        }
    }
    default:
    {
        return;
    }
    }
}

void WebSocketManager::handleWsJsonMessage(StaticJsonDocument<INPUT_DOC_SIZE> message, uint8_t num)
{
    DynamicJsonDocument doc = DynamicJsonDocument(1024);
    doc["tag"] = message["tag"];
    const char *type = message.getMember("type").as<const char *>();
    if (String("pause").equals(type))
    {
        InternalEvent *event = new InternalEvent(Event::PAUSE);
        xQueueSend(*this->queueOut, &event, 0);
    }
    else if (String("resume").equals(type))
    {
        InternalEvent *event = new InternalEvent(Event::RESUME);
        xQueueSend(*this->queueOut, &event, 0);
    }
    else if (String("transpose").equals(type))
    {
        int amount = message.getMember("data").as<int>();
        InternalEvent *event = new InternalEvent(Event::TRANSPOSE, amount);
        xQueueSend(*this->queueOut, &event, 0);
    }
    else if (String("request").equals(type))
    {
        int id = message.getMember("data").as<int>();
        InternalEvent *event = new InternalEvent(Event::REQUEST, id);
        xQueueSend(*this->queueOut, &event, 0);
    }
    else if (String("ls").equals(type))
    {
        doc["type"] = "ls";
        doc.createNestedArray("data");
        int i = 0;
        while (1)
        {
            String path = String("/") + toString(i);
            if (!SPIFFS.exists(path))
            {
                break;
            }

            File file = SPIFFS.open(path, "r");
            char *name = (char *)malloc(16);
            file.readBytes(name, 15);
            name[15] = 0;

            doc["data"].add(name);
            free(name);
            i += 1;
        }
    }
    else if (String("motorStream").equals(type))
    {
        boolean enabled = message.getMember("data").as<boolean>();
        Serial.printf("motorStream: %d\n", enabled);
        synchronizedBufferReset();
        sendMotor = enabled;
    }
    else if (String("upload").equals(type))
    {
        if (openFileSocket())
        {
            doc["data"] = "ok";
        }
        else
        {
            doc["type"] = "upload";
            doc["data"] = "error";
        }
    }
    else if (String("delete").equals(type))
    {
        int id = message.getMember("data").as<int>();
        if (deleteFile(id))
        {
            doc["data"] = "ok";
        }
        else
        {
            doc["type"] = "delete";
            doc["data"] = "error";
            doc["error"] = "File not found";
        }
    }
    String response;
    serializeJson(doc, response);
    webSocket.sendTXT(num, response);
}
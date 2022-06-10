#include <WebSocketsServer.h>
#include <Arduino.h>
#include <SPIFFS.h>
#include <FileUploadHandler.h>
#include <ArduinoJson.h>
#include <util.h>

WebSocketsServer socket = WebSocketsServer(82);
boolean opened = false;

boolean queried = false;

ulong openTime = 0;

File writeFile;
String fileHash;
String filePath;
long fileSize;
long filePosition;

uint8_t uploaderId;

// start sending file payload format : {"type":"uploadInfo","fileHash":"hash","fileSize":size}
void handle(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    if (!opened)
        return;
    if (queried && type == WStype_BIN && num == uploaderId)
    {
        if(filePosition + length < fileSize){
            writeFile.write(payload, length);
            filePosition += length;
        }
        else{
            writeFile.write(payload, fileSize - filePosition);
            writeFile.close();
            // reopen
            File verifFile = SPIFFS.open(filePath, "r");
            
            StaticJsonDocument<128> response;
            if(fileHash.equals(file_md5(verifFile))){
                response["type"] = "uploadSuccess";
                verifFile.close();
            }
            else{
                Serial.printf("md5 mismatch %s != %s\n", fileHash.c_str(), file_md5(verifFile).c_str());

                response["type"] = "uploadError";
                response["error"] = "hash";
                // delete file
                verifFile.close();
                SPIFFS.remove(filePath);
            }
            // send response
            String resStr;
            serializeJson(response, resStr);
            socket.sendTXT(uploaderId, resStr);
            socket.close();

            fileHash = "";
            filePath = "";
            fileSize = 0;
            filePosition = 0;
            queried = false;
            opened = false;
        }
    }
    if (!queried && type == WStype_TEXT)
    {
        StaticJsonDocument<64> doc;
        DeserializationError error = deserializeJson(doc, payload);
        StaticJsonDocument<128> response;
        if (error)
        {
            response["type"] = "error";
            response["error"] = "deserializeJson error";
        }
        else
        {
            String type = doc["type"];
            if (type == "uploadInfo")
            {
                fileSize = doc["fileSize"];

                // check if there is space in the file system
                int freeSpace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
                if (freeSpace < fileSize)
                {
                    response["type"] = "error";
                    response["error"] = "not enough space";
                }
                else
                {

                    String path = String("/") + toString(getFirstDispoId());
                    filePath = path;
                    if (SPIFFS.exists(path))
                    {
                        response["type"] = "error";
                        response["error"] = "file already exists";
                    }
                    else
                    {

                        fileHash = doc["fileHash"].as<String>();

                        response["type"] = "ready";
                        response["filePath"] = filePath;
                        uploaderId = num;
                        writeFile = SPIFFS.open(path, "a");
                        Serial.println("w " + path);
                        queried = true;
                    }
                }
            }
        }
        String responseStr;
        serializeJson(response, responseStr);
        socket.sendTXT(num, responseStr);
    }
}

boolean openFileSocket()
{
    if (opened)
    {
        return false;
    }
    socket.close();
    socket = WebSocketsServer(82);
    socket.begin();
    opened = true;
    queried = false;
    filePosition = 0;
    openTime = millis();
    socket.onEvent([=](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                   { handle(num, type, payload, length); });
    return true;
};
void tickFileSocket(){
    if (opened)
    {
        socket.loop();
        if(!queried && millis() - openTime > 5000){
            opened = false;
            socket.broadcastTXT("{\"type\":\"query_timeout\"}");
            socket.close();
        }
        else if(millis() - openTime > 60000){ // 1 minute timeout
            opened = false;
            writeFile.close();
            // delete file
            SPIFFS.remove(filePath);
            socket.broadcastTXT("{\"type\":\"upload_timeout\"}");
            socket.close();
        }
    }
};
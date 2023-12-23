#include <Arduino.h>
//#include <ArduinoOTA.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <esp_task_wdt.h>
#include <wifiConfig.h>

#include <FakeTrack.h>
#include <InternalEvent.h>
#include <MusicMotor.h>
#include <MusicTrack.h>
#include <WebSocketManager.h>
#include <config.h>

QueueHandle_t xQueueWSToMusic;
QueueHandle_t xQueueMusicToWS;

WebSocketManager webSocketManager =
    WebSocketManager(&xQueueMusicToWS, &xQueueWSToMusic);

MusicTrack track;
InternalEvent *internalEvent;

WiFiMulti wifiMulti;

TaskHandle_t Task1;
unsigned long timer = 0;
unsigned long oldTime = 0;
unsigned long delta = 0;
void musicTask(void *pvParameters) {
    disableLoopWDT();
    for (;;) {
        //ArduinoOTA.handle();
        webSocketManager.tick();
    }
}

void setup() {
    Serial.begin(115200);
    // put your setup code here, to run once:
    generateNotes();

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);

    for (uint8_t i = 0; i < MOTOR_NUMBER; i++) {
        motors[i] = MusicMotor(motorsPins[i], i);
        Serial.printf("Motor %d %d\n", i, motors[i].id);
    }

    // mount SPIFFS
    SPIFFS.begin();
    Serial.println("SPIFFS mounted");

    wifiMulti.addAP(WIFI_SSID_PHONE, WIFI_PASS_PHONE);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

    while (wifiMulti.run(50000U) != WL_CONNECTED) {
        delay(500);
    }

    xQueueWSToMusic = xQueueCreate(MAX_EVENTS_IN_QUEUE, sizeof(InternalEvent));
    xQueueMusicToWS = xQueueCreate(MAX_EVENTS_IN_QUEUE, sizeof(InternalEvent));

    Serial.println(WiFi.localIP());
    webSocketManager.setup();

    /*ArduinoOTA.setPort(3132);
    ArduinoOTA.setHostname("StepperESP");
    ArduinoOTA.setPassword((const char *)OTA_PASS);
    ArduinoOTA.begin();*/

    track = MusicTrack(&xQueueWSToMusic, &xQueueMusicToWS);

    disableCore0WDT();
    xTaskCreatePinnedToCore(
        musicTask, /* Task function. */
        "music",   /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        8,         /* priority of the task */
        &Task1,    /* Task handle to keep track of created task */
        0);        /* pin task to core 0 */
    micros();
}

void loop() {
    oldTime = timer;
    timer = micros();
    if (oldTime > timer) {
        delta = 0; // ignore overflow
    } else {
        delta = timer - oldTime;
    }

    track.tick(delta);
}
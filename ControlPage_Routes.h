#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


struct Command
{
    int8_t x = 0; // -100..100 (ліво/право)
    int8_t y = 0; // -100..100 (назад/вперед)
};

class ControlPage_Router
{

private:
    AsyncWebServer server{80}; 
    Command command;
    unsigned long lastCmdMs = 0;

    const char *page; // вказівник на HTML-сторінку, передається ззовні

    static constexpr unsigned long TIMEOUT_MS = 200;

public:
    explicit ControlPage_Router(const char *page) : page(page) {}

    void setupRoutes()
    {
        // Маршрут "/" — віддаємо HTML сторінку
        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *req)
                  { req->send_P(200, "text/html", page); });

        // Маршрут "/control" — отримуємо команду від джойстика
        server.on("/control", HTTP_GET, [this](AsyncWebServerRequest *req)
                  {
            if (req->hasParam("x") && req->hasParam("y")) {
                command.x = (int8_t)constrain(req->getParam("x")->value().toInt(), -100, 100);
                command.y = (int8_t)constrain(req->getParam("y")->value().toInt(), -100, 100);
                lastCmdMs = millis();
            }
            req->send(200, "text/plain", "OK"); });

        // Ping для перевірки з'єднання
        server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *req)
                  { req->send(200, "text/plain", "PONG"); });

        server.begin();
        Serial.println("✓ Web server started. Open: http://" + WiFi.softAPIP().toString());
    }

    // Повертає останню команду. Якщо тайм-аут — скидає до нуля
    Command getCommand()
    {
        if (millis() - lastCmdMs > TIMEOUT_MS)
        {
            command.x = 0;
            command.y = 0;
        }
        return command;
    }

    String getIP() const { return WiFi.softAPIP().toString(); }
};

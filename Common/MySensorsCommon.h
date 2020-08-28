#pragma once
#include <MySensorsCustomConfig.h>
#include <MySensors.h>

#define MAX_SEND_ATTEMPTS 5

class ISensor {
public:
    virtual void setup() { }
    virtual void present() = 0;
    virtual bool report() = 0;
    static constexpr uint8_t InvalidSensorId = 255;
};

class MessageSender
{
private:
    volatile bool _acked = false;
public:
    MessageSender() { 
        
    }

    bool handleAck(const MyMessage &message) {
        if (message.isAck()) {
            this->_acked = true;
            #ifdef MY_DEBUG
            Serial.println("ACK received.");
            #endif
            return true;
        }
        return false;
    }

    void send(MyMessage& message) {
        this->_acked = false;
        for (byte i = 0; i < MAX_SEND_ATTEMPTS; i++) {
            ::send(message, true);
            ::wait(40);
            if (this->_acked) {
                #ifdef MY_DEBUG
                Serial.println("GW ACK");
                #endif
                break;
            }
            #ifdef MY_DEBUG
            Serial.println("GW NACK");
            #endif
            ::wait(40 * (1 << i));
        }
    }

    ~MessageSender() { }
};



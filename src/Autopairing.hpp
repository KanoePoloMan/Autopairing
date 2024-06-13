#ifndef __AUTOPAURING_HPP__
#define __AUTOPAURING_HPP__

#include "main.hpp"

#define CLOUD_PIN 14
#define BUFFER_SIZE 245

const uint32_t timeout_micros = (int)(1.0 / 74880 * 1E6) * 20;
uint32_t send_timeout = 0;

uint32_t pinsTimer[2];

uint8_t peerAddr[6];
int paired = 0;

uint8_t buf_recv[BUFFER_SIZE];
uint8_t buf_send[BUFFER_SIZE];

typedef enum {
    PAIRING,
    DATA,
    UPLOAD_COMMAND,
    OTHER_DATA
} MessageType;
typedef enum {
    RESET,
    BOOT,
    TEST,
    PAIRED_IS_OK
} SubcommandType;
typedef enum {
    FLASH,
    PERVOPROHODETS
} SenderType;

typedef struct MessageStruct {
    uint8_t msgType;//тип сообщения
    uint8_t command;//команда подтипа сообщения
    bool senderType;//тип отправителя сообщения
    uint8_t password;//пароль
    uint8_t msgLen;
    uint8_t message[BUFFER_SIZE];//сообщение

    MessageStruct();
} MessageStruct;

void masterPairing();
void slavePairing();

void messageSend();
void messageRecv(uint8_t *mac, uint8_t *incomiingData, uint8_t len);

bool checkPassword(uint8_t *mac, uint8_t password);

void timersCheck();

void writeToEEPROM(uint8_t *data, uint8_t len);
void readFromEEPROM(uint8_t *data, uint8_t len);

#endif
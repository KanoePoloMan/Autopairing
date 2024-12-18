#ifndef __AUTOPAIRING_HPP__
#define __AUTOPAIRING_HPP__

#include "main.hpp"

#define BUFFER_SIZE 245

#define DTR 13
#define RTS 12

#define RESET_32 12
#define BOOT_32 13

extern const uint32_t timeout_micros;
extern uint32_t send_timeout;

extern uint32_t pinsTimer[2];

extern uint8_t peerAddr[6];
extern int paired;

extern uint32_t lastSendPackageTime;
extern uint32_t lastRecvPackageTime;

// uint8_t buf_recv[BUFFER_SIZE];
// uint8_t buf_send[BUFFER_SIZE];

typedef enum {
    PAIRING,
    DATA,
    UPLOAD_COMMAND,
    OTHER_DATA
} MessageType;

//TODO добавить команду изменения канала
//Команду изменения мощности сигнала
//Команду изменения скорости прошивки между дистанционными платами
typedef enum {
    RESET,
    BOOT,
    TEST,
    PAIRED_IS_OK,
    PING,
    SETUP_PING,
    CHANGE_BAUDRATE
} SubcommandType;
typedef enum {
    FLASH,
    PERVOPROHODETS
} SenderType;

typedef struct MessageStruct {
    uint8_t msgType;//тип сообщения
    uint8_t command;//команда подтипа сообщения
    uint8_t senderType;//тип отправителя сообщения
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

void uploadCheckMaster();

void pingConnection();

#endif
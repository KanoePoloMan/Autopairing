#include "UartBuffer.hpp"

uint8_t* UartBuffer::getUartBuffer() {
    return uartBuffer;
}
uint8_t UartBuffer::getPos() {
    return uartPos;
}
void UartBuffer::addSym(uint8_t sym) {
    uartBuffer[uartPos] = sym;
    uartPos++;
    if(uartPos >= MAX_BUFFER_SIZE) uartPos = 0;
}
int16_t UartBuffer::findSequence(uint8_t* sequence, uint8_t length) {
    uint8_t temp[MAX_BUFFER_SIZE];
    memcpy(temp, uartBuffer + uartPos, MAX_BUFFER_SIZE - uartPos);
    memcpy(temp + (MAX_BUFFER_SIZE - uartPos), uartBuffer, uartPos);

    uint8_t first = 0;
    uint8_t finded = 0;
    for(int i = 0; i < MAX_BUFFER_SIZE; i++) {
        if(temp[i + 0] == sequence[0]) first = i;
        else continue;
        for(int j = 0; j < length; j++) {
            
            if(temp[i + j] == sequence[j]) finded++;
            else finded = 0;
        }
        if(finded == length) return (uint8_t)(first + uartPos);
    }
    
    return -1
}
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
uint8_t UartBuffer::findSequence(uint8_t* sequence) {
    uint8_t temp[MAX_BUFFER_SIZE];
    memcpy(temp, uartBuffer + uartPos, MAX_BUFFER_SIZE - 1 - uartPos);
    memcpy(temp + (MAX_BUFFER_SIZE - uartPos), uartBuffer, uartPos);

    uint8_t* result = (uint8_t *)strstr((char *)temp, (char *)sequence);
    uint8_t pos = result - temp;
    return uartPos + pos; 
}
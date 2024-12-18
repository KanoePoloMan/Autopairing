#ifndef __UART_BUFFER_HPP_
#define __UART_BUFFER_HPP_

#define MAX_BUFFER_SIZE 256

#include "stdlib.h"
#include "string.h"

class UartBuffer {
    private:
        uint8_t uartBuffer[MAX_BUFFER_SIZE];
        uint8_t uartPos = 0;
    public:
        uint8_t* getUartBuffer();
        uint8_t getPos();

        void addSym(uint8_t sym);
        /**
         * @return Возвращает -1, если последовательность не найдена, или элемент начала последовательности
         */
        uint8_t findSequence(uint8_t* sequence);
};

#endif //__UART_BUFFER_HPP_
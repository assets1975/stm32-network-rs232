
#ifndef _rs232_h_
#define _rs232_h_

void rs232_setup();
void rs232_writeString(const char* str);
uint16_t rs232_readLine(char* buffer, uint16_t size);
void rs232_clearBuffer();
void rs232_usartIrq();

#endif

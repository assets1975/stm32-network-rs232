
#include "debug.h"
#include <utils/utils.h>
#include <stdio.h>
#include <string.h>
#include "platform_config.h"
#include <utils/ringbufferdma.h>
#include <contiki/core/sys/process.h>

PROCESS(debug_process, "Debug");

#define DEBUG_RX_BUFFER_SIZE 32
RingBufferDmaU8 debugRxRing;
uint8_t debugRxBuffer[DEBUG_RX_BUFFER_SIZE];

void _debug_processLine(const char* line);

void debug_setup() {
  printf("debug_setup\n");
  RingBufferDmaU8_initUSARTRx(&debugRxRing, &DEBUG_UART, debugRxBuffer, DEBUG_RX_BUFFER_SIZE);
  process_start(&debug_process, NULL);
}

void _debug_processLine(const char* line) {
  if (strlen(line) == 0) {
  } else {
    printf("invalid debug command: %s\n", line);
  }
  printf("> ");
}

PROCESS_THREAD(debug_process, ev, data) {
  char line[30];

  PROCESS_BEGIN();
  while(1) {
    PROCESS_PAUSE();
    while(RingBufferDmaU8_available(&debugRxRing) > 0) {
      if(RingBufferDmaU8_readLine(&debugRxRing, line, sizeof(line)) > 0) {
	strTrimRight(line);
	_debug_processLine(line);
      }
    }
  }
  PROCESS_END();
}

#ifdef __GNUC__
#  define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#  define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#  define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#  define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE {
  HAL_UART_Transmit(&DEBUG_UART, (uint8_t *)&ch, 1, MAX_TIMEOUT);
  return ch;
}

GETCHAR_PROTOTYPE {
  while(RingBufferDmaU8_available(&debugRxRing) == 0);
  return RingBufferDmaU8_read(&debugRxRing);
}

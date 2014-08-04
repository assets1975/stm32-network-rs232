#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <string.h>
#include "network.h"
#include "uip/uip_arp.h"
#include "uip/uip-conf.h"
#include "platform_config.h"
#include "debug.h"

uint8_t _network_uip_headerLength = 0;

static int _nework_handle_connection(struct network_uip_state* state);

void network_setup() {
  debug_write_line("?BEGIN network_setup");

  uip_ipaddr_t ipaddr;
  uip_ipaddr_t gatewayAddr;
  uip_ipaddr_t netmaskAddr;

  uip_ethaddr.addr[0] = MAC_ADDRESS[0];
  uip_ethaddr.addr[1] = MAC_ADDRESS[1];
  uip_ethaddr.addr[2] = MAC_ADDRESS[2];
  uip_ethaddr.addr[3] = MAC_ADDRESS[3];
  uip_ethaddr.addr[4] = MAC_ADDRESS[4];
  uip_ethaddr.addr[5] = MAC_ADDRESS[5];

  uip_ipaddr(&ipaddr, IP_ADDRESS[0], IP_ADDRESS[1], IP_ADDRESS[2], IP_ADDRESS[3]);
  uip_sethostaddr(ipaddr);

  uip_ipaddr(&gatewayAddr, GATEWAY_ADDRESS[0], GATEWAY_ADDRESS[1], GATEWAY_ADDRESS[2], GATEWAY_ADDRESS[3]);
  uip_setdraddr(gatewayAddr);

  uip_ipaddr(&netmaskAddr, NETMASK_ADDRESS[0], NETMASK_ADDRESS[1], NETMASK_ADDRESS[2], NETMASK_ADDRESS[3]);
  uip_setnetmask(netmaskAddr);

  enc28j60_setup(&uip_ethaddr);

  uip_init();
  uip_arp_init();

  uip_listen(HTONS(NETWORK_PORT));

  debug_write_line("?END network_setup");
}

void network_tick() {
  enc28j60_tick();
}

void enc28j60_reset_assert() {
  GPIO_ResetBits(ENC28J60_RESET_PORT, ENC28J60_RESET_PIN);
}

void enc28j60_reset_deassert() {
  GPIO_SetBits(ENC28J60_RESET_PORT, ENC28J60_RESET_PIN);
}

void enc28j60_spi_assert() {
  GPIO_ResetBits(ENC28J60_CS_PORT, ENC28J60_CS_PIN);
}

void enc28j60_spi_deassert() {
  GPIO_SetBits(ENC28J60_CS_PORT, ENC28J60_CS_PIN);
}

uint8_t enc28j60_spi_transfer(uint8_t d) {
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI1, d);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(SPI1);
}

void uip_appcall() {
  struct network_uip_state* state = (struct network_uip_state*)&(uip_conn->appstate);

  if(uip_connected()) {
    PSOCK_INIT(&state->p, state->inputbuffer, sizeof(state->inputbuffer));
  }

  _nework_handle_connection(state);
}

void uip_udp_appcall() {
}

static int _nework_handle_connection(struct network_uip_state* state) {
  PSOCK_BEGIN(&state->p);

  PSOCK_SEND_STR(&state->p, "Hello. What is your name?\n");
  PSOCK_READTO(&state->p, '\n');
  strncpy(state->name, state->inputbuffer, sizeof(state->name));
  PSOCK_SEND_STR(&state->p, "Hello ");
  PSOCK_SEND_STR(&state->p, state->name);
  PSOCK_CLOSE(&state->p);

  PSOCK_END(&state->p);
}

/*
 * UART.h
 *
 *  Created on: Oct 17, 2013
 *  Author: Amanda
 */

#ifndef UART_H_
#define UART_H_

#define UART_TX_BUF_LEN 128
#define UART_RX_BUF_LEN 256

extern int uartStart;
extern int id;
extern char uartRxBuf[UART_RX_BUF_LEN];

//char checksum[10];


void initUART(void);
void uartSend(char vec);
void uartRead(char a);
void Delay(void);

void clockSetup();

void uartEnable(void);
void uartDisable(void);

int uartSendAvailable(void);
char uart_is_tx_clear(void);

#endif /* UART_H_ */


/*
 * UART.h
 *
 *  Created on: Oct 9, 2013
 *      Author: Administrator
 */

#ifndef UART_H_
#define UART_H_

#define UART_TX_BUF_LEN 128
#define UART_RX_BUF_LEN 256

//char checksum[10];


void initUART(void);
void uart_send(int len, char vec);
void uart_read(char a);
void Delay(void);

void clockSetup();

void uart_enable(void);
void uart_disable(void);

int uart_send_available(void);
char uart_is_tx_clear(void);

#endif /* UART_H_ */

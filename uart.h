#ifndef UART_H_
#define UART_H_

void uart0_init(void);
void uart0_send_byte(uint8_t byte);
void uart0_send_string(const char *s);
uint8_t uart0_receive_byte(void);
void uart0_send_hex8(uint8_t val);
char convert_lower_case(char s);
void uart0_send_uint32_hex(uint32_t val);

#endif /* UART_H_ */

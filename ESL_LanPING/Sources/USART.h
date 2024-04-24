#ifndef USART_H
#define USART_H


#define EXT_OC          18432000   // Exetrnal ocilator MAINCK
#define MCK             48054857   // MCK (PLLRC div by 2)
#define MCKKHz          (MCK/1000) //


//#define BR   115200                        /* Baud Rate */
//#define BRD  (MCK/16/BR)                    /* Baud Rate Divisor */

#define BR    115200                        /* Baud Rate */

#define BRD  (MCK/16/BR)                    /* Baud Rate Divisor */

void uart0_init (void);
int uart0_putc(int ch);
int uart0_putchar (int ch);
int uart0_puts ( char* s );
int uart0_getc ( void );
void InterraptUSART (void);
unsigned int crc(unsigned char *buf,int start,int cnt);
#define USART_INTERRUPT_LEVEL		7
#endif

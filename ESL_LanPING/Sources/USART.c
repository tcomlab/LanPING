#include "AT91SAM7S32.h"
#include "USART.h"
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

AT91S_DBGU * pUSART = AT91C_BASE_DBGU;      /* Global Pointer to USART0 */
// AT91S_PMC * pPMC = AT91C_BASE_PMC;

//-------------------------------------------------------------------------------
void RX_interrapt (void)
{
  unsigned char byte;
  byte = pUSART->DBGU_RHR;
  byte = byte;
  //uart0_puts("OK");
}
//-------------------------------------------------------------------------
void uart0_init (void) {                   /* Initialize Serial Interface */

  // AT91S_PMC * pPMC = AT91C_BASE_PMC;
  // pPMC->PMC_PCER = (1 << AT91C_ID_US0);
  // AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_US0);
  *AT91C_PIOA_OER = AT91C_PA10_DTXD;
  *AT91C_PIOA_ODR = AT91C_PA9_DRXD;
  *AT91C_PIOA_ASR = AT91C_PA10_DTXD | AT91C_PA9_DRXD;
  *AT91C_PIOA_PDR = AT91C_PA10_DTXD | AT91C_PA9_DRXD;

  *AT91C_DBGU_MR = AT91C_US_PAR_NONE | AT91C_US_CHMODE_NORMAL;
  *AT91C_DBGU_IDR= 0xffffffff;

  *AT91C_DBGU_BRGR = MCK / (115200 * 16);
  *AT91C_DBGU_CR =  AT91C_US_RXEN | AT91C_US_TXEN;


}
//-------------------------------------------------------------------------------
int uart0_putc(int ch) 
{
	while (!(pUSART->DBGU_CSR & AT91C_US_TXRDY));   /* Wait for Empty Tx Buffer */
	return (pUSART->DBGU_THR = ch);                 /* Transmit Character */
}	
//-------------------------------------------------------------------------------
int uart0_putchar (int ch) {                      /* Write Character to Serial Port */

  if (ch == '\n')  {                            /* Check for LF */
    uart0_putc( '\r' );                         /* Output CR */
  }
  return uart0_putc( ch );                     /* Transmit Character */
}
//-------------------------------------------------------------------------------
int uart0_puts ( char* s )
{
	while ( *s ) uart0_putchar( *s++ );
	return 0;
}
//-------------------------------------------------------------------------------
int uart0_kbhit( void ) /* returns true if character in receive buffer */
{
	if ( pUSART->DBGU_CSR & AT91C_US_RXRDY) {
		return 1;
	}
	else {
		return 0;
	}
}
//-------------------------------------------------------------------------------
int uart0_getc ( void )  /* Read Character from Serial Port */
{    

  while (!(pUSART->DBGU_CSR & AT91C_US_RXRDY));   /* Wait for Full Rx Buffer */
  return (pUSART->DBGU_RHR);                      /* Read Character */
}
//-------------------------------------------------------------------------------


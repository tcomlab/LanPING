/*
	CP2200 driver for uIP TCP/IP Stack S.Perzborn [14.03.2007]
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 *
 */

#include "main.h"
//#include <intrinsics.h>
#include "CP2200.h"
#include "uip.h"
#include "uip_arp.h" 
//#include "AT91SAM7S32.h"

 
void write_CP2200(int adr, char value);
char read_CP2200(int adr);
void Init_CP2000(void);
static void cpWriteMac(char adr,int value);
extern unsigned char setting_buf[SETTING_BUF_SIZE];
#define ON 1
#define OFF 0
//extern void Led (char stat);
struct uip_eth_addr eaddr;
void network_device_init(void);
u16_t network_device_read(void);
void network_device_send(void);

//#include "USART.h"

extern void delay_ms( unsigned short delay);

static char CP2200_status=0;
/*
 void itohex(char *hexstr, int dez, char length)
{
	char i=0;
	char tmp=0;
	if (length > 3)
	{
		tmp=(dez & 0xF000) >> 12;
		if (tmp > 9)
			hexstr[(int)i] = tmp + 'A' - 10;
		else
			hexstr[(int)i] = tmp + '0';
		i++;
	}
	if (length > 2)
	{
		tmp=(dez & 0x0F00) >> 8;
		if (tmp > 9)
			hexstr[(int)i] = tmp + 'A' - 10;
		else
			hexstr[(int)i] = tmp + '0';
		i++;
	}
	if (length > 1)
	{
		tmp=(dez & 0x00F0) >> 4;
		if (tmp > 9)
			hexstr[(int)i] = tmp + 'A' - 10;		
		else
			hexstr[(int)i] = tmp + '0';
			
		i++;
	}
	if (length > 0)
	{
		tmp=dez & 0x000F;
		if (tmp > 9)
			hexstr[(int)i] = tmp + 'A' - 10;			
		else
			hexstr[(int)i] = tmp + '0';
		i++;
	} 
	hexstr[(int)i]=0;
}
*/
// ---------------------------------------------
/*
unsigned char CP2200_ReadTXBuffer(unsigned int addr)
{
	write_CP2200(RAMADDRL,addr & 0xFF); 
	write_CP2200(RAMADDRH,addr >> 8); 
	return (read_CP2200(RAMTXDATA));
}
*/
// ---------------------------------------------
void CP2200_WriteTXBuffer(unsigned int addr, unsigned char value)
{
	write_CP2200(RAMADDRL,addr & 0xFF); 
	write_CP2200(RAMADDRH,addr >> 8); 
	write_CP2200(RAMTXDATA,value); 
}

// ---------------------------------------------
/*
unsigned char CP2200_ReadRXBuffer(unsigned int addr)
{
	write_CP2200(RAMADDRL, (char) (addr & 0xFF)); 
	write_CP2200(RAMADDRH,(char) (addr >> 8)); 
	return (read_CP2200(RAMRXDATA));
}
*/
// ---------------------------------------------
/*
void CP2200_WriteRXBuffer(unsigned int addr, unsigned char value)
{
	write_CP2200(RAMADDRL,addr & 0xFF); 
	write_CP2200(RAMADDRH,addr >> 8); 
	write_CP2200(RAMRXDATA,value); 
}
*/
// ---------------------------------------------
u16_t CP2200_ReadPacket(void)
{
	// uip_buf
	// get packet len
		u16_t packetlen;
	u16_t i;
	packetlen = read_CP2200(CPLENH);
	packetlen = packetlen << 8;
	packetlen |= read_CP2200(CPLENL);
	for (i=0;i<packetlen;i++)
	{
		*(uip_buf+i) = read_CP2200(RXAUTORD);
	}
	return(packetlen);
}

// ---------------------------------------------
u16_t network_device_read(void)
{
	u16_t  packetlen=0;
	int cp_bufptr; // Zeiger auf Paket Puffer
	CP2200_status = read_CP2200(CP2200_INT0);
	if ((CP2200_status & 0x02) == 0x02)
	{
		//uart0_puts("CP2200 buffer full device read\r\n");	
		write_CP2200(RXCN,0x01);
		write_CP2200(RXCN,0x00);
	}
	CP2200_status=0;
	// Paket im Empfangspuffer?
	if (((read_CP2200(CPINFOH) &0x80) != 0) && (read_CP2200(CPINFOL) &0x80) != 0)
	{
		//PORTD &= ~(1<<PD6); // set PB0 = LED OFF
		
		// Zeiger auf Paket Puffer merken
		cp_bufptr = read_CP2200(RXFIFOHEADH);
		cp_bufptr = cp_bufptr << 8;
		cp_bufptr |= read_CP2200(RXFIFOHEADL);
	
		packetlen = CP2200_ReadPacket();
		write_CP2200(RXCN,0x06); // discharge package

		//	write_CP2200(RXCN,0x04); // discharge package
	}
	return(packetlen);
}

// ---------------------------------------------
void CP2200_WritePacket(void)
{
	//unsigned int len;
	unsigned int addr;
	unsigned int t;
	// wait for previous packet complete
	while (read_CP2200(TXBUSY) != 0x00);
	// set transmit buffer pointer
	write_CP2200(TXSTARTH,0x00);
	write_CP2200(TXSTARTL,0x00);
	
	//hwsend(&uip_buf[0], UIP_LLH_LEN);
	addr=0;
	for (t=0;t < UIP_LLH_LEN;t++)
	{
		CP2200_WriteTXBuffer(addr++, *(uip_buf + t));
	}

	if(uip_len <= UIP_LLH_LEN + UIP_TCPIP_HLEN)
	{

		//hwsend(&uip_buf[UIP_LLH_LEN], uip_len - UIP_LLH_LEN);
		for (t=UIP_LLH_LEN;t < uip_len;t++)
		{
			CP2200_WriteTXBuffer(addr++, *(uip_buf + t));
		}
	}
	else
	{
		//hwsend(&uip_buf[UIP_LLH_LEN], UIP_TCPIP_HLEN);
		for (t=UIP_LLH_LEN;t < UIP_TCPIP_HLEN+UIP_LLH_LEN;t++)
		{
			CP2200_WriteTXBuffer(addr++, *(uip_buf + t));
		}
		//hwsend(uip_appdata, uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN);
		for (t=0;t < ((uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN)+1);t++)
		{
			CP2200_WriteTXBuffer(addr++, *(((unsigned char *)uip_appdata) + t));
		}
	}
//  for (addr=0;addr<uip_len;addr++)
//	{
//		CP2200_WriteTXBuffer(addr, *(uip_buf + addr));
//	}
	
	// set packet end
	//write_CP2200(TXENDH,(char) (uip_len >> 8));
	//write_CP2200(TXENDL,(char) (uip_len & 0xFF));
	write_CP2200(TXENDH,(char) (addr >> 8));
	write_CP2200(TXENDL,(char) (addr & 0xFF));
	// set packet start
	write_CP2200(TXSTARTH,0x00);
	write_CP2200(TXSTARTL,0x00);
	write_CP2200(TXCN,0x01); // send packet
	//uart_puts_P(PSTR("[sp] "));	
}

// ---------------------------------------------
void network_device_send(void)
{
	CP2200_WritePacket();
}




void write_CP2200(int adr,char value)
{        
       unsigned char tmp;
        //AT91C_BASE_PIOA->PIO_CODR |= CS;
       
        AT91C_BASE_PIOA->PIO_OER |= (D0|D1|D2|D3|D4|D5|D6|D7);
	AT91C_BASE_PIOA->PIO_SODR |= ALE; // set ALE 
        

        AT91C_BASE_PIOA->PIO_SODR |= (adr << D0_); // output address
        adr ^= 0xFF;
        AT91C_BASE_PIOA->PIO_CODR |=  (adr << D0_); // output address
        
	AT91C_BASE_PIOA->PIO_CODR |= ALE; // clear ALE
        
	AT91C_BASE_PIOA->PIO_CODR |= WR; // clear WR#
        
	tmp = value;
	AT91C_BASE_PIOA->PIO_SODR |= (tmp << D0_); // output address
        tmp ^= 0xFF;
        AT91C_BASE_PIOA->PIO_CODR |= (tmp << D0_); // output address
        
	AT91C_BASE_PIOA->PIO_SODR |= WR;  // set WR#
        
	tmp = value;
	AT91C_BASE_PIOA->PIO_SODR |= (tmp << D0_); // output address
        tmp ^= 0xFF;
        AT91C_BASE_PIOA->PIO_CODR |= (tmp << D0_); // output address
        
       // AT91C_BASE_PIOA->PIO_SODR |= CS;

}


char read_CP2200(int adr)
{
	unsigned char value;
        unsigned int tmp;
        
       // AT91C_BASE_PIOA->PIO_CODR |= CS;
        
        AT91C_BASE_PIOA->PIO_OER = (D0|D1|D2|D3|D4|D5|D6|D7);
	AT91C_BASE_PIOA->PIO_SODR = ALE; // set ALE
        
        tmp = adr;   
	AT91C_BASE_PIOA->PIO_SODR = (tmp << D0_); // output address
        tmp ^= 0xFF;
        AT91C_BASE_PIOA->PIO_CODR = (tmp << D0_); // output address
        
	AT91C_BASE_PIOA->PIO_CODR = ALE;; // clear ALE
        
        tmp = adr; 
	AT91C_BASE_PIOA->PIO_SODR = (tmp << D0_); // output address
        tmp ^= 0xFF;
        AT91C_BASE_PIOA->PIO_CODR = (tmp << D0_); // output address
        
	AT91C_BASE_PIOA->PIO_CODR = RD; // clear RD#
        AT91C_BASE_PIOA->PIO_ODR = (D0|D1|D2|D3|D4|D5|D6|D7);
	value = (AT91C_BASE_PIOA->PIO_PDSR >> D0_); // Waitstate
	value = (AT91C_BASE_PIOA->PIO_PDSR >> D0_); // Waitstate
	value = (AT91C_BASE_PIOA->PIO_PDSR >> D0_); // Waitstate
	value = (AT91C_BASE_PIOA->PIO_PDSR >> D0_); // Waitstate
	value = (AT91C_BASE_PIOA->PIO_PDSR >> D0_); // input D0..7
	AT91C_BASE_PIOA->PIO_SODR = RD; // set RD#
        
       // AT91C_BASE_PIOA->PIO_SODR |= CS;
        
	return(value);
}
// ---------------------------------------------
void network_device_init(void)
{
	//char hexstr[10];
	uip_ipaddr_t ipaddr;
	//struct uip_eth_addr eaddr;
	unsigned int i;
	
	Init_CP2000();
//#ifndef DBG_BORD
	// own mac address (get from CP2200 flash memory)
	write_CP2200(FLASHADDRL,0xFA);
	write_CP2200(FLASHADDRH,0x1F);
	//uart0_puts("\nMAC:");
	for (i=0;i<6;i++)
	{
		eaddr.addr[i]=read_CP2200(FLASHAUTORD); // mac address from CP2200 
		//itohex(hexstr,eaddr.addr[i] , 2);
		//uart0_puts(hexstr);
		
	}
	//uart0_puts("\n");
//#else
//        eaddr.addr[0] = 0x00;
//        eaddr.addr[1] = 0x16;
//        eaddr.addr[2] = 0xD4;
//        eaddr.addr[3] = 0x65;
//        eaddr.addr[4] = 0xE8;
//        eaddr.addr[5] = 0x11;
//#endif
        // set mac address of this modules
    uip_setethaddr(eaddr);
	// set netmask
	//uip_ipaddr(ipaddr, MSKA,MSKB,MSKC,MSKD);
	ipaddr[0] = SET->NetMask[0];
	ipaddr[1] = SET->NetMask[1]; 
	uip_setnetmask(ipaddr);
	// set ip address of this module
    //    uip_ipaddr(ipaddr, IPA,IPB,IPC,IPD);
	ipaddr[0] = SET->IPDev[0];
	ipaddr[1] = SET->IPDev[1];
	uip_sethostaddr(ipaddr);
	// set gateway address (default router address)
	//uip_ipaddr(ipaddr, GWA,GWB,GWC,GWD);
	ipaddr[0] = SET->Geteway[0];
	ipaddr[1] = SET->Geteway[1];
	uip_setdraddr(ipaddr);
}

// ---------------------------------------------
void Init_CP2000(void)
{
	volatile unsigned char tmp;
	unsigned int tmp16;
	unsigned int timeout;
   AT91C_BASE_PIOA->PIO_CODR = (RES);
	//uart0_puts("MAC_PHY reset on\r\n");
   delay_ms(200);
   AT91C_BASE_PIOA->PIO_SODR = (RES);
   //uart0_puts("MAC_PHY reset\n");
     
        
  //reset CP220x
  read_CP2200(CP2200_INT0); //clear CP2200 INT0
  write_CP2200(SWRST,4);
  while (!(read_CP2200(CP2200_INT0)& 0x20)); //wait for reset complete

  
//PHY INIT
  write_CP2200(PHYCF,0xFE);
  write_CP2200(PHYCN,0x00);
  delay_ms(1);
  read_CP2200(CP2200_INT1); 
  write_CP2200(PHYCN,0x80);
 delay_ms(1);
  write_CP2200(PHYCN,0xE0);
  delay_ms(75);
	timeout=3000;
  while ((!(read_CP2200(CP2200_INT1)& 0x01)) && (timeout > 0))
	{
		delay_ms(10);
		timeout--;
	}
	if (timeout < 1)
	{
		//uart0_puts("timeout n1 er1\r\n");
                while(1);
	}
	timeout=3000;
  while ((!(read_CP2200(PHYCN)& 0x01)) && (timeout > 0))
	{
		delay_ms(10);
	        timeout--;
	}
	if (timeout < 1)
	{
		//uart0_puts("timeout  n1 er2\r\n");
                while(1);
	}
	//MAC INIT
  cpWriteMac(MACCF,0x40B3);
  cpWriteMac(IPGT,0x0015);
  cpWriteMac(IPGR,0x0C12);
  cpWriteMac(MAXLEN,0x05EE);
  write_CP2200(FLASHADDRL,0xFA);
  write_CP2200(FLASHADDRH,0x1F);
  tmp16 = 0x0000;
	tmp16 = read_CP2200(FLASHAUTORD);
  tmp16 |=read_CP2200(FLASHAUTORD)<<8;
  cpWriteMac(MACAD2,tmp16);
	tmp16 = 0x0000;
  tmp16 = read_CP2200(FLASHAUTORD);
  tmp16 |= read_CP2200(FLASHAUTORD)<<8;
  cpWriteMac(MACAD1,tmp16);
  tmp = 0x0000;
	tmp16 = read_CP2200(FLASHAUTORD);
  tmp16 |= read_CP2200(FLASHAUTORD)<<8;
  cpWriteMac(MACAD0,tmp16);
	write_CP2200(IOPWR,0x0C);
  cpWriteMac(MACCN,0x01);
	write_CP2200(INT0EN,0x03);
	write_CP2200(INT1EN,0x00);
	read_CP2200(CP2200_INT0); 
	read_CP2200(CP2200_INT1); 
}


// ---------------------------------------------
static void cpWriteMac(char adr,int value)
{
  write_CP2200(MACADDR,adr);
  write_CP2200(MACDATAH,value>>8);
  write_CP2200(MACDATAL,value);
  write_CP2200(MACRW,1);
}

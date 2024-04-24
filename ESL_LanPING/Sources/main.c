// ------------------------------------------------------------------------/
// Main execution file
// Pinger device v 3.0.0
// ------------------------------------------------------------------------/
//-------------------------------------------------------------------------/
#include "main.h"
#include "CP2200.h"
//#include "USART.h"
#include "telnetd.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
#include "at91flash.h"
#include "shell.h"
#include <stdio.h>
#define BUF ((struct uip_eth_hdr *)&uip_buf[0]) // изернет буфер
#define UIP_CONF_IPV6 0

struct timer periodic_timer, arp_timer,time_to_ping,timeout_ping,sec_timer,reboot_timer;
/*---------------------------------------------------------------------------*/
extern void write_CP2200(int adr, char value);
extern char read_CP2200(int adr);
extern void network_device_init(void);
extern u16_t network_device_read(void);
extern void network_device_send(void);
extern unsigned char autorization;
extern char ping_flag; // если пришол ответ от хоста то флаг устанавливается
unsigned char setting_buf[SETTING_BUF_SIZE]; // буфер настроек
unsigned char DEVICE_STATE = 0; // состояние устройства если 1 то устройство в режиме стоп!
char sendPING = 0;
//static unsigned char setting_buf[SETTING_BUF_SIZE]; // буфер настроек
extern unsigned char DEBUG_TELNET;
clock_time_t  sec_counter;
unsigned char s_uip_send=0;
unsigned int reboot_counter = 0;
unsigned char no_reboot=0;
uip_ipaddr_t trgetaddr;
unsigned char server_visible = 0;
//******************************************************************************
#define ON 1
#define OFF 0
//-------------------------------------------------------------------------------
void debug_out(char *buf)
{
	if ((autorization == 2)&&(DEBUG_TELNET))
	{
		shell_output("DEBUG:",buf);
	}
	//else uart0_puts(buf);
}
//------------------------------------------------------------------------------
static inline void hal_WDR(void) // сброс сторожевого таймера
{
	*AT91C_WDTC_WDCR = 0xA5000001;
}
//-------------------------------------------------------------------------------
void Led (char stat) // управление светодиодом
{
	if (stat == 0) {AT91C_BASE_PIOA->PIO_CODR = Led1;}
	if (stat == 1) {AT91C_BASE_PIOA->PIO_SODR = Led1;}
}
//-------------------------------------------------------------------------------
void delay_ms( unsigned short delay) // задержка
{
	unsigned short count,a;
	unsigned long count2;

	for (count = 0; count < delay; count++) {
		for (count2 = 0;count2 <= 1850 ; count2++) {
			a = 1;
			a = a;
			WDT_RES();
		}
	}
}
//-------------------------------------------------------------------------------
void reset_switch (void) // сброс оборудования
{
	AT91C_BASE_PIOA->PIO_SODR = (REL);
	delay_ms(SET->time*1000);          // используется время из настройки
	AT91C_BASE_PIOA->PIO_CODR = (REL);
}
///----------------------------------------------------------------------
void timer0_c_irq_handler(void) // Секундный таймер
{
	static int time = 115;
	AT91PS_TC TC_pt = AT91C_BASE_TC0;
	unsigned int dummy;
	dummy = TC_pt->TC_SR;
	dummy = dummy;
	if (--time == 0)
	{
		// 1 Sekunden Timer
		time=115;
		sec_counter++;
	}
}
//---------------------------------------------------------------------
void clock_init(void)
{
	unsigned int dummy;
	__disable_interrupt();
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_TC0 ;
	AT91C_BASE_TC0->TC_IDR = AT91C_TC_CPCS;
	AT91C_BASE_PMC->PMC_PCER = ( 1 <<  AT91C_ID_TC0 ) ;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS ;
	AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF ;
	dummy = AT91C_BASE_TC0->TC_SR;
	dummy = dummy;
	AT91C_BASE_TC0->TC_CMR = 0x01 ;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN ;
	AT91C_BASE_AIC->AIC_IDCR =  0x1 << AT91C_ID_TC0 ;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_TC0] = (unsigned int) timer0_c_irq_handler ;
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_TC0] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | 1  ;
	AT91C_BASE_AIC->AIC_ICCR =  0x1 << AT91C_ID_TC0 ;
	AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
	AT91C_BASE_AIC->AIC_IECR = 0x1 << AT91C_ID_TC0;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG ;
	__enable_interrupt();
}
//--------------------------------------------------------------------------------
clock_time_t clock_time(void) // функция системного времени
{
	return(sec_counter);
}
void uip_log(char *text) // системный лог
{
	//uart_puts(text);
}
//--------------------------------------------------------------------------------
void wdt_init(void) // настройка сторожевого пса
{
  AT91C_BASE_WDTC->WDTC_WDMR = 0x3FF | (0x3FF << 16) | AT91C_WDTC_WDRSTEN | AT91C_WDTC_WDDBGHLT | AT91C_WDTC_WDIDLEHLT;
}
//---------------------------------------------------------------------------------
void config_mcu (void) // настройка портов процессора
{
	AT91C_BASE_PMC->PMC_PCER = ( 1 << AT91C_ID_PIOA );
#ifdef SWS01
        AT91C_BASE_PIOA->PIO_PER = (D0|D1|D2|D3|D4|D5|D6|D7|RD|WR|ALE|Led1|REL|RES);
#else
	AT91C_BASE_PIOA->PIO_PER = (D0|D1|D2|D3|D4|D5|D6|D7|RD|WR|ALE|Led1|REL|RES|SWDEF);
        AT91C_BASE_PIOA->PIO_ODR = SWDEF;
#endif
	AT91C_BASE_PIOA->PIO_OER = (D0|D1|D2|D3|D4|D5|D6|D7|RD|WR|ALE|Led1|REL|RES);
	
    AT91C_BASE_PIOA->PIO_SODR = (RD|WR|RES);
	AT91C_BASE_PIOA->PIO_CODR = (ALE|Led1|REL);
}
//------------------------------------------------------------------------------
void read_device_setting (void) // Read device setting
{
    __disable_interrupt();
	unsigned char i=0,m=0;
	for (i=0;i<(sizeof(setting_buf)-1);i+=4) 
	{
		setting_buf[i+3] = ( *((unsigned long*)ADRESS_FLASH_SETING+m))>>24;
		setting_buf[i+2] = ( *((unsigned long*)ADRESS_FLASH_SETING+m))>>16;
		setting_buf[i+1] = ( *((unsigned long*)ADRESS_FLASH_SETING+m))>>8;
		setting_buf[i+0] = ( *((unsigned long*)ADRESS_FLASH_SETING+m))>>0;
                m++;
	}
    __enable_interrupt();
}
//-----------------------------------------------------------------------------
void default_setting_set (void) // установка завоцких установок
{
	uip_ipaddr_t tempaddr;
	// default login
    SET->login[0] = 'a';
    SET->login[1] = 'd';
	SET->login[2] = 'm';
	SET->login[3] = 'i';
	SET->login[4] = 'n';
	SET->login[5] = '\0';
	// default password
	SET->password[0] = '5';
    SET->password[1] = '4';
	SET->password[2] = '3';
	SET->password[3] = '2';
	SET->password[4] = '1';
	SET->password[5] = '\0';
	// Set default ip adres
	uip_ipaddr(tempaddr, 192,168,0,254);
    SET->IPDev[0] = tempaddr[0];
	SET->IPDev[1] = tempaddr[1];
	// Set default NetMask adres
	uip_ipaddr(tempaddr, 255,255,255,0);
    SET->NetMask[0] = tempaddr[0];
	SET->NetMask[1] = tempaddr[1];
    // Set default Geteway adres
	uip_ipaddr(tempaddr, 192,168,0,1);
    SET->Geteway[0] = tempaddr[0];
	SET->Geteway[1] = tempaddr[1];
	// Set default ip adres
	uip_ipaddr(tempaddr, 192,168,0,1);
    SET->Target[0] = tempaddr[0];
	SET->Target[1] = tempaddr[1];
    // Set default period
	SET->period = 300;        // Период пингования
	SET->percent = 10 ;      // процент потерь пакетов
    SET->time = 60;          // Time reboot
	SET->size = 1024;          // Нагрузка пакета длинна пакета
	SET->count = 10;         // Количество передаваемых пакетов ICMP
//	write_device_setting();
}
//------------------------------------------------------------------------------
//unsigned char reboot (void)// Функция перезагрузки оборудования
//{
//	if(timer_expired(&reboot_timer)) // Если прошло время перезагрузки
//	{
//		if (!no_reboot)AT91C_BASE_PIOA->PIO_CODR = (REL);  // Включаем оборудование
//		//if (!no_reboot)AT91C_BASE_PIOA->PIO_SODR = (REL);
//                return 1;
//	}
//	else
//	{
//		if (!no_reboot)AT91C_BASE_PIOA->PIO_SODR = (REL); // Врубаем реле и обестачиваем оборудование
//	       // if (!no_reboot)AT91C_BASE_PIOA->PIO_CODR = (REL);
//        }
//	return 0;
//}
//------------------------------------------------------------------------------
unsigned int sec_counter_local=9999;
//------------------------------------------------------------------------------
void poll_ping (void)
{

	static unsigned int count_send_icmp = 0;
	static unsigned char time_out_ping=0;
	static unsigned char TOGLE_PING=1;
    static unsigned char no_answer=0;
    static unsigned char Reboot=0;
    //char ascii[128];
    int test;
//------------------------------------------------- если флаг ребута установлен
    if (Reboot) // Управляем ребутом
    {
    	if(timer_expired(&reboot_timer)) // Если прошло время перезагрузки
    	{
    		if (!no_reboot)AT91C_BASE_PIOA->PIO_CODR = (REL);  // Включаем оборудование
    		Reboot = 0;
                WDT_RES();
    	}
    	else
    	{
    		if (!no_reboot)AT91C_BASE_PIOA->PIO_SODR = (REL); // Врубаем реле и обестачиваем оборудование
                WDT_RES();
    	}

    }
//------------------------------------------------------------------------------

	if (timer_expired(&sec_timer)) // Заходим сюда раз в секунду ->
	{
		timer_restart(&sec_timer);
		sec_counter_local++;
		// проверяем отосланы ли все ICMP пакеты
		if (count_send_icmp >= SET->count)
		{
		  test = (int)(((float)SET->count/100)*SET->percent);
		  //sprintf(ascii, "count_send_icmp=%d no_answer=%d test=%d",count_send_icmp,no_answer,test);
		  //debug_out(ascii);
		  if (no_answer > test )
		  {
		 	Reboot = 1;
		 	timer_restart(&reboot_timer);
		 	reboot_counter++; // Инкремент счётчика перегрузок
		  }
		  sec_counter_local=0;
		  count_send_icmp = 0;
		  TOGLE_PING=1;
		  no_answer=0;
		}
		//---------------------------------------------------------------------
		// Проверяем не пришло ли время попинговать
		if (sec_counter_local > SET->period)
		{
			if (TOGLE_PING)
			{
				s_uip_send = 1; // Высталяем флаг что хотим попинговать
				TOGLE_PING = 0; // Сюда больше незаходим
				return;
			}

			if (ping_flag&(1<<0)) // Флаг установлен значит есть ответ
			{
				time_out_ping=0; // Тайм аут пинга 0
				s_uip_send = 1;  // Высталяем флаг что хотим попинговать
                Led(ON);         // Зажигаем светодиод что видим цель
                server_visible = 1; // Ставим флаг видимости цели
                count_send_icmp++; //Отосланый пакет считать зачотным автар маладец!!!
                return;
			}
			else time_out_ping++; // Инкримент тайм аута пинга
                  

			if (time_out_ping >= 5)
			{
				Led(OFF);  // Тушим светодиод ибо цель не видна
				s_uip_send = 1; // Высталяем флаг что хотим попинговать
                no_answer++; // Инкримент счётчика неответа
                server_visible = 0; // Снимаем флаг видимости цели
                time_out_ping = 0;
                count_send_icmp++;//Отосланый пакет считать зачотным автар маладец!!!
                return;
			}
		}
	}
	// -----------------------------------------------------------------
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
int main(void) // главный модуль программы
{
    unsigned long* wrptr = (unsigned long*)ADRESS_FLASH_SETING;
	int i=0;
	char togle = 0;
	int len_buf = 0;
	int arp_count =0;
    at91flashInit();
 //   uart0_init();
    wdt_init();
    if (wrptr[0] == 0xFFFFFFFF)
    {
      default_setting_set(); // Если чистая память то записывам туда
      //uart0_puts("load default setting\n");
      at91flashSetLock(ADRESS_FLASH_SETING,0);
      at91flashWrite(ADRESS_FLASH_SETING,setting_buf,sizeof(setting_buf));
    }
    else
    {
      read_device_setting();
      //uart0_puts("load stored in flash setting\n");
    }
	timer_set(&periodic_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 1);
	timer_set(&time_to_ping,SET->period); // periodic call send ping
	timer_set(&timeout_ping,5); // ping time out timer
	timer_set(&sec_timer,1);
	if(SET->time == 0) no_reboot = 1;
	else timer_set(&reboot_timer,SET->time);
	reboot_counter = 0;
	//---------------------------------------------------------
	config_mcu();   // инициализация служб и оборудования
	//---------------------------------------------------------
#ifdef SWS01
        
#else
	if ((AT91C_BASE_PIOA->PIO_PDSR & SWDEF)==0)
	{
		Led(ON);
		delay_ms(100);
		Led(OFF);
		delay_ms(100);
		Led(ON);
		delay_ms(100);
		Led(OFF);
		delay_ms(100);
		Led(ON);
	    default_setting_set(); // Если чистая память то записывам туда
	    //uart0_puts("load default setting\n");
	    at91flashSetLock(ADRESS_FLASH_SETING,0);
	    at91flashWrite(ADRESS_FLASH_SETING,setting_buf,sizeof(setting_buf));
		Led(ON);
		delay_ms(100);
		Led(OFF);
		delay_ms(100);
		Led(ON);
		delay_ms(100);
		Led(OFF);
		while(1);
	}
#endif
	//---------------------------------------------------------
	network_device_init();
	clock_init();
	uip_arp_init();
	uip_init();
	telnetd_init();
        //----------------------------------------------
//        uip_ipaddr_t ipaddr;
//        uip_ipaddr(&ipaddr, 192,168,5,51);
//        uip_connect(&ipaddr, HTONS(1468));
	//----------------------------------------------
	//uart0_puts("Start System\n");
//=--------------------------------------------------------------------------------------
	while(1)
	{
	//-----------------------------------------------------------------------------------
	WDT_RES();
	uip_len = network_device_read();
	if(uip_len > 0)
	{
	 if(BUF->type == htons(UIP_ETHTYPE_IP))
	  {
		uip_arp_ipin();
		uip_input();
	   /* If the above function invocation resulted in data that
		  should be sent out on the network, the global variable
		 uip_len is set to a value > 0. */
	   if(uip_len > 0)
		  {
			uip_arp_out();
			network_device_send();
		  }
	  }
	 else
	 if(BUF->type == htons(UIP_ETHTYPE_ARP))
	  {
	   uip_arp_arpin();
	   /* If the above function invocation resulted in data that
		  should be sent out on the network, the global variable
		  uip_len is set to a value > 0. */
	   if(uip_len > 0)
		{
		 network_device_send();
		}
	  }
	}
	else
	{
	if(timer_expired(&periodic_timer))
	  {
	   timer_reset(&periodic_timer);
	   for(i = 0; i < UIP_CONNS; i++)
		{
		  uip_periodic(i);
		  /* If the above function invocation resulted in data that
			 should be sent out on the network, the global variable
			 uip_len is set to a value > 0. */
		  if(uip_len > 0)
			{
			   uip_arp_out();
			   network_device_send();
			}
	   }
	  }
	  /* Call the ARP timer function every 10 seconds. */
	if(timer_expired(&arp_timer))
	  {
		timer_reset(&arp_timer);
		uip_arp_timer();
	  }
	//--------------------------------------------------------------------
	if (DEVICE_STATE)
	{
		if (timer_expired(&sec_timer))
		{
			timer_restart(&sec_timer);
			if (togle == 0) {Led(ON);togle = 1;}
	        else
			if (togle == 1) {Led(OFF);togle = 0;}
		}
	}
	else
	{
		if (s_uip_send)
		{

			trgetaddr[0] = SET->Target[0];
			trgetaddr[1] = SET->Target[1];
			uip_ping(trgetaddr,128,SET->size);
			len_buf = uip_len;
            uip_arp_ipin();
            uip_arp_out();
            if (len_buf == uip_len) // Если найден MAC адрес
            {
              s_uip_send = 0;
              network_device_send();// Отсылаем пинг
            }
            else
            {						// Если не найден MAC адрес
            	if (arp_count == 20)
				{
					s_uip_send = 0;
					arp_count = 0;
				}
            	else
				{
					s_uip_send = 1;
					arp_count++;
				}

            	network_device_send();
            }
		}
		else
		{
			poll_ping();
		}
	}
	//-------------------------------------------------------------------
	}
	//-------------------------------------------------------------------
	}
}
// enf of programm

	

//------------------------------------------------------------------------------
#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdlib.h>   // malloc
#include <intrinsics.h>
#include "AT91SAM7S32.h"
#include "setting.h"
#include "uip.h"

#define WDT_RES()  hal_WDR()

//#define CPU_CLK_Hz 20000000
//#define CPU_FREQ   20000000
//#define CPU_CLK_kHz (unsigned long)(CPU_CLK_Hz/1000)  

//#define delay_ns(x) __delay_cycles(x*CPU_CLK_kHz*0.000001)
//#define delay_us(x) __delay_cycles(x*(CPU_CLK_Hz/1000000))
//#define delay_ms(x) __delay_cycles(x*(CPU_CLK_Hz/1000))
//#define delay_s(x)  __delay_cycles(x*CPU_CLK_Hz)

//unsigned char setting_buf []

#define ADRESS_FLASH_SETING 0x007f00 // Adres weare saved setting in device

//------------------------------------------------------------------------
struct device_hdt_setting 
{
 unsigned char login[10],    // Login to acces
               password[10]; // Password to acces
 uip_ipaddr_t   IPDev,    // Device ip adress
                NetMask,  // Netmask
                Geteway,  // Getaway ip adress
                Target;   // Target device
 unsigned short period,       // Period of the ping
                count,        // ICMP packet sent of period
	        percent,      // Persent drooping paket
	        time,         // Time reboot equpment
                size;        // Size data ICMP packet
};
//----------------------------------------------------------------------------
#define SETTING_BUF_SIZE 53

#define SET ((struct device_hdt_setting *)&setting_buf[0])
//----------------------------------------------------------------------------
// HARDWE SECTIONS

//---------------------------------------------------
#ifdef PROTO_BORD
#define SWDEF (1<<3)
#define RD  (1<<12)
#define WR  (1<<11)
#define ALE (1<<7) 
#define RES (1<<21)
#define Led1 (1<<0)
#define REL  (1<<1)
#define D0 (1<<13)
#define D1 (1<<14)
#define D2 (1<<15)
#define D3 (1<<16)
#define D4 (1<<17)
#define D5 (1<<18)
#define D6 (1<<19)
#define D7 (1<<20)
#define D0_ 13
#endif
//---------------------------------------------------
#ifdef PWD01
#define SWDEF (1<<16)
#define RD   (1<<11)
#define WR   (1<<12)
#define ALE  (1<<8) 
#define Led1 (1<<14)
#define REL (1<<20)
#define RES (1<<13)
#define D0 (1<<0)
#define D1 (1<<1)
#define D2 (1<<2)
#define D3 (1<<3)
#define D4 (1<<4)
#define D5 (1<<5)
#define D6 (1<<6)
#define D7 (1<<7)
#define D0_ 0
#endif
//-----------------------------------------------------
#ifdef SWS01
//#define SWDEF (1<<16)
#define RD  (1<<12) // ?????? -
#define WR  (1<<13) // ?????? -
#define ALE (1<<8) //
#define Led1 (1<<14)
#define REL (1<<16)
#define RES (1<<15)
#define D0 (1<<0)
#define D1 (1<<1)
#define D2 (1<<2)
#define D3 (1<<3)
#define D4 (1<<4)
#define D5 (1<<5)
#define D6 (1<<6)
#define D7 (1<<7)
#define D0_ 0
#endif
//---------------------------------------------------------
#ifdef SM1
#define SWDEF (1<<16)
#define RD   (1<<20)
#define WR   (1<<19)
#define ALE  (1<<8) 
#define Led1 (1<<10)
#define REL (1<<15)
#define RES (1<<9)
#define D0 (1<<0)
#define D1 (1<<1)
#define D2 (1<<2)
#define D3 (1<<3)
#define D4 (1<<4)
#define D5 (1<<5)
#define D6 (1<<6)
#define D7 (1<<7)
#define D0_ 0
#endif
//-----------------------------------------------------------
#endif

//------------------------------------------------------------------

#include "shell.h"
#include "main.h"
#include "at91flash.h"
#include <string.h>
#include <stdio.h>
#include "timer.h"

struct ptentry {
  char *commandstr;
  void (* pfunc)(char *str);
};

#define SHELL_PROMPT "cmd#"
unsigned char autorization = 0;
unsigned char DEBUG_TELNET=0;
#define DEVICE_STOP  1
#define DEVICE_START 0
extern unsigned char DEVICE_STATE;
extern unsigned char setting_buf[SETTING_BUF_SIZE];
extern void reset_switch (void);
extern struct uip_eth_addr eaddr;
extern void delay_ms( unsigned short delay);
extern unsigned int reboot_counter;
extern unsigned char server_visible;
extern clock_time_t  sec_counter;
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
//void inttostr(char *str, unsigned int i)
//{
//  str[0] = '0' + i / 100;
//  if(str[0] == '0') {
//    str[0] = ' ';
//  }
//  str[1] = '0' + (i / 10) % 10;
//  if(str[0] == ' ' && str[1] == '0') {
//    str[1] = ' ';
//  }
//  str[2] = '0' + i % 10;
//  str[3] = ' ';
//  str[4] = 0;
//}
/*---------------------------------------------------------------------------*/
void help(char *str)
{
  shell_output("show   - show settings","");
  //shell_prompt("\n");
  shell_output("reboot - reboot equipment","");
 // shell_prompt("\n");
  shell_output("reload - reboot watch dog device","");
 // shell_prompt("\n");
  shell_output("stop   - stop monitoring network","");
 // shell_prompt("\n");
  shell_output("start  - start monitoring network","");
 // shell_prompt("\n");
  shell_output("set    - set device paramiter ","");
 // shell_prompt("\n");
  shell_output("exit   - exit from telnet","");
}
//---------------------------------------------------------------------------------
void set_help(void)
{
  shell_output("ip xxx.xxx.xxx.xxx ","- device ip address ");
 // shell_prompt("\n");
  shell_output("mask xxx.xxx.xxx.xxx ","- device mask ");
 // shell_prompt("\n");
  shell_output("gw xxx.xxx.xxx.xxx ","- gateway ");
 // shell_prompt("\n");
  shell_output("tar xxx.xxx.xxx.xxx ","- target device ");
 // shell_prompt("\n");
  shell_output("period xx - send ","test packet period sec");
//  shell_prompt("\n");
  shell_output("count xxx - send ","test packet ICMP request");
 // shell_prompt("\n");
  shell_output("size xxxx - ICMP ","data bytes");
 // shell_prompt("\n");
  shell_output("percent xxx - % ","bad ICMP packet");
 // shell_prompt("\n");
  shell_output("time xxx - time ","reboot equipment");
}
/*---------------------------------------------------------------------------*/
void unknown(char *str)
{
  if(strlen(str) > 0) {
    shell_output("Unknown command: ", str);
  }
}
/*---------------------------------------------------------------------------*/
void rebbot_eqip(char *str)
{
  autorization = 0;
  shell_output("resetting equipment...","");
	reset_switch();
        shell_output("resetting equipment...OK!!","");
}
/*---------------------------------------------------------------------------*/
void reload_device(char *str)
{
  autorization = 0;
#ifdef DBG_BORD
 shell_output("reboot....","");
 (*void)0x0000;
 while(1);
#else
 shell_output("reboot....system","");
 while(1);
#endif
}
/*---------------------------------------------------------------------------*/
void stop_device(char *str)
{
	DEVICE_STATE = DEVICE_STOP;
        shell_output("pinger stopped","");
}
/*---------------------------------------------------------------------------*/
void start_device(char *str)
{
	DEVICE_STATE = DEVICE_START;
        shell_output("pinger started","");
}
/*---------------------------------------------------------------------------*/
char* ip_tostring (uip_ipaddr_t tempaddr)
{
    static char asciiIP[16];

    sprintf(asciiIP, "%i.%i.%i.%i",
            (unsigned char)(tempaddr[0]),
            (unsigned char)(tempaddr[0] >> 8),
            (unsigned char)(tempaddr[1]),
            (unsigned char)(tempaddr[1] >> 8));

    return asciiIP;
}
/*-------------------------------------------------------------------------------*/
char stringtoip(int ofset,char* ipstr,unsigned char *ip )
{
        unsigned int val,i;
        ipstr = ipstr + ofset;
        for(i = 0; i < 4; i++)
        {
                if ((*ipstr != ' ')&&(*ipstr != '\0'))
                {
                        val = 0;
                        while ((*ipstr != '\0')&&(*ipstr != ' ')&& *ipstr != '.')
                        {
                                val *= 10;
                                val += *ipstr - '0';
                                ipstr++;
                        }
                        ip[i]= val;
                        if ((*ipstr != '\0')&&(*ipstr != ' '))
                        	ipstr++;
                }
        }
        return 1;
}
/*---------------------------------------------------------------------------*/
void show_device(char *str)
{
  char param[10];
  shell_output("-----------------------------------------------------","");
  shell_output("Ip: ",ip_tostring(SET->IPDev));
  shell_output("Mask: ",ip_tostring(SET->NetMask));
  shell_output("GateWay: ",ip_tostring(SET->Geteway));
  shell_output("Target: ",ip_tostring(SET->Target));
  sprintf(param,"%d sec",SET->time);
  shell_output("Reboot equipment time: ",param);
  sprintf(param,"%d sec",SET->period);
  shell_output("Ping interval: ",param);
  sprintf(param,"%d sec",SET->count);
  shell_output("ICMP send packet: ",param);
  sprintf(param,"%d",SET->size);
  shell_output("ICMP data: ",param);
  sprintf(param,"%d %%",SET->percent);
  shell_output("Percent bad packet: ",param);
  sprintf(param,"%d ",reboot_counter);
  shell_output("-----------------------------------------------------","");
  shell_output("Reboot counter: ",param);
  if (server_visible) shell_output("Observed server visible: yes","");
  else shell_output("Observed server visible: no","");
  sprintf(param,"%d sec",sec_counter);
  shell_output("Time after inclusion: ",param);
  if (DEVICE_STATE) shell_output("Device state: stop","");
  else shell_output("Device state: start/work","");
  shell_output("-----------------------------------------------------","");
}
//----------------------------------------------------------------------------//
int stringtoint(char *value)
{
  int result;
  if (!value || !(result = atoi(value)))
  /* the number 0 is always an incorrect result ... */
  {
    result = -1;
  }
  return(result);
}
//----------------------------------------------------------------------------//
void set_parametr (char *str)
{
    unsigned char options = 0,i=0;
    unsigned short tmp=0;
    unsigned char ip[4];//,string[11];
    str = str + 3; // убираем set
    while(*str++ > 0) // Парсим строку и ищим наши настройки
    {
    //----------------------------------------------------------------------------//
      if (strncmp(str,"help",4)==0)
      {
    	  set_help();
    	  return;
      }
      //---------------------------------------------------------------------------//
      if (strncmp(str,"ip",2)==0)
      {
    	 if (stringtoip(3,str,ip))
    	 {
    		 SET->IPDev[0] = (ip[0])|(ip[1]<<8);
    		 SET->IPDev[1] = (ip[2])|(ip[3]<<8);
    		 options = 1;
    	 }
    	 else  shell_output("wrong argument", "IP address");
      }
      //----------------------------------------------------------------------------//
      if (strncmp(str,"mask",4)==0)
      {
    	 if (stringtoip(5,str,ip))
    	 {
			 SET->NetMask[0] = (ip[0])|(ip[1]<<8);
			 SET->NetMask[1] = (ip[2])|(ip[3]<<8);
			 options = 1;
    	 }
    	 else  {shell_output("wrong argument", "Mask address"); return;}
      }
      //----------------------------------------------------------------------------//
      if (strncmp(str,"gw",2)==0)
      {
    	 if (stringtoip(3,str,ip))
    	 {
			 SET->Geteway[0] = (ip[0])|(ip[1]<<8);
			 SET->Geteway[1] = (ip[2])|(ip[3]<<8);
			 options = 1;
    	 }
	     else {shell_output("wrong argument", "Gateway address"); return;}
      }
      //----------------------------------------------------------------------------//
      if (strncmp(str,"tar",3)==0)
      {
    	 if(stringtoip(4,str,ip))
    	 {
			 SET->Target[0] = (ip[0])|(ip[1]<<8);
			 SET->Target[1] = (ip[2])|(ip[3]<<8);
			 options = 1;
         }
    	 else  {shell_output("wrong argument", "Target ip address"); return;}
      }
      //----------------------------------------------------------------------------//
      if (strncmp(str,"login",5)==0)
      {
    	  str = str + 5;
    	  i = 0;
    	  while(*str++ > '\0')
    	  {
			SET->login[i] = (*str);
                        i++;
    	  }
          SET->login[i] = '\0'; 
          options = 1;
      }
      //----------------------------------------------------------------------------//
      if (strncmp(str,"passwd",6)==0)
      {
          str = str + 6;
    	  i = 0;
    	  while(*str++ > '\0')
    	  {
			SET->password[i] = (*str);
                        i++;
    	  }
          SET->password[i] = '\0'; 
          options = 1;
      }
      //----------------------------------------------------------------------------//
      if(strncmp(str,"size",4)==0)
      {
          str = str + 5;
          tmp = stringtoint(str);
          if (tmp > 1024)
          {
        	  shell_output("errore:max size 1024 byte", "");
        	  return;
          }
          SET->size = tmp;
          options = 1;
      }
      //----------------------------------------------------------------------------//
      if(strncmp(str,"count",5)==0)
      {
          str = str + 6;
          tmp = stringtoint(str);
          //if (tmp > 1024)
          //{
        	//  shell_output("errore:max size 1024 byte", "");
          //	  return;
          //}
          SET->count = tmp;
          options = 1;
      }
      //----------------------------------------------------------------------------//
      if(strncmp(str,"period",6)==0)
      {
          str = str + 7;
          tmp = stringtoint(str);
          //if (tmp > 1024)
          //{
        	//  shell_output("errore:max size 1024 byte", "");
          //	  return;
          //}
          SET->period = tmp;
          options = 1;
      }
      //----------------------------------------------------------------------------//
      if(strncmp(str,"percent",7)==0)
      {
          str = str + 8;
          tmp = stringtoint(str);
          if (tmp > 100)
          {
        	  shell_output("errore:max parameter 100%", "");
          	  return;
          }
          SET->percent = tmp;
          options = 1;
      }
      //----------------------------------------------------------------------------//
      if(strncmp(str,"time",4)==0)
      {
          str = str + 5;
          tmp = stringtoint(str);
          //if (tmp > 100)
          //{
        //	  shell_output("errore:max parameter 100%", "");
          //	  return;
          //}
          SET->time = tmp;
          options = 1;
      }
      //----------------------------------------------------------------------------//
    }
    //----------------------------------------------------------------------------//
	if (!options) shell_output("wrong argument", "");
        else
        	{
                delay_ms(100);
                __disable_interrupt();
        	    at91flashSetLock(ADRESS_FLASH_SETING,0);
                at91flashWrite(ADRESS_FLASH_SETING,setting_buf,sizeof(setting_buf));
                __enable_interrupt();
				shell_output("paramet saved", "");
        	}
}
/*---------------------------------------------------------------------------*/
//struct ptentry parsetab[] =
//  {
//   {"help",	help},
//   {"show",	show_device},
//   {"reboot",	rebbot_eqip},
//   {"reload",	reload_device},
//   {"stop",	stop_device},
//   {"start",	start_device},
//   {"set",	set_parametr},
//   {"exit",	shell_quit},
//   {'\0', unknown}
//  };

/*---------------------------------------------------------------------------*/
void shell_init(void)
{
	//
}
/*---------------------------------------------------------------------------*/
void shell_start(void)
{
  shell_output("Welcome to LanPING Device v1.4.2 ", "http://www.esl.ho.ua");
//#ifndef DBG_BORD
  shell_prompt("Login:");
//#else
//  shell_prompt(SHELL_PROMPT);
//#endif
}
/*---------------------------------------------------------------------------*/

void parse(char *strt)
{
//  struct ptentry *p;
//for(p = t; p->commandstr != NULL; ++p) {
//if(strncmp(p->commandstr, str, strlen(p->commandstr)) == 0) {
//  break;
//}
//}

//  p->pfunc(str);
  if (strncmp(strt,"help"  ,4)==0){  help(strt);return;}
  if (strncmp(strt,"set"   ,3)==0){  set_parametr(strt);return;}
  if (strncmp(strt,"start" ,5)==0){  start_device(strt);return;}
  if (strncmp(strt,"stop"  ,4)==0){  stop_device(strt);return;}
  if (strncmp(strt,"exit"  ,4)==0){  shell_quit(strt);return;}
  if (strncmp(strt,"show"  ,4)==0){  show_device(strt);return;}
  if (strncmp(strt,"reboot",6)==0){  rebbot_eqip(strt);return;}
  if (strncmp(strt,"reload",6)==0){  reload_device(strt);return;}
  if (strncmp(strt,"DEBUG" ,5)==0){  DEBUG_TELNET=1;shell_output("DEBUG:","ENABLE");return;}

  unknown(strt);
}
/*----------------------------------------------------------------------------*/
void shell_input(char *cmd)
{
//#ifndef DBG_BORD
  if (autorization == 0)
  {
     if (strcmp(SET->login,cmd) == 0)
	 {
       autorization = 1;
       //shell_prompt("\xff\xfe\x01");
       shell_output("Password: ","");
       return;
     }
     shell_output("Bad login, press any key.....","");
     shell_quit("end");
     autorization = 0;
  }
    if (autorization == 1)
  {
     if (strcmp(SET->password,cmd) == 0) 
     {
       autorization = 2;
       //shell_prompt("\xff\xfb\x01");
       shell_output("Welcome","");
       shell_prompt(SHELL_PROMPT);
       return;
     }
      shell_prompt("Bad login, press any key.....");
      shell_quit("end");
      autorization = 0;
    
  }
    if (autorization == 2)
  {
    //parse(cmd, parsetab);
    parse(cmd);
    shell_prompt(SHELL_PROMPT);
  }
//#else
//    parse(cmd, parsetab);
//    shell_prompt(SHELL_PROMPT);
//#endif

}
/*---------------------------------------------------------------------------*/

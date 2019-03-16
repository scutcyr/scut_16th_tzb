#include "includes.h" 
#include "malloc.h"  
#include "spblcd.h"
#include "spb.h"
#include "common.h" 
#include "ebook.h"
#include "settings.h"
#include "picviewer.h"
#include "audioplay.h"
#include "videoplay.h"
#include "calendar.h" 	 	 
#include "notepad.h"
#include "paint.h"
#include "camera.h"
#include "recorder.h"
#include "appplay.h"
#include "smsplay.h"

#include "usart3.h"
#include "sim900a.h"
#include "mpu6050.h"
#include "wm8978.h"
#include "lsens.h"
#include "usb_app.h"
//----------------------------------------------------------------//
//                      һ�ֻ����������������Ƶ����ܼҾ�ϵͳ      //
//                      2019��03��13��                            //
//                      �汾��1.0.0                               //
//                      Github:https://github.com/scutcyr         //
//                      ���䣺eecyryou@mail.scut.edu.cn           //
//----------------------------------------------------------------//
//
//     ���ܽ���
//     1��ͨ������HC-05ʵ���ֻ�ң�ؿ���/���ţ�����/�صƣ�������/�ط���
//     2���ֻ��ɲ�ѯ���ż�¼�Լ������û�����Ϣ
//     3������ģ����ƿ���/���ţ�����/�صƣ�������/�ط���
//     4������ʶ��+����ģ�鿪��
//--------------------------------------------------------------------------------------------
// Ӳ��         I/O��
// Light      PFout(4)
// Fan        PFout(5)
// Door       PFout(10)
// BEEP       PGout(14)
// Window
// Motor_IN1  PFout(6)
// Motor_IN2  PFout(7)
// Motor_IN3  PFout(8)
// Motor_IN4  PFout(9)



//--------------------------------------------------------------------------------------------
// ����ͨ��ָ��                                ʵ�ֹ���
// -------------------------------------------------------------------------------------------
// �Ǽ�root�û�
// root�û�Android ID                        �Ǽ�root�û���ֻ���״�ʹ�û��߸�ʽ����ʹ����Ч��������Ч

// ע���û�
// root�û�Android ID+'T'+��ͨ�û�Android ID �����ͨ�û�
// root�û�Android ID+'S'+��ͨ�û�Android ID ɾ����ͨ�û�

// ����ָ��-----Door
// 'D'+root�û�Android ID+'0'                PF10��0��LED1��������
// 'D'+root�û�Android ID+'1'                PF10��1��LED1��������
// 'D'+��ͨ�û�Android ID+'0'                PF10��0��LED1��������
// 'D'+��ͨ�û�Android ID+'1'                PF10��1��LED1��������

// ����ָ��-----Light
// 'L'+root�û�Android ID+'0'                PF4��0���ص�
// 'L'+root�û�Android ID+'1'                PF4��1������
// 'L'+��ͨ�û�Android ID+'0'                PF4��0���ص�
// 'L'+��ͨ�û�Android ID+'1'                PF4��1������

// ����ָ��-----Fan
// 'F'+root�û�Android ID+'0'                PF5��0���ط���
// 'F'+root�û�Android ID+'1'                PF5��1��������
// 'F'+��ͨ�û�Android ID+'0'                PF5��0���ط���
// 'F'+��ͨ�û�Android ID+'1'                PF5��1��������

// ��ѯ�����û�
// 'U'+root�û�Android ID                    ���������û���Ϣ
// 'U'+��ͨ�û�Android ID                    ���ظ����û���Ϣ

// ��ѯ���ż�¼
// 'J'+root�û�Android ID                    ���������û����ż�¼
// 'J'+��ͨ�û�Android ID                    ���ظ����û����ż�¼

//
//--------------------------------------------------------------------------------------------
// ����                                         ʵ�ֹ���
// ��ɫ��λ��                                ��λ           

// ��RESET��λ���뿪������(�Լ����)��,Ѹ�ٰ����°���:
// ��סKEY0����:����ǿ�ƽ���У׼����,�Դ���������У׼(���޵�����,�Ե�������Ч)
// ��סKEY1����:����ǿ�ƽ����ֿ����,�����ֿ�
// ��סKEY2����:����ǿ�Ʋ�������SPI FLASH,����ʹ��SD�����ٸ���ϵͳ�ļ�
// ---��ʾ:��סKEY0~KEY2��Ҫһֱ��ס,�����ĵȴ�.ֱ��������Ӧ����/������ʾ

//--------------------------------------------------------------------------------------------

// �޸�ʱ�䣺2019��03��13��


//--------------------------------------------------------------------------------------------
// ����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
// ����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.
#define FLASH_DATA_SAVE    0X080FF000

// 0~3:num
// 4~19:root user Android ID
// 20~35:common user Android ID 1
// ...
// 132~147:common user Android ID 8
u8 flash_data_save[148];   

// ���ڴ��root ID
const u8 id_root_buf[]={"7659483c763e8649"};
// �û�id����������find_id[0]Ϊroot�û�id
u8 find_id[9]={4,20,36,52,68,84,100,116,132};

#define COMMON_BUF_LENTH sizeof(id_common_buf)
#define ROOT_BUF_LENTH sizeof(id_root_buf)
#define COMMON_BUF_SIZE COMMON_BUF_LENTH/4+((COMMON_BUF_LENTH%4)?1:0)
#define ROOT_BUF_SIZE ROOT_BUF_LENTH/4+((ROOT_BUF_LENTH%4)?1:0)

/////////////////////////��ʾ��Ϣ///////////////////////////////////

//ɾ�����������û�
u8*const delethbt_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"���Ѿ�ɾ�����������û�",	
"���Ѿ�ɾ�����������û�",
"���Ѿ�ɾ�����������û�", 
};

//ɾ�����������û�
u8*const ndelethbt_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"��û��ɾ�������û�",	
"��û��ɾ�������û�",	
"��û��ɾ�������û�",	 
};

//����������ʾ��Ϣ
u8*const aboutus_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"һ�ֻ����������������Ƶ����ܼҾ�ϵͳ\r\
�ֻ�ͨ���������ӱ�ϵͳ����ʵ�ֿ���\r\
ͨ����������+����ʶ��ʵ�ְ�ȫ�ļҾӿ���\r\
��Ʒ����:���ܼҾ�",	
"һ�ֻ����������������Ƶ����ܼҾ�ϵͳ\r\
�ֻ�ͨ���������ӱ�ϵͳ����ʵ�ֿ���\r\
ͨ����������+����ʶ��ʵ�ְ�ȫ�ļҾӿ���\r\
��Ʒ����:���ܼҾ�",
"һ�ֻ����������������Ƶ����ܼҾ�ϵͳ\r\
�ֻ�ͨ���������ӱ�ϵͳ����ʵ�ֿ���\r\
ͨ����������+����ʶ��ʵ�ְ�ȫ�ļҾӿ���\r\
��Ʒ����:���ܼҾ�",	 
};

//������ʾ��Ϣ
u8*const dizhen_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"Σ�վ�����\r\
��������\r\
����������\r\
������������\r\
������������\r\
������������",	
"Σ�վ�����\r\
��������\r\
����������\r\
������������\r\
������������\r\
������������",
"Σ�վ�����\r\
��������\r\
����������\r\
������������\r\
������������\r\
������������", 
};

 
/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      		10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ��8�ֽڶ���	
__align(8) static OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//��������
//�����������ȼ�
#define USART_TASK_PRIO       	7 
//���������ջ��С
#define USART_STK_SIZE  		    128
//�����ջ��8�ֽڶ���	
__align(8) static OS_STK USART_TASK_STK[USART_STK_SIZE];
//������
void usart_task(void *pdata);
							 
//������
//�����������ȼ�
#define MAIN_TASK_PRIO       		6 
//���������ջ��С
#define MAIN_STK_SIZE  					1200
//�����ջ��8�ֽڶ���	
__align(8) static OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);

//��������
//�����������ȼ�
#define WATCH_TASK_PRIO       	3 
//���������ջ��С
#define WATCH_STK_SIZE  		   	256
//�����ջ��8�ֽڶ���	
__align(8) static OS_STK WATCH_TASK_STK[WATCH_STK_SIZE];
//������
void watch_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////	 

//�ⲿ�ڴ����(���֧��1M�ֽ��ڴ����)
//x,y:����
//fsize:�����С
//����ֵ:0,�ɹ�;1,ʧ��.
u8 system_exsram_test(u16 x,u16 y,u8 fsize)
{  
	u32 i=0;  	  
	u16 temp=0;	   
	u16 sval=0;	//�ڵ�ַ0����������	  				   
  	LCD_ShowString(x,y,lcddev.width,y+fsize,fsize,"Ex Memory Test:   0KB"); 
	//ÿ��1K�ֽ�,д��һ������,�ܹ�д��1024������,�պ���1M�ֽ�
	for(i=0;i<1024*1024;i+=1024)
	{
		FSMC_SRAM_WriteBuffer((u8*)&temp,i,2);
		temp++;
	}
	//���ζ���֮ǰд�������,����У��		  
 	for(i=0;i<1024*1024;i+=1024) 
	{
  		FSMC_SRAM_ReadBuffer((u8*)&temp,i,2);
		if(i==0)sval=temp;
 		else if(temp<=sval)break;//�������������һ��Ҫ�ȵ�һ�ζ��������ݴ�.	   		   
		LCD_ShowxNum(x+15*(fsize/2),y,(u16)(temp-sval+1),4,fsize,0);//��ʾ�ڴ�����  
 	}
	if(i>=1024*1024)
	{
		LCD_ShowxNum(x+15*(fsize/2),y,i/1024,4,fsize,0);//��ʾ�ڴ�ֵ  		
		return 0;//�ڴ�����,�ɹ�
	}
	return 1;//ʧ��
}
//��ʾ������Ϣ
//x,y:����
//fsize:�����С
//x,y:����.err:������Ϣ
void system_error_show(u16 x,u16 y,u8*err,u8 fsize)
{
	POINT_COLOR=RED;
 	while(1)
	{
		LCD_ShowString(x,y,lcddev.width,lcddev.height,fsize,err);
		delay_ms(400);
		LCD_Fill(x,y,lcddev.width,y+fsize,BLACK);
		delay_ms(100);
	} 
}
//��������SPI FLASH(��������Դ��ɾ��),�Կ��ٸ���ϵͳ.
//x,y:����
//fsize:�����С
//x,y:����.err:������Ϣ
//����ֵ:0,û�в���;1,������
u8 system_files_erase(u16 x,u16 y,u8 fsize)
{
	u8 key;
	u8 t=0;
	POINT_COLOR=RED;
	LCD_ShowString(x,y,lcddev.width,lcddev.height,fsize,"Erase all system files?");
	while(1)
	{
		t++;
		if(t==20)LCD_ShowString(x,y+fsize,lcddev.width,lcddev.height,fsize,"KEY0:NO / KEY2:YES");
		if(t==40)
		{
			gui_fill_rectangle(x,y+fsize,lcddev.width,fsize,BLACK);//�����ʾ
			t=0;

		}
		key=KEY_Scan(0);
		if(key==KEY0_PRES)//������,�û�ȡ����
		{ 
			gui_fill_rectangle(x,y,lcddev.width,fsize*2,BLACK);//�����ʾ
			POINT_COLOR=WHITE;
			return 0;
		}
		if(key==KEY2_PRES)//Ҫ����,Ҫ��������
		{
			LCD_ShowString(x,y+fsize,lcddev.width,lcddev.height,fsize,"Erasing SPI FLASH...");
			W25QXX_Erase_Chip();
			LCD_ShowString(x,y+fsize,lcddev.width,lcddev.height,fsize,"Erasing SPI FLASH OK");
			delay_ms(600);
			return 1;
		}
		delay_ms(10);
	}
}
//�ֿ����ȷ����ʾ.
//x,y:����
//fsize:�����С 
//����ֵ:0,����Ҫ����;1,ȷ��Ҫ����
u8 system_font_update_confirm(u16 x,u16 y,u8 fsize)
{
	u8 key;
	u8 t=0;
	u8 res=0;
	POINT_COLOR=RED;
	LCD_ShowString(x,y,lcddev.width,lcddev.height,fsize,"Update font?");
	while(1)
	{
		t++;
		if(t==20)LCD_ShowString(x,y+fsize,lcddev.width,lcddev.height,fsize,"KEY0:NO / KEY2:YES");
		if(t==40)
		{
			gui_fill_rectangle(x,y+fsize,lcddev.width,fsize,BLACK);//�����ʾ
			t=0;

		}
		key=KEY_Scan(0);
		if(key==KEY0_PRES)break;//������ 
		if(key==KEY2_PRES){res=1;break;}//Ҫ���� 
		delay_ms(10);
	}

	gui_fill_rectangle(x,y,lcddev.width,fsize*2,BLACK);//�����ʾ
	POINT_COLOR=WHITE;
	return res;
}
//ϵͳ��ʼ��
void system_init(void)
{
 	u16 okoffset=162;
 	u16 ypos=0;
	u16 j=0;
	u16 temp=0;
	u8 res;
	u32 dtsize,dfsize;
	u8 *stastr=0;
	u8 *version=0; 
	u8 verbuf[12];
	u8 fsize;
	u8 icowidth;
	
 	Stm32_Clock_Init(336,8,2,7);//����ʱ��,168Mhz 
	//exeplay_app_check();		    //����Ƿ���Ҫ����APP����.  
	delay_init(168);			      //��ʱ��ʼ��  
	uart_init(84,115200);		    //��ʼ�����ڲ�����Ϊ115200
	usart3_init(42,115200);		  //��ʼ������2������Ϊ115200
	usmart_dev.init(84);		    //��ʼ��USMART
 	LED_Init();					        //��ʼ��LED 
	BEEP_Init();                //��ʼ��������
 	Lsens_Init();               //��ʼ������������
	CODETECT_Init();            //��ʼ��CO��⴫����
	LCD_Init();					        //LCD��ʼ��  
 	HC05_Init();                //������ʼ��
	RS485_Init(42,9600);		    //��ʼ��RS485
	KEY_Init();					        //������ʼ�� 
 	FSMC_SRAM_Init();			      //��ʼ���ⲿSRAM
 	AT24CXX_Init();    			    //EEPROM��ʼ��
	W25QXX_Init();				      //��ʼ��W25Q128
	Adc_Init();					        //��ʼ���ڲ��¶ȴ����� 
 	Lsens_Init();				        //��ʼ������������
	my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMCCM);		    //��ʼ��CCM�ڴ�� 
	
	tp_dev.init(); 
	gui_init();	  
	piclib_init();				//piclib��ʼ��	  
	slcd_dma_init();
	usbapp_init(); 
	exfuns_init();				//FATFS �����ڴ�
	
	version=mymalloc(SRAMIN,31);//����31���ֽ��ڴ�
REINIT://���³�ʼ��
	LCD_Clear(WHITE);			//����
	POINT_COLOR= BLACK; //WHITE;
	BACK_COLOR= WHITE; //BLACK;
	j=0;   
/////////////////////////////////////////////////////////////////////////
//��ʾ��Ȩ��Ϣ
	ypos=2;
	if(lcddev.width==240)
	{
		fsize=12;
		icowidth=18;
		okoffset=190;
	}else if(lcddev.width==320)
	{
		fsize=16;
		icowidth=24;
		okoffset=250;	
	}else if(lcddev.width==480)
	{
		fsize=24;
		icowidth=36;
		okoffset=370;
	}

	LCD_ShowString(icowidth+5*2,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "SmartHome System");
	LCD_ShowString(icowidth+5*2,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,"Copyright (C) 2019-2029");    
	app_get_version(verbuf,HARDWARE_VERSION,2);
	strcpy((char*)version,"HARDWARE:");
	strcat((char*)version,(const char*)verbuf);
	strcat((char*)version,", SOFTWARE:");
	app_get_version(verbuf,SOFTWARE_VERSION,3);
	strcat((char*)version,(const char*)verbuf);
	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,version);
	sprintf((char*)verbuf,"LCD ID:%04X",lcddev.id);		//LCD ID��ӡ��verbuf����
	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, verbuf);	//��ʾLCD ID 
//////////////////////////////////////////////////////////////////////////
//��ʼӲ������ʼ��
	WM8978_Init();			//��ֹ�����ҽ�
	app_wm8978_volset(0);	//�ر��������
	Light=1; //����
  Fan=1;	 //������
  Door=1;  //����
	
	if(HC05_Get_Role()==1)
	{
		HC05_Set_Cmd("AT+ROLE=0");
		HC05_Set_Cmd("AT+RESET");	//��λATK-HC05ģ��
		delay_ms(200);
	}

	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "CPU:STM32F407ZGT6 168Mhz");
	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "FLASH:1024KB  SRAM:192KB");	
	if(system_exsram_test(5,ypos+fsize*j,fsize))system_error_show(5,ypos+fsize*j++,"EX Memory Error!",fsize);
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,"OK");			 
	my_mem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��,��������ڴ���֮��
	Light=0; //�ص�
  Fan=0;	 //�ط���
  Door=0;  //����
	
	if(W25QXX_ReadID()!=W25Q128)//��ⲻ��W25Q128
	{	 
		system_error_show(5,ypos+fsize*j++,"Ex Flash Error!!",fsize); 
	}else temp=16*1024;//16M�ֽڴ�С
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Ex Flash:     KB");			   
	LCD_ShowxNum(5+9*(fsize/2),ypos+fsize*j,temp,5,fsize,0);//��ʾflash��С  
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");   
	//����Ƿ���Ҫ����SPI FLASH?
	res=KEY_Scan(1);//
	if(res==KEY2_PRES)
	{
		res=system_files_erase(5,ypos+fsize*j,fsize);
		if(res)goto REINIT; 
	}
    //RTC���
  LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "RTC Check...");			   
 	if(RTC_Init())system_error_show(5,ypos+fsize*(j+1),"RTC Error!",fsize);//RTC���
	else 
	{
		calendar_get_time(&calendar);//�õ���ǰʱ��
		calendar_get_date(&calendar);//�õ���ǰ����
		LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");			   
	}
	//���SPI FLASH���ļ�ϵͳ
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "FATFS Check...");//FATFS���			   
  	f_mount(fs[0],"0:",1); 		//����SD��  
  	f_mount(fs[1],"1:",1); 		//���ع���FLASH. 
 	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");			   
	//SD�����
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SD Card:     MB");//FATFS���
	temp=0;	
 	do
	{
		temp++;
 		res=exf_getfree("0:",&dtsize,&dfsize);//�õ�SD��ʣ��������������
		delay_ms(200);		   
	}while(res&&temp<5);//�������5��
 	if(res==0)//�õ���������
	{ 
		gui_phy.memdevflag|=1<<0;	//����SD����λ.
		temp=dtsize>>10;//��λת��ΪMB
		stastr="OK";
 	}else 
	{
 		temp=0;//������,��λΪ0
		stastr="ERROR";
	}
 	LCD_ShowxNum(5+8*(fsize/2),ypos+fsize*j,temp,5,fsize,0);					//��ʾSD��������С
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,stastr);	//SD��״̬			   
	//W25Q128���,����������ļ�ϵͳ,���ȴ���.
	temp=0;	
 	do
	{
		temp++;
 		res=exf_getfree("1:",&dtsize,&dfsize);//�õ�FLASHʣ��������������
		delay_ms(200);		   
	}while(res&&temp<20);//�������20��		  
	if(res==0X0D)//�ļ�ϵͳ������
	{
		LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Flash Disk Formatting...");	//��ʽ��FLASH
		res=f_mkfs("1:",1,4096);//��ʽ��FLASH,1,�̷�;1,����Ҫ������,8������Ϊ1����
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:SmartHome");				//����Flash���̵�����Ϊ��SmartHome
			LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");//��־��ʽ���ɹ�
 			res=exf_getfree("1:",&dtsize,&dfsize);//���»�ȡ����
		}
	}   
	if(res==0)//�õ�FLASH��ʣ��������������
	{
		gui_phy.memdevflag|=1<<1;	//����SPI FLASH��λ.
		LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Flash Disk:     KB");//FATFS���			   
		temp=dtsize; 	   
 	}else system_error_show(5,ypos+fsize*(j+1),"Flash Fat Error!",fsize);	//flash �ļ�ϵͳ���� 
 	LCD_ShowxNum(5+11*(fsize/2),ypos+fsize*j,temp,5,fsize,0);						//��ʾFLASH������С
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,"OK");			//FLASH��״̬	
	//U�̼��
//	usbapp_mode_set(0);												//����ΪU��ģʽ
//	temp=0; 
// 	while(usbx.hdevclass==0&&temp<1000)	//�ȴ�U�̱����,���ȴ�5��
//	{
//		usbapp_pulling();
//		if((usbx.bDeviceState&(1<<6))==0&&temp>300)break;//1.5����֮��,û�м�⵽�豸����,��ֱ������,���ٵȴ�
//		delay_ms(5); 
//		temp++;	
//	}
//	if(usbx.hdevclass==1)//��⵽��U�� 
//	{
//		fs[2]->drv=2;
//		f_mount(fs[2],"2:",1); 					//���ع���U��
// 		res=exf_getfree("2:",&dtsize,&dfsize);	//�õ�U��ʣ��������������     
//	}else res=0XFF;
//	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "U Disk:     MB");	//U��������С			   
// 	if(res==0)//�õ���������
//	{
//		gui_phy.memdevflag|=1<<2;		//����U����λ.
//		temp=dtsize>>10;//��λת��ΪMB
//		stastr="OK";
// 	}else 
//	{
// 		temp=0;//������,��λΪ0
//		stastr="ERROR";
//	}
// 	LCD_ShowxNum(5+7*(fsize/2),ypos+fsize*j,temp,5,fsize,0);					//��ʾU��������С
//	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,stastr);	//U��״̬	
	//TPAD���		 
 	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "TPAD Check...");			   
 	if(TPAD_Init(8))system_error_show(5,ypos+fsize*(j+1),"TPAD Error!",fsize);//�����������
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK"); 
	//MPU6050���   
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "MPU6050 Check...");			   
 	if(MPU_Init())system_error_show(5,ypos+fsize*(j+1),"MPU6050 Error!",fsize);//ADXL345���
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK"); 
	//24C02���
   	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "24C02 Check...");			   
 	if(AT24CXX_Check())system_error_show(5,ypos+fsize*(j+1),"24C02 Error!",fsize);//24C02���
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");  
  	//WM8978���			   
 	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "WM8978 Check...");			   
 	if(WM8978_Init())system_error_show(5,ypos+fsize*(j+1),"WM8978 Error!",fsize);//WM8978���
	else 
	{
		LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");	
		app_wm8978_volset(0);				//�ر�WM8978�������		    		   
  	}
	//�ֿ���									    
   	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Font Check...");
	res=KEY_Scan(1);//��ⰴ��			   
	if(res==KEY1_PRES)//���£�ȷ��
	{
		res=system_font_update_confirm(5,ypos+fsize*(j+1),fsize);
	}else res=0;
	if(font_init()||(res==1))//�������,������岻����/ǿ�Ƹ���,������ֿ�	
	{
		res=0;//������Ч
 		if(update_font(5,ypos+fsize*j,fsize,"0:")!=0)//��SD������
		{
			TIM3_Int_Init(100-1,8400-1);//����TIM3 ��ѯUSB,10ms�ж�һ��	
 			if(update_font(5,ypos+fsize*j,fsize,"2:")!=0)//��U�̸���
			{ 
				system_error_show(5,ypos+fsize*(j+1),"Font Error!",fsize);	//�������
			}
			TIM3->CR1&=~(1<<0);//�رն�ʱ��3
		}			
		LCD_Fill(5,ypos+fsize*j,lcddev.width,ypos+fsize*(j+1),BLACK);//����ɫ
    	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Font Check...");			   
 	} 
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");//�ֿ���OK
	//ϵͳ�ļ����
   	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Files Check...");			   
 	while(app_system_file_check("1"))//ϵͳ�ļ����
	{
		LCD_Fill(5,ypos+fsize*j,lcddev.width,ypos+fsize*(j+1),BLACK);		//����ɫ
    	LCD_ShowString(5,ypos+fsize*j,(fsize/2)*8,fsize,fsize, "Updating");	//��ʾupdating	
		app_boot_cpdmsg_set(5,ypos+fsize*j,fsize);		//���õ�����
 		temp=0;
		TIM3_Int_Init(100-1,8400-1);					//����TIM3 ��ѯUSB,10ms�ж�һ��	 
		if(app_system_file_check("0"))					//���SD��ϵͳ�ļ�������
		{ 
			if(app_system_file_check("2"))res=9;		//���Ϊ�����õ���	
			else res=2;									//���ΪU��	
		}else res=0;									//���ΪSD��
		if(res==0||res==2)								//�����˲Ÿ���
		{	
			sprintf((char*)verbuf,"%d:",res);  
			if(app_system_update(app_boot_cpdmsg,verbuf))//����?
			{
				system_error_show(5,ypos+fsize*(j+1),"SYSTEM File Error!",fsize);
			} 
		}
		TIM3->CR1&=~(1<<0);								//�رն�ʱ��3 
		LCD_Fill(5,ypos+fsize*j,lcddev.width,ypos+fsize*(j+1),BLACK);//����ɫ
    	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Files Check..."); 
		if(app_system_file_check("1"))//������һ�Σ��ټ�⣬������в�ȫ��˵��SD���ļ��Ͳ�ȫ��
		{
			system_error_show(5,ypos+fsize*(j+1),"SYSTEM File Lost!",fsize);
		}else break;	
	}
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");	
 	//��������� 
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Touch Check...");			   
	res=KEY_Scan(1);//��ⰴ��			   
	if(TP_Init()||(res==KEY0_PRES&&(tp_dev.touchtype&0X80)==0))//�и���/������KEY0�Ҳ��ǵ�����,ִ��У׼ 	
	{
		if(res==1)TP_Adjust();
		res=0;//������Ч
		goto REINIT;				//���¿�ʼ��ʼ��
	}
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");//���������OK
   	//ϵͳ��������			   
 	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Parameter Load...");			   
 	if(app_system_parameter_init())system_error_show(5,ypos+fsize*(j+1),"Parameter Load Error!",fsize);//��������
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");			   
  	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Starting...");   
	myfree(SRAMIN,version);	 
	delay_ms(1500);  
}  
//main����	  					
int main(void)
{ 	
  system_init();		//ϵͳ��ʼ�� 
 	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  						    
}
extern OS_EVENT * audiombox;	//��Ƶ������������
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 	   
	OSStatInit();		//��ʼ��ͳ������.�������ʱ1��������	
 	app_srand(OSTime);
  audiombox=OSMboxCreate((void*) 0);	//��������
	OS_ENTER_CRITICAL();//�����ٽ���(�޷����жϴ��)    
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);						   
 	OSTaskCreate(usart_task,(void *)0,(OS_STK*)&USART_TASK_STK[USART_STK_SIZE-1],USART_TASK_PRIO);						   
	OSTaskCreate(watch_task,(void *)0,(OS_STK*)&WATCH_TASK_STK[WATCH_STK_SIZE-1],WATCH_TASK_PRIO); 					   
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();	//�˳��ٽ���(���Ա��жϴ��)
} 
	
//������
void main_task(void *pdata)
{
	// ����ģ��
	u8 len=0;
	u8 rs485buf[5];
// 	u8 t=0;	
	u8 res_bt=0;
	u8 selx; 
	u16 tcnt=0;
	spb_init();			//��ʼ��SPB����
	spb_load_mui();		//����SPB������
	slcd_frame_show(0);	//��ʾ����
	
	while(1)
	{
//---------------------------------------------------------------------------------------
//  ����ģ��
//  �������ƿ���/���š�����/�صơ�������/�ط���	
		RS485_Receive_Data(rs485buf,&len);
		if(len)//���յ������ݣ�����ģ����STM32ͨ��
		{
			if(len>5)len=5;//�����5������.
				switch(rs485buf[1])
				{   
						case 1:face_yuyin_play();break;    //����:01	
					  case 2:Door=0;break;		//����:02 	
						case 3:Light=1;break;	  //����:03 			
						case 4:Light=0;break;   //�ص�:04		
						case 5:Fan=1;break;	 		//������:05	
						case 6:Fan=0;break; 		//�ط���:06		 		
						case 9:
						{
							rs485buf[0]=0;
							rs485buf[1]=0;
							audio_stop_req(&audiodev);//����ֹͣ����

						}
							break;//������		 	
						case 0xA:alarm.ringsta&=~(1<<7);break;//������		 		 
						case 0xC:OPEN_Window();break;//����		
						case 0xD:CLOSE_Window();break;//�ش�		
						case 0xE:    //˯��ģʽ
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
						case 0xF:    //ʡ��ģʽ
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
				}
				
		
		}
		selx=spb_move_chk(); 
		system_task_return=0;//���˳���־ 
		switch(selx)//������˫���¼�
		{   
		  case 0:break;
			case 1:break;
			case 2:break;
			case 3:break;
			case 4:break;
			case 5:break;
			case 6:break;
			case 7:break;
			case 8:face_manager_play();break;   //��������  
			case 9:audio_play();break;		 			//�鿴����  
			case 10:ebook_play();break;	   			//�鿴����  
			case 11:sysset_play();break;   			//ϵͳ����
			case 12:appplay_frec();break;	 			//�����Ǽ�
	    case 13:recorder_play();break; 			//�������� 
			case 14:notepad_play();break;  			//�ı�����	 ԭ���±� 
 			case 15:
			{
				res_bt=window_msg_box((lcddev.width-200)/2,(lcddev.height-80)/2,200,80,"",(u8*)APP_BLUETOOTH_CAPTION_TBL[gui_phy.language],12,0,0X03,0);
					
					if(res_bt==1)
					{
					flash_data_save[0]=0;  //��¼�����Android_ID��������ʽ��Ϊ0			
					STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
					app_muti_remind_msg((lcddev.width-250)/2,(lcddev.height-200)/2,250,106,APP_REMIND_CAPTION_TBL[gui_phy.language],delethbt_remindmsg_tbl[gui_phy.language]);	
					res_bt=0;
					}
					else
					{
					app_muti_remind_msg((lcddev.width-250)/2,(lcddev.height-200)/2,250,106,APP_REMIND_CAPTION_TBL[gui_phy.language],ndelethbt_remindmsg_tbl[gui_phy.language]);	
					res_bt=0;
					}
			}
			break;		 			//�鿴�ֻ�,�����û�
			case 16:
				app_muti_remind_msg((lcddev.width-250)/2,(lcddev.height-200)/2,250,200,APP_REMIND_CAPTION_TBL[gui_phy.language],aboutus_remindmsg_tbl[gui_phy.language]);
				break;		 			//��������  
			case 17:app_play();break;		   			//���ܼҾ�Home �鿴˵��
 			case 18:calendar_play();break;		   			//����Ԥ��  
		
		}
		
//		RS485_Receive_Data(rs485buf,&len);
//		if(len)//���յ������ݣ�����ģ����STM32ͨ��
//		{
//			if(len>5)len=5;//�����5������.
//				switch(rs485buf[1])
//				{   
//						case 1:face_yuyin_play();break;    //����:01	
//					  case 2:Door=0;break;		//����:02 	
//						case 3:Light=1;break;	  //����:03 			
//						case 4:Light=0;break;   //�ص�:04		
//						case 5:Fan=1;break;	 		//������:05	
//						case 6:Fan=0;break; 		//�ط���:06		 		
//						case 9:
//						{
//							rs485buf[0]=0;
//							rs485buf[1]=0;
//							audio_stop_req(&audiodev);//����ֹͣ����

//						}
//							break;//������		 	
//						case 0xA:alarm.ringsta&=~(1<<7);break;//������		 		 
//						case 0xC:OPEN_Window();break;//����		
//						case 0xD:CLOSE_Window();break;//�ش�		
//						case 0xE:    //˯��ģʽ
//						{
//							Door=0;
//							Light=0;
//							Fan=0;
//						};break;
//						case 0xF:    //ʡ��ģʽ
//						{
//							Door=0;
//							Light=0;
//							Fan=0;
//						};break;
//				}
//		
//		}
		

		if(selx!=0XFF)spb_load_mui();//��ʾ������
		
		
		delay_ms(1000/OS_TICKS_PER_SEC);//��ʱһ��ʱ�ӽ���
		tcnt++;
		if(tcnt==500)	//500������Ϊ1����
		{
			tcnt=0;
			spb_stabar_msg_show(0);//����״̬����Ϣ��������Ϣ
		}

		
	}
} 
extern vu8 frec_running;
//ִ�����ҪʱЧ�ԵĴ���
void usart_task(void *pdata)
{	    
	// ����ģ��
	u8 len2=0;
	u8 rs485buf2[5];
// 	u8 t2=0;
	u16 alarmtimse=0;
	pdata=pdata;
	while(1)
	{			  
		delay_ms(100);	
    RS485_Receive_Data(rs485buf2,&len2);	
		if(len2)//���յ������ݣ�����ģ����STM32ͨ��
		{
			if(len2>5)len2=5;//�����5������.
				if(rs485buf2[0]==0&&rs485buf2[1]==0X0A)
				{   
						alarm.ringsta&=~(1<<7);//������		 
        }
		
		}		
		if(alarm.ringsta&1<<7)//ִ������ɨ�躯��
		{
			calendar_alarm_ring(alarm.ringsta&0x3);//����
			alarmtimse++;

			if(alarmtimse>300)//����300����,5��������
			{
				alarm.ringsta&=~(1<<7);//�ر�����	
				alarmtimse=0;
			}

	 	}else if(alarmtimse)
		{		 
			alarmtimse=0;

		}
		if(systemset.lcdbklight==0)app_lcd_auto_bklight();	//�Զ��������
		if(frec_running==0)printf("in:%d,ex:%d,ccm:%d\r\n",my_mem_perused(0),my_mem_perused(1),my_mem_perused(2));//��ӡ�ڴ�ռ����
	}
}

vu8 system_task_return;		//����ǿ�Ʒ��ر�־.
//��������
void watch_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0; 
	// CO���
	u8 COflag=0;
	// MPU6050
	float pitch,roll,yaw; 
	short temp1,temp2,temp3;
	u8 tmpu6050=0;
	u8 dizhenflag=0;
  // ����ģ��
	u8 len=0;
	u8 rs485buf[5];
 	u8 t=0;
	//
 	u8 rerreturn=0; 
	u8 res; 
	u8 key;
 	pdata=pdata;

	
	while(1)
	{			  

//---------------------------------------------------------------------------------------
//  ����ģ��
//  �������ƿ���/���š�����/�صơ�������/�ط���	
		RS485_Receive_Data(rs485buf,&len);
		if(len)//���յ������ݣ�����ģ����STM32ͨ��
		{
			if(len>5)len=5;//�����5������.
 			//for(i=0;i<len;i++) LCD_ShowxNum(30+i*32,500,rs485buf[i],1,16,0X80);	//��ʾ����

//			if(rs485buf[0]==0)     					
//			{
				switch(rs485buf[1])
				{   
				
						case 1:face_yuyin_play();break;    //����:01
						case 2:Door=0;break;		//����:02 	
						case 3:Light=1;break;	  //����:03 			
						case 4:Light=0;break;   //�ص�:04		
						case 5:Fan=1;break;	 		//������:05	
						case 6:Fan=0;break; 		//�ط���:06		 		
						case 9:
						{
							rs485buf[0]=0;
							rs485buf[1]=0;
							audio_stop_req(&audiodev);//����ֹͣ����

						}
					
							break;//������		 	
						case 0xA:alarm.ringsta&=~(1<<7);break;//������		 		 
						case 0xC:OPEN_Window();break;//����		
						case 0xD:CLOSE_Window();break;//�ش�		
						case 0xE:    //˯��ģʽ
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
						case 0xF:    //ʡ��ģʽ
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
					
				}
				
			if(Door==0)
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//�����ʾ
					 POINT_COLOR=RED;
					 LCD_ShowString(250,0,200,16,16,"Door Close!");
					 
				 }
				 else
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//�����ʾ
					 POINT_COLOR=GREEN;
					 LCD_ShowString(250,0,200,16,16,"Door Open!");
				 }
//		}
		
		}		
		
		
		
//--------------------------------------------------------------------------------------------
//	һ����̼���
    if(COD==0)
		{
			BEEP=0; //����������
			Show_Str(350,5,80,16,"CO����",16,0);
			COflag=1;
		}
		else
		{
			if(COflag==1)
						{
							BEEP=1;   //�������
							LCD_Fill(350,5,450,25,BLACK);	//�����ʾ
							COflag=0;
						}
		}
		
		
//--------------------------------------------------------------------------------------------
//	��������		
		if(USART3_RX_STA&0X8000)			//�������յ�һ�������ˣ��ֻ���STM32ͨ��
		{
			// ����ͨ�Ų��ֲ���
			u8 i,j,temp;
//			u8 time=0;
			u8 flag=1;
			u8 reclen=0;
			u8 num;	
			LCD_Fill(100,550,400,240,WHITE);	//�����ʾ
 			reclen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			USART3_RX_BUF[reclen]=0;	 	//���������
 			USART3_RX_STA=0;
// �Ǽ�root�û�
// root�û�Android ID                        �Ǽ�root�û���ֻ���״�ʹ�û��߸�ʽ����ʹ����Ч��������Ч
			if(reclen==16)     //�Ǽ�root�û�
			{
				//  �����������ģ�幦��
				
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
        if(num==0)
				{
						flash_data_save[0]=1;  //��¼�����Android_ID��������ʼ��Ϊ1
						num=flash_data_save[0];
						for(j=0;j<9;j++)   //��ʼ��ʱ��ȫ��id����Ϊroot�û���id
						{
							for(i=0;i<16;i++)
								{ //��ʼ��ROOT
									temp=find_id[j]+i;					
									flash_data_save[temp]=USART3_RX_BUF[i];											
								}
						}				
						STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				}
        else 
				{
				  //u3_printf("%s\r\n","Error:Root user has existed!"); //����ָ��
					//LCD_ShowString(30,500,500,16,16,"Error:Root user has existed,could not change!");  //��ʾ
				}
				
			}

// ����ָ��-----Door
// 'D'+root�û�Android ID+'0'                PF10��0��LED1��������
// 'D'+root�û�Android ID+'1'                PF10��1��LED1��������
// 'D'+��ͨ�û�Android ID+'0'                PF10��0��LED1��������
// 'D'+��ͨ�û�Android ID+'1'                PF10��1��LED1��������

// ����ָ��-----Light
// 'L'+root�û�Android ID+'0'                PF7��0���ص�
// 'L'+root�û�Android ID+'1'                PF7��1������
// 'L'+��ͨ�û�Android ID+'0'                PF7��0���ص�
// 'L'+��ͨ�û�Android ID+'1'                PF7��1������

// ����ָ��-----Fan
// 'F'+root�û�Android ID+'0'                PF5��0���ط���
// 'F'+root�û�Android ID+'1'                PF5��1��������
// 'F'+��ͨ�û�Android ID+'0'                PF5��0���ط���
// 'F'+��ͨ�û�Android ID+'1'                PF5��1��������
			
			if(reclen==18)   //ָ���Ϊ18 
			 {
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
				 if(USART3_RX_BUF[0]==68)   //D:68  ����ָ��-----Door
				{
					for(j=0;j<num;j++)
					{
						for(i=0;i<16;i++)
							 {
								 if(flash_data_save[i+find_id[j]]!=USART3_RX_BUF[i+1]) break;
								 else if(i==15)
											 {
													if(USART3_RX_BUF[17]==48) 
													{
														Door=0; //����,0:48
													}
													if(USART3_RX_BUF[17]==49) 
													{
														Door=1; //����,1:49
													}
											 }							 
							 }
					 }
 
				}
				
				 if(USART3_RX_BUF[0]==76)   //L:76  ����ָ��-----Light
					{
					for(j=0;j<num;j++)
						{
							for(i=0;i<16;i++)
								 {
									 if(flash_data_save[i+find_id[j]]!=USART3_RX_BUF[i+1]) break;
									 else if(i==15)
												 {
														if(USART3_RX_BUF[17]==48) 
														{
															Light=0;    //PF7��0���ص�
														}
														if(USART3_RX_BUF[17]==49) 
														{
															Light=1;    //PF7��1������ 
														}

												 }							 
								 }
						 }
					}
				 if(USART3_RX_BUF[0]==70)   //F:70  ����ָ��-----Fan
					{
					for(j=0;j<num;j++)
						{
							for(i=0;i<16;i++)
								 {
									 if(flash_data_save[i+find_id[j]]!=USART3_RX_BUF[i+1]) break;
									 else if(i==15)
												 {
														if(USART3_RX_BUF[17]==48) 
														{
															Fan=0;    //PF5��0���ط���
														}
														if(USART3_RX_BUF[17]==49) 
														{
															Fan=1;    //PF5��1��������
														}
												 }							 
								 }
						 }
					}

			}
			 
// ��ѯ�����û�
// 'U'+root�û�Android ID                    ���������û���Ϣ
// 'U'+��ͨ�û�Android ID                    ���ظ����û���Ϣ

// ��ѯ���ż�¼
// 'J'+root�û�Android ID                    ���������û����ż�¼
// 'J'+��ͨ�û�Android ID                    ���ظ����û����ż�¼			
			if(reclen==17)   //ָ���Ϊ17 
			 {
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
				 if(USART3_RX_BUF[0]==85)   //U:85  
				{
					//LCD_ShowString(30,400,500,16,16,"USART3_RX_BUF[0]==85");
					
					for(j=0;j<num;j++)
					{
						for(i=0;i<16;i++)
							 {
								 if(flash_data_save[i+find_id[j]]!=USART3_RX_BUF[i+1]) break;
								 else if(i==15)
											 {
												 //if(j==0)  //root�û�
												 //{
													 //LCD_ShowString(30,400,500,16,16,"Root chaxun test!");
													 //u3_printf("%s\r\n",(u32*)flash_data_save); //����ָ��
													 

													 
												 //}

											 }							 
							 }
					 }
				}
			
				 if(USART3_RX_BUF[0]==74)   //J:74 
					{
					for(j=0;j<num;j++)
						{
							for(i=0;i<16;i++)
								 {
									 if(flash_data_save[i+find_id[j]]!=USART3_RX_BUF[i+1]) break;
									 else if(i==15)
												 {
													 if(j==0)  //root�û�
													 {
													 }

												 }							 
								 }
						 }
					}

			}			
			
			
// ע���û�
// root�û�Android ID+'T'+��ͨ�û�Android ID �����ͨ�û�
// root�û�Android ID+'S'+��ͨ�û�Android ID ɾ����ͨ�û�				
			if(reclen==33)   //��ӻ�ɾ���û�
			{
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
        LCD_ShowxNum(130,600,num,4,16,0);
        //�ж��Ƿ�Ϊroot�û�����
				for(i=0;i<16;i++)
				  {
					if(!((USART3_RX_BUF[i]==flash_data_save[i+find_id[0]])))
					  {
							//LCD_ShowString(30,500,500,16,16,"Error:Sorry,you are mot my manager!");
					   flag=0;
					  }
				  }
				//��root�û�	
				  if(flag)
						 { //root�û�����һ���µ�common id
							 //LCD_ShowString(30,500,500,25,25,"Message:HELLO,my manager!");
							 if(USART3_RX_BUF[16]=='T')   //����û�
							 {										
									if(num<8)
									{
										 for(i=0;i<16;i++)
												 {
													 flash_data_save[i+find_id[num]]=USART3_RX_BUF[17+i];
												 }
											num=num+1;   //��ͨ�û���Ŀ����1 
											flash_data_save[0]=num;
											num=flash_data_save[0];
											LCD_ShowxNum(130,600,num,4,16,0);
											STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);	 
											//LCD_ShowString(30,500,500,16,16,"Message:Add User Successfully!");//��ʾ�������	 										
									}
									else 
									{
										//LCD_ShowString(30,500,500,25,25,"Error:User registered full!");		
									}										
						   
							 }
							 if(USART3_RX_BUF[16]=='S')   //ɾ���û�
							 {
									u8 delete_temp=0;								 
									if(num>1)
									{
										 for(j=num;j>=1;j--)
										{
											 for(i=0;i<16;i++)
													 {
														 if(flash_data_save[i+find_id[j]]!=USART3_RX_BUF[17+i]) break;
														 if(i==15) delete_temp=j;
													 }
											if(delete_temp>0) break;		 
										};
											if(delete_temp>0)
											{
												for(i=0;i<16;i++)   //����num-1��common�û���id�Ƶ���ɾ���û���id����
														 {
															 flash_data_save[i+find_id[delete_temp]]=flash_data_save[i+find_id[num]];
														 }
												for(i=0;i<16;i++)   
														 {
															 flash_data_save[i+find_id[num]]=flash_data_save[i+find_id[0]]; //��root�û�id��������λ��
														 }		 
												num=num-1;   //��ͨ�û���Ŀ����1
												flash_data_save[0]=num;	
												num=flash_data_save[0];
												LCD_ShowxNum(130,600,num,4,16,0);
												STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);	 
												//LCD_ShowString(30,500,200,16,16,"Delete User Finished!");//��ʾ�������	 												 
												
											}
											else
											{
												//LCD_ShowString(30,500,200,16,16,"Error:No this user!");//û��Ҫɾ�����û�
											}
									}
									else 
									{
										//LCD_ShowString(30,500,210,25,25,"Error:No common user!"); //û����ͨ�û�		
									
									}										
							 }
						 }
			}

			//  ��ʾ����ʾ������		
			if(Door==0)
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//�����ʾ
					 POINT_COLOR=RED;
					 LCD_ShowString(250,0,200,16,16,"Door Close!");
					 
				 }
				 else
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//�����ʾ
					 POINT_COLOR=GREEN;
					 LCD_ShowString(250,0,200,16,16,"Door Open!");
				 }
	
		}


//--------------------------------------------------------------------------------------------
//	MPU6050������
		if(tmpu6050==5)
		{
			tmpu6050=0;
			if((audiodev.status&(1<<7))==0)		//û���ڷŸ�
			{

				if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)//��ȡDMP����
				{  
					temp1=pitch*10;
					temp2=roll*10;
					temp3=yaw*10;
					if(temp1<-70.0||temp1>70.0)  //С��-4�Ȼ��ߴ���4��
					{
						BEEP=0;   //����������
						Show_Str(350,5,50,16,"����",16,0);
						dizhenflag=1;
					}
					else if(temp2<-70.0||temp2>70.0)  //С��-4�Ȼ��ߴ���4��
					{
						BEEP=0;   //����������
						Show_Str(350,5,50,16,"����",16,0);
						dizhenflag=1;
					}
					else if(temp3<-70.0||temp3>70.0)  //С��-4�Ȼ��ߴ���4��
					{
						BEEP=0;   //����������
						Show_Str(350,5,50,16,"����",16,0);
						dizhenflag=1;
					}
					else
					{
						if(dizhenflag==1)
						{
							BEEP=1;   //�������
							LCD_Fill(350,5,450,25,BLACK);	//�����ʾ
							dizhenflag=0;
						}
					}
				}
				delay_ms(10);
			}
		}
		tmpu6050=tmpu6050+1;
		
		
//  ���Ӵ���		
		if(alarm.ringsta&(1<<7))//������ִ��
		{
			calendar_alarm_msg((lcddev.width-200)/2,(lcddev.height-160)/2);//���Ӵ���
		}
//  gif����
		if(gifdecoding)//gif���ڽ�����
		{
			key=pic_tp_scan();
			if(key==1||key==3)gifdecoding=0;//ֹͣGIF����
		}

//  TPADɨ��
		if(rerreturn)//�ٴο�ʼTPADɨ��ʱ���һ
		{
			rerreturn--;
			delay_ms(15);//������ʱ��	
 		}else if(TPAD_Scan(0))		//TPAD������һ��,�˺���ִ��,������Ҫ15ms.
		{
			rerreturn=10;			//�´α���100ms�Ժ�����ٴν���
			system_task_return=1;
			if(gifdecoding)gifdecoding=0;	//���ٲ���gif
		}
//  SD�����		
		if((t%60)==0)//900ms���Ҽ��1��
		{ 
			//SD����λ���
			if((DCMI->CR&0X01)==0)//����ͷ��������ʱ��,�ſ��Բ�ѯSD��
			{
				OS_ENTER_CRITICAL();//�����ٽ���(�޷����жϴ��) 
				res=SD_GetState();	//��ѯSD��״̬
				OS_EXIT_CRITICAL();	//�˳��ٽ���(���Ա��жϴ��) 
				if(res==0XFF)
				{
					gui_phy.memdevflag&=~(1<<0);//���SD������λ 
					OS_ENTER_CRITICAL();//�����ٽ���(�޷����жϴ��) 
					SD_Init();			//���¼��SD�� 
					OS_EXIT_CRITICAL();	//�˳��ٽ���(���Ա��жϴ��) 
				}else if((gui_phy.memdevflag&(1<<0))==0)//SD����λ?
				{
					f_mount(fs[0],"0:",1);		//���¹���sd��
					gui_phy.memdevflag|=1<<0;	//���SD����λ��		
				} 
			}
 
		}  
		delay_ms(10);
	}
}
//Ӳ��������
void HardFault_Handler(void)
{
	u32 i;
	u8 t=0;
	u32 temp;
	temp=SCB->CFSR;					//fault״̬�Ĵ���(@0XE000ED28)����:MMSR,BFSR,UFSR
 	printf("CFSR:%8X\r\n",temp);	//��ʾ����ֵ
	temp=SCB->HFSR;					//Ӳ��fault״̬�Ĵ���
 	printf("HFSR:%8X\r\n",temp);	//��ʾ����ֵ
 	temp=SCB->DFSR;					//����fault״̬�Ĵ���
 	printf("DFSR:%8X\r\n",temp);	//��ʾ����ֵ
   	temp=SCB->AFSR;					//����fault״̬�Ĵ���
 	printf("AFSR:%8X\r\n",temp);	//��ʾ����ֵ

 	while(t<5)
	{
		t++;
		for(i=0;i<0X1FFFFF;i++);
 	}

}


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
//                      一种基于蓝牙和语音控制的智能家居系统      //
//                      2019年03月13日                            //
//                      版本：1.0.0                               //
//                      Github:https://github.com/scutcyr         //
//                      邮箱：eecyryou@mail.scut.edu.cn           //
//----------------------------------------------------------------//
//
//     功能介绍
//     1、通过蓝牙HC-05实现手机遥控开门/关门，开灯/关灯，开风扇/关风扇
//     2、手机可查询开门记录以及所有用户的信息
//     3、语音模块控制开门/关门，开灯/关灯，开风扇/关风扇
//     4、人脸识别+语音模块开门
//--------------------------------------------------------------------------------------------
// 硬件         I/O口
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
// 蓝牙通信指令                                实现功能
// -------------------------------------------------------------------------------------------
// 登记root用户
// root用户Android ID                        登记root用户，只在首次使用或者格式化后使用有效，否则无效

// 注册用户
// root用户Android ID+'T'+普通用户Android ID 添加普通用户
// root用户Android ID+'S'+普通用户Android ID 删除普通用户

// 开门指令-----Door
// 'D'+root用户Android ID+'0'                PF10置0，LED1亮，关门
// 'D'+root用户Android ID+'1'                PF10置1，LED1暗，开门
// 'D'+普通用户Android ID+'0'                PF10置0，LED1亮，关门
// 'D'+普通用户Android ID+'1'                PF10置1，LED1暗，开门

// 开灯指令-----Light
// 'L'+root用户Android ID+'0'                PF4置0，关灯
// 'L'+root用户Android ID+'1'                PF4置1，开灯
// 'L'+普通用户Android ID+'0'                PF4置0，关灯
// 'L'+普通用户Android ID+'1'                PF4置1，开灯

// 风扇指令-----Fan
// 'F'+root用户Android ID+'0'                PF5置0，关风扇
// 'F'+root用户Android ID+'1'                PF5置1，开风扇
// 'F'+普通用户Android ID+'0'                PF5置0，关风扇
// 'F'+普通用户Android ID+'1'                PF5置1，开风扇

// 查询所有用户
// 'U'+root用户Android ID                    返回所有用户信息
// 'U'+普通用户Android ID                    返回个人用户信息

// 查询开门记录
// 'J'+root用户Android ID                    返回所有用户开门记录
// 'J'+普通用户Android ID                    返回个人用户开门记录

//
//--------------------------------------------------------------------------------------------
// 按键                                         实现功能
// 红色复位键                                复位           

// 按RESET复位进入开机界面(自检界面)后,迅速按以下按键:
// 按住KEY0不放:可以强制进入校准界面,对触摸屏进行校准(仅限电阻屏,对电容屏无效)
// 按住KEY1不放:可以强制进入字库更新,更新字库
// 按住KEY2不放:可以强制擦除整个SPI FLASH,方便使用SD卡快速更新系统文件
// ---提示:按住KEY0~KEY2是要一直按住,并耐心等待.直到进入相应界面/出现提示

//--------------------------------------------------------------------------------------------

// 修改时间：2019年03月13日


//--------------------------------------------------------------------------------------------
// 设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
// 否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.
#define FLASH_DATA_SAVE    0X080FF000

// 0~3:num
// 4~19:root user Android ID
// 20~35:common user Android ID 1
// ...
// 132~147:common user Android ID 8
u8 flash_data_save[148];   

// 用于存放root ID
const u8 id_root_buf[]={"7659483c763e8649"};
// 用户id索引，其中find_id[0]为root用户id
u8 find_id[9]={4,20,36,52,68,84,100,116,132};

#define COMMON_BUF_LENTH sizeof(id_common_buf)
#define ROOT_BUF_LENTH sizeof(id_root_buf)
#define COMMON_BUF_SIZE COMMON_BUF_LENTH/4+((COMMON_BUF_LENTH%4)?1:0)
#define ROOT_BUF_SIZE ROOT_BUF_LENTH/4+((ROOT_BUF_LENTH%4)?1:0)

/////////////////////////提示信息///////////////////////////////////

//删除所有蓝牙用户
u8*const delethbt_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"你已经删除所有蓝牙用户",	
"你已经删除所有蓝牙用户",
"你已经删除所有蓝牙用户", 
};

//删除所有蓝牙用户
u8*const ndelethbt_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"你没有删除蓝牙用户",	
"你没有删除蓝牙用户",	
"你没有删除蓝牙用户",	 
};

//关于我们提示信息
u8*const aboutus_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"一种基于蓝牙和语音控制的智能家居系统\r\
手机通过蓝牙连接本系统可以实现控制\r\
通过语音控制+人脸识别实现安全的家居控制\r\
作品属性:智能家居",	
"一种基于蓝牙和语音控制的智能家居系统\r\
手机通过蓝牙连接本系统可以实现控制\r\
通过语音控制+人脸识别实现安全的家居控制\r\
作品属性:智能家居",
"一种基于蓝牙和语音控制的智能家居系统\r\
手机通过蓝牙连接本系统可以实现控制\r\
通过语音控制+人脸识别实现安全的家居控制\r\
作品属性:智能家居",	 
};

//地震提示信息
u8*const dizhen_remindmsg_tbl[GUI_LANGUAGE_NUM]=
{
"危险警报：\r\
地震啦！\r\
地震啦！！\r\
地震啦！！！\r\
地震啦！！！\r\
地震啦！！！",	
"危险警报：\r\
地震啦！\r\
地震啦！！\r\
地震啦！！！\r\
地震啦！！！\r\
地震啦！！！",
"危险警报：\r\
地震啦！\r\
地震啦！！\r\
地震啦！！！\r\
地震啦！！！\r\
地震啦！！！", 
};

 
/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      		10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈，8字节对齐	
__align(8) static OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//串口任务
//设置任务优先级
#define USART_TASK_PRIO       	7 
//设置任务堆栈大小
#define USART_STK_SIZE  		    128
//任务堆栈，8字节对齐	
__align(8) static OS_STK USART_TASK_STK[USART_STK_SIZE];
//任务函数
void usart_task(void *pdata);
							 
//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       		6 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					1200
//任务堆栈，8字节对齐	
__align(8) static OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);

//监视任务
//设置任务优先级
#define WATCH_TASK_PRIO       	3 
//设置任务堆栈大小
#define WATCH_STK_SIZE  		   	256
//任务堆栈，8字节对齐	
__align(8) static OS_STK WATCH_TASK_STK[WATCH_STK_SIZE];
//任务函数
void watch_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////	 

//外部内存测试(最大支持1M字节内存测试)
//x,y:坐标
//fsize:字体大小
//返回值:0,成功;1,失败.
u8 system_exsram_test(u16 x,u16 y,u8 fsize)
{  
	u32 i=0;  	  
	u16 temp=0;	   
	u16 sval=0;	//在地址0读到的数据	  				   
  	LCD_ShowString(x,y,lcddev.width,y+fsize,fsize,"Ex Memory Test:   0KB"); 
	//每隔1K字节,写入一个数据,总共写入1024个数据,刚好是1M字节
	for(i=0;i<1024*1024;i+=1024)
	{
		FSMC_SRAM_WriteBuffer((u8*)&temp,i,2);
		temp++;
	}
	//依次读出之前写入的数据,进行校验		  
 	for(i=0;i<1024*1024;i+=1024) 
	{
  		FSMC_SRAM_ReadBuffer((u8*)&temp,i,2);
		if(i==0)sval=temp;
 		else if(temp<=sval)break;//后面读出的数据一定要比第一次读到的数据大.	   		   
		LCD_ShowxNum(x+15*(fsize/2),y,(u16)(temp-sval+1),4,fsize,0);//显示内存容量  
 	}
	if(i>=1024*1024)
	{
		LCD_ShowxNum(x+15*(fsize/2),y,i/1024,4,fsize,0);//显示内存值  		
		return 0;//内存正常,成功
	}
	return 1;//失败
}
//显示错误信息
//x,y:坐标
//fsize:字体大小
//x,y:坐标.err:错误信息
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
//擦除整个SPI FLASH(即所有资源都删除),以快速更新系统.
//x,y:坐标
//fsize:字体大小
//x,y:坐标.err:错误信息
//返回值:0,没有擦除;1,擦除了
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
			gui_fill_rectangle(x,y+fsize,lcddev.width,fsize,BLACK);//清除显示
			t=0;

		}
		key=KEY_Scan(0);
		if(key==KEY0_PRES)//不擦除,用户取消了
		{ 
			gui_fill_rectangle(x,y,lcddev.width,fsize*2,BLACK);//清除显示
			POINT_COLOR=WHITE;
			return 0;
		}
		if(key==KEY2_PRES)//要擦除,要重新来过
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
//字库更新确认提示.
//x,y:坐标
//fsize:字体大小 
//返回值:0,不需要更新;1,确认要更新
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
			gui_fill_rectangle(x,y+fsize,lcddev.width,fsize,BLACK);//清除显示
			t=0;

		}
		key=KEY_Scan(0);
		if(key==KEY0_PRES)break;//不更新 
		if(key==KEY2_PRES){res=1;break;}//要更新 
		delay_ms(10);
	}

	gui_fill_rectangle(x,y,lcddev.width,fsize*2,BLACK);//清除显示
	POINT_COLOR=WHITE;
	return res;
}
//系统初始化
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
	
 	Stm32_Clock_Init(336,8,2,7);//设置时钟,168Mhz 
	//exeplay_app_check();		    //检测是否需要运行APP代码.  
	delay_init(168);			      //延时初始化  
	uart_init(84,115200);		    //初始化串口波特率为115200
	usart3_init(42,115200);		  //初始化串口2波特率为115200
	usmart_dev.init(84);		    //初始化USMART
 	LED_Init();					        //初始化LED 
	BEEP_Init();                //初始化蜂鸣器
 	Lsens_Init();               //初始化光敏传感器
	CODETECT_Init();            //初始化CO监测传感器
	LCD_Init();					        //LCD初始化  
 	HC05_Init();                //蓝牙初始化
	RS485_Init(42,9600);		    //初始化RS485
	KEY_Init();					        //按键初始化 
 	FSMC_SRAM_Init();			      //初始化外部SRAM
 	AT24CXX_Init();    			    //EEPROM初始化
	W25QXX_Init();				      //初始化W25Q128
	Adc_Init();					        //初始化内部温度传感器 
 	Lsens_Init();				        //初始化光敏传感器
	my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMCCM);		    //初始化CCM内存池 
	
	tp_dev.init(); 
	gui_init();	  
	piclib_init();				//piclib初始化	  
	slcd_dma_init();
	usbapp_init(); 
	exfuns_init();				//FATFS 申请内存
	
	version=mymalloc(SRAMIN,31);//申请31个字节内存
REINIT://重新初始化
	LCD_Clear(WHITE);			//白屏
	POINT_COLOR= BLACK; //WHITE;
	BACK_COLOR= WHITE; //BLACK;
	j=0;   
/////////////////////////////////////////////////////////////////////////
//显示版权信息
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
	sprintf((char*)verbuf,"LCD ID:%04X",lcddev.id);		//LCD ID打印到verbuf里面
	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, verbuf);	//显示LCD ID 
//////////////////////////////////////////////////////////////////////////
//开始硬件检测初始化
	WM8978_Init();			//防止喇叭乱叫
	app_wm8978_volset(0);	//关闭音量输出
	Light=1; //开灯
  Fan=1;	 //开风扇
  Door=1;  //开门
	
	if(HC05_Get_Role()==1)
	{
		HC05_Set_Cmd("AT+ROLE=0");
		HC05_Set_Cmd("AT+RESET");	//复位ATK-HC05模块
		delay_ms(200);
	}

	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "CPU:STM32F407ZGT6 168Mhz");
	LCD_ShowString(5,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "FLASH:1024KB  SRAM:192KB");	
	if(system_exsram_test(5,ypos+fsize*j,fsize))system_error_show(5,ypos+fsize*j++,"EX Memory Error!",fsize);
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,"OK");			 
	my_mem_init(SRAMEX);		//初始化外部内存池,必须放在内存检测之后
	Light=0; //关灯
  Fan=0;	 //关风扇
  Door=0;  //关门
	
	if(W25QXX_ReadID()!=W25Q128)//检测不到W25Q128
	{	 
		system_error_show(5,ypos+fsize*j++,"Ex Flash Error!!",fsize); 
	}else temp=16*1024;//16M字节大小
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Ex Flash:     KB");			   
	LCD_ShowxNum(5+9*(fsize/2),ypos+fsize*j,temp,5,fsize,0);//显示flash大小  
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");   
	//检测是否需要擦除SPI FLASH?
	res=KEY_Scan(1);//
	if(res==KEY2_PRES)
	{
		res=system_files_erase(5,ypos+fsize*j,fsize);
		if(res)goto REINIT; 
	}
    //RTC检测
  LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "RTC Check...");			   
 	if(RTC_Init())system_error_show(5,ypos+fsize*(j+1),"RTC Error!",fsize);//RTC检测
	else 
	{
		calendar_get_time(&calendar);//得到当前时间
		calendar_get_date(&calendar);//得到当前日期
		LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");			   
	}
	//检查SPI FLASH的文件系统
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "FATFS Check...");//FATFS检测			   
  	f_mount(fs[0],"0:",1); 		//挂载SD卡  
  	f_mount(fs[1],"1:",1); 		//挂载挂载FLASH. 
 	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");			   
	//SD卡检测
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SD Card:     MB");//FATFS检测
	temp=0;	
 	do
	{
		temp++;
 		res=exf_getfree("0:",&dtsize,&dfsize);//得到SD卡剩余容量和总容量
		delay_ms(200);		   
	}while(res&&temp<5);//连续检测5次
 	if(res==0)//得到容量正常
	{ 
		gui_phy.memdevflag|=1<<0;	//设置SD卡在位.
		temp=dtsize>>10;//单位转换为MB
		stastr="OK";
 	}else 
	{
 		temp=0;//出错了,单位为0
		stastr="ERROR";
	}
 	LCD_ShowxNum(5+8*(fsize/2),ypos+fsize*j,temp,5,fsize,0);					//显示SD卡容量大小
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,stastr);	//SD卡状态			   
	//W25Q128检测,如果不存在文件系统,则先创建.
	temp=0;	
 	do
	{
		temp++;
 		res=exf_getfree("1:",&dtsize,&dfsize);//得到FLASH剩余容量和总容量
		delay_ms(200);		   
	}while(res&&temp<20);//连续检测20次		  
	if(res==0X0D)//文件系统不存在
	{
		LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Flash Disk Formatting...");	//格式化FLASH
		res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:SmartHome");				//设置Flash磁盘的名字为：SmartHome
			LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");//标志格式化成功
 			res=exf_getfree("1:",&dtsize,&dfsize);//重新获取容量
		}
	}   
	if(res==0)//得到FLASH卡剩余容量和总容量
	{
		gui_phy.memdevflag|=1<<1;	//设置SPI FLASH在位.
		LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Flash Disk:     KB");//FATFS检测			   
		temp=dtsize; 	   
 	}else system_error_show(5,ypos+fsize*(j+1),"Flash Fat Error!",fsize);	//flash 文件系统错误 
 	LCD_ShowxNum(5+11*(fsize/2),ypos+fsize*j,temp,5,fsize,0);						//显示FLASH容量大小
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,"OK");			//FLASH卡状态	
	//U盘检测
//	usbapp_mode_set(0);												//设置为U盘模式
//	temp=0; 
// 	while(usbx.hdevclass==0&&temp<1000)	//等待U盘被检测,最多等待5秒
//	{
//		usbapp_pulling();
//		if((usbx.bDeviceState&(1<<6))==0&&temp>300)break;//1.5秒钟之内,没有检测到设备插入,则直接跳过,不再等待
//		delay_ms(5); 
//		temp++;	
//	}
//	if(usbx.hdevclass==1)//检测到了U盘 
//	{
//		fs[2]->drv=2;
//		f_mount(fs[2],"2:",1); 					//挂载挂载U盘
// 		res=exf_getfree("2:",&dtsize,&dfsize);	//得到U盘剩余容量和总容量     
//	}else res=0XFF;
//	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "U Disk:     MB");	//U盘容量大小			   
// 	if(res==0)//得到容量正常
//	{
//		gui_phy.memdevflag|=1<<2;		//设置U盘在位.
//		temp=dtsize>>10;//单位转换为MB
//		stastr="OK";
// 	}else 
//	{
// 		temp=0;//出错了,单位为0
//		stastr="ERROR";
//	}
// 	LCD_ShowxNum(5+7*(fsize/2),ypos+fsize*j,temp,5,fsize,0);					//显示U盘容量大小
//	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize,stastr);	//U盘状态	
	//TPAD检测		 
 	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "TPAD Check...");			   
 	if(TPAD_Init(8))system_error_show(5,ypos+fsize*(j+1),"TPAD Error!",fsize);//触摸按键检测
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK"); 
	//MPU6050检测   
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "MPU6050 Check...");			   
 	if(MPU_Init())system_error_show(5,ypos+fsize*(j+1),"MPU6050 Error!",fsize);//ADXL345检测
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK"); 
	//24C02检测
   	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "24C02 Check...");			   
 	if(AT24CXX_Check())system_error_show(5,ypos+fsize*(j+1),"24C02 Error!",fsize);//24C02检测
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");  
  	//WM8978检测			   
 	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "WM8978 Check...");			   
 	if(WM8978_Init())system_error_show(5,ypos+fsize*(j+1),"WM8978 Error!",fsize);//WM8978检测
	else 
	{
		LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");	
		app_wm8978_volset(0);				//关闭WM8978音量输出		    		   
  	}
	//字库检测									    
   	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Font Check...");
	res=KEY_Scan(1);//检测按键			   
	if(res==KEY1_PRES)//更新？确认
	{
		res=system_font_update_confirm(5,ypos+fsize*(j+1),fsize);
	}else res=0;
	if(font_init()||(res==1))//检测字体,如果字体不存在/强制更新,则更新字库	
	{
		res=0;//按键无效
 		if(update_font(5,ypos+fsize*j,fsize,"0:")!=0)//从SD卡更新
		{
			TIM3_Int_Init(100-1,8400-1);//启动TIM3 轮询USB,10ms中断一次	
 			if(update_font(5,ypos+fsize*j,fsize,"2:")!=0)//从U盘更新
			{ 
				system_error_show(5,ypos+fsize*(j+1),"Font Error!",fsize);	//字体错误
			}
			TIM3->CR1&=~(1<<0);//关闭定时器3
		}			
		LCD_Fill(5,ypos+fsize*j,lcddev.width,ypos+fsize*(j+1),BLACK);//填充底色
    	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Font Check...");			   
 	} 
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");//字库检测OK
	//系统文件检测
   	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Files Check...");			   
 	while(app_system_file_check("1"))//系统文件检测
	{
		LCD_Fill(5,ypos+fsize*j,lcddev.width,ypos+fsize*(j+1),BLACK);		//填充底色
    	LCD_ShowString(5,ypos+fsize*j,(fsize/2)*8,fsize,fsize, "Updating");	//显示updating	
		app_boot_cpdmsg_set(5,ypos+fsize*j,fsize);		//设置到坐标
 		temp=0;
		TIM3_Int_Init(100-1,8400-1);					//启动TIM3 轮询USB,10ms中断一次	 
		if(app_system_file_check("0"))					//检查SD卡系统文件完整性
		{ 
			if(app_system_file_check("2"))res=9;		//标记为不可用的盘	
			else res=2;									//标记为U盘	
		}else res=0;									//标记为SD卡
		if(res==0||res==2)								//完整了才更新
		{	
			sprintf((char*)verbuf,"%d:",res);  
			if(app_system_update(app_boot_cpdmsg,verbuf))//更新?
			{
				system_error_show(5,ypos+fsize*(j+1),"SYSTEM File Error!",fsize);
			} 
		}
		TIM3->CR1&=~(1<<0);								//关闭定时器3 
		LCD_Fill(5,ypos+fsize*j,lcddev.width,ypos+fsize*(j+1),BLACK);//填充底色
    	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Files Check..."); 
		if(app_system_file_check("1"))//更新了一次，再检测，如果还有不全，说明SD卡文件就不全！
		{
			system_error_show(5,ypos+fsize*(j+1),"SYSTEM File Lost!",fsize);
		}else break;	
	}
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");	
 	//触摸屏检测 
	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "Touch Check...");			   
	res=KEY_Scan(1);//检测按键			   
	if(TP_Init()||(res==KEY0_PRES&&(tp_dev.touchtype&0X80)==0))//有更新/按下了KEY0且不是电容屏,执行校准 	
	{
		if(res==1)TP_Adjust();
		res=0;//按键无效
		goto REINIT;				//重新开始初始化
	}
	LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");//触摸屏检测OK
   	//系统参数加载			   
 	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Parameter Load...");			   
 	if(app_system_parameter_init())system_error_show(5,ypos+fsize*(j+1),"Parameter Load Error!",fsize);//参数加载
	else LCD_ShowString(5+okoffset,ypos+fsize*j++,lcddev.width,lcddev.height,fsize, "OK");			   
  	LCD_ShowString(5,ypos+fsize*j,lcddev.width,lcddev.height,fsize, "SYSTEM Starting...");   
	myfree(SRAMIN,version);	 
	delay_ms(1500);  
}  
//main函数	  					
int main(void)
{ 	
  system_init();		//系统初始化 
 	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  						    
}
extern OS_EVENT * audiombox;	//音频播放任务邮箱
//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	pdata = pdata; 	   
	OSStatInit();		//初始化统计任务.这里会延时1秒钟左右	
 	app_srand(OSTime);
  audiombox=OSMboxCreate((void*) 0);	//创建邮箱
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)    
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);						   
 	OSTaskCreate(usart_task,(void *)0,(OS_STK*)&USART_TASK_STK[USART_STK_SIZE-1],USART_TASK_PRIO);						   
	OSTaskCreate(watch_task,(void *)0,(OS_STK*)&WATCH_TASK_STK[WATCH_STK_SIZE-1],WATCH_TASK_PRIO); 					   
	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断)
} 
	
//主任务
void main_task(void *pdata)
{
	// 语音模块
	u8 len=0;
	u8 rs485buf[5];
// 	u8 t=0;	
	u8 res_bt=0;
	u8 selx; 
	u16 tcnt=0;
	spb_init();			//初始化SPB界面
	spb_load_mui();		//加载SPB主界面
	slcd_frame_show(0);	//显示界面
	
	while(1)
	{
//---------------------------------------------------------------------------------------
//  语音模块
//  语音控制开门/关门、开灯/关灯、开风扇/关风扇	
		RS485_Receive_Data(rs485buf,&len);
		if(len)//接收到有数据，语音模块与STM32通信
		{
			if(len>5)len=5;//最大是5个数据.
				switch(rs485buf[1])
				{   
						case 1:face_yuyin_play();break;    //开门:01	
					  case 2:Door=0;break;		//关门:02 	
						case 3:Light=1;break;	  //开灯:03 			
						case 4:Light=0;break;   //关灯:04		
						case 5:Fan=1;break;	 		//开风扇:05	
						case 6:Fan=0;break; 		//关风扇:06		 		
						case 9:
						{
							rs485buf[0]=0;
							rs485buf[1]=0;
							audio_stop_req(&audiodev);//请求停止播放

						}
							break;//关音乐		 	
						case 0xA:alarm.ringsta&=~(1<<7);break;//关闹钟		 		 
						case 0xC:OPEN_Window();break;//开窗		
						case 0xD:CLOSE_Window();break;//关窗		
						case 0xE:    //睡眠模式
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
						case 0xF:    //省电模式
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
				}
				
		
		}
		selx=spb_move_chk(); 
		system_task_return=0;//清退出标志 
		switch(selx)//发生了双击事件
		{   
		  case 0:break;
			case 1:break;
			case 2:break;
			case 3:break;
			case 4:break;
			case 5:break;
			case 6:break;
			case 7:break;
			case 8:face_manager_play();break;   //人脸管理  
			case 9:audio_play();break;		 			//查看语音  
			case 10:ebook_play();break;	   			//查看留言  
			case 11:sysset_play();break;   			//系统管理
			case 12:appplay_frec();break;	 			//人脸登记
	    case 13:recorder_play();break; 			//语音留言 
			case 14:notepad_play();break;  			//文本留言	 原记事本 
 			case 15:
			{
				res_bt=window_msg_box((lcddev.width-200)/2,(lcddev.height-80)/2,200,80,"",(u8*)APP_BLUETOOTH_CAPTION_TBL[gui_phy.language],12,0,0X03,0);
					
					if(res_bt==1)
					{
					flash_data_save[0]=0;  //记录保存的Android_ID个数，格式化为0			
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
			break;		 			//查看手机,蓝牙用户
			case 16:
				app_muti_remind_msg((lcddev.width-250)/2,(lcddev.height-200)/2,250,200,APP_REMIND_CAPTION_TBL[gui_phy.language],aboutus_remindmsg_tbl[gui_phy.language]);
				break;		 			//关于我们  
			case 17:app_play();break;		   			//智能家居Home 查看说明
 			case 18:calendar_play();break;		   			//天气预报  
		
		}
		
//		RS485_Receive_Data(rs485buf,&len);
//		if(len)//接收到有数据，语音模块与STM32通信
//		{
//			if(len>5)len=5;//最大是5个数据.
//				switch(rs485buf[1])
//				{   
//						case 1:face_yuyin_play();break;    //开门:01	
//					  case 2:Door=0;break;		//关门:02 	
//						case 3:Light=1;break;	  //开灯:03 			
//						case 4:Light=0;break;   //关灯:04		
//						case 5:Fan=1;break;	 		//开风扇:05	
//						case 6:Fan=0;break; 		//关风扇:06		 		
//						case 9:
//						{
//							rs485buf[0]=0;
//							rs485buf[1]=0;
//							audio_stop_req(&audiodev);//请求停止播放

//						}
//							break;//关音乐		 	
//						case 0xA:alarm.ringsta&=~(1<<7);break;//关闹钟		 		 
//						case 0xC:OPEN_Window();break;//开窗		
//						case 0xD:CLOSE_Window();break;//关窗		
//						case 0xE:    //睡眠模式
//						{
//							Door=0;
//							Light=0;
//							Fan=0;
//						};break;
//						case 0xF:    //省电模式
//						{
//							Door=0;
//							Light=0;
//							Fan=0;
//						};break;
//				}
//		
//		}
		

		if(selx!=0XFF)spb_load_mui();//显示主界面
		
		
		delay_ms(1000/OS_TICKS_PER_SEC);//延时一个时钟节拍
		tcnt++;
		if(tcnt==500)	//500个节拍为1秒钟
		{
			tcnt=0;
			spb_stabar_msg_show(0);//更新状态栏信息，顶部信息
		}

		
	}
} 
extern vu8 frec_running;
//执行最不需要时效性的代码
void usart_task(void *pdata)
{	    
	// 语音模块
	u8 len2=0;
	u8 rs485buf2[5];
// 	u8 t2=0;
	u16 alarmtimse=0;
	pdata=pdata;
	while(1)
	{			  
		delay_ms(100);	
    RS485_Receive_Data(rs485buf2,&len2);	
		if(len2)//接收到有数据，语音模块与STM32通信
		{
			if(len2>5)len2=5;//最大是5个数据.
				if(rs485buf2[0]==0&&rs485buf2[1]==0X0A)
				{   
						alarm.ringsta&=~(1<<7);//关闹钟		 
        }
		
		}		
		if(alarm.ringsta&1<<7)//执行闹钟扫描函数
		{
			calendar_alarm_ring(alarm.ringsta&0x3);//闹铃
			alarmtimse++;

			if(alarmtimse>300)//超过300次了,5分钟以上
			{
				alarm.ringsta&=~(1<<7);//关闭闹铃	
				alarmtimse=0;
			}

	 	}else if(alarmtimse)
		{		 
			alarmtimse=0;

		}
		if(systemset.lcdbklight==0)app_lcd_auto_bklight();	//自动背光控制
		if(frec_running==0)printf("in:%d,ex:%d,ccm:%d\r\n",my_mem_perused(0),my_mem_perused(1),my_mem_perused(2));//打印内存占用率
	}
}

vu8 system_task_return;		//任务强制返回标志.
//监视任务
void watch_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0; 
	// CO监测
	u8 COflag=0;
	// MPU6050
	float pitch,roll,yaw; 
	short temp1,temp2,temp3;
	u8 tmpu6050=0;
	u8 dizhenflag=0;
  // 语音模块
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
//  语音模块
//  语音控制开门/关门、开灯/关灯、开风扇/关风扇	
		RS485_Receive_Data(rs485buf,&len);
		if(len)//接收到有数据，语音模块与STM32通信
		{
			if(len>5)len=5;//最大是5个数据.
 			//for(i=0;i<len;i++) LCD_ShowxNum(30+i*32,500,rs485buf[i],1,16,0X80);	//显示数据

//			if(rs485buf[0]==0)     					
//			{
				switch(rs485buf[1])
				{   
				
						case 1:face_yuyin_play();break;    //开门:01
						case 2:Door=0;break;		//关门:02 	
						case 3:Light=1;break;	  //开灯:03 			
						case 4:Light=0;break;   //关灯:04		
						case 5:Fan=1;break;	 		//开风扇:05	
						case 6:Fan=0;break; 		//关风扇:06		 		
						case 9:
						{
							rs485buf[0]=0;
							rs485buf[1]=0;
							audio_stop_req(&audiodev);//请求停止播放

						}
					
							break;//关音乐		 	
						case 0xA:alarm.ringsta&=~(1<<7);break;//关闹钟		 		 
						case 0xC:OPEN_Window();break;//开窗		
						case 0xD:CLOSE_Window();break;//关窗		
						case 0xE:    //睡眠模式
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
						case 0xF:    //省电模式
						{
							Door=0;
							Light=0;
							Fan=0;
						};break;
					
				}
				
			if(Door==0)
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//清除显示
					 POINT_COLOR=RED;
					 LCD_ShowString(250,0,200,16,16,"Door Close!");
					 
				 }
				 else
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//清除显示
					 POINT_COLOR=GREEN;
					 LCD_ShowString(250,0,200,16,16,"Door Open!");
				 }
//		}
		
		}		
		
		
		
//--------------------------------------------------------------------------------------------
//	一氧化碳监测
    if(COD==0)
		{
			BEEP=0; //蜂鸣器报警
			Show_Str(350,5,80,16,"CO超标",16,0);
			COflag=1;
		}
		else
		{
			if(COflag==1)
						{
							BEEP=1;   //解除报警
							LCD_Fill(350,5,450,25,BLACK);	//清除显示
							COflag=0;
						}
		}
		
		
//--------------------------------------------------------------------------------------------
//	蓝牙控制		
		if(USART3_RX_STA&0X8000)			//蓝牙接收到一次数据了，手机与STM32通信
		{
			// 蓝牙通信部分参数
			u8 i,j,temp;
//			u8 time=0;
			u8 flag=1;
			u8 reclen=0;
			u8 num;	
			LCD_Fill(100,550,400,240,WHITE);	//清除显示
 			reclen=USART3_RX_STA&0X7FFF;	//得到数据长度
			USART3_RX_BUF[reclen]=0;	 	//加入结束符
 			USART3_RX_STA=0;
// 登记root用户
// root用户Android ID                        登记root用户，只在首次使用或者格式化后使用有效，否则无效
			if(reclen==16)     //登记root用户
			{
				//  添加增加人脸模板功能
				
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
        if(num==0)
				{
						flash_data_save[0]=1;  //记录保存的Android_ID个数，初始化为1
						num=flash_data_save[0];
						for(j=0;j<9;j++)   //初始化时，全部id设置为root用户的id
						{
							for(i=0;i<16;i++)
								{ //初始化ROOT
									temp=find_id[j]+i;					
									flash_data_save[temp]=USART3_RX_BUF[i];											
								}
						}				
						STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				}
        else 
				{
				  //u3_printf("%s\r\n","Error:Root user has existed!"); //发送指令
					//LCD_ShowString(30,500,500,16,16,"Error:Root user has existed,could not change!");  //提示
				}
				
			}

// 开门指令-----Door
// 'D'+root用户Android ID+'0'                PF10置0，LED1亮，关门
// 'D'+root用户Android ID+'1'                PF10置1，LED1暗，开门
// 'D'+普通用户Android ID+'0'                PF10置0，LED1亮，关门
// 'D'+普通用户Android ID+'1'                PF10置1，LED1暗，开门

// 开灯指令-----Light
// 'L'+root用户Android ID+'0'                PF7置0，关灯
// 'L'+root用户Android ID+'1'                PF7置1，开灯
// 'L'+普通用户Android ID+'0'                PF7置0，关灯
// 'L'+普通用户Android ID+'1'                PF7置1，开灯

// 风扇指令-----Fan
// 'F'+root用户Android ID+'0'                PF5置0，关风扇
// 'F'+root用户Android ID+'1'                PF5置1，开风扇
// 'F'+普通用户Android ID+'0'                PF5置0，关风扇
// 'F'+普通用户Android ID+'1'                PF5置1，开风扇
			
			if(reclen==18)   //指令长度为18 
			 {
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
				 if(USART3_RX_BUF[0]==68)   //D:68  开门指令-----Door
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
														Door=0; //关门,0:48
													}
													if(USART3_RX_BUF[17]==49) 
													{
														Door=1; //开门,1:49
													}
											 }							 
							 }
					 }
 
				}
				
				 if(USART3_RX_BUF[0]==76)   //L:76  开灯指令-----Light
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
															Light=0;    //PF7置0，关灯
														}
														if(USART3_RX_BUF[17]==49) 
														{
															Light=1;    //PF7置1，开灯 
														}

												 }							 
								 }
						 }
					}
				 if(USART3_RX_BUF[0]==70)   //F:70  风扇指令-----Fan
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
															Fan=0;    //PF5置0，关风扇
														}
														if(USART3_RX_BUF[17]==49) 
														{
															Fan=1;    //PF5置1，开风扇
														}
												 }							 
								 }
						 }
					}

			}
			 
// 查询所有用户
// 'U'+root用户Android ID                    返回所有用户信息
// 'U'+普通用户Android ID                    返回个人用户信息

// 查询开门记录
// 'J'+root用户Android ID                    返回所有用户开门记录
// 'J'+普通用户Android ID                    返回个人用户开门记录			
			if(reclen==17)   //指令长度为17 
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
												 //if(j==0)  //root用户
												 //{
													 //LCD_ShowString(30,400,500,16,16,"Root chaxun test!");
													 //u3_printf("%s\r\n",(u32*)flash_data_save); //发送指令
													 

													 
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
													 if(j==0)  //root用户
													 {
													 }

												 }							 
								 }
						 }
					}

			}			
			
			
// 注册用户
// root用户Android ID+'T'+普通用户Android ID 添加普通用户
// root用户Android ID+'S'+普通用户Android ID 删除普通用户				
			if(reclen==33)   //添加或删除用户
			{
				STMFLASH_Read(FLASH_DATA_SAVE,(u32*)flash_data_save,37);
				num=flash_data_save[0];
        LCD_ShowxNum(130,600,num,4,16,0);
        //判断是否为root用户操作
				for(i=0;i<16;i++)
				  {
					if(!((USART3_RX_BUF[i]==flash_data_save[i+find_id[0]])))
					  {
							//LCD_ShowString(30,500,500,16,16,"Error:Sorry,you are mot my manager!");
					   flag=0;
					  }
				  }
				//是root用户	
				  if(flag)
						 { //root用户存入一个新的common id
							 //LCD_ShowString(30,500,500,25,25,"Message:HELLO,my manager!");
							 if(USART3_RX_BUF[16]=='T')   //添加用户
							 {										
									if(num<8)
									{
										 for(i=0;i<16;i++)
												 {
													 flash_data_save[i+find_id[num]]=USART3_RX_BUF[17+i];
												 }
											num=num+1;   //普通用户数目增加1 
											flash_data_save[0]=num;
											num=flash_data_save[0];
											LCD_ShowxNum(130,600,num,4,16,0);
											STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);	 
											//LCD_ShowString(30,500,500,16,16,"Message:Add User Successfully!");//提示传送完成	 										
									}
									else 
									{
										//LCD_ShowString(30,500,500,25,25,"Error:User registered full!");		
									}										
						   
							 }
							 if(USART3_RX_BUF[16]=='S')   //删除用户
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
												for(i=0;i<16;i++)   //将第num-1个common用户的id移到被删除用户的id那里
														 {
															 flash_data_save[i+find_id[delete_temp]]=flash_data_save[i+find_id[num]];
														 }
												for(i=0;i<16;i++)   
														 {
															 flash_data_save[i+find_id[num]]=flash_data_save[i+find_id[0]]; //用root用户id覆盖最后的位置
														 }		 
												num=num-1;   //普通用户数目减少1
												flash_data_save[0]=num;	
												num=flash_data_save[0];
												LCD_ShowxNum(130,600,num,4,16,0);
												STMFLASH_Write(FLASH_DATA_SAVE,(u32*)flash_data_save,37);	 
												//LCD_ShowString(30,500,200,16,16,"Delete User Finished!");//提示传送完成	 												 
												
											}
											else
											{
												//LCD_ShowString(30,500,200,16,16,"Error:No this user!");//没有要删除的用户
											}
									}
									else 
									{
										//LCD_ShowString(30,500,210,25,25,"Error:No common user!"); //没有普通用户		
									
									}										
							 }
						 }
			}

			//  显示屏提示开关门		
			if(Door==0)
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//清除显示
					 POINT_COLOR=RED;
					 LCD_ShowString(250,0,200,16,16,"Door Close!");
					 
				 }
				 else
				 {
					 LCD_Fill(250,5,350,25,BLACK);	//清除显示
					 POINT_COLOR=GREEN;
					 LCD_ShowString(250,0,200,16,16,"Door Open!");
				 }
	
		}


//--------------------------------------------------------------------------------------------
//	MPU6050地震监测
		if(tmpu6050==5)
		{
			tmpu6050=0;
			if((audiodev.status&(1<<7))==0)		//没有在放歌
			{

				if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)//读取DMP数据
				{  
					temp1=pitch*10;
					temp2=roll*10;
					temp3=yaw*10;
					if(temp1<-70.0||temp1>70.0)  //小于-4度或者大于4度
					{
						BEEP=0;   //蜂鸣器报警
						Show_Str(350,5,50,16,"地震",16,0);
						dizhenflag=1;
					}
					else if(temp2<-70.0||temp2>70.0)  //小于-4度或者大于4度
					{
						BEEP=0;   //蜂鸣器报警
						Show_Str(350,5,50,16,"地震",16,0);
						dizhenflag=1;
					}
					else if(temp3<-70.0||temp3>70.0)  //小于-4度或者大于4度
					{
						BEEP=0;   //蜂鸣器报警
						Show_Str(350,5,50,16,"地震",16,0);
						dizhenflag=1;
					}
					else
					{
						if(dizhenflag==1)
						{
							BEEP=1;   //解除报警
							LCD_Fill(350,5,450,25,BLACK);	//清除显示
							dizhenflag=0;
						}
					}
				}
				delay_ms(10);
			}
		}
		tmpu6050=tmpu6050+1;
		
		
//  闹钟处理		
		if(alarm.ringsta&(1<<7))//闹钟在执行
		{
			calendar_alarm_msg((lcddev.width-200)/2,(lcddev.height-160)/2);//闹钟处理
		}
//  gif解码
		if(gifdecoding)//gif正在解码中
		{
			key=pic_tp_scan();
			if(key==1||key==3)gifdecoding=0;//停止GIF解码
		}

//  TPAD扫描
		if(rerreturn)//再次开始TPAD扫描时间减一
		{
			rerreturn--;
			delay_ms(15);//补充延时差	
 		}else if(TPAD_Scan(0))		//TPAD按下了一次,此函数执行,至少需要15ms.
		{
			rerreturn=10;			//下次必须100ms以后才能再次进入
			system_task_return=1;
			if(gifdecoding)gifdecoding=0;	//不再播放gif
		}
//  SD卡检测		
		if((t%60)==0)//900ms左右检测1次
		{ 
			//SD卡在位检测
			if((DCMI->CR&0X01)==0)//摄像头不工作的时候,才可以查询SD卡
			{
				OS_ENTER_CRITICAL();//进入临界区(无法被中断打断) 
				res=SD_GetState();	//查询SD卡状态
				OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断) 
				if(res==0XFF)
				{
					gui_phy.memdevflag&=~(1<<0);//标记SD卡不在位 
					OS_ENTER_CRITICAL();//进入临界区(无法被中断打断) 
					SD_Init();			//重新检测SD卡 
					OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断) 
				}else if((gui_phy.memdevflag&(1<<0))==0)//SD不在位?
				{
					f_mount(fs[0],"0:",1);		//重新挂载sd卡
					gui_phy.memdevflag|=1<<0;	//标记SD卡在位了		
				} 
			}
 
		}  
		delay_ms(10);
	}
}
//硬件错误处理
void HardFault_Handler(void)
{
	u32 i;
	u8 t=0;
	u32 temp;
	temp=SCB->CFSR;					//fault状态寄存器(@0XE000ED28)包括:MMSR,BFSR,UFSR
 	printf("CFSR:%8X\r\n",temp);	//显示错误值
	temp=SCB->HFSR;					//硬件fault状态寄存器
 	printf("HFSR:%8X\r\n",temp);	//显示错误值
 	temp=SCB->DFSR;					//调试fault状态寄存器
 	printf("DFSR:%8X\r\n",temp);	//显示错误值
   	temp=SCB->AFSR;					//辅助fault状态寄存器
 	printf("AFSR:%8X\r\n",temp);	//显示错误值

 	while(t<5)
	{
		t++;
		for(i=0;i<0X1FFFFF;i++);
 	}

}


 
#include "FM1208.H"
#include "des.h"
#include <string.h>
#include "bsp_serial.h"
//extern unsigned char Pcb;					//CPU¿¨APDUÖ¸Áî·Ö×éºÅ
  unsigned char  DataBuf[64];
  unsigned char  g_cReceBuf[64]; 



/////×Ô¶¨Òå³Ö¿¨ÈË»ù±¾ĞÅÏ¢ÄÚÈİ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char static CardTpye=0x00;	//¿¨ÀàĞÍ±êÊ¶£º00  £¨ÕâÖ»ÊÇÒ»¸ö´úÂë£¬±ÈÈç00±íÊ¾ÖĞ¹úÒøĞĞ£¬01±íÊ¾¹¤ÉÌÒøĞĞµÈµÈ£©
unsigned char static StaffType=0x00;  //±¾ĞĞÖ°¹¤±êÊ¶	ÕâÖ»ÊÇÒ»¸ö´úÂë£¬ÀıÈç00±íÊ¾¹ó±öÓÃ»§£¬01±íÊ¾ÆÕÍ¨ÓÃ»§µÈµÈ

char name[32] = {"ËÕÅÖ×Ó\0"};	//³Ö¿¨ÈËĞÕÃû
unsigned char static DocumentNum[]={0x31 ,0x31 ,0x30 ,0x31 ,0x30 ,0x32 ,0x39 ,0x38 ,0x31 ,0x32 ,0x31 ,0x38 ,0x30 ,0x30 ,0x31 ,0x30};//³Ö¿¨ÈËµÄÖ¤¼şºÅÂë	  
unsigned char static DocumentTpye=0x05;//³Ö¿¨ÈËµÄÖ¤¼şÀàĞÍ	±ÈÈç05±íÊ¾Éí·İÖ¤£¬06±íÊ¾»¤ÕÕµÈµÈ  ,05ÊÇ²»ÊÇ´ú±íÉí·İÖ¤£¬²»ÖªµÀ,ÒªÈ¥ÎÊÎÊÒøĞĞµÄÈË,Éí·İÖ¤¼şµÄ´úºÅÊÇÊ²Ã´

#define	 NameLen     strlen (name)	 //¼ÆËãĞÕÃûµÄ³¤¶È

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//-------------------ÒÔÏÂÎªËùÒªÓÃµ½Ö¸ÁîµÄDATAÊı¾İ£¬½¨ÔÚMFÄ¿Â¼ÏÂµÄËùÓĞÎÄ¼ş¼°ÃÜÔ¿-----------------------------------------------------//
//
//ÎªÍ³Ò»·½±ã¹ÜÀíÔÄ¶Á´úÂë£¬ÎÒÃÇÔÚËùÓĞµÄÎÄ¼şÃûºóÃæ¼Ó¸ö_XXXX,±íÊ¾¾ÍÊÇÎÄ¼şµÄ±êÊ¶ºÅ£¬//														
//ÀıÈç£º FM1208_MF_Key_0000  :±íÊ¾FM1208 CPU¿¨ µÄMF ÏÂ KEYÎÄ¼ş£¨¼´ÃÜÔ¿ÎÄ¼ş£©±êÊ¶ºÅÎª0000
//


//½¨ÃÜÔ¿ÎÄ¼ş
unsigned char static FM1208_MF_Key_0000[] ={0x3F, 0x00, 0x50, 0x01, 0xF0, 0xFF, 0xFF};	

//1¡¢39×°ÔØ16×Ö½ÚµÄÍâ²¿ÈÏÖ¤ÃÜÔ¿//	
unsigned char static  FM1208_MF_ExKey[] ={0x39,0xF0,0xF0,0xAA,0x88,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	
									
//2¡¢36×°ÔØ8×Ö½ÚµÄÏßÂ·±£»¤ÃÜÔ¿	   
unsigned char static  FM1208_MF_LineCKey[]={0x36,0xF0,0xF0,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//½¨¶¨³¤¼ÇÂ¼ÎÄ¼ş£º¶¨³¤¼ÇÂ¼ÎÄ¼şÄÚÈİÎª¡°ÎÄ¼ş¿ØÖÆĞÅÏ¢£¨file control information£© 
unsigned char static  FM1208_MF_RECORD[]={0x2A,0x02,0x13,0xF0,0x00,0xFF,0xFF};

//×°ÔØ ¼ÇÂ¼ÎÄ¼şµÄÄ¿Â¼Ãû ¼°PBOC µÄ ASC ÂëµÄÄÚÈİ
unsigned char static  FM1208_MF_RECORD_DATA[]={0x61,0x11,0x4F,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x50,0x04,0x50,0x42,0x4F,0x43};





//-------------------ÒÔÏÂÎªËùÒªÓÃµ½Ö¸ÁîµÄDATAÊı¾İ£¬½¨ÔÚDFÄ¿Â¼ÏÂµÄËùÓĞÎÄ¼ş¼°ÃÜÔ¿-----------------------------------------------------//
//
//ÎªÍ³Ò»·½±ã¹ÜÀíÔÄ¶Á´úÂë£¬ÎÒÃÇÔÚËùÓĞµÄÎÄ¼şÃûºóÃæ¼Ó¸ö_XXXX,±íÊ¾¾ÍÊÇÎÄ¼şµÄ±êÊ¶ºÅ£¬//
//ÀıÈç£º FM1208_DF_3F01  :±íÊ¾FM1208 CPU¿¨ µÄDF ÏÂ	±êÊ¶ºÅÎª3F01
//


//´´½¨DFÎÄ¼ş,±êÊ¶ÃûÎª3F01//
unsigned char static  FM1208_DF_3F01[] ={0x38,0x03,0x6F,0xF0,0xF0,0x95,0xFF,0xFF,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01};

//½¨Á¢DFÏÂµÄÃÜÔ¿ÎÄ¼ş
unsigned char static FM1208_DF_Key_0000[] ={0x3F,0x01,0x8F,0x95,0xF0,0xFF,0xFF};	//½¨ÃÜÔ¿ÎÄ¼ş

//1¡¢×°ÔØTACÃÜÔ¿
unsigned char static  FM1208_DF_TACKey[] ={0x34,0xF0,0x02,0x00,0x01,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34};

//2¡¢ÏßÂ·±£»¤ÃÜÔ¿£¬MAC¼ÆËãÓÃµ½µÄ   £¨ÓÉSAM¿¨µÄMACÃÜÔ¿Ò»¼¶·ÖÉ¢µÃ³ö£©
unsigned char static  FM1208_DF_LineCKey[] ={0x36,0xF0,0x02,0xFF,0x33,0x7C,0x51,0xF0,0x79,0x10,0xD8,0x03,0x8D,0x20,0x97,0xE5,0x8F,0xDA,0x79,0x3F,0x1E};

//3¡¢Íâ²¿ÈÏÖ¤ÃÜÔ¿  £¨ÓÉSAM¿¨·ÖÉ¢ËùµÃ£©
unsigned char static  FM1208_DF_ExKey[] ={0x39,0xF0,0x02,0x44,0xFF,0x5F,0xD6,0x76,0x0C,0xDF,0x6D,0xD3,0x93,0xFF,0xDD,0x6C,0x5C,0xCB,0x65,0x85,0xBF};


//4¡¢Ïû·ÑÃÜÔ¿  ÓÃPSAM¿¨µÄÏû·ÑÃÜÔ¿ÖÖ×Ó·ÖÉ¢ËùÈ¡µÃ
unsigned char static  FM1208_DF_DPKey[] ={0x3E,0xF0,0x02,0x00,0x01,0x3E,0x5E,0x67,0xF2,0xFF,0x23,0x09,0x29,0x9F,0x84,0x20,0xF4,0x3D,0xE1,0x39,0x2B};

//5¡¢È¦´æÃÜÔ¿    (ÓÃPSAM¿¨µÄMAC¡¢¼ÓÃÜÃÜÔ¿£¬È»ºóÓÃ»§¿¨ĞòÁĞºÅ×÷ÎªÒò×Ó·ÖÉ¢³öÀ´£¬×÷ÎªÈ¦´æÃÜÔ¿£¬ÕâÑù¾ÍÃ¿ÕÅÓÃ»§¿¨µÄÃÜÔ¿²»Í¬)
unsigned char static  FM1208_DF_CZKey[] ={0x3F,0xF0,0x02,0x00,0x01,0x4A,0x17,0xCA,0x82,0x98,0xD5,0x28,0x65,0x14,0x0A,0xBE,0xAA,0x10,0x8A,0x4E,0x00 }; 
	  
//6¡¢¿ÚÁîPINÃÜÔ¿	  
unsigned char static  FM1208_DF_PIN[]={0x3A,0xF0,0xEF,0x01,0x33,0x12,0x34,0x56,0xFF,0xFF,0xFF,0xFF,0xFF}; 

//7¡¢ÄÚ²¿ÃÜÔ¿	  £¨ÓÉSAM¿¨·ÖÉ¢ËùµÃ£©
unsigned char static  FM1208_DF_INKey[]={0x30,0xF0,0xF0,0xFF,0x33,0x1E,0xE2,0xFC,0x1F,0x6C,0x47,0xD2,0x55,0xBF,0x4F,0xE9,0x41,0xF6,0x02,0xFB,0x29};




//½¨Á¢0015 ¹«¹²Ó¦ÓÃ»ù±¾Êı¾İÎÄ¼ş	 ÊÇ´øÏßÂ·±£»¤µÄ¶ş½øÖÆÎÄ¼ş  
unsigned char static  FM1208_DF_0015[] ={0xA8,0x00,0x1E,0xF0,0xF0,0xFF,0xFF};

//¹«¹²Ó¦ÓÃ»ù±¾Êı¾İÎÄ¼şÄÚÈİ  ×¢Òâ: ÕâÀïÓĞÒ»¸öÓÃ»§¿¨ĞòÁĞºÅ£º00062016102000000001£¬È¡ºó8¸ö×Ö½Ú£¬ÎÒÃÇ¸øÃ¿ÕÅÓÃ»§¿¨·¢¿¨Ê±ÉèÖÃÎª²»Í¬
unsigned char static  FM1208_DF_0015_DATA[] ={0x11,0x11,0x22,0x22,0x33,0x33,0x00,0x06,0x03,0x01,0x00,0x06,0x20,0x16,0x10,0x20,0x00,0x00,0x00,0x01,0x20,0x16,0x10,0x20,0x20,0x19,0x12,0x31,0x55,0x66};//


//½¨Á¢0016 ED/EP³Ö¿¨ÈË»ù±¾ĞÅÏ¢Êı¾İµÄÎÄ¼ş£¬Èç£¬ĞÕÃû¡¢Ö¤¼şµÈĞÅÏ¢ ÊÇ´øÏßÂ·±£»¤µÄ¶ş½øÖÆÎÄ¼ş   
unsigned char static  FM1208_DF_0016[] ={0xA8,0x00,0x27,0xF0,0xF0,0xFF,0xFF};

											
//½¨0018 ED/EPÓ¦ÓÃµÄ½»Ò×Ã÷Ï¸Ñ­»·¼ÇÂ¼ÎÄ¼ş£º ÎÒÃÇÖ»Òª°ÑÕâ¸öÎÄ¼ş½¨ºÃ¼´¿É£¬µ±ÎÒÃÇ½»Ò×³É¹¦ºóCOSÏµÍ³»á×Ô¶¯Ğ´Èë£¬ÎŞĞèÎÒÃÇ×Ô¼ºĞ´ÈëµÄ¡£
unsigned char static  FM1208_DF_0018[] ={0x2E,0x0A,0x17,0xF1,0xEF,0xFF,0xFF};

//½¨0001 EDµç×Ó´æÕÛÎÄ¼ş
unsigned char static  FM1208_DF_0001[] ={0x2F,0x02,0x08,0xF1,0x00,0xFF,0x18};

//½¨0002 EPµç×ÓÇ®°üÎÄ¼ş		
unsigned char static  FM1208_DF_0002[] ={0x2F,0x02,0x08,0xF0,0x00,0xFF,0x18};



//////////////////ÎÄ¼ş±êÊ¶µÄ¶¨Òå/////////////////////////////////////////////////////////////////////////////////
//
//ÎªÍ³Ò»·½±ã¹ÜÀíÔÄ¶Á´úÂë£¬ÎÒÃÇÔÚËùÓĞµÄÎÄ¼şÃûºóÃæ¼Ó¸ö_Flag,±íÊ¾¾ÍÊÇ¸Ã¶ÔÓ¦ÎÄ¼şµÄ±êÊ¶//

//unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MFÎÄ¼ş±êÊ¶3F00
unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MFÏÂµÄÃÜÔ¿ÎÄ¼ş±êÊ¶0000
unsigned char static ExKey_Flag       = {0x00};           //MFÏÂµÄÍâ²¿ÃÜÔ¿±êÊ¶00
unsigned char static LineKey_Flag     = {0x01}; 		    //MFÏÂµÄÏßÂ·±£»¤ÃÜÔ¿±êÊ¶01
unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MFÏÂµÄ¶¨³¤¼ÇÂ¼ÎÄ¼ş±êÊ¶0001
unsigned char static RECORD_Flag      = {0x08}; 		    //MFÏÂµÄ¶¨³¤¼ÇÂ¼±êÊ¶08

unsigned char static FM1208_DF_3F01_Flag[] ={0x3f,0x01};	//DFÎÄ¼ş±êÊ¶3F01
unsigned char static KEY_File_0000_Flag[2] ={0x00,0x00};  //DFÏÂµÄÃÜÔ¿ÎÄ¼ş±êÊ¶0000				   
unsigned char static FM1208_DF_TACKey_Flag={0x00};		//DFÏÂµÄTACÃÜÔ¿±êÊ¶00
unsigned char static FM1208_DF_LineCKey_Flag={0x00};		//DFÏÂµÄÏßÂ·±£»¤ÃÜÔ¿±êÊ¶00
unsigned char static FM1208_DF_ExKey_Flag  = {0x00};      //DFÏÂµÄÍâ²¿ÃÜÔ¿±êÊ¶00
unsigned char static FM1208_DF_Inital_Key_Flag  = {0x00}; //DFÏÂµÄÄÚ²¿ÃÜÔ¿±êÊ¶00



unsigned char static FM1208_DF_DPKey_Flag={0x00};			//DFÏÂµÄÏû·ÑÃÜÔ¿±êÊ¶00
unsigned char static FM1208_DF_CZKey_Flag={0x00};			//DFÏÂµÄÈ¦´æÃÜÔ¿±êÊ¶00
unsigned char static FM1208_DF_PIN_Flag={0x00};			//DFÏÂµÄ¿ÚÁîPINÃÜÔ¿±êÊ¶00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//¹«¹²Ó¦ÓÃ»ù±¾Êı¾İÎÄ¼ş±êÊ¶0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//³Ö¿¨ÈË»ù±¾ĞÅÏ¢Êı¾İµÄÎÄ¼ş±êÊ¶0016
unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //½»Ò×Ã÷Ï¸ÎÄ¼ş±êÊ¶0018
unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//µç×Ó´æÕÛÎÄ¼ş±êÊ¶0001
unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//µç×ÓÇ®°üÎÄ¼ş±êÊ¶0002


unsigned char  Set_CpuCard(void)
{
	 char st;
	 unsigned int  sta;
	 unsigned char tt[2];
	 unsigned char  Challenge[8]={0,0,0,0,0,0,0,0};
	 unsigned char  stdata[3];



	    st=SendUartStatus(FM1208_CreateFile(KEY_File_0000, sizeof(FM1208_MF_Key_0000), FM1208_MF_Key_0000 )); //½¨ÃÜÔ¿ÎÄ¼ş
		if(st)	
		return 1;		//´íÎó·µ»Ø	

		UART_Send_Str("±êÊ¶Îª0000µÄÃÜÔ¿ÎÄ¼ş½¨Á¢³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);

		st=SendUartStatus(FM1208_Load_Key(ExKey_Flag, sizeof(FM1208_MF_ExKey), FM1208_MF_ExKey));	//×°ÔØÍâ²¿ÈÏÖ¤ÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("Íâ²¿ÈÏÖ¤ÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&ExKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;

							

 		st=SendUartStatus(FM1208_Load_Key(LineKey_Flag, sizeof(FM1208_MF_LineCKey), FM1208_MF_LineCKey));	//×°ÔØÏßÂ·±£»¤ÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("ÏßÂ·±£»¤ÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&LineKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;



 		st=SendUartStatus(FM1208_CreateFile(RECORD_File, sizeof(FM1208_MF_RECORD), FM1208_MF_RECORD));	//½¨Á¢¶¨³¤¼ÇÂ¼ÎÄ¼ş
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("½¨Á¢¶¨³¤¼ÇÂ¼ÎÄ¼ş³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(AppendRecord(RECORD_Flag, sizeof(FM1208_MF_RECORD_DATA), FM1208_MF_RECORD_DATA));	 //Ğ´¶¨³¤¼ÇÂ¼ÎÄ¼şÄÚÈİ
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("Ğ´¶¨³¤¼ÇÂ¼ÎÄ¼şÄÚÈİ³É¹¦!");
		UART_Send_Enter();
	  	UART_Send_Enter();
		//delay(5);;

	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_3F01_Flag, sizeof(FM1208_DF_3F01), FM1208_DF_3F01 )); //½¨DFÎÄ¼ş
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("½¨DFÎÄ¼ş3F01³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(FM1208_SelectFile(FM1208_DF_3F01_Flag,DataBuf,tt)); //´ò¿ªCPU¿¨3F01ÎÄ¼ş//
		if(st)	
		return 1;		//´íÎó·µ»Ø
		
		UART_Send_Str("3F01ÎÄ¼ş´ò¿ª³É¹¦:"); 
		UART_Put_Num(&DataBuf[2],tt[0]);	 //´®¿ÚÖúÊÖÏÔÊ¾·µ»ØµÄÊıİ
	    UART_Send_Enter();
		//delay(5);;
		

	    st=SendUartStatus(FM1208_CreateFile(KEY_File_0000_Flag, sizeof(FM1208_DF_Key_0000), FM1208_DF_Key_0000 )); //½¨DFÏÂµÄÃÜÔ¿ÎÄ¼ş
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂ±êÊ¶Îª0000µÄÃÜÔ¿ÎÄ¼ş½¨Á¢³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_TACKey_Flag, sizeof(FM1208_DF_TACKey), FM1208_DF_TACKey));	//×°ÔØTACÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("TACÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&FM1208_DF_TACKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;
	

		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_LineCKey_Flag, sizeof(FM1208_DF_LineCKey), FM1208_DF_LineCKey));	//×°ÔØÏßÂ·±£»¤ÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂµÄÏßÂ·±£»¤ÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª");
		UART_Put_Num(&FM1208_DF_LineCKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_ExKey_Flag, sizeof(FM1208_DF_ExKey), FM1208_DF_ExKey));	//×°ÔØÍâ²¿ÈÏÖ¤ÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂµÄÍâ²¿ÈÏÖ¤ÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&FM1208_DF_ExKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_DPKey_Flag, sizeof(FM1208_DF_DPKey), FM1208_DF_DPKey));	//×°ÔØÏû·ÑÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂµÄÏû·ÑÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&FM1208_DF_DPKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_CZKey_Flag, sizeof(FM1208_DF_CZKey), FM1208_DF_CZKey));	//×°ÔØÈ¦´æÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂµÄÈ¦´æÃÜÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&FM1208_DF_CZKey_Flag,1);
		UART_Send_Enter();
		//delay(5);;


		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_PIN_Flag, sizeof(FM1208_DF_PIN), FM1208_DF_PIN));	//×°ÔØ¿ÚÁîPINÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂµÄ¿ÚÁîPINÔ¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&FM1208_DF_PIN_Flag,1);
		UART_Send_Enter();
		//delay(5);;

		st=SendUartStatus(FM1208_Load_Key(FM1208_DF_Inital_Key_Flag, sizeof(FM1208_DF_INKey), FM1208_DF_INKey));	//×°ÔØÄÚ²¿ÃÜÔ¿
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂµÄÄÚ²¿Ô¿×°ÔØ³É¹¦,±êÊ¶Îª:");
		UART_Put_Num(&FM1208_DF_Inital_Key_Flag,1);
		UART_Send_Enter();
		//delay(5);;





		sta=FM1208_CreateFile(FM1208_DF_0015_Flag, sizeof(FM1208_DF_0015), FM1208_DF_0015 );  //½¨DFÏÂµÄ0015ÎÄ¼ş
		if(sta!=0x9000)	//µ±·µ»ØÎª01Ê±£¬±íÊ¾³¬Ê±
		{
			 tt[0]=sta>>8;
		     if(tt[0]==0x01)	  //·µ»Ø01Ê±£¬ĞèÒªµÈ´ıÊ±£¬½øÈë
			 {
				while(1)
				{
					WTX_Cmd(stdata,tt);
					if(stdata[2]!=0x01)
					{
						sta=0x9000;
						break;
					}
					else
					{
						__nop();
						//delay_ms(5);
					}
				}
			 }
		}

		st=SendUartStatus(sta);
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂ±êÊ¶Îª0015ÎÄ¼ş½¨Á¢³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		FM1208_GetChallenge(Challenge);	 //È¡Ëæ»úÊı	
		MAC1(FM1208_DF_0015_Flag[1], sizeof(FM1208_DF_0015_DATA), FM1208_DF_0015_DATA, &FM1208_DF_LineCKey[5],Challenge,DataBuf,tt);
		st=SendUartStatus(FM1208_Update_Binary(FM1208_DF_0015_Flag[1],DataBuf, tt[0] ));    //Ö´ĞĞ°Ñ¼ÓÏßÂ·±£»¤µÄÊı¾İĞ´Èë0015ÖĞ¡£
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("0015¹«¹²Ó¦ÓÃ»ù±¾ÄÚÈİĞ´Èë³É¹¦!");
		UART_Send_Enter();
	  	UART_Send_Enter();
		//delay(5);;


	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0016_Flag, sizeof(FM1208_DF_0016), FM1208_DF_0016 )); //½¨DFÏÂµÄ0016ÎÄ¼ş
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("DFÏÂ±êÊ¶Îª0016ÎÄ¼ş½¨Á¢³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;


		FM1208_GetChallenge(Challenge);	 				 //È¡Ëæ»úÊı		
		DataBuf[0]=CardTpye;							 //¿¨ÀàĞÍ±êÊ¶
		DataBuf[1]=StaffType;							 //±¾ĞĞÖ°¹¤±êÊ¶
		memcpy(&DataBuf[2], name, NameLen+1);			 //³Ö¿¨ÈËĞÕÃû
		memset(&DataBuf[NameLen+2], 0x00, 20-NameLen);	 //ĞÕÃûºóÃæ²¹0
		memcpy(&DataBuf[22], DocumentNum, 16);			 //³Ö¿¨ÈËµÄÖ¤¼şºÅÂë
		DataBuf[38]=DocumentTpye;						 //³Ö¿¨ÈËµÄÖ¤¼şÀàĞÍ


		MAC1(FM1208_DF_0016_Flag[1], 0x27, DataBuf, &FM1208_DF_LineCKey[5],Challenge,g_cReceBuf,tt);
		st=SendUartStatus(FM1208_Update_Binary(FM1208_DF_0016_Flag[1],g_cReceBuf, tt[0] ));    //Ö´ĞĞ°Ñ¼ÓÏßÂ·±£»¤µÄÊı¾İĞ´Èë0016ÖĞ¡£
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("0016ÓÃ»§¿¨»ù±¾ĞÅÏ¢Ğ´Èë³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;	



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0018_Flag, sizeof(FM1208_DF_0018), FM1208_DF_0018 )); //½¨DFÏÂµÄ0018ÎÄ¼ş
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("½¨DFÏÂµÄ0018Ñ­»·½»Ò×¼ÇÂ¼ÎÄ¼ş³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0001_Flag, sizeof(FM1208_DF_0001), FM1208_DF_0001 )); //½¨DFÏÂµÄµç×Ó´æÕÛÎÄ¼şED
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("½¨DFÏÂµÄµç×Ó´æÕÛEDÎÄ¼ş³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();
		//delay(5);;



	    st=SendUartStatus(FM1208_CreateFile(FM1208_DF_0002_Flag, sizeof(FM1208_DF_0002), FM1208_DF_0002 )); //½¨DFÏÂµÄµç×ÓÇ®°üÎÄ¼şEP
		if(st)	
		return 1;		//´íÎó·µ»Ø
		UART_Send_Str("½¨DFÏÂµÄµç×ÓÇ®°üEPÎÄ¼ş³É¹¦!");
		UART_Send_Enter();
		UART_Send_Enter();


		return 0;


}


//*************************************************************************
// º¯ÊıÃû	£ºPcdSwitchPCB(void)
// ÃèÊö		£ºÇĞ»»·Ö×éºÅ
// Èë¿Ú		£ºÎŞ
// ³ö¿Ú		£ºÎŞ
// ·µ»Ø		£ºÎŞ
//*************************************************************************

//void PcdSwitchPCB(void)
//{
//	switch(Pcb)
//	{
//		case 0x00:
//			Pcb=0x0A;
//			break;
//		case 0x0A:
//			Pcb=0x0B;
//			break;
//		case 0x0B:
//			Pcb=0x0A;
//			break;
//	}
//}

//*************************************************************************
// º¯ÊıÃû	£ºpro_reset
// ÃèÊö		£º×ªÈëAPDUÃüÁî¸ñÊ½CPU¿¨¸´Î»²Ù×÷
// Èë¿Ú		£º ÎŞ
// ³ö¿Ú		£º DataOut Êä³öµÄÊı¾İ£¬	rlen Êä³öÊı¾İµÄ³¤¶È
// ·µ»Ø		£º³É¹¦·µ»Ø _OK = 0
//*************************************************************************

char pro_reset(unsigned char *DataOut, unsigned char *rlen)
{

    char status;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

	memset(MfComData.MfData, 0x00, 64);  //Çå0

	 
	//PcdSetTmo(4);


  //  MfComData.MfCommand = PCD_TRANSCEIVE;		 //·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = 0xE0;
    MfComData.MfData[1] = 0x60;
      

//    status = PcdComTransceive(pi);
    if (status == 0)
    {

	   memcpy(DataOut, &MfComData.MfData[0], MfComData.MfLength);
	   *rlen=MfComData.MfLength/8;
		
       return   0;
    }

	 return  status;
	 
}

//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºFM1208_SelectFile
// ÃèÊö		£ºÑ¡ÔñÎÄ¼ş
// Èë¿Ú		£ºpDataIn : ÎÄ¼ş±êÊ¶·û»òÕßDF Ãû³Æ
// ³ö¿Ú		£ºpDataOut Êä³öÊı¾İ   Out_LenÊä³öµÄÊı¾İ³¤¶È
// ·µ»Ø		£ºÖ´ĞĞ³É¹¦sst=9000
//*************************************************************************
 
unsigned int FM1208_SelectFile(unsigned char* pDataIn, unsigned char* pDataOut,unsigned char * Out_Len)
{
  
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

//    MfComData.MfCommand = PCD_TRANSCEIVE;			  //·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî
	memset(MfComData.MfData, 0x00, 64);  //Çå0
	 
	//PcdSetTmo(13);


//	PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x00;				// CLA
	MfComData.MfData[3] = 0xA4;				// INS-Select File 			
	MfComData.MfData[4] = 0x00;				// P1						 
	MfComData.MfData[5] = 0x00;				// P2					
	MfComData.MfData[6] = 0x02;				// LC	
 	MfComData.MfData[7] = pDataIn[0];		// File					
	MfComData.MfData[8] = pDataIn[1];		// File	
   
	MfComData.MfLength  = 9;	   //ÎªMfComData.MfDataµÄËùÓĞÊı¾İ³¤¶ÈÎª9¸ö×Ö½Ú


//    status = PcdComTransceive(pi);					   //Ğ´Ö¸Áî
    if (status == 0)
    {

		 memcpy(pDataOut, MfComData.MfData, MfComData.MfLength);	//DATA	


	    *Out_Len=MfComData.MfLength/8;	

		st[0] = pDataOut[*Out_Len-2];   //Ö´ĞĞµÄ·µ»Ø×´Ì¬
		st[1] = pDataOut[*Out_Len-1];

		sst=st[0];
		sst=(sst<<8)|st[1];

		*Out_Len=MfComData.MfLength/8-2;//¼õÈ¥Ç°ÃæµÄÁ½¸ö×Ö½Ú	


		return sst;

    }
	else  
	{	 
		return 0xFF;
	}
	  
}

//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºWTX_Cmd
// ÃèÊö		£º´Ëº¯ÊıÖ÷Òª×÷ÓÃÊÇ£¬µ±¶ÔCPU¿¨½¨ÎÄ¼ş»òÉ¾³ıÎÄ¼şÊ±£¬ĞèÒªÌá¹©µÈ´ıÊ±¼ä£¬
//            µ±Ö´ĞĞCPU¿¨Ö¸Áî·µ»Ø01Ê±£¬ÎÒÃÇÔò°Ñ01·¢ËÍ»Ø¸øCPU¿¨£¬Ö±µ½µÈµ½²»ÔÙ·µ»Ø01ÔòÖ´ĞĞ³É¹¦
// Èë¿Ú		£ºpDataIn : ÒªÖ´ĞĞµÄÖ¸Áî					
// ³ö¿Ú		£ºpDataOut£ºÊä³öÖ´ĞĞºóµÄ·µ»ØÊı¾İ   Out_LenÊä³öµÄÊı¾İ³¤¶È
// ·µ»Ø		£º0
//*************************************************************************

char  WTX_Cmd(unsigned char* pDataOut,unsigned char * Out_Len)
{
  
    char status;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

 
	memset(MfComData.MfData, 0x00, 64);  //Çå0

	 
//	PcdSetTmo(13);

//    MfComData.MfCommand = PCD_TRANSCEIVE;			//·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî

    MfComData.MfData[0] = 0xFA;
    MfComData.MfData[1] = 0x00;
	MfComData.MfData[2] = 0X01;				
		
	MfComData.MfLength  = 3;	               //ÎªMfComData.MfDataµÄËùÓĞÊı¾İ³¤¶È

									

//    status = PcdComTransceive(pi);					   //Ğ´Ö¸Áî

    if (status == 0)
    {
 	   
		memcpy(pDataOut, MfComData.MfData, MfComData.MfLength);	//DATA	

	    *Out_Len=MfComData.MfLength/8-2;		//¼õÈ¥Ç°ÃæµÄÁ½¸ö×Ö½Ú
        return   0;
    }
	else
	{
		return 0xFF;
	}
	
 }

 //void FM_GET_CHALLENGE(uint8_t togetsize,uint8_t*challenge)
//{
//	uint16_t i = 0;
//	    gAPDU_Send.Header.CLA = 0;
//		gAPDU_Send.Header.INS = 0x84;
//		gAPDU_Send.Header.P1 = 0;
//		gAPDU_Send.Header.P2 = 0;
//		gAPDU_Send.Body.LC = togetsize;
//		if(CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
//		{
//			//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
//			if(gAPDU_rxLen >= 2)
//			{
//				gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
//				gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
//			}
//			if( gAPDU_repbuf.SW1 == 0x67)
//			{
//				//³¤¶È´íÎó
//			}
//			if((gAPDU_repbuf.SW1 == 0x6a)&&(gAPDU_repbuf.SW2 == 0x81))
//			{
//				//²»Ö§³Ö´Ë¹¦ÄÜ£¨ÎŞMF »ò¿¨Æ¬ÒÑËø¶¨£©
//			}
//			printf("<<----\r\n");
//			for(i= 0; i < gAPDU_rxLen;i++)
//				printf("%02x ", gAPDU_repbuf.Data[i]);
//			printf("\r\n");
//		}
// 

//}
//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºFM1208_GetChallenge
// ÃèÊö		£º¶ÁÈ¡Ëæ»úÊı
// Èë¿Ú		£ºÎŞ			  		
// ³ö¿Ú		£º pRan: 4¸ö×Ö½ÚµÄËæ»úÊı
// ·µ»Ø		£ºÖ´ĞĞ³É¹¦sst=0X9000
//*************************************************************************
unsigned int FM1208_GetChallenge(unsigned char* pRan)
{

    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
 
	uint16_t i = 0;
	gAPDU_Send.Header.CLA 	= 0;
	gAPDU_Send.Header.INS 	= 0x84;
	gAPDU_Send.Header.P1	= 0;
	gAPDU_Send.Header.P2 	= 0;
	gAPDU_Send.Body.LC 		= 	0x04;
	if(status = CardTransmit((uint8_t*)&gAPDU_Send.Header.CLA,5,(uint8_t*)gAPDU_repbuf.Data,&gAPDU_rxLen) == 0)
	{
		 
		//int CardTransmit( uint8_t * capdu, uint16_t capdulen, uint8_t * rapdu, uint16_t * rapdulen)
		if(gAPDU_rxLen >= 2)
		{
			gAPDU_repbuf.SW1 = gAPDU_repbuf.Data[gAPDU_rxLen-2];
			gAPDU_repbuf.SW2 = gAPDU_repbuf.Data[gAPDU_rxLen-1];				
		}
		if( gAPDU_repbuf.SW1 == 0x67)
		{
			//³¤¶È´íÎó
		}
		if((gAPDU_repbuf.SW1 == 0x6a)&&(gAPDU_repbuf.SW2 == 0x81))
		{
			//²»Ö§³Ö´Ë¹¦ÄÜ£¨ÎŞMF »ò¿¨Æ¬ÒÑËø¶¨£©
		}
//		printf("<<----\r\n");
//		for(i= 0; i < gAPDU_rxLen;i++)
//			printf("%02x ", gAPDU_repbuf.Data[i]);
//		printf("\r\n");
	}
	if (status == 0)
	{
		st[0] = gAPDU_repbuf.SW1;	 //0X90
		st[1] = gAPDU_repbuf.SW1; 	 //0X00
		sst=st[0];
		sst=(sst<<8)|st[1];
		if(sst==0x9000)
		{
			memcpy(pRan, gAPDU_repbuf.Data, 4);
		}
				
		return sst;
	}
	else
		return 0xFF;
	}	
	 

}

//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºFM1208_CreateFile
// ÃèÊö		£º´´½¨ÎÄ¼ş
// Èë¿Ú		£ºfileid :´´½¨µÄÎÄ¼ş±êÊ¶
//            Len £º±¨ÎÄÊı¾İµÄ³¤¶È       
//			  pDataIn£º´´½¨ÎÄ¼ş±¨ÎÄµÄÊı¾İ
//
// ³ö¿Ú		£ºÎŞ
// ·µ»Ø		£ºÖ´ĞĞ³É¹¦sst=0X9000
//*************************************************************************
 
unsigned int  FM1208_CreateFile(unsigned char *fileid, unsigned char Len, unsigned char *pDataIn )
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

//    MfComData.MfCommand = PCD_TRANSCEIVE;			  //·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî
	memset(MfComData.MfData, 0x00, 64);  //Çå0
	    //CRCĞ£Ñé
//	PcdSetTmo(13);


	//PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x80;				// CLA
	MfComData.MfData[3] = 0xE0;			    //INS-Create File 			
	MfComData.MfData[4] = fileid[0];		//P1						 
	MfComData.MfData[5] = fileid[1];		//P2					
	MfComData.MfData[6] = Len;				//Lc
			
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //ÎªMfComData.MfDataµÄËùÓĞÊı¾İ³¤¶È										

	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA
	   

//    status = PcdComTransceive(pi);					   //Ğ´Ö¸Áî
 	if (status == 0)							   //Ö¸ÁîÖ´ĞĞ³É¹¦
	{
		st[0] = MfComData.MfData[2];
		st[1] = MfComData.MfData[3]; 

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;									   //Ö¸ÁîÖ´ĞĞ³É¹¦ºó£¬·µ»ØCPU¿¨µÄÖ´ĞĞ½á¹ûÊÇ·ñ³É¹¦·µ»Ø9000
	}
	else						//Ö¸ÁîÖ´ĞĞÊ§°Ü

		return 0xFF;
}

//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºFM1208_Load_Key
// ÃèÊö		£º×°ÔØÃÜÔ¿
// Èë¿Ú		£ºKeysign:ÃÜÔ¿±êÊ¶
//			  Len £º±¨ÎÄÊı¾İµÄ³¤¶È 		
//			  pDataIn£º×°ÔØÃÜÔ¿ÎÄ¼ş±¨ÎÄµÄÊı¾İ
//					
//					
// ³ö¿Ú		£ºÎŞ
// ·µ»Ø		£ºÖ´ĞĞ³É¹¦sst=0X9000
//*************************************************************************

unsigned int FM1208_Load_Key(unsigned char Keysign, unsigned char Len, unsigned char* pDataIn)
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

   // MfComData.MfCommand = PCD_TRANSCEIVE;			  //·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî
	memset(MfComData.MfData, 0x00, 64);  //Çå0
	 	   //CRCĞ£Ñé
	//PcdSetTmo(13);




	//PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x80;				// CLA
	MfComData.MfData[3] = 0xD4;			    //INS-Write Key 			
	MfComData.MfData[4] = 0x01;				//P1						 
	MfComData.MfData[5] = Keysign;			//p2-key flag									
	MfComData.MfData[6] = Len;				//Lc		
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //ÎªMfComData.MfDataµÄËùÓĞÊı¾İ³¤¶È										


	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA	 
    

	  

//    status = PcdComTransceive(pi);					   //Ğ´Ö¸Áî
	if (status == 0)
	{
		st[0] = MfComData.MfData[2];	 //0X90
		st[1] = MfComData.MfData[3]; 	 //0X00

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;
	}
	else
		return 0xFF;

}


//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºAppendRecord
// ÃèÊö		£ºĞ´¼ÇÂ¼ÎÄ¼ş
// Èë¿Ú		£ºfileid ¼ÇÂ¼ÎÄ¼şµÄ¶ÌÎÄ±êÊ¶
//			  Len £º±¨ÎÄÊı¾İµÄ³¤¶È 	
//			  pDataIn£ºĞ´Èë¼ÇÂ¼ÎÄ¼şµÄ±¨ÎÄÊı¾İ
//				
//				
// ³ö¿Ú		£ºÎŞ
// ·µ»Ø		£ºÖ´ĞĞ³É¹¦sst=0X9000
//*************************************************************************

unsigned int AppendRecord(unsigned char fileid, unsigned char Len, unsigned char* pDataIn)
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

  //  MfComData.MfCommand = PCD_TRANSCEIVE;			  //·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî
	memset(MfComData.MfData, 0x00, 64);  //Çå0
	 
//	PcdSetTmo(13);




//	PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x00;				// CLA
	MfComData.MfData[3] = 0xE2;			    //INS 			
	MfComData.MfData[4] = 0x00;				//P1						 
	MfComData.MfData[5] = fileid;			//p2 									
	MfComData.MfData[6] = Len;				//Lc		
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //ÎªMfComData.MfDataµÄËùÓĞÊı¾İ³¤¶È										


	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA	 
    

	  

//    status = PcdComTransceive(pi);					   //Ğ´Ö¸Áî
	if (status == 0)
	{
		st[0] = MfComData.MfData[2];	 //0X90
		st[1] = MfComData.MfData[3]; 	 //0X00

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;
	}
	else
		return 0xFF;

}

//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£ºFM1208_Update_Binary
// ÃèÊö		£ºĞ´¶ş½øÖÆÎÄ¼ş£¬ÊÇ´øÏßÂ·±£»¤
// Èë¿Ú		£ºFile_Flag: ¶ş½øÖÆÎÄ¼şµÄ¶ÌÎÄ±êÊ¶
//            pDataIn :ĞèÒªĞ´ÈëµÄ¶ş½øÖÆÊı¾İ
//            Len:¶ş½øÖÆÊı¾İµÄ³¤¶È
//		  		
// ³ö¿Ú		£ºÎŞ
// ·µ»Ø		£ºÖ´ĞĞ³É¹¦sst=0X9000
//*************************************************************************

unsigned int FM1208_Update_Binary(unsigned char File_Flag,unsigned char *pDataIn, unsigned char Len )
{
    char status;
	unsigned char  st[2];
	unsigned int   sst=0;
    struct TranSciveBuffer MfComData;    
    struct TranSciveBuffer *pi;
    pi = &MfComData;

   // MfComData.MfCommand = PCD_TRANSCEIVE;			  //·¢ËÍ²¢½ÓÊÕÊı¾İµÄÃüÁî
	memset(MfComData.MfData, 0x00, 64);  //Çå0
	 
//	PcdSetTmo(13);




	//PcdSwitchPCB();
//    MfComData.MfData[0] = Pcb;
    MfComData.MfData[1] = 0x00;

	MfComData.MfData[2] = 0x04;				//CLA
	MfComData.MfData[3] = 0xD6;			    //INS 			
	MfComData.MfData[4] = 0x80|File_Flag;	//P1						 
	MfComData.MfData[5] = 0x00;			    //p2 									
	MfComData.MfData[6] = Len;				//Lc		
	MfComData.MfLength  = MfComData.MfData[6]+7;	   //ÎªMfComData.MfDataµÄËùÓĞÊı¾İ³¤¶È										

	memcpy(&MfComData.MfData[7], pDataIn, Len);			//DATA	 



//    status = PcdComTransceive(pi);					   //Ğ´Ö¸Áî
	if (status == 0)
	{
		st[0] = MfComData.MfData[2];	 //0X90		   
		st[1] = MfComData.MfData[3]; 	 //0X00

		sst=st[0];
		sst=(sst<<8)|st[1];

		return sst;
	}
	else
		return 0xFF;




}
 
//*************************************************************************
// ¹ãÖİºãÌìÌÔ±¦µê¼¼Êõ×Ü¹¤±àĞ´
// º¯ÊıÃû	£º MAC1
// ÃèÊö		£º °Ñ4¸ö×Ö½ÚµÄMAC¼ÆËã½á¹û¼ÓÔÚ¶ş½øÖÆÎÄ¼şÖ¸ÁîÊı¾İµÄÎ²²¿
// Èë¿Ú		£º File_Flag ¶ÌÎÄ¼ş±êÊ¶  
//		       Len ÒªĞ´ÈëµÄÊı¾İ³¤¶È
//		       pDataIn ÒªĞ´ÈëµÄÊı¾İ
//		       LineCKey ÏßÂ·±£»¤ÃÜÔ¿
//             Challenge Ëæ»úÊı
//             
// ³ö¿Ú     £º pDataOut	 ÒÑ¼Ó4¸ö×Ö½ÚµÄMACÖµµÄÖ¸ÁîÊı¾İ
// ·µ»Ø		£º ÎŞ
//*************************************************************************
void MAC1(unsigned char File_Flag, unsigned char Len, unsigned char *pDataIn, unsigned char *LineCKey, unsigned char *Challenge,unsigned char *pDataOut,unsigned char *Le)
{

	unsigned char    MData[64];   
  	unsigned char    fout[4]={0,0,0,0};
 
	MData[0] = 0x04;			
	MData[1] = 0xD6;			    			
	MData[2] = 0x80|File_Flag;							 
	MData[3] = 0x00;			    									
	MData[4] = Len+4;
	 
	memcpy(&MData[5], pDataIn, Len);					  
    PBOC_MAC(&LineCKey[0], Challenge, MData, Len+5,fout);	
	
	memcpy(&MData[Len+5], fout, 4);	     //Êı¾İÎ²²¿¼ÓÉÏ4Î»µÄMAC
	
								
	Le[0]=MData[4];						 //Êä³öÊı¾İµÄ³¤¶È
	memcpy(pDataOut, &MData[5],Le[0]);	 //Êä³öµÄÊı¾İ

}


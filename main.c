#include "timer.h"
#include "canbus.h"
#include "sys.h"
#include "rtc1.h"
#include <stdio.h>
#include <stdlib.h>

void system_paly_audio(u8 audiotime)
{
	u8 audio_cmd[2];
	read_dgus_vp(0xA0,audio_cmd,1);
	audio_cmd[1] = audiotime;			//播放段数 ,audiotime*8ms
	write_dgus_vp(0xA0,audio_cmd,1);
}

//stanisyalarin id lerin kenara yazmasi ucun bunu eledim
u16 meShutdownPs[]  		=	{1065, 1066, 1067, 1068};
u16 meSlowdownPs[]			=	{1076, 1077};
u16 meAlarmPs[]					=	{1069, 1070, 1071, 1072, 1017, 1018, 1006, 1021, 1022, 1010, 1023, 1074, 1075, 1078};
u16 meShutdownSb[]			=	{2066, 2067, 2068, 2069};
u16 meSlowdownSb[]			=	{2077, 2078};
u16 meAlarmSb[]					=	{2070, 2071, 2072, 2073, 2021, 2022, 2006, 2025, 2026, 2010, 2027, 2075, 2076, 2079};
u16 cppGbAlarmPs[]			=	{1073, 1079, 1081, 1082, 1080, 1059, 1060, 1083, 1026, 1084, 1085, 1086,1008};
u16 cppGbAlarmSb[]			=	{2074, 2080, 2082, 2083, 2081, 2047, 2048, 2084, 2030, 2085, 2086, 2087,2008};
u16 generatorsAlarm[]		=	{1087, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1014, 1027, 1028, 1029, 1016, 1030, 1095, 2088, 2089, 2090, 2091, 2092, 2093, 2094, 2095, 2014, 2031, 2032, 2033, 2016, 2034, 2096, 2111, 2112};
u16 thrustersAlarms[]		=	{1097, 1098, 1099, 1100, 1102, 1103, 1104, 1105, 1107, 1108, 1109, 1110, 1111, 1112, 1037, 1038, 1039, 1040, 1041, 1042, 2052, 2053, 2054, 2055, 2056, 2057, 2058};
u16 steeringGearAlarm[]	=	{1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 4086, 4087, 4088, 4089, 4090, 4091, 4092};
u16 msbEsbAlarm[]				=	{2097, 2098, 2099, 2100, 2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110};
u16 wtdHatchesAlarm[]		=	{2037, 2038, 2039, 2040, 2041, 2042, 2043, 2044, 2045, 2046};
u16 tanksBilgesAlarm[]	=	{3066, 3067, 3068, 3069, 3070, 3071, 3072, 3073, 3074, 3079, 3080, 3081, 3082, 3083, 3084, 3085, 3086, 3087, 3088, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013, 3014, 3015, 3016, 3017, 3018, 3019, 3020, 3021, 3022, 3023, 3024, 3025, 3026, 3027, 3028, 3029, 3030, 3031, 3032, 3033, 3034, 3035, 3036, 3037, 3038, 3039, 3040, 3041};
u16 machnineryAlarm[]		=	{1061, 1062, 1063, 1064, 1001, 2065, 2049, 2051, 3065, 3094, 3095, 3096, 3097, 3042, 3098, 3099, 3100, 3101, 3102, 3103, 3104, 3105, 3108, 3110, 3111, 3112, 3053, 3054, 3055, 3056, 3057, 3058, 3059, 3060, 3061, 3062, 3063, 3064, 3001, 3002, 3003, 4065, 4066, 4067, 4068, 4069, 4070, 4071, 4074, 4075, 4076, 4077, 4078, 4079, 4006, 4008, 4010, 4012, 4082, 4083, 4084, 4085, 4014, 4016, 4018, 4020, 4022, 4024};			

//ardicil duzulmesi
u16* textChannelsID[]	=	{meShutdownPs, meSlowdownPs, meAlarmPs, meShutdownSb, meSlowdownSb, meAlarmSb, cppGbAlarmPs, cppGbAlarmSb, generatorsAlarm, thrustersAlarms, steeringGearAlarm, msbEsbAlarm, wtdHatchesAlarm, tanksBilgesAlarm, machnineryAlarm};

size_t textChannelsLengths[] = {sizeof(meShutdownPs) / sizeof(u16), sizeof(meSlowdownPs) / sizeof(u16), sizeof(meAlarmPs) / sizeof(u16), sizeof(meShutdownSb) / sizeof(u16), sizeof(meSlowdownSb) / sizeof(u16), sizeof(meAlarmSb) / sizeof(u16), sizeof(cppGbAlarmPs) / sizeof(u16), sizeof(cppGbAlarmSb) / sizeof(u16), sizeof(generatorsAlarm) / sizeof(u16), sizeof(thrustersAlarms) / sizeof(u16), sizeof(steeringGearAlarm) / sizeof(u16), sizeof(msbEsbAlarm) / sizeof(u16), sizeof(wtdHatchesAlarm) / sizeof(u16), sizeof(tanksBilgesAlarm) / sizeof(u16), sizeof(machnineryAlarm) / sizeof(u16)};
// usteki bunun yerine kececek
//	u8 textChannelsLenght[] = {4, 2, 14, 4, 2, 14, 13, 13, 32, 27, 17, 14, 10, 56, 68};

u8 analogIDsOrder1[]  = 	{0, 0, 0, 0}; //ME Shutdown - PS
u8 analogIDsOrder2[]  = 	{0, 0}; //meSlowdownPs
u8 analogIDsOrder3[]  = 	{0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}; //meAlarmPs
u8 analogIDsOrder4[]  = 	{0, 0, 0, 0}; //meShutdownSb
u8 analogIDsOrder5[]  = 	{0, 0}; //meSlowdownSb
u8 analogIDsOrder6[]  = 	{0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}; //meAlarmSb
u8 analogIDsOrder7[]  = 	{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1}; //cppGbAlarmPs
u8 analogIDsOrder8[]  = 	{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1}; //cppGbAlarmSb
u8 analogIDsOrder9[]	= 	{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0}; //generatorsAlarm
u8 analogIDsOrder10[]	= 	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //thrustersAlarms
u8 analogIDsOrder11[]	= 	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //steeringGearAlarm
u8 analogIDsOrder12[] = 	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //msbEsbAlarm
u8 analogIDsOrder13[] = 	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //wtdHatchesAlarm
u8 analogIDsOrder14[] = 	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}; //tanksBilgesAlarm
u8 analogIDsOrder15[] = 	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}; //machnineryAlarm
	
	u8* analogIDsOrder[]	=	{analogIDsOrder1, analogIDsOrder2, analogIDsOrder3, analogIDsOrder4, analogIDsOrder5, analogIDsOrder6, analogIDsOrder7, analogIDsOrder8, analogIDsOrder9, analogIDsOrder10, analogIDsOrder11, analogIDsOrder12, analogIDsOrder13, analogIDsOrder14, analogIDsOrder15};
	
u8 statusOrder1[]   = 		{0, 0, 0, 2}; //ME Shutdown - PS
u8 statusOrder2[]   = 		{0, 0}; //meSlowdownPs
u8 statusOrder3[]   = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //meAlarmPs
u8 statusOrder4[]   = 		{0, 0, 0, 3}; //meShutdownSb
u8 statusOrder5[]   = 		{0, 0}; //meSlowdownSb
u8 statusOrder6[]   = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //meAlarmSb
u8 statusOrder7[]   = 		{2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0}; //cppGbAlarmPs
u8 statusOrder8[]   = 		{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0}; //cppGbAlarmSb
u8 statusOrder9[]		= 		{3, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}; //generatorsAlarm
u8 statusOrder10[]	= 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0}; //thrustersAlarms
u8 statusOrder11[]  = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //steeringGearAlarm
u8 statusOrder12[]  = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //msbEsbAlarm
u8 statusOrder13[]  = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //wtdHatchesAlarm
u8 statusOrder14[]  = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //tanksBilgesAlarm
u8 statusOrder15[]  = 		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0, 4, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0}; //machnineryAlarm			

	u8* statusOrder[]	=	{statusOrder1, statusOrder2, statusOrder3, statusOrder4, statusOrder5, statusOrder6, statusOrder7, statusOrder8, statusOrder9, statusOrder10, statusOrder11, statusOrder12, statusOrder13, statusOrder14, statusOrder15};
																 
u8 countTheChannels = 0;	
u8 dynamicPageLimit = 0;
u16 channelNumbers[15] = {10544, 8497, 16434, 9011, 9268, 9525, 24118, 9783, 10808, 10297, 16737, 16994, 17251, 17508, 17765};														 
u8 alarmOnBefore[15][68] = 0; //siqnalin evveliki halina baxmaq ucun olan verable   ///
																 ////// bu yuxardakinde yer azalsin deye duzelde bilerik
u16 stationAlarmArray[100];
															 
int signalChannel[100] = 0;
int signalNumber[100] = 0;
//int signalNu[100] = 0;
int alarmNumber[100] = 0;
															 
float group_13[] 		= {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,6.573,6.573,1.8,1.8,1.2,5.833,5.833,1.142,1.142,1.2,2,2,3,3,3.8,1.5,2.3,1.2,1.2,1,2,2,4.4,4.133,6.8,6.8,4.852,4.852,6.8,6.8,6.8,5.9,5.9,5.857,5.857,2.9,2.9}; //56
float group_13_100[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100}; //56

	
void main()
{
	
	extern float *group[];
	///extern u8 alarmOn[15][68];
	extern u8 *alarmOn[];
	
	extern u16 stationAlarm[4];
	
	
	extern u16 stationTest;
	
	extern u16 idNumber[10];
	
	extern u16 idOrder[10];
	
	extern u16 test[2];
	
	extern u8 num[10];
	
	u16 zero[]={0,1,2,3,4,5,6,7,8,9};
	
	u8 pageTextWriteFlag = 1;
	u8 pageTextWriteFlagA = 1;
	u8 pageState=0;
	u8 pageStateA=0;
	u8 channelState=0;
	u8 iLimit=0;
	
	u8 channelPerPage = 8;
	
	u8 mute[2];
	
	
	u8 cc = 0;
	
	int number = 42;
	
	//extern u8 tmp[14][4][2];
	
	
	enum channel_state{meShutdownPs,meSlowdownPs,meAlarmPs,meShutdownSb,meSlowdownSb,meAlarmSb,cppGbAlarmPs,cppGbAlarmSb,generatorsAlarm,
										 thrustersAlarms,steeringGearAlarm,msbEsbAlarm,wtdHatchesAlarm,tanksBilgesAlarm,machnineryAlarm,Others,};
		
	enum pageNumber{page1 , page2 , page3 , page4 , page5 , page6 , page7 , page8 , page9 , page10,}; //page nomreleri
	
	char *text_page[] = {"Page 1  ","Page 2  ","Page 3  ","Page 4  ","Page 5  ","Page 6  ","Page 7  ","Page 8  ","Page 9  ","Page 10 ","Page 11 ","Page 12 ","Page 13 "}; //pagelerin adlari
	
	char *unit[] = {"    ","  C ","bar ","  % ","rpm "}; // unitlerin adlari
	//char *status[] = {"     RUN","      OK","   ALARM","    HIGH","    STOP","Fade out","      SF","     LOW",}; // statuslarin adlari
	char *status[] = {"      OK","   ALARM","    STOP","     RUN","     LOW","    HIGH","Fade out","      SF",}; // statuslarin adlari
	char str[70][30] = 0;

	char empty[] = {"     "};
	char emptyS[] = {"        "};
	
	//enum unitSelect{null_space,temprature,presure,percentage,revolution,};
	
	u8 unitSelect = 0;  // hansi unit in qoyulmasi lazim oldugun deyir
	
	
	
	char *space[]={"                        "}; // bolsuq lazim olan yerlere
		
		//NORICAN System
	
	const char *textGroup1[]	=	{"ME Overspeed Trip     PS","ME L.O.P Low Trip     PS","ME Emerg. Stop        PS","ME Running            PS", "                        "};
	const char *textGroup2[]	=	{"ME F.W Press.Low SLD  PS","ME HT Wat.Tmp.Hi SLD  PS"};
	const char *textGroup3[]	=	{"ME Fuel Oil Leakage   PS","ME F.O Diff.Prs.Hi    PS","ME L.O Diff.Prs.Hi    PS","ME T/C L.O.P Low      PS","ME F.O Press.Low      PS","ME L.O Press.Low      PS","ME L.O Temp.Hi        PS","ME Speed              PS",
															 "ME F.W Press.Low      PS","ME HT Wat.Tmp.Hi      PS","ME Sea Water Prs. Low PS","ME Ctrl .AC Pow.Fail  PS","ME Ctrl .DC Pow.Fail  PS","ME Overload           PS"};
	const char *textGroup4[]	=	{"ME Overspeed Trip     SB","ME L.O.P Low Trip     SB","ME Emerg. Stop        SB","ME Running            SB"};
	const char *textGroup5[]	=	{"ME F.W Press.Low SLD  SB","ME HT Wat.Tmp.Hi SLD  SB"};
	const char *textGroup6[]	=	{"ME Fuel Oil Leakage   SB","ME F.O Diff.Prs.Hi    SB","ME L.O Diff.Prs.Hi    SB","ME T/C L.O.P Low      SB","ME F.O Press.Low      SB","ME L.O Press.Low      SB","ME L.O Temp.Hi        SB","ME Speed              SB",
															 "ME F.W Press.Low      SB","ME HT Wat.Tmp.Hi      SB","ME Sea Water Prs. Low SB","ME Ctrl .AC Pow.Fail  SB","ME Ctrl .DC Pow.Fail  SB","ME Overload           SB"};
	const char *textGroup7[]	=	{"CPP Pump Auto Start   PS","CPP Hyd.Oil Temp.Hi   PS","CPP S.Tube Gra.Tk.Lo  PS","CPP S.Tube Cir.Tk.Lo  PS","CPP S.Tube Br.Tmp Hi  PS","CPP Hyd.Oil Prs.Low   PS","CPP Hyd.Oil Prs.Hi    PS","GB L.O.P Low SLD      PS",
															 "GB L.O.P Low          PS","GB L.O.P Low Trip     PS","CPP #1 Pump Run       PS","CPP #2 Pump Run       PS","GB L.O Temp.Hi        PS"};
	const char *textGroup8[]	=	{"CPP Pump Auto Start   SB","CPP Hyd.Oil Temp.Hi   SB","CPP S.Tube Gra.Tk.Lo  SB","CPP S.Tube Cir.Tk.Lo  SB","CPP S.Tube Br.Tmp Hi  SB","CPP Hyd.Oil Prs.Low   SB","CPP Hyd.Oil Prs.Hi    SB","GB L.O.P Low SLD      SB",
															 "GB L.O.P Low          SB","GB L.O.P Low Trip     SB","CPP #1 Pump Run       SB","CPP #2 Pump Run       SB","GB L.O Temp.Hi        SB"};
	const char *textGroup9[]	=	{"DG Running            PS","DG Overspeed Trip     PS","DG L.O.P Low Trip     PS","DG F.W Temp.Hi Trip   PS","DG Emerg.Stop         PS","DG Fail To Start      PS","DG F.O Leakage        PS","DG Water Lvl.Low      PS",
															 "DG L.O Temp.Hi        PS","DG L.O.P Low          PS","DG Jack Water Prs. Lo PS","DG F.O Press.Low      PS","DG F.W Temp.Hi        PS","DG Exh.Gas Temp.Hi    PS","DG Power Failure      PS","DG Running            SB",
															 "DG Overspeed Trip     SB","DG L.O.P Low Trip     SB","DG F.W Temp.Hi Trip   SB","DG Emerg.Stop         SB","DG Fail To Start      SB","DG F.O Leakage        SB","DG Water Lvl.Low      SB","DG L.O Temp.Hi        SB",
															 "DG L.O.P Low          SB","DG Jack Water Prs. Lo SB","DG F.O Press.Low      SB","DG F.W Temp.Hi        SB","DG Exh.Gas Temp.Hi    SB","DG Power Failure      SB","E.Gen Common Fault      ","E.Gen Running           "};
	const char *textGroup10[]	=	{"BT Analog Station Fail  ","BT Ctrl.Source Pow. Fail","BT Main Source Pow. Fail","BT Hyd Oil Lvl.Low      ","BT Hyd Oil Temp.Too Hi  ","BT Hyd.Oil Filter Clog. ","BT Min.Hyd.Oil.Prs.Low  ","BT Hyd.Motor Overload   ",
															 "ST Hyd Overload         ","ST Analog Station Fail  ","ST Ctrl.Source Pow. Fail","ST Main Source Pow. Fail","ST Oil Lvl.Low          ","ST Overcurrent          ","ST Hyd.Oil Filter Clog. ","ST Min.Hyd.Oil Prs.Low  ",
															 "ST Temp.>155 Deg C      ","ST Temp.>170 Deg C      ","ST Transf.Tmp>110 Deg C ","ST Transf.Tmp>150 Deg C ","BT Eng.Overspeed Trip   ","BT Eng.E.Stop           ","BT Eng.Run              ","BT Eng.Fuel Leakage     ",
															 "BT Eng.L.O.P Low        ","BT Eng.Water Temp. Hi   ","BT Eng.Water Lvl. Low   "};
	const char *textGroup11[]	=	{"SG 1 HP 1 coil. Fault   ","SG 1 HP 2 coil. Fault   ","SG 1 HP 1 Low oil LVl   ","SG 1 HP 2 Low oil LVl   ","SG 1 Warning Sys. Fail  ","SG 2 HP 1 coil. Fault   ","SG 2 HP 2 coil. Fault   ","SG 2 HP 1 Low oil LVl   ",
															 "SG 2 HP 2 Low oil LVl   ","SG 2 Warning Sys. Fail  ","SG Amplifier 1 Fault    ","SG Amplifier 2 Fault    ","SG Amplifier 3 Fault    ","SG Amplifier 4 Fault    ","SG Autopilot Fault      ","SG Autopilot Sys.1 Fail.",
															 "SG Autopilot Sys.2 Fail."};
	const char *textGroup12[]	=	{"MSB Volt Hi             ","MSB Volt Low            ","MSB Freq.Hi             ","MSB Freq.Low            ","MSB 440V Low Insulation ","MSB 230V Low Insulation ","MSB Preferential Trip   ","MSB Abnormal            ",
															 "ESB 440V Low Insulation ","ESB 230V Low Insulation ","MSB PLC Failure         ","#1 Batt.Char.24VDC Fail ","#2 Batt.Char.24VDC Fail ","#3 Batt.Char.24VDC Fail "};
	const char *textGroup13[]	=	{"ER AFT End WTD        PS","ER AFT End WTD        SB","ER Cement Compt. WTD  PS","ER Cement Compt. WTD  SB","Cement Compt. FWD WTD   ","M.Deck-ER Access WTD  PS","M.Deck-Sick Bay WTD   SB","M.Deck-SG Access WTH. PS",
															 "FWD-M.Deck Store WTH. PS","FWD-Bosun Store WTH.  SB"};
	const char *textGroup14[]	=	{"SG Thruster Tun. Lvl.Hi ","AFT Cofferdam Lvl.Hi  PS","AFT Cofferdam Lvl.Hi  SB","Shaft Tun.Lvl.Hi      PS","Shaft Tun.Lvl.Hi      SB","Shaft Tun.FWD Lvl. Hi PS","Shaft Tun.FWD Lvl. Hi SB","ER AFT Lvl.Hi         PS",
															 "ER AFT Lvl.Hi         SB","ER FWD Lvl.Hi         PS","ER FWD Lvl.Hi         SB","Bulk Rm.AFT Lvl. Hi   PS","Bulk Rm.AFT Lvl. Hi   SB","Cofferdam Lvl.Hi        ","BT Sew.Hold Tk Lvl.Hi   ","DB Void Lvl.Hi          ",
															 "Bt Rm.Lvl.Hi            ","Dirty Oil Tk.Lv.Hi      ","Bilge Hold.Tk.Lvl. Hi   ","#1 F.O Tk.Lvl.Hi      PS","#1 F.O Tk.Lvl.Hi      SB","#2 F.O Tk.Lvl.Hi      PS","#2 F.O Tk.Lvl.Hi      SB","#2 F.O Tk.Lvl.Hi      CT",
															 "#3 F.O Tk.Lvl.Hi      PS","#3 F.O Tk.Lvl.Hi      SB","#4 F.O Tk.Lvl.Hi      PS","#4 F.O Tk.Lvl.Hi      SB","#4 F.O Tk.Lvl.Hi      CT","#5 F.O Tk.Lvl.Hi      PS","#5 F.O Tk.Lvl.Hi      SB","F.O Day Tk.Lvl.Hi     PS",
															 "F.O Day Tk.Lvl.Hi     SB","ME L.O Store.Tk Hi    PS","Gen L.O Store.Tk.Hi   SB","Hyd.Oil Store.Tk.Hi   SB","Dirty Oil Tk.Lvl.Hi   PS","Bilge Hold.Tk.Lvl.Hi  SB","Sewage Hold.Tk.Lvl.Hi   ","Foam Tk.Lvl.Hi        SB",
															 "Detergent Tk.Lvl.Hi   PS","#1 Mud Tk.Lvl.Hi      CT","#2 Mud Tk.Lvl.Hi      CT","#1 Pot.Wat.Tk.Lvl.Hi  PS","#1 Pot.Wat.Tk.Lvl.Hi  SB","#2 Pot.Wat.Tk.Lvl.Hi  PS","#2 Pot.Wat.Tk.Lvl.Hi  SB","#1 DW & WB Tk.Lvl.Hi  CT",
															 "#2 DW & WB Tk.Lvl.Hi  PS","#2 DW & WB Tk.Lvl.Hi  SB","#3 DW & WB Tk.Lvl.Hi  PS","#3 DW & WB Tk.Lvl.Hi  SB","#4 DW & WB Tk.Lvl.Hi  PS","#4 DW & WB Tk.Lvl.Hi  SB","#5 DW & WB Tk.Lvl.Hi  PS","#5 DW & WB Tk.Lvl.Hi  SB"};
	const char *textGroup15[]	=	{"L.O Separator Fail      ","L.O Heater Alarm Fail   ","Light Column Fail       ","Main Supply Fail        ","Bkup. Supply Fail       ","CO2 System Power Fail   ","FIFI Common Fault       ","Fire Alarm              ",
															 "Main Fire Pump Start    ","F.O Day Tk.Lvl.Hi(2)  PS","F.O Day Tk.Lvl.Low    PS","F.O Day Tk.Lvl.Hi(2)  SB","F.O Day Tk.Lvl.Low    SB","Starting Air Press. Low ","F.O Purifier Fail       ","Overflow Tk.#4 Lvl.Hi CT",
															 "F.O Day Tk.E.Gen.Lv.Hi  ","F.O Day Tk.E.Gen.Lv.Low ","F.O Day Tk.BT.Lvl.Hi    ","F.O Day Tk.BT.Lvl.Low   ","Overflow Tk.#4 Lvl.Hi PS","Overflow Tk.#4 Lvl.Hi SB","Sewage Treat.Tk.Lv.Hi   ","Elec.Anti Foul.Syst.Abn ",
															 "Work.Air Comp.Com. Alarm","Freezing Rm.Repose Alarm","Man Lock.In Alarm       ","Hyd.Pp.Tow.Stop Fail 1  ","Hyd.Pp.Tow.Stop Fail 2  ","Oily Wat.Sep.15 PPM Alm ","#1 F.O Transf.Pump Run  ","#2 F.O Transf.Pump Run  ",
															 "ME S.W Stby.Pump Run  PS","ME S.W Stby.Pump Run  SB","ME F.W Stby.Pump Run  PS","ME F.W Stby.Pump Run  SB","ME L.O Stby.Pump Run  PS","ME L.O Stby.Pump Run  SB","ME F.O Stby.Pump Run  PS","ME F.O Stby.Pump Run  SB",
															 "ME Cl.Stby.L.O.Pp.Run PS","ME Cl.Stby.L.O.Pp.Run SB","Air-Con.Fan Run         ","Air-Con.Fan Abnormal    ","Air-Con. #1 Comp. Run   ","Air-Con. #1 Comp. Abn.  ","Air-Con. #2 Comp. Run   ","Air-Con. #2 Comp. Abn.  ",
															 "#1 ER Vent.Fan Trip     ","#2 ER Vent.Fan Trip     ","#1 Ref.Comp.Run         ","#1 Ref.Comp. Abnormal   ","#2 Ref.Comp.Run         ","#2 Ref.Comp. Abnormal   ","Sh.Gen #1 Wind. Tmp.H PS","Sh.Gen #2 Wind. Tmp.H PS",
															 "Sh.Gen #3 Wind. Tmp.H PS","Sh.Gen F.Bear. Tmp.H  PS","#1 M.Air Compressor Run ","#1 M.Air Comp.Com. Fault","#2 M.Air Compressor Run ","#2 M.Air Comp.Com. Fault","Sh.Gen R.Bear.Tmp.H   PS","Sh.Gen #1 Wind. Tmp.H SB",
															 "Sh.Gen #2 Wind. Tmp.H SB","Sh.Gen #3 Wind. Tmp.H SB","Sh.Gen F.Bear.Tmp.H   SB","Sh.Gen R.Bear.Tmp.H   SB"};
	
const char **textChannels[]	=	{textGroup1, textGroup2, textGroup3, textGroup4, textGroup5, textGroup6, textGroup7, textGroup8, textGroup9, textGroup10, textGroup11, textGroup12, textGroup13, textGroup14, textGroup15};

size_t	textSizes[]	=	{sizeof(textGroup1) / sizeof(textGroup1[0]), sizeof(textGroup2) / sizeof(textGroup2[0]), sizeof(textGroup3) / sizeof(textGroup3[0]), sizeof(textGroup4) / sizeof(textGroup4[0]), sizeof(textGroup5) / sizeof(textGroup5[0]), sizeof(textGroup6) / sizeof(textGroup6[0]), sizeof(textGroup7) / sizeof(textGroup7[0]), sizeof(textGroup8) / sizeof(textGroup8[0]), sizeof(textGroup9) / sizeof(textGroup9[0]), sizeof(textGroup10) / sizeof(textGroup10[0]), sizeof(textGroup11) / sizeof(textGroup11[0]), sizeof(textGroup12) / sizeof(textGroup12[0]), sizeof(textGroup13) / sizeof(textGroup13[0]), sizeof(textGroup14) / sizeof(textGroup14[0]), sizeof(textGroup15) / sizeof(textGroup15[0])};
		//ME Shutdown - PS				 	 //OK                                                                      
		//ME Slowdown - PS				 	 //OK
		//ME Alarm - PS		         	 //OK
		//ME Shutdown - SB		     	 //OK
		//ME Slowdown - SB         	 //OK
		//ME Alarm - SB            	 //OK
		//CPP / GB Alarm - PS        //OK
		//CPP / GB Alarm - SB        //OK
		//Generators Alarm           //OK
		//thrusters Alarms           //OK                                                                            
		//Steering Gear Alarm        //OK
		//MSB/ESB Alarm              //OK
		//WTD / Hatches Alarm        //OK
		//Tanks / Bilges Alarm       //OK
		//Machninery Alarm           //OK
		//Others                     //OK
	/*
	char *textChannels16[]={"                        ","ME Start.Air Prs. Low PS","ME Ctrl.Air Prs. Low  PS","CPP Hyd.Oil Tk.Lv. Hi PS","DG Ref.Junction       PS","BT Engine Overload      ","                        ","BT Oil Press.Low        ",
													"BT Hyd.Standby Start    ","ST Main Motor Temp. Hi  ","Hyd.Pump 1 Phase Fail   ","Hyd.Pump 1 Cool. Alarm  ","Hyd.Pump 1 On Service   ","Hyd.Pump 2 Overload     ","Hyd.Pump 2 Phase Fail   ","Hyd.Pump  Cool. Alarm   ",
													"Hyd.Pump 2 On Service   ","                        ","ME Start.Air Prs. Low SB","ME Ctrl.Air Prs. Low  SB","CPP Hyd.Oil Tk.Lv.Hi  SB","DG Ref.Junction       SB","E.Gen L.O.P Low         ","E.Gen L.O Temp.Hi       ",
													"E.Gen F.W Press.Low     ","E.Gen F.W Temp.Hi       ","E.Gen F.O Press.Low     ","Clutch 1 Engaged        ","Clutch 2 Engaged        ","F.W Exp.Tk.Lvl.Low      ","M.Deck-SG Access WTD  PS","ME Recess Lvl.Hi      PS",
													"ME Recess Lvl.Hi      SB","GB Recess Lvl.Hi      PS","GB Recess Lvl.Hi      SB","Sludge Tk.Lvl.Hi        ","F.O Serv.Tk.Lvl.Hi    PS","F.O Serv.Tk.Lvl.Low   PS","F.O Serv.Tk.Lvl.Hi    SB","F.O Serv.Tk.Lvl.Low   SB",
													"BT F.O Tk.Lvl.Hi        ","BT F.O Tk.Lvl.Low       ","Imp.Curr.Cat.Prot.Abn.  ","Air-Con.Comp. Alarm-ECR ","Air-Con.Comp. Alarm-WH  ","Freezing Rm.Response Sig","ECR Air-Con Run         "};
	
		u8 textChannels16Lenght = 47;
			*/										
																 
																 
	char *text[100];
		

		
	char string0[100] = 0;
	char string1[100] = 0;	
	char string2[100] = 0;	

	
	char string[10][50] = 0;
		
	u8 canReg[4];
												 
	u8 date[]={0x16,0x03,0x12,0x17,0x3b,0x20}; // tarix deyerleri
	
	u8 dateRead[5][2];  ///HMI terfinden set olan tarix ve saat;
	u8 dateSet[6];      ///dataRead daki reqemleri Rtc_set_time funksiyasiya uygun hala getrimke ucun
	


	

	
	u8 ok[2];
	u8 channels[2];
	u16 channelsSum=0;
	u8 rightLeftPage[2];
	u8 rightLeftPageA[2];
	u8 pageLimit=0;  			// seyfe limitin belirleyir
	u8 pageLimitA=0;  			// seyfe limitin belirleyir
	u8 pageLimitCounterA=0;  			// seyfe limitin belirleyir
	u8 channelLimit=0; 		// son seyfede olan signal sayi
	u8 i=0,j=0,alarmOrder=0;				//alarmOrder deyiskeni yuzdene alarmi surusduren counterdi
	int m=0 , mm=0, mmm=0, mmmm =0, mnn = 0;
	int signalGroup=0, signalOrder=0;
	u8 alarmState=0;
	u8 testSend[] = {1,2};
	u16 alarmStartEnd[2];
	u8 alarmStartEnd8bit[4];
	
	T0_Init();						//定时器0初始化
	//Baud rate:125K{0x3F,0x40,0x72,0x00},250K{0x1F,0x40,0x72,0x00},500K{0x0F,0x40,0x72,0x00},1M{0x07,0x40,0x72,0x00}
	canReg[0] = 0x3f;
	canReg[1] = 0x40;
	canReg[2] = 0x72;
	canReg[3] = 0x00;
	CanBusInit(canReg);
	rtc_init();
	EA = 1;
	StartTimer(0,50);
	StartTimer(1,1000);
	StartTimer(2,2000);
	StartTimer(3,1000);
	StartTimer(4,5000);
	//Rtc_set_time(date);
	

	
	while(1){
		
		if(GetTimeOutFlag(3))
		{
			CanTx(0x054,0,1, testSend); //can dataa gonderme ilk id di ikinci 0 standar di eger 0x80 olsa idi extended olacaqdi. 4 lentght di. testsend yazanda dataa di.
			StartTimer(3,1000);
		}

		
	
		if(GetTimeOutFlag(0))
		{
			//canRxTreat();           //receive example
			rdtime();
			//StartTimer(0,10);
		}
		CanErrorReset();
		
		canRxTreat();
		
		read_dgus_vp(0x1010,&ok,1);  // ok knopkasina basildi yoxsa yox;
		//write_dgus_vp(0x1012,&ok,1); // ok u gormek ucun yazdigim setir
			
		if((ok[1])>= 1) //ok a basildi yoxsa yox ona baxir
		{
			read_dgus_vp(0x1000,&dateRead[0][0],1);		dateSet[0] = dateRead[0][1];
			read_dgus_vp(0x1002,&dateRead[1][0],1);		dateSet[1] = dateRead[1][1];
			read_dgus_vp(0x1004,&dateRead[2][0],1);		dateSet[2] = dateRead[2][1];
			read_dgus_vp(0x1006,&dateRead[3][0],1);		dateSet[3] = dateRead[3][1];
			read_dgus_vp(0x1008,&dateRead[4][0],1);		dateSet[4] = dateRead[4][1];
			dateSet[5]=0;
			Rtc_set_time(dateSet); // ayarlanan datelare bu funcksiyalara yolla
			write_dgus_vp(0x1010,(u8*)&zero,1); // ok a basildigi ucun sifirla
		}
			
		read_dgus_vp(0x1300,&channels,1);  //  channels knopkalarin oxu
		
		channelsSum = ((u16)channels[1]) + ((u16)channels[0] << 8);   // onlari normal reqem haklina getir
			
		for(countTheChannels = 0;countTheChannels < 15; countTheChannels++){	//kanallari say
			if(channelsSum == channelNumbers[countTheChannels]) //kanal knopkasina a basildi yoxsa yox ona baxir
			{
				*text = *textChannels[countTheChannels];
				channelState = countTheChannels;
				write_dgus_vp(0x1300,(u8*)&zero,1); // knopka deyerin 0 ele
				pageTextWriteFlag = 1;
				pageLimit=  textChannelsLengths[countTheChannels]/channelPerPage;  			//page sayi 1
				channelLimit=textChannelsLengths[countTheChannels]%channelPerPage;      //son seyfede olan signal sayi
				write_dgus_vp(0x1012,(u8*)&zero,1); ////page 1 e qayid
				if(channelLimit == 0)
				{
					pageLimit--;
				}
			}
		}

		read_dgus_vp(0x1012,&rightLeftPage,1);  //sola yada saga basildigin oxu
		
		if(rightLeftPage[1] != pageState) // eger sola yada saga basildisa
		{
			if(rightLeftPage[1] >= pageLimit)
			{
				write_dgus_vp(0x1012,(u8*)&zero+pageLimit*2,1);
			}
			pageState = rightLeftPage[1];
			pageTextWriteFlag = 1;
		}
		
		if(pageTextWriteFlag == 1){
			write_dgus_vp(0x10C8,text_page[pageState],5); //pageni registere yaz
			if((rightLeftPage[1] == pageLimit) && (channelLimit != 0))  //son seyfededise ve son seyfede kanal varsa
			{
				for(i=0;i<channelLimit;i++){      //0 dan son seyfedeki kanal sayina qeder saydir    
					write_dgus_vp((0x1018 + 12*i),(text[0]+(pageState)*25*channelPerPage+25*i),12); //son seyfe ucun channeleri yazdir
					cc = i+pageState*channelPerPage;
					if((channelState == meAlarmPs && (cc==6 || cc==9)) || (channelState == meAlarmSb && (cc==6 || cc==9)) || (channelState == generatorsAlarm && (cc==8 || cc==12 || cc==13 || cc==23 || cc==27 || cc==28)) || (channelState == machnineryAlarm && (cc==54 || cc==55 || cc==56 || cc==57 || cc==62 || cc==63 || cc==64 || cc==65 || cc==66 || cc==67)))   // temp yazdiran if
					{
						unitSelect = 1; // temprature demekdi 1
					}
					else if((channelState == meAlarmPs && cc==7) || (channelState == meAlarmSb && cc==7)) // rpm yazdiran if
					{
						unitSelect = 4; // rpm demekdi 4
					}
					else if((channelState == tanksBilgesAlarm) && cc > 18) // % yazdiran if
					{
						unitSelect = 3; // % demekdi 3
					}
					else if((channelState == meAlarmPs && (cc==4 || cc==5 || cc==8 || cc==10)) || (channelState == meAlarmSb && (cc==4 || cc==5 || cc==8 || cc==10)) || (channelState == cppGbAlarmPs && cc==8) || (channelState == cppGbAlarmSb && cc==8) || (channelState == generatorsAlarm && (cc==9 || cc==10 || cc==11 || cc==24 || cc==25 || cc==26)) || (channelState == machnineryAlarm && cc==13)) // bar yazdiran if
					{
						unitSelect = 2; // bar demekdi 2
					}
					else
					{
						unitSelect = 0; // eger unit qoyumayacaqsa 0
					}
					write_dgus_vp((0x1078+2*i),unit[0] + 5*unitSelect,2);
				}
				for(i=channelLimit;i<channelPerPage;i++) // geri qalan yerlere bosluqlarin yazilmasi
				{
					write_dgus_vp((0x1018 + 12*i),(space[0]),12); // 12 charda yazilan karakterlerin sayinin 2 ye bolunmesi
					write_dgus_vp((0x1078+2*i),unit[0],2);
				}
			}
			else
			{
				for(i=0;i<channelPerPage;i++){  // son seyfede deyilikse yaz hamisin rahat kanallarin
					write_dgus_vp((0x1018 + 12*i),(text[0]+(pageState)*25*channelPerPage+25*i),12);	// 12 charda yazilan karakterlerin sayinin 2 ye bolunmesi
					cc = i+pageState*channelPerPage;
					if((channelState == meAlarmPs && (cc==6 || cc==9)) || (channelState == meAlarmSb && (cc==6 || cc==9)) || (channelState == generatorsAlarm && (cc==8 || cc==12 || cc==13 || cc==23 || cc==27 || cc==28)) || (channelState == machnineryAlarm && (cc==54 || cc==55 || cc==56 || cc==57 || cc==62 || cc==63 || cc==64 || cc==65 || cc==66 || cc==67)))   // temp yazdiran if
					{
						unitSelect = 1; // temprature demekdi 1
					}
					else if((channelState == meAlarmPs && cc==7) || (channelState == meAlarmSb && cc==7)) // rpm yazdiran if
					{
						unitSelect = 4; // rpm demekdi 4
					}
					else if(channelState == tanksBilgesAlarm && cc > 18) // % yazdiran if
					{
						unitSelect = 3; // rpm demekdi 3
					}
					else if((channelState == meAlarmPs && (cc==4 || cc==5 || cc==8 || cc==10)) || (channelState == meAlarmSb && (cc==4 || cc==5 || cc==8 || cc==10)) || (channelState == cppGbAlarmPs && cc==8) || (channelState == cppGbAlarmSb && cc==8) || (channelState == generatorsAlarm && (cc==9 || cc==10 || cc==11 || cc==24 || cc==25 || cc==26)) || (channelState == machnineryAlarm && cc==13)) // bar yazdiran if
					{
						unitSelect = 2; // bar demekdi 2
					}
				else
				{
					unitSelect = 0; // eger unit qoyumayacaqsa 0
				}
				write_dgus_vp((0x1078+2*i),unit[0] + 5*unitSelect,2);	
			}
			}
			pageTextWriteFlag = 0;
		}
			
		for(countTheChannels = 0;countTheChannels < 15; countTheChannels++){	//kanallari axtarmaq ucun sayir
			if(channelState == countTheChannels){	//hansi kanalda oldugun axtarir
				if((rightLeftPage[1] == pageLimit) && (channelLimit != 0)){  //son seyfededise ve son seyfede kanal varsa
					dynamicPageLimit = channelLimit;
				}
				else{	//orta seyfelerdedise veya son dolu seyfededise
					dynamicPageLimit = 8;
				}
				for(i=0;i<8;i++){	//8 eded signal ucun loop
					if((analogIDsOrder[countTheChannels][i+pageState*8] == 1) && (i<dynamicPageLimit)){//bura hansilarinin analog oldugun gosterir ve eger limiti kecmeyibse
						if(countTheChannels == 13){
							sprintf(str[i+pageState*8],"%.2f",(group[countTheChannels][i+pageState*8] / group_13[i+pageState*8] * group_13_100[i+pageState*8]));//bura analoga cevirir
						}
						else{
							sprintf(str[i+pageState*8],"%.2f",group[countTheChannels][i+pageState*8]);//bura analoga cevirir
						}
						write_dgus_vp((0x1088+i*4),str[i+pageState*8],4);	//yazdir
					}
					else{
						write_dgus_vp((0x1088+i*4),empty,4); //analog deyilse bosluq yazdir
					}
					if(i<dynamicPageLimit){ //kanal limiti yoxlanilir
						
						if(statusOrder[countTheChannels][i+pageState*8]==0){// OK ALARM yazir
							if(alarmOn[channelState][i+pageState*8] == 1){	//eger alarm olubsa
								//write_dgus_vp((0x10A8+i*4),status[0]+18,4);	// ALARM 
								write_dgus_vp((0x10A8+i*4),status[0]+9,4);	// ALARM 
							}
							else{
								//write_dgus_vp((0x10A8+i*4),status[0]+9,4);	//OK
								write_dgus_vp((0x10A8+i*4),status[0],4);	//OK
							}
						}
						else if(statusOrder[countTheChannels][i+pageState*8]==1){// OK ALARM yazir
							if(alarmOn[channelState][i+pageState*8] == 1){	//eger alarm olubsa
								//write_dgus_vp((0x10A8+i*4),status[0]+9,4);	//OK
								write_dgus_vp((0x10A8+i*4),status[0],4);	//OK
							}
							else{
								//write_dgus_vp((0x10A8+i*4),status[0]+18,4);	// ALARM 
								write_dgus_vp((0x10A8+i*4),status[0]+9,4);	// ALARM 
							}
						}
						else if(statusOrder[countTheChannels][i+pageState*8]==2){
							if(alarmOn[channelState][i+pageState*8] == 1){	//eger alarm olubsa
								//write_dgus_vp((0x10A8+i*4),status[0],4);	// RAN
								write_dgus_vp((0x10A8+i*4),status[0]+27,4);	// RAN
							}
							else{
								//write_dgus_vp((0x10A8+i*4),status[0]+36,4); // STOP //onuda unutmaki bezi run stop siqnallarinin yeri deyisikdi
								write_dgus_vp((0x10A8+i*4),status[0]+18,4); // STOP //onuda unutmaki bezi run stop siqnallarinin yeri deyisikdi
							}
						}
						else if(statusOrder[countTheChannels][i+pageState*8]==3){
							if(alarmOn[channelState][i+pageState*8] == 1){	//eger alarm olubsa
								//write_dgus_vp((0x10A8+i*4),status[0]+36,4); // STOP //onuda unutmaki bezi run stop siqnallarinin yeri deyisikdi
								write_dgus_vp((0x10A8+i*4),status[0]+18,4); // STOP //onuda unutmaki bezi run stop siqnallarinin yeri deyisikdi
							}
							else{
								//write_dgus_vp((0x10A8+i*4),status[0],4);	// RAN
								write_dgus_vp((0x10A8+i*4),status[0]+27,4);	// RAN
							}
						}
						else if(statusOrder[countTheChannels][i+pageState*8]==4){
							if(alarmOn[channelState][i+pageState*8] == 1){ //eger alarm olubsa
								//write_dgus_vp((0x10A8+i*4),status[0]+27,4);	// HIGH
								write_dgus_vp((0x10A8+i*4),status[0]+45,4);	// HIGH
							}
							else{
								//write_dgus_vp((0x10A8+i*4),status[0]+63,4);	// LOW
								write_dgus_vp((0x10A8+i*4),status[0]+36,4);	// LOW
							}
						}
						else if(statusOrder[countTheChannels][i+pageState*8]==5){
							if(alarmOn[channelState][i+pageState*8] == 1){ //eger alarm olubsa
								//write_dgus_vp((0x10A8+i*4),status[0]+63,4);	// LOW
								write_dgus_vp((0x10A8+i*4),status[0]+36,4);	// LOW
							}
							else{
								//write_dgus_vp((0x10A8+i*4),status[0]+27,4);	// HIGH
								write_dgus_vp((0x10A8+i*4),status[0]+45,4);	// HIGH
							}
						}
					}
					else
					{
						write_dgus_vp((0x10A8+i*4),emptyS,4);
					}
				}
			}
		}
		
		
		
		read_dgus_vp(0x1016,&rightLeftPageA,1);

		
		
		
		if(rightLeftPageA[1] != pageStateA){ // eger sola yada saga basildisa alarm seyfesinde
			
			StartTimer(1,1000);	// sigranl teleb eleme timerin baslat eger knopkaya basilibsa.
			
			for(mmmm = 0; mmmm < 10; mmmm++){
				idNumber[mmmm] = 0;
				idOrder[mmmm] = 0;
				sprintf(string1,"                  ");
				write_dgus_vp(0x1500+18*mmmm,string1,9);
				sprintf(string1,"      ");
				write_dgus_vp(0x1310+mmmm*4,string1,4);
				sprintf(string1,"                        ");
				write_dgus_vp(0x1339+12*mmmm,string1,12);
			}

			/*
			alarmStartEnd[0] = pageStateA * 10;	//seyfeni deyisdikce istenen seyfe sayisi deyisir baslangic
			alarmStartEnd[1] = pageStateA * 10 + 10;	//seyfeni deyisdikce istenen seyfe sayisi deyisir bitis
			
			alarmStartEnd8bit[0] = (u8)alarmStartEnd[0];
			alarmStartEnd8bit[1] = (u8)(alarmStartEnd[0] >> 8);
			alarmStartEnd8bit[2] = (u8)alarmStartEnd[1];
			alarmStartEnd8bit[3] = (u8)(alarmStartEnd[1] >> 8);
			CanTx(0x660,0,4, alarmStartEnd8bit); //can dataa gonderme ilk id di ikinci 0 standar di eger 0x80 olsa idi extended olacaqdi. 4 lentght di. testsend yazanda dataa di.
			*/			
			
			/*
			if(rightLeftPageA[1] >= pageLimitA){
				write_dgus_vp(0x1016,(u8*)&zero+pageLimitA*2,1);
			}
			*/
			//write_dgus_vp(0x1016,(u8*)&zero+pageLimitA*2,1);
			
			/*
			for(mmmm = 0; mmmm < 1000; mmmm++){
			}
			*/
			
			pageStateA = rightLeftPageA[1];
			
			/*
			alarmStartEnd[0] = pageStateA * 10;	//seyfeni deyisdikce istenen seyfe sayisi deyisir baslangic
			alarmStartEnd[1] = pageStateA * 10 + 10;	//seyfeni deyisdikce istenen seyfe sayisi deyisir bitis
			
			alarmStartEnd8bit[0] = (u8)alarmStartEnd[0];
			alarmStartEnd8bit[1] = (u8)(alarmStartEnd[0] >> 8);
			alarmStartEnd8bit[2] = (u8)alarmStartEnd[1];
			alarmStartEnd8bit[3] = (u8)(alarmStartEnd[1] >> 8);
			CanTx(0x660,0,4, alarmStartEnd8bit); //can dataa gonderme ilk id di ikinci 0 standar di eger 0x80 olsa idi extended olacaqdi. 4 lentght di. testsend yazanda dataa di.
			*/
			pageTextWriteFlagA = 1;
		}
		


		
		write_dgus_vp(0x15B4,text_page[pageStateA],4);
			
		if ((stationAlarm[0] == 1) || (stationAlarm[1] == 1) || (stationAlarm[2] == 1) || (stationAlarm[3] == 1)){
			alarmState = 1;
		}
		else{
			alarmState = 0;
		}
		
		
		read_dgus_vp(0x1102,&mute,1);
			
		if((mute[1] == 0) && (alarmState == 1))
		{
			if(GetTimeOutFlag(2))
				{
					system_paly_audio(25);
					StartTimer(2,500);
				}
		}
		
					if(idOrder[mnn] != pageStateA*10 + mnn){
							
				alarmStartEnd[0] = pageStateA * 10 + mnn;	//seyfeni deyisdikce istenen seyfe sayisi deyisir baslangic
				alarmStartEnd[1] = pageStateA * 10 + mnn + 1;	//seyfeni deyisdikce istenen seyfe sayisi deyisir bitis
							
				alarmStartEnd8bit[0] = (u8)alarmStartEnd[0];
				alarmStartEnd8bit[1] = (u8)(alarmStartEnd[0] >> 8);
				alarmStartEnd8bit[2] = (u8)alarmStartEnd[1];
				alarmStartEnd8bit[3] = (u8)(alarmStartEnd[1] >> 8);
				CanTx(0x660,0,4, alarmStartEnd8bit); //can dataa gonderme ilk id di ikinci 0 standar di eger 0x80 olsa idi extended olacaqdi. 4 lentght di. testsend yazanda dataa di.
				mnn++;
			}
			else{
				mnn++;
			}
			
			if(mnn >= 10){
				mnn = 0;
			}
		
			
		//if(pageTextWriteFlagA == 1){
			
			for(m=0;m<10;m++)
			{
				write_dgus_vp((0x1500 + m*18),"                  ",9);
				
				sprintf(string1,"%d/%d/",idNumber[m],idOrder[m]);
				
				//bu variyanti yaz yoxla
				/*
				if(((pageLimitCounterA%10 == 0) && (pageLimitCounterA != 0)) || (rightLeftPageA[1] != pageLimitA) || (m < pageLimitCounterA%10))
				{
					write_dgus_vp(0x1500+18*m,string1,9);
					write_dgus_vp(0x1339+12*m,textChannels[signalChannel[m+10*rightLeftPageA[1]]][alarmNumber[m+10*rightLeftPageA[1]]],12);
					sprintf(string[m],"%d",idNumber[m]);
					write_dgus_vp(0x1310+m*4,string[m],3);
				}
				else
				{
					sprintf(string1,"                  ");
					write_dgus_vp(0x1500+18*m,string1,9);
					write_dgus_vp(0x1339+12*m,textChannels[0][4],12);
					sprintf(string[m],"  ");
					write_dgus_vp(0x1310+m*4,string[m],3);
				}
				*/
				
					write_dgus_vp(0x1500+18*m,string1,9);
					
					for(mm =0; mm < 15; mm++){
						for(mmm =0; mmm < textChannelsLengths[mm]; mmm++){
							if(textChannelsID[mm][mmm] == idNumber[m]){
								write_dgus_vp(0x1339+12*m,textChannels[mm][mmm],12);
							}
						}
					}
					
					//write_dgus_vp(0x1339+12*m,textChannels[signalChannel[m+10*rightLeftPageA[1]]][alarmNumber[m+10*rightLeftPageA[1]]],12);
					sprintf(string[m],"%d",idNumber[m]);
					//sprintf(string[m],"%d",test[0]);
					write_dgus_vp(0x1310+m*4,string[m],4);
					
					
					//write_dgus_vp(0x2000,num,10);
					
					
					//if(GetTimeOutFlag(1)){
					/*
						if(idOrder[m] != pageStateA*10 + m){
							
							alarmStartEnd[0] = pageStateA * 10 + m;	//seyfeni deyisdikce istenen seyfe sayisi deyisir baslangic
							alarmStartEnd[1] = pageStateA * 10 + m + 1;	//seyfeni deyisdikce istenen seyfe sayisi deyisir bitis
							
							alarmStartEnd8bit[0] = (u8)alarmStartEnd[0];
							alarmStartEnd8bit[1] = (u8)(alarmStartEnd[0] >> 8);
							alarmStartEnd8bit[2] = (u8)alarmStartEnd[1];
							alarmStartEnd8bit[3] = (u8)(alarmStartEnd[1] >> 8);
							CanTx(0x660,0,4, alarmStartEnd8bit); //can dataa gonderme ilk id di ikinci 0 standar di eger 0x80 olsa idi extended olacaqdi. 4 lentght di. testsend yazanda dataa di.
						}
						*/
					//}
					
				
			}
		//pageTextWriteFlagA = 0;
		//}
			
			

		

	}
}


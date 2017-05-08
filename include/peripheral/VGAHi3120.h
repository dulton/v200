
#ifndef __VGA_HI3120_H__

#define __VGA_HI3120_H__



#define HI3120_SET_BRIGHT 					0x20	/* 设置亮度*/
#define HI3120_SET_CONTRAST				0x21  /* 设置对比度	*/
#define HI3120_SET_HUE						0x22   /*设置色度*/
#define HI3120_SET_SAT						0x23  /*设置饱和度*/
#define HI3120_INIT							0x24  /*设置3120 初始化*/
#define HI3120_SETNORM						0x25  /*设置显示模式*/
#define HI3120_HV							0x26   /* 设置显示显示尺寸和显示位置 */


// 设置显示模式
#define	MODE_PAL_TO_800x600P60			0    /*PAL 输入 输出 800 X600 	*/
#define	MODE_NTSC_TO_800x600P60			1   /*NTSC 输入输出800X600	*/
#define 	MODE_PAL_TO_1024x768P60			2   /*pal 输入 输出1024 X 768	*/
#define 	MODE_NTSC_TO_1024x768P60		3   /*NTSC 输入输出1024X768	*/
 #define	MODE_PAL_TO_1366x768P60			4  /*pal 输入 输出1366 X768	*/
#define 	MODE_NTSC_TO_1366x768P60		5  /*PAL 输入输出1366x 768	*/
#define 	MODE_TEST_COLORBAR				6  /* 测试模式	*/


typedef struct 
{
	unsigned int 	m_u32X; 	/*设置显示的x 起点坐标 */
	unsigned int 	m_u32Y; /*设置显示 Y起点坐标*/
	unsigned char m_u32Bri; /*设置亮度1- 63*/
	unsigned char m_u32Con;/* 设置对比度1- 63 */
	unsigned char m_u32Hue; /*设置色度1-63*/
	unsigned char m_u32Sat; /* 设置饱和度 1- 63*/
	unsigned char m_u32Mode; /* 设置显示模式 0--5 */
	
}STRCT_HI3120_SET;



	class  VGA_HI3120 
	{
		

		private:

				int m_n3120Fd;
				
				static VGA_HI3120 *m_pInstance;
		public:

				VGA_HI3120();
				~VGA_HI3120();

				static VGA_HI3120* Instance();

				int SetHi3120WorkMode(int cmd, STRCT_HI3120_SET  ModeSet);

	};

#endif 



#ifndef __VGA_NVP5000_H__

#define __VGA_NVP5000_H__

#define _USE_27Mhz_

#define	NORMSET		0x10	//制式
#define	RESOLUTION		0x11	//分辨率
#define   ANALON			0x12

typedef struct 
{
	int 	m_32X; 	/*设置显示的x 起点坐标 */
	int 	m_32Y; /*设置显示 Y起点坐标*/
	int	m_8Bri; /*设置亮度*/
	int 	m_8Con;/* 设置对比度 */
	int 	m_8Hue; /*设置色度*/
	int 	m_8Sat; /* 设置饱和度 */
	char m_8Norm; /* 设置制式PAL(0) NTSC(1)*/
	char	m_8Resoult;/*设置分辨率800*600(0) 1024*768(1) 1280*1024(2)*/
	
}NVP5000_SET;

	class  VGA_NVP5000 
	{
		

		private:

				int nvp5000Fd;
				
				static VGA_NVP5000 *m_pInstance;
		public:

				VGA_NVP5000();
				~VGA_NVP5000();

				static VGA_NVP5000* Instance();
				int SetNVP5000WorkMode(int cmd, NVP5000_SET  ModeSet);

	};

#endif 


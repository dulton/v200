#ifndef __MT9D131_H__

#define __MT9D131_H__

#define MT_DC_SET_BRIGHT          0x40    /* bright£¬value_range:0~255 */
//#define MT_DC_SET_HUE             0x08 
#define MT_DC_SET_SATURATION      0x09    /* saturation£¬value_range£º0~255 */
//#define MT_DC_SET_SATURATION      0x09    /* saturation£¬value_range£º0~128 */
#define MT_DC_SET_CONTRAST        0x41
#define MT_DC_SET_BLUEPLUS        0x37  
#define MT_DC_SET_REDPLUS         0x36
#define MT_DC_SET_GREENPLUS       0x45
#define MT_DC_SET_BLACKOUTPUT	  0x39    /*0x08 µ¥É«£¬0x00 ²ÊÉ«*/
#define MT_DC_SET_MIRROR          0x03    /* 0x04 Normal ,0x01->V_MIRROR,
                                          0x02->H_MIRROR,0x03 HV_MIRROR */
#define MT_DC_SET_POWERFREQ       0x4
class MT9d131
{
	

	private:

			int mt9d131Fd;
			
			static MT9d131 *m_pInstance;
	public:

			MT9d131();
			~MT9d131();
			static MT9d131* Instance();
			int fd;
			int WriteMt9d131Register(int type,int value);
			int ReadMt9d131Register(int type,int *value);
			

};

#endif 



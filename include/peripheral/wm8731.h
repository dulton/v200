#ifndef __WM8731__H
#define __WM8731__H
#define Magic_WM8731            'F'
#define SELECT_WM8731_LINEIN_OR_MICIN		_IOW(Magic_WM8731,0,unsigned char)
#define SET_WM8731_LINEIN_VOLUM 				_IOW(Magic_WM8731,1,unsigned char)
#define SET_WM8731_INPUT_MUTE		       	_IOW(Magic_WM8731,2,unsigned char)
#define SET_WM8731_DACOUT_MUTE       	       	_IOW(Magic_WM8731,3,unsigned char)
class WM8731
{

private:
	
	int 	m_n32Fd;
	
	static WM8731   *m_pInstance;
	
public:

	WM8731();
	
	~WM8731();

	static WM8731*  Instance();

	int WriteWm8731Register(int type,int value);
};

#endif


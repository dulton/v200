

#ifndef _INPUT_DEVICE_H__
#define _INPUT_DEVICE_H__

void* GetMouseEvent(void *arg);


int CreateMouseEventThread();


typedef struct
{

	unsigned int	m_uMouseExist;
	unsigned int    m_uLeftRel;
	unsigned int 	m_uRightRel;
	unsigned int 	m_uWheelRel;
	unsigned int	m_uXDir;
	unsigned int	m_uYDir;

} MouseEvent;


class MouseInputEvent
{

	private :
		static  MouseInputEvent*m_pInstance;

		MouseEvent			m_MouseEvent;

		int  ProbeMouseExist();

		void HandleMouseData(unsigned char *data, int len);


	public:

		MouseInputEvent();
		~MouseInputEvent();

		static MouseInputEvent*Instance();

		int HandleMoudeEvent();

		int CreateMouseEventThread();

		int GetMouseInputEvent(MouseEvent *event);


};


#endif 


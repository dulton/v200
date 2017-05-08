
#ifndef 	__GUI_TYPES_H
#define 	__GUI_TYPES_H


#include "hi_type.h"
#include "common.h"
#include <linux/fb.h>

#define MENU_STRING_OFFSET		5

//定义每个页面的显示的页面属性的约定.
#define MAX_GUI_PAGES			64	// per session
#define MAX_GUI_PANELS			32	// per page
#define MAX_GUI_EVENTS 			32	// per object
#define MAX_GUI_BITMAPS 			32	// per page
#define MAX_GUI_COMMANDS 		720	// per session
#define MAX_GUI_MENUS			64	// per page
#define MAX_GUI_MENUITEMS		64	// per menu
#define MAX_GUI_STRINGS			128	// per page
#define MAX_GUI_LISTITEMS		64	// per list
#define MAX_GUI_EVENTBITMAPS	80	// per session
#define MAX_GUI_SELECTS			20	// per page
#define MAX_GUI_SELECTITEMS  	32
#define MAX_GUI_BMPBTN			32	// per page

#define MAX_GUI_BUTTONS 		64	// per page / menu item

#define GUI_INVALID_COMMAND_ID	 0

// object ID masks

#define MENU_IDMASK					0x00010000
#define STRING_IDMASK				0x00020000
#define BUTTON_IDMASK				0x00040000
#define PANEL_IDMASK				0x00080000

#define ENTER_COMMAND_MASK			0x00001000
#define TXTCHANGE_COMMAND_MASK  	0x00002000


// object id check-up macros
#define ISMENU(x) 			(((x) & MENU_IDMASK) && ((x) >> 16))
#define ISSTRING(x) 			(((x) & STRING_IDMASK) && ((x) >> 17))
#define ISBUTTON(x) 			(((x) & BUTTON_IDMASK) && ((x) >> 18))
#define ISPANEL(x)			(((x) & PANEL_IDMASK) && ((x) >> 19))


#define	CURSOR_COLOR				0x6000

#define	STRING_CONTROL_LEN		33

typedef enum 
{
	LINK_BUTTON = 0,
	LINK_STRING,
	LINK_PAGE,
	
} GuiLinkType;


/*一个对象的三个状态:
	可用,已选择,不可用.*/
typedef enum 
{
	STATE_ENABLED = 0,
	STATE_SELECTED,	
	STATE_DISABLED	
	
} GuiObjectState;


/*对象的选择方向*/
typedef enum 
{
	SELECT_LEFT = 0,
	SELECT_UP,
	SELECT_RIGHT,
	SELECT_DOWN	
	
}GuiObjectSelectDirection;


/*文本的位置*/
typedef enum 
{
	ALIGN_LEFT = 0,
	ALIGN_RIGHT,
	ALIGN_CENTER,
	ALIGN_UP,
	ALIGN_DOWN,
	ALIGN_DOWN_NOTE,
}GuiObjectTextAlignment;


/*字符串类型*/
typedef enum 
{
	STRING_UNKNOW 		= 0,
	STRING_PASSWORD 		= 1,
	STRING_NUM 			= 2,
	STRING_DATE 			= 3,
	STRING_IP 				= 4,
	STRING_MAC 			= 5,
	STRING_CHAR 			= 6,
	STRING_CHOOSE 			= 7,
	STRING_BUTTON 			= 8,
	STRING_COMBOX 		= 9,
	STRING_COMBOX_INDEX	= 10,
	STRING_EDIT 			= 11,
	STRING_STATIC 			= 12,
	STRING_MENU 			= 13,
	STRING_MENU_EXPAND	= 14,
	STRING_TIME 			= 15,
	STRING_TIMER 			= 16,
	STRING_CHECK 			= 17,
	STRING_LIST 			= 18,
	STRING_LIST_COLUMN 	= 19,
	STRING_IPBOX 			= 20,
	STRING_MACBOX 		= 21,
	STRING_PROCESS 		= 22,
	STRING_BUTTON_NOTE	= 23,
	STRING_PLAYER			= 24,
	STRING_OUTLINE_FILLBOX, // 只有此类型才可以画边框和填充
	
}GuiObjectStringType;


typedef enum
{
	DISABLE_VISIBLE = 0,
	ENABLE_VISIBLE,
	
}OBJVISIBLE_STATE;


#define MAX_OSD_STATUS			32	

typedef enum
{

	OSD_NULLMODE					= 0,	 	// 空模式
	OSD_LIVEMODE 					= 1, 	// 1  在直通画面模式
	OSD_PLAYBACKMODE 				= 2,		// 2 在回放画面中
	OSD_INTERFACEMODE			= 3, 	// 3 在操作界面中
	OSD_LOGINMODE					= 4,		// 4  处于已经登录状态
	OSD_PLAYCURFILE				= 5,  	// 5  播放当前文件
	OSD_ADJUST_LCS				= 6, 	// 6  调整模拟量
	OSD_DATETIME_MODE			= 7,		// 7 刷新时间信息
	OSD_FORMAT_HDD_MODE			= 8,		// 格式化硬盘
	OSD_MDSET_MODE				= 9,
	OSD_PTZ_MODE					= 10,
	OSD_TESTALARM_MODE			= 11,
	OSD_COLOR_MODE				= 12,	//颜色调整模式
	OSD_SOFTKEY					= 13,	//软键盘打开
	OSD_REBOOT_MODE				= 14,	//重启系统中
	OSD_SHUTDOWN_MODE			= 15,	//关闭系统中
	OSD_EXPAND_MENU				= 16,	//扩展菜单
	OSD_PTZ_IR_INPUT				= 17,	//红外云台输入
	OSD_3G_MODE 					= 18,	// 3G设置页中
	OSD_SYSTEM_MENU				= 19,	// 主菜单
}OSDMODETYPE;


typedef enum
{
	/*英文字符8X16点阵*/
	ENGLISH_8_16 = 0,

	/*英文字符10 X 20点阵*/
	ENGLISH_10_20,

	/*英文字符12 X 24点阵*/
	ENGLISH_12_24,

	/*英文字符15 X 30点阵*/
	ENGLISH_15_30,

	/*中文字符16 X 16点阵*/
	CHINESE_16_16,

	/*中文字符20 X 20点阵*/
	CHINESE_20_20,

	/*中文字符24 X 24点阵*/
	CHINESE_24_24,

	/*中文字符30 X 30点阵*/
	CHINESE_30_30
	
}FontType;

typedef enum
{
	STYLE_NUMBER = 0,
	STYLE_CHAR,
	STYLE_TIME,
	STYLE_DATE_YEAR,
	STYLE_DATE_MONTH,
	STYLE_DATE_DAY,
	STYLE_MAC,
}INPUT_STYLE;

typedef enum
{
	NOTE_ALARM 	= 0,
	NOTE_HOME_ALARM ,
	NOTE_DISALARM ,
	NOTE_CLEAN_ALARM,
	NOTE_SYS_DEF,
	NOTE_DEL_USER,
	NOTE_STORAGE_SET,
	NOTE_TVMODE_SET,
	NOTE_INPUT_ERROR,
	NOTE_ENCODE_RECODE,
	NOTE_ENCODE_RESTART,
	NOTE_CENTER_OPT,
	NOTE_HDD_FORMAT,
	NOTE_HDD_WAIT,
	NOTE_BACKUP_FILE,
	NOTE_RESTART,
	NOTE_SHUTDOWN,
	NOTE_NETSET_LISTEN,
	NOTE_PLEAR_ERROPT,
	NOTE_3GSET_REBOOT,
	NOTE_ZONESET_DEFAULT,
	NOTE_EAVS,
	NOTE_LOGIN_ERR,
	NOTE_CMSFAIL,
	NOTE_VERIFYUSERFAIL,
}NoteBoxType;

typedef enum
{
	IDNULL = 0,
	IDYES,
	IDYESNO,
}NoteBoxMode;

#ifndef    RELEASE_APP
#define	PICTURE_DIR			"/mnt/icon/"
#define   FONTFILE_DIR			"/mnt/fontfile/"
#define   PICTURE_NOTE			"/mnt/dvr_bmp/info/"

#else 
#define	PICTURE_DIR			"/app/icon/"
#define   FONTFILE_DIR			"/app/fontfile/"
#define   PICTURE_NOTE			"/app/dvr_bmp/info/"
#endif 

/*OSD page definition*/
typedef struct 
{	
	char	*			m_filename;	// bitmap file
	unsigned short 	m_x;		// for now full size at (0,0)
	unsigned short		m_y;
	unsigned short		m_width;
	unsigned short		m_height;
	int				m_isDisplayed;
	unsigned int		m_backgroundcolor;		/*0x00000000-0x00ffffff  */
	int 				m_fbnumber;
	
} PageObject;


/* Text Button definition*/
typedef struct  
{
	char	*			m_filename;		// NULL for text buttons, otherwise if selectedFile = NULL contains 3 states (enabled, focused, disabled)
	char	*			m_selectedFile;	// selected view, if NULL, file is 3 state
	unsigned short 	m_x;
	unsigned short 	m_y;
	unsigned short 	m_width;
	unsigned short 	m_height;
	GuiObjectState 	m_state;
	char *			m_link;
	char *			m_target;
	char	*			m_text;
	FontType 			m_fonttype;	
	int   				m_hasfocus;
	int				m_nbType;				// buttom type 
	int 				m_fbnumber;
	int				m_backgroundcolor;		/*ignored if bitmapped0x00000000-0x00ffffff  */
	int 				m_foregroudcolor;
	int				m_outlinecolor;			/* ignored if bitmapped  */
	int				m_selectedoutlinecolor;		/* 选中后的外框图*/
	int				m_disablecolor;			/*不可操作的按钮色*/
	int				m_activecolor;
} ButtonObject;


/* String object definition*/
typedef  struct
{
	unsigned short 		m_x;
	unsigned short 		m_y;
	unsigned short 		m_width;
	unsigned short 		m_height;
	char *				m_text;
	FontType 				m_fonttype;
	GuiObjectStringType	m_type;
	int					m_hasfocus;
	unsigned int 			m_foregroundcolor;
	unsigned int 			m_backgroundcolor;
	unsigned int 			m_outlinecolor;
	unsigned int			m_validgroudcolor;
	GuiObjectTextAlignment 	m_textalign;
	unsigned int 			m_selectioncolor;
	unsigned char 			m_maxlength;
	short 				m_selcharindex;
	int					m_fbnumber;
	unsigned int			m_noempty;
	unsigned short		m_value;
} StringObject;


typedef struct  
{
	int 		m_fbNumber;
	int		m_x;
	int		m_y;
	int		m_width;
	int		m_height;
	char 		m_filename[128];
	
} BitmapObject;


typedef struct
{
	int 		m_id;
	char* 	name;
	char* 	m_event;			/* used if part of event bitmap list  */
	BitmapObject object;
	
} guiBitmapType;


/* MenuItem object definition*/
typedef struct  
{
	char* 			m_text;
	unsigned char 		m_index;
	char*			m_picfilename;
	char*			m_selpicfilename;
	int				m_hasfocus;
	int 				m_fbnumber;
	unsigned short 	m_x;
	unsigned short 	m_y;
	unsigned short 	m_width;
	unsigned short 	m_height;
	
} MenuItemObject;


typedef struct 
{
	unsigned short 	m_x;
	unsigned short 	m_y;
	unsigned short 	m_width;
	unsigned short 	m_height;
	unsigned int 		m_foregroundcolor;
	unsigned int 		m_backgroundcolor;
	char *			m_parent;
	unsigned int 		m_selectioncolor;
	char	*			m_filename;		// NULL for text buttons, otherwise if selectedFile = NULL contains 3 states (enabled, focused, disabled)
	FontType 			m_fonttype;
	int 				m_isDisplayed;
	int 				m_fbnumber;
	
}MenuObject;


typedef struct
{
	char 			m_LogInPageName[32];
	char 			m_SysMenuPageName[32];
	char 			m_MainMenuPageName[32];
	char 			m_RecordQueryPageName[32];
	char			m_SysInfoPageName[32];
	char			m_SysSettingPageName[32];
	char			m_AdvacedOptionPageName[32];
	char			m_FileManagePageName[32];
	char			m_SaftyPageName[32];
	char			m_HZinputPageName[32];
	char			m_AsciiInputPageName[32];
	char			m_DigitInputPageName[32];
	char			m_logofile[64];

	int 			m_LogInPageId;
	int 			m_SysMenuPageId;
	int 			m_MainMenuPageId;
	int 			m_RecordQueryPageId;
	int			m_SysInfoPageId;
	int			m_SysSettingPageId;
	int			m_AdvacedOptionPageId;
	int			m_FileManagePageId;
	int			m_SaftyPageId;
	int			m_HZinputPageId;
	int			m_AsciiInputPageId;
	int			m_DigitInputPageId;
	
	unsigned 	short 	m_backgroudcolor;
	
} guiPlayerParams;


typedef struct 
{
	char *			m_name;
	unsigned int 		m_id;
	ButtonObject 		m_object;
	char* 			m_keyleft;
	char* 			m_keyup;
	char* 			m_keyright;
	char* 			m_keydown;
	char* 			m_onclick;
	char* 			m_onselect;
	char*			m_clickcmdname;
	unsigned int		m_nCommandId;
	unsigned short 	m_nbEvents;
	unsigned short		m_nVisible;
	
}GuiButton;

typedef struct 
{
	char *			m_name;
	MenuItemObject 	m_object;
	unsigned short 	m_nbEvents;
	char *			m_keydown;
	char *			m_keyup;
	char *			m_onclick;
	char *			m_target;	
	char	 *			m_clickcmdname;
	unsigned int		m_nCommandId;	
	int				m_nVisible;
	
}GuiMenuItem;


typedef struct 
{
	char *			m_name;
	unsigned int 		m_id;
	GuiMenuItem *		m_menuItem[MAX_GUI_MENUITEMS];
	MenuObject 		m_object;
	unsigned short 	m_nbMenuItems;
	unsigned char 		m_selectedMenuitemIndex;
	unsigned short		m_nVisible;

}GuiMenu;


typedef struct 
{
	char* 			m_name;
	unsigned int 		m_id;
	StringObject 		m_object;
	char* 			m_keyleft;
	char* 			m_keyup;
	char* 			m_keyright;
	char* 			m_keydown;
	char	*			m_ontextchange;
	char*			m_onclick;
	char*			m_clickcmdname;	
	char*			m_symbolinput;
	char*			m_target;
	char*			m_onselect;
	char*			m_delclick;
	unsigned int		m_nCommandId;	
	unsigned int		m_InputCmdId;
	unsigned short 	m_nbEvents;
	unsigned short		m_nVisible;
	
	
}GuiString;


typedef struct 
{
	char* 			m_name;
	unsigned int 		m_id;
	PageObject 		m_object;
	char *			m_selectObj;
	char *			m_parent;
	char	*			m_handleload;
	char *			m_onclose;
	GuiButton* 		m_buttons[MAX_GUI_BUTTONS];
	GuiMenu* 		m_menus[MAX_GUI_MENUS];
	GuiString* 		m_strings[MAX_GUI_STRINGS];

	unsigned short 	m_nbMenus;
	unsigned short 	m_nbButtons;
	unsigned short 	m_nbStrings;
	unsigned short	m_nVisible;
	unsigned int		m_nfocusObjId;
	
}GuiPage;

typedef struct
{
	char*		 m_name;
	unsigned int  	 m_id;

} GuiCommandType;


typedef struct
{
	int		 m_id;
	int		 m_linkId;
	int		 m_validLink;
	int		  m_spring;

}typeCommandInfo;


/*线定义*/
typedef struct 
{
	int	m_fbNumber;	
	int	m_x;
	int	m_y;
	int	m_width;
	int	m_height;
	int	m_color;
	
} LineObject ;


/*框定义*/
typedef struct  
{
	int	m_fbNumber;
	int	m_x;
	int	m_y;
	int	m_height;
	int	m_width;
	int	m_color;
	int	m_outlinenum; 	/*框线厚度*/
	
} OutLineObject;


/*填充框定义*/
typedef struct  
{
	int  m_fbNumber;	
	int	m_x;
	int	m_y;
	int	m_height;
	int	m_width;
	int	m_color;
	int   m_transpanrent;
	
} FilledObject;


//颜色数据类型这个要与COLOR_BITS保持一致
#if defined(SUPPORT_COLOR_ARGB8888)
	#define color_t unsigned long
#else
	#define color_t unsigned short
#endif

#define BLUE   	0x001f
#define RED    	0x7C00
#define GREEN  	0x03e0


/*********************************************************
	透明颜色，使用这个颜色可以将整个
	FRAMEBUFFER清除，这样就可以看到直通画面
 *********************************************************/
#define HI3511_TRANSPARENT_COLOR		0x5de8


typedef enum _COLOR_BITS_
{
	COLOR_BITS_16 = 16,
	COLOR_BITS_32 = 32
	
}COLOR_BITS;		//色彩位深


typedef enum _OSD_LAY_WHITCH_
{
	OSD_LAY_0 = 0,
	OSD_LAY_1,
	OSD_LAY_ALL
	
}OSD_LAY_NUM;


// framebuffer设备名字
#define DEVNAME_LEN		20


typedef struct _SURFACE_S
{
	HI_U32	x;
	HI_U32	y;
	HI_U32	w;
	HI_U32	h;     
	unsigned char isSync;	//是否把临时缓存的东西放入显存
	
}SURFACE_S;


typedef enum 
{
	FB_DOUBLE_BUFFER,
	FB_SINGLE_BUFFER
	
}FB_BUFFER_TYPE;


typedef struct _osdlayerinfo_type_{
	//osd层信息结构体
	
	/*FRAMEBUFFER映射地址*/
	void *mapped_mem;		
	
	/*用于显示映射地址*/
	void *DisplayMem;			

	/*显示的物理地址*/
	HI_U8 *DisPlayPhyAddr;	

	/*临时，主要用于抗闪烁*/
	void *TmpMem;		

	/*临时对应的物理地址*/	
	HI_U8 *TmpPhyAddr;	

	//代理内存地址，如果是单缓存就使用DislayMem,否则使用tmpMem;	
	void *ProxyMem;	

	/*映射长度*/
       unsigned long mapped_memlen;

	/*FRAMEBUFFER的文件描述符*/
	int FrameBufferFd;		

	/*设备文件*/
	char file[DEVNAME_LEN + 1];	

	/*固定信息*/
       struct fb_fix_screeninfo finfo;	

	/*可变信息*/
       struct fb_var_screeninfo vinfo;
	   
}osdlayerinfo_t;

#endif 



#ifndef _GUIXMLPARSER_H_
#define _GUIXMLPARSER_H_

#include "tinyxml.h"
#include "common.h"
#include "GuiTypes.h"

#define GUI_XML_ROOT						"Gui_3511"
#define GUI_PICTURE_DIR_ATTR        			"picturedir"
#define GUI_LOGINPAGE_ATTR					"PgLogin"
#define GUI_SYSMENUPAGE_ATTR				"pgSysMenu"
#define GUI_MAINMENUPAGE_ATTR				"PgMainMenu"
#define GUI_RECORDQUERYPAGE_ATTR			"pgRecQuery"
#define GUI_SYSINFOPAGE_ATTR				"pgSysInfo"
#define GUI_SYSSETTINGPAGE_ATTR				"pgSysSetting"
#define GUI_ADVACEDOPTPAGE_ATTR			"pgAdvacedOpt"
#define GUI_FILEMANAGEPAGE_ATTR				"pgFileManage"
#define GUI_SAFTYPAGE_ATTR					"pgSafty"
#define GUI_HZINPUTPAGE_ATTR				"pgHZinput"
#define GUI_ASCIIINPUTPAGE_ATTR				"pgAsciiInput"
#define GUI_DIGITINPUTPAGE_ATTR				"pgDigitInput"


#define GUI_DEVICEPAGE_ATTR					"devicePage"
#define GUI_EVENTSEARCHPAGE_ATTR			"eventsearchPage"
#define GUI_PLAYBACKPAGE_ATTR				"playbackPage"
#define GUI_LOGOFILE_ATTR					"logofile"
#define GUI_PAGELIST_NODE					"pageList"
#define GUI_PAGE_NODE						"page"
#define GUI_COMMANDLIST_NODE				"commandList"
#define GUI_COMMAND_NODE					"command"
#define GUI_STRINGLIST_NODE					"stringList"
#define GUI_STRING_NODE						"string"
#define GUI_BUTTONLIST_NODE					"buttonList"
#define GUI_BUTTON_NODE					"button"
#define GUI_SELECTLIST_NODE					"selectList"
#define GUI_SELECTITEMLIST_NODE				"selectItemList"
#define GUI_SELECT_NODE						"select"
#define GUI_EVENTLIST_NODE					"eventList"
#define GUI_EVENT_NODE						"event"
#define GUI_MENULIST_NODE					"menulist"
#define GUI_MENU_NODE						"menu"
#define GUI_MENUITEM_NODE					"menuItem"
#define GUI_PANELLIST_NODE					"panelList"
#define GUI_PANEL_NODE						"panel"

#define GUI_NAME_ATTR						"name"
#define GUI_TEXT_ATTR						"text"
#define GUI_TEXT_ALIGN_ATTR					"textalign"
#define GUI_WIDTH_ATTR						"width"
#define GUI_HEIGHT_ATTR						"height"
#define GUI_X_ATTR							"x"
#define GUI_Y_ATTR							"y"
#define GUI_TYPE_ATTR						"type"
#define GUI_FILE_ATTR						"picfile"
#define GUI_SEL_FILE_ATTR					"selectpicfile"
#define GUI_SELOBJECT_ATTR					"selectedobj"
#define GUI_OUTLINECOLOR_ATTR				"outlinecolor"
#define GUI_FRAMEBUFFER_ATTR				"fbnumber"
#define GUI_BACKGROUNDCOLOR_ATTR 			"backgroundcolor"
#define GUI_FOREGROUNDCOLOR_ATTR			"foregroundcolor"
#define GUI_SELECTIONCOLOR_ATTR			"selectioncolor"
#define GUI_VALIDGROUDCOCLOR_ATTR			"validgroundcolor"
#define GUI_ACTIVECOLOR_ATTR				"activecolor"
#define GUI_DISABLECOLOR_ATTR				"disablecolor"
#define GUI_FONTTYPE_ATTR					"fonttype"
#define GUI_STRINGTYPE_ATTR					"type"
#define GUI_STRINGVALUE_ATTR					"value"

#define GUI_KEYDOWN_ATTR					"down"
#define GUI_KEYUP_ATTR						"up"
#define GUI_KEYLEFT_ATTR						"left"
#define GUI_KEYRIGHT_ATTR					"right"
#define GUI_INDEX_ATTR						"index"
#define GUI_COMMAND_ATTR					"command"
#define GUI_CLICKCOMMAND_ATTR				"clickcommand"
#define GUI_CLICKCOMMANDID_ATTR			"clickcmdid"
#define GUI_COMMAND_NODE					"command"
#define GUI_COMMANDLIST_NODE				"commandList"
#define GUI_COMMAND_NAME					"commondname"
#define GUI_LINK_ATTR						"link"
#define GUI_HASFOCUS_ATTR					"selected"
#define GUI_PARENT_ATTR						"parent"
#define GUI_MAXLENGTH_ATTR					"maxlength"
#define GUI_TARGET_ATTR						"target"
#define GUI_ONCLICK_ATTR					"onclick"
#define GUI_ONSELECT_ATTR					"onselect"
#define GUI_ONTEXTCHANGE_ATTR				"ontextchange"
#define GUI_TEXTCHANGECMDID_ATTR			"textchangecmdid"
#define GUI_HANDLELOAD_ATTR				"handleload"
#define GUI_ONCLOSE_ATTR					"onclose"
#define GUI_TEXTINPUT_ATTR					"onsymbolinput"
#define GUI_TEXTDEL_ATTR						"ondelclick"
#define GUI_VISIBLE_ATTR						"visible"


#define GUI_FONT_ENGLISH_8_16				"ENGLISH_8_16"
#define GUI_FONT_ENGLISH_10_20				"ENGLISH_10_20"
#define GUI_FONT_ENGLISH_12_24				"ENGLISH_12_24"
#define GUI_FONT_ENGLISH_15_30				"ENGLISH_15_30"
#define GUI_FONT_CHINESE_16_16				"CHINESE_16_16"
#define GUI_FONT_CHINESE_20_20				"CHINESE_20_20"
#define GUI_FONT_CHINESE_24_24				"CHINESE_24_24"
#define GUI_FONT_CHINESE_30_30				"CHINESE_30_30"

#define GUI_STRING_TYPE_UNKNOW			"STRING_UNKNOW"
#define GUI_STRING_TYPE_PASSWORD			"STRING_PASSWORD"
#define GUI_STRING_TYPE_NUM				"STRING_NUM"
#define GUI_STRING_TYPE_DATE				"STRING_DATE"
#define GUI_STRING_TYPE_TIME				"STRING_TIME"
#define GUI_STRING_TYPE_IP					"STRING_IP"
#define GUI_STRING_TYPE_MAC				"STRING_MAC"
#define GUI_STRING_TYPE_CHAR				"STRING_CHAR"
#define GUI_STRING_TYPE_CHOOSE			"STRING_CHOOSE"
#define GUI_STRING_TYPE_BUTTON			"STRING_BUTTON"
#define GUI_STRING_TYPE_COMBOX			"STRING_COMBOX"
#define GUI_STRING_TYPE_COMBOXINDEX		"STRING_COMBOX_INDEX"
#define GUI_STRING_TYPE_EDIT				"STRING_EDIT"
#define GUI_STRING_TYPE_STATIC				"STRING_STATIC"
#define GUI_STRING_TYPE_MENU				"STRING_MENU"
#define GUI_STRING_TYPE_EXMENU			"STRING_MENU_EXPAND"
#define GUI_STRING_TYPE_TIME				"STRING_TIME"
#define GUI_STRING_TYPE_TIMER				"STRING_TIMER"
#define GUI_STRING_TYPE_CHECK				"STRING_CHECK"
#define GUI_STRING_TYPE_LIST				"STRING_LIST"
#define GUI_STRING_TYPE_LISTCOLUMN		"STRING_LIST_COLUMN"
#define GUI_STRING_TYPE_IPBOX				"STRING_IPBOX"
#define GUI_STRING_TYPE_MACBOX			"STRING_MACBOX"
#define GUI_STRING_TYPE_PROCESS			"STRING_PROCESS"
#define GUI_STRING_TYPE_PLAYER			"STRING_PLAYER"


#define GUI_TEXT_ALIGNMENT_LEFT				"ALIGN_LEFT"
#define GUI_TEXT_ALIGNMENT_RIGHT			"ALIGN_RIGHT"
#define GUI_TEXT_ALIGNMENT_CENTER			"ALIGN_CENTER"



//界面xml  文档解析类
class XMLPARSER
{
public:
	XMLPARSER();

	~XMLPARSER();

	int FreePara();

	char *MallocAndDuplicateAscii (const char *string);
	
	void CopyAscii (char *dest, const char *src);

	int  DoParsing (char *xmlfile);

	int  GetPages(GuiPage *pages [ MAX_GUI_PAGES ], unsigned int * nPages);
	
	int  GetPlayParamers(guiPlayerParams *playerParams);

	int GetCommands(GuiCommandType *commands[MAX_GUI_COMMANDS], unsigned int  *nCommands);

private:
	void FreePlayerParams();
	
	int  ParsePageList(TiXmlElement *pnode);
	
	GuiPage* ParsePage(TiXmlElement *pnode, GuiPage *pages);
	
	void FreePage(GuiPage *page);

	int  ParseStringList(TiXmlElement *pnode, GuiString *strings[MAX_GUI_STRINGS], unsigned short *nbStrings);
	
	GuiString *ParseString(TiXmlElement *pnode, GuiString *string);
	
	void FreeString(GuiString *string);

	int  ParseButtonList(TiXmlElement *pnode, GuiButton *buttons[MAX_GUI_BUTTONS], unsigned short *nbButtons);
	
	GuiButton *ParseButton(TiXmlElement *pnode, GuiButton *button);
	
	void FreeButton(GuiButton *button);

	int  ParseMenuList(TiXmlElement *pnode, GuiMenu *menus[MAX_GUI_MENUS], unsigned short *nbMenus);
	
	GuiMenu *ParseMenu(TiXmlElement *pnode, GuiMenu *menu);
	
	GuiMenuItem *ParseMenuItem(TiXmlElement * pnode, GuiMenuItem * menuitem);
	
	void FreeMenuItem(GuiMenuItem * menuitem);
	
	void FreeMenu(GuiMenu *menu);

	FontType GetFontType(const char *strFonttype);
	
	GuiObjectStringType GetStringType(const char * strType);
	
	GuiObjectTextAlignment GetTextAlignment(const char * strAlignment);

	int GetControlState(const char *state);

	GuiCommandType *ParseCommand(TiXmlElement *pnode);

	void FreeCommand(GuiCommandType *cmd);

	int ParseCommandList(TiXmlElement *pnode);

private:
	GuiPage* m_Pages[MAX_GUI_PAGES];

	GuiCommandType* m_Commands[MAX_GUI_COMMANDS];
	
	guiPlayerParams m_playerParams;

	unsigned short m_nbPages;
	
	unsigned short  m_nbCommands;
		
	// these are used only to set ids. pages use their index number as id
	unsigned int m_nextButtonId;
	unsigned int  m_nextMenuId;
	unsigned int  m_nextStringId;
};

#endif

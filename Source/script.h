/*
AutoHotkey

Copyright 2003-2005 Chris Mallett

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef script_h
#define script_h

#include "stdafx.h" // pre-compiled headers
#include "defines.h"
#include "SimpleHeap.h" // for overloaded new/delete operators.
#include "keyboard.h" // for modLR_type
#include "var.h" // for a script's variables.
#include "WinGroup.h" // for a script's Window Groups.
#include "Util.h" // for FileTimeToYYYYMMDD(), strlcpy()
#include "resources\resource.h"  // For tray icon.
#ifdef AUTOHOTKEYSC
	#include "lib/exearc_read.h"
#endif

#include "os_version.h" // For the global OS_Version object
EXTERN_OSVER; // For the access to the g_os version object without having to include globaldata.h
EXTERN_G;

enum ExecUntilMode {NORMAL_MODE, UNTIL_RETURN, UNTIL_BLOCK_END, ONLY_ONE_LINE};

// It's done this way so that mAttribute can store a pointer or one of these constants.
// If it is storing a pointer for a given Action Type, be sure never to compare it
// for equality against these constants because by coincidence, the pointer value
// might just match one of them:
#define ATTR_NONE ((void *)0)
#define ATTR_LOOP_UNKNOWN ((void *)1)
#define ATTR_LOOP_NORMAL ((void *)2)
#define ATTR_LOOP_FILE ((void *)3)
#define ATTR_LOOP_REG ((void *)4)
#define ATTR_LOOP_READ_FILE ((void *)5)
#define ATTR_LOOP_PARSE ((void *)6)
typedef void *AttributeType;

enum FileLoopModeType {FILE_LOOP_INVALID, FILE_LOOP_FILES_ONLY, FILE_LOOP_FILES_AND_FOLDERS, FILE_LOOP_FOLDERS_ONLY};
enum VariableTypeType {VAR_TYPE_INVALID, VAR_TYPE_NUMBER, VAR_TYPE_INTEGER, VAR_TYPE_FLOAT
	, VAR_TYPE_TIME	, VAR_TYPE_DIGIT, VAR_TYPE_XDIGIT, VAR_TYPE_ALNUM, VAR_TYPE_ALPHA
	, VAR_TYPE_UPPER, VAR_TYPE_LOWER, VAR_TYPE_SPACE};

// But the array that goes with these actions is in globaldata.cpp because
// otherwise it would be a little cumbersome to declare the extern version
// of the array in here (since it's only extern to modules other than
// script.cpp):
enum enum_act {
// Seems best to make this one zero so that it will be the ZeroMemory() default within
// any POD structures that contain an action_type field:
  ACT_INVALID = FAIL  // These should both be zero for initialization and function-return-value purposes.
, ACT_ASSIGN, ACT_ASSIGNEXPR, ACT_ADD, ACT_SUB, ACT_MULT, ACT_DIV
, ACT_ASSIGN_FIRST = ACT_ASSIGN, ACT_ASSIGN_LAST = ACT_DIV
, ACT_REPEAT // Never parsed directly, only provided as a translation target for the old command (see other notes).
, ACT_ELSE   // Parsed at a lower level than most commands to support same-line ELSE-actions (e.g. "else if").
, ACT_IFBETWEEN, ACT_IFNOTBETWEEN, ACT_IFIN, ACT_IFNOTIN, ACT_IFCONTAINS, ACT_IFNOTCONTAINS, ACT_IFIS, ACT_IFISNOT
, ACT_IFEXPR  // i.e. if (expr)
 // *** *** *** KEEP ALL OLD-STYLE/AUTOIT V2 IFs AFTER THIS (v1.0.20 bug fix). *** *** ***
 , ACT_FIRST_IF_ALLOWING_SAME_LINE_ACTION
 // *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 // ACT_IS_IF_OLD() relies upon ACT_IFEQUAL through ACT_IFLESSOREQUAL being adjacent to one another
 // and it relies upon the fact that ACT_IFEQUAL is first in the series and ACT_IFLESSOREQUAL last.
, ACT_IFEQUAL = ACT_FIRST_IF_ALLOWING_SAME_LINE_ACTION, ACT_IFNOTEQUAL, ACT_IFGREATER, ACT_IFGREATEROREQUAL
, ACT_IFLESS, ACT_IFLESSOREQUAL
, ACT_FIRST_COMMAND // i.e the above aren't considered commands for parsing/searching purposes.
, ACT_IFWINEXIST = ACT_FIRST_COMMAND
, ACT_IFWINNOTEXIST, ACT_IFWINACTIVE, ACT_IFWINNOTACTIVE
, ACT_IFINSTRING, ACT_IFNOTINSTRING
, ACT_IFEXIST, ACT_IFNOTEXIST, ACT_IFMSGBOX
, ACT_FIRST_IF = ACT_IFBETWEEN, ACT_LAST_IF = ACT_IFMSGBOX  // Keep this range updated with any new IFs that are added.
, ACT_MSGBOX, ACT_INPUTBOX, ACT_SPLASHTEXTON, ACT_SPLASHTEXTOFF, ACT_PROGRESS, ACT_SPLASHIMAGE
, ACT_TOOLTIP, ACT_TRAYTIP, ACT_INPUT
, ACT_TRANSFORM, ACT_STRINGLEFT, ACT_STRINGRIGHT, ACT_STRINGMID
, ACT_STRINGTRIMLEFT, ACT_STRINGTRIMRIGHT, ACT_STRINGLOWER, ACT_STRINGUPPER
, ACT_STRINGLEN, ACT_STRINGGETPOS, ACT_STRINGREPLACE, ACT_STRINGSPLIT, ACT_SPLITPATH, ACT_SORT
, ACT_ENVSET, ACT_ENVUPDATE
, ACT_RUNAS, ACT_RUN, ACT_RUNWAIT, ACT_URLDOWNLOADTOFILE
, ACT_GETKEYSTATE
, ACT_SEND, ACT_SENDRAW, ACT_CONTROLSEND, ACT_CONTROLSENDRAW, ACT_CONTROLCLICK, ACT_CONTROLMOVE, ACT_CONTROLGETPOS
, ACT_CONTROLFOCUS, ACT_CONTROLGETFOCUS, ACT_CONTROLSETTEXT, ACT_CONTROLGETTEXT, ACT_CONTROL, ACT_CONTROLGET
, ACT_COORDMODE, ACT_SETDEFAULTMOUSESPEED, ACT_MOUSEMOVE, ACT_MOUSECLICK, ACT_MOUSECLICKDRAG, ACT_MOUSEGETPOS
, ACT_STATUSBARGETTEXT
, ACT_STATUSBARWAIT
, ACT_CLIPWAIT, ACT_KEYWAIT
, ACT_SLEEP, ACT_RANDOM
, ACT_GOTO, ACT_GOSUB, ACT_ONEXIT, ACT_HOTKEY, ACT_SETTIMER, ACT_THREAD, ACT_RETURN, ACT_EXIT
, ACT_LOOP, ACT_BREAK, ACT_CONTINUE
, ACT_BLOCK_BEGIN, ACT_BLOCK_END
, ACT_WINACTIVATE, ACT_WINACTIVATEBOTTOM
, ACT_WINWAIT, ACT_WINWAITCLOSE, ACT_WINWAITACTIVE, ACT_WINWAITNOTACTIVE
, ACT_WINMINIMIZE, ACT_WINMAXIMIZE, ACT_WINRESTORE
, ACT_WINHIDE, ACT_WINSHOW
, ACT_WINMINIMIZEALL, ACT_WINMINIMIZEALLUNDO
, ACT_WINCLOSE, ACT_WINKILL, ACT_WINMOVE, ACT_WINMENUSELECTITEM, ACT_PROCESS
, ACT_WINSET, ACT_WINSETTITLE, ACT_WINGETTITLE, ACT_WINGETCLASS, ACT_WINGET, ACT_WINGETPOS, ACT_WINGETTEXT
, ACT_SYSGET, ACT_POSTMESSAGE, ACT_SENDMESSAGE
// Keep rarely used actions near the bottom for parsing/performance reasons:
, ACT_PIXELGETCOLOR, ACT_PIXELSEARCH, ACT_IMAGESEARCH
, ACT_GROUPADD, ACT_GROUPACTIVATE, ACT_GROUPDEACTIVATE, ACT_GROUPCLOSE
, ACT_DRIVESPACEFREE, ACT_DRIVE, ACT_DRIVEGET
, ACT_SOUNDGET, ACT_SOUNDSET, ACT_SOUNDGETWAVEVOLUME, ACT_SOUNDSETWAVEVOLUME, ACT_SOUNDPLAY
, ACT_FILEAPPEND, ACT_FILEREAD, ACT_FILEREADLINE, ACT_FILEDELETE, ACT_FILERECYCLE, ACT_FILERECYCLEEMPTY
, ACT_FILEINSTALL, ACT_FILECOPY, ACT_FILEMOVE, ACT_FILECOPYDIR, ACT_FILEMOVEDIR
, ACT_FILECREATEDIR, ACT_FILEREMOVEDIR
, ACT_FILEGETATTRIB, ACT_FILESETATTRIB, ACT_FILEGETTIME, ACT_FILESETTIME
, ACT_FILEGETSIZE, ACT_FILEGETVERSION
, ACT_SETWORKINGDIR, ACT_FILESELECTFILE, ACT_FILESELECTFOLDER, ACT_FILEGETSHORTCUT, ACT_FILECREATESHORTCUT
, ACT_INIREAD, ACT_INIWRITE, ACT_INIDELETE
, ACT_REGREAD, ACT_REGWRITE, ACT_REGDELETE
, ACT_SETKEYDELAY, ACT_SETMOUSEDELAY, ACT_SETWINDELAY, ACT_SETCONTROLDELAY, ACT_SETBATCHLINES
, ACT_SETTITLEMATCHMODE, ACT_SETFORMAT, ACT_FORMATTIME
, ACT_SUSPEND, ACT_PAUSE
, ACT_AUTOTRIM, ACT_STRINGCASESENSE, ACT_DETECTHIDDENWINDOWS, ACT_DETECTHIDDENTEXT, ACT_BLOCKINPUT
, ACT_SETNUMLOCKSTATE, ACT_SETSCROLLLOCKSTATE, ACT_SETCAPSLOCKSTATE, ACT_SETSTORECAPSLOCKMODE
, ACT_KEYHISTORY, ACT_LISTLINES, ACT_LISTVARS, ACT_LISTHOTKEYS
, ACT_EDIT, ACT_RELOAD, ACT_MENU, ACT_GUI, ACT_GUICONTROL, ACT_GUICONTROLGET
, ACT_EXITAPP
, ACT_SHUTDOWN
// Make these the last ones before the count so they will be less often processed.  This helps
// performance because this one doesn't actually have a keyword so will never result
// in a match anyway.  UPDATE: No longer used because Run/RunWait is now required, which greatly
// improves syntax checking during load:
//, ACT_EXEC
// It's safer not to do this here.  It's better set by a
// calculation immediately after the array is declared and initialized,
// at which time we know its true size:
// , ACT_COUNT
};

enum enum_act_old {
  OLD_INVALID = FAIL  // These should both be zero for initialization and function-return-value purposes.
  , OLD_SETENV, OLD_ENVADD, OLD_ENVSUB, OLD_ENVMULT, OLD_ENVDIV
  // ACT_IS_IF_OLD() relies on the items in this next line being adjacent to one another and in this order:
  , OLD_IFEQUAL, OLD_IFNOTEQUAL, OLD_IFGREATER, OLD_IFGREATEROREQUAL, OLD_IFLESS, OLD_IFLESSOREQUAL
  , OLD_LEFTCLICK, OLD_RIGHTCLICK, OLD_LEFTCLICKDRAG, OLD_RIGHTCLICKDRAG
  , OLD_HIDEAUTOITWIN, OLD_REPEAT, OLD_ENDREPEAT
  , OLD_WINGETACTIVETITLE, OLD_WINGETACTIVESTATS
};

// It seems best not to include ACT_SUSPEND in the below, since the user may have marked
// a large number of subroutines as "Suspend, Permit".  Even PAUSE is iffy, since the user
// may be using it as "Pause, off/toggle", but it seems best to support PAUSE because otherwise
// hotkey such as "#z::pause" would not be able to unpause the script if its MaxThreadsPerHotkey
// was 1 (the default).
#define ACT_IS_ALWAYS_ALLOWED(ActionType) (ActionType == ACT_EXITAPP || ActionType == ACT_PAUSE \
	|| ActionType == ACT_EDIT || ActionType == ACT_RELOAD || ActionType == ACT_KEYHISTORY \
	|| ActionType == ACT_LISTLINES || ActionType == ACT_LISTVARS || ActionType == ACT_LISTHOTKEYS)
#define ACT_IS_ASSIGN(ActionType) (ActionType >= ACT_ASSIGN_FIRST && ActionType <= ACT_ASSIGN_LAST)
#define ACT_IS_IF(ActionType) (ActionType >= ACT_FIRST_IF && ActionType <= ACT_LAST_IF)
#define ACT_IS_IF_OLD(ActionType, OldActionType) (ActionType >= ACT_FIRST_IF_ALLOWING_SAME_LINE_ACTION && ActionType <= ACT_LAST_IF) \
	&& (ActionType < ACT_IFEQUAL || ActionType > ACT_IFLESSOREQUAL || (OldActionType >= OLD_IFEQUAL && OldActionType <= OLD_IFLESSOREQUAL))
	// All the checks above must be done so that cmds such as IfMsgBox (which are both "old" and "new")
	// can support parameters on the same line or on the next line.  For example, both of the above are allowed:
	// IfMsgBox, No, Gosub, XXX
	// IfMsgBox, No
	//     Gosub, XXX

#define ATTACH_THREAD_INPUT \
	bool threads_are_attached = false;\
	DWORD my_thread  = GetCurrentThreadId();\
	DWORD target_thread = GetWindowThreadProcessId(target_window, NULL);\
	if (target_thread && target_thread != my_thread && !IsWindowHung(target_window))\
		threads_are_attached = AttachThreadInput(my_thread, target_thread, TRUE) != 0;

#define DETACH_THREAD_INPUT \
	if (threads_are_attached)\
		AttachThreadInput(my_thread, target_thread, FALSE);

// Notes about the below macro:
// One of the menus in the menu bar has been displayed, and the we know the user is is still in
// the menu bar, even moving to different menus and/or menu items, until WM_EXITMENULOOP is received.
// Note: It seems that when window's menu bar is being displayed/navigated by the user, our thread
// is tied up in a message loop other than our own.  In other words, it's very similar to the
// TrackPopupMenuEx() call used to handle the tray menu, which is why g_MenuIsVisible can be used
// for both types of menus to indicate to MainWindowProc() that timed subroutines should not be
// checked or allowed to launch during such times.  Also, "break" is used rather than "return 0"
// to let DefWindowProc()/DefaultDlgProc() take whatever action it needs to do for these.
#define HANDLE_MENU_LOOP \
	case WM_ENTERMENULOOP:\
		g_MenuIsVisible = MENU_TYPE_BAR;\
		break;\
	case WM_EXITMENULOOP:\
		g_MenuIsVisible = MENU_TYPE_NONE;\
		break;

#define IS_PERSISTENT (Hotkey::sHotkeyCount || Hotstring::sHotstringCount || Hotkey::HookIsActive() || g_persistent)

// Since WM_COMMAND IDs must be shared among all menus and controls, they are carefully conserved,
// especially since there are only 65,535 possible IDs.  In addition, they are assigned to ranges
// to minimize the need that they will need to be changed in the future (changing the ID of a main
// menu item, tray menu item, or a user-defined menu item [by way of increasing MAX_CONTROLS_PER_GUI]
// is bad because some scripts might be using PostMessage/SendMessage to automate AutoHotkey itself).
// For this reason, the following ranges are reserved:
// 0: unused (possibly special in some contexts)
// 1: IDOK
// 2: IDCANCEL
// 3 to 1002: GUI window control IDs (these IDs must be unique only within their parent, not across all GUI windows)
// 1003 to 65299: User Defined Menu IDs
// 65300 to 65399: Standard tray menu items.
// 65400 to 65534: main menu items (might be best to leave 65535 unused in case it ever has special meaning)
enum CommandIDs {CONTROL_ID_FIRST = IDCANCEL + 1
	, ID_USER_FIRST = 1003  // The first ID available for user defined menu items. Do not change this (see above for why).
	, ID_USER_LAST = 65299  // The last. Especially do not change this due to scripts using Post/SendMessage to automate AutoHotkey.
	, ID_TRAY_FIRST, ID_TRAY_OPEN = ID_TRAY_FIRST
	, ID_TRAY_HELP, ID_TRAY_WINDOWSPY, ID_TRAY_RELOADSCRIPT
	, ID_TRAY_EDITSCRIPT, ID_TRAY_SUSPEND, ID_TRAY_PAUSE, ID_TRAY_EXIT
	, ID_TRAY_LAST = ID_TRAY_EXIT // But this value should never hit the below. There is debug code to enforce.
	, ID_MAIN_FIRST = 65400, ID_MAIN_LAST = 65534}; // These should match the range used by resource.h

#define GUI_INDEX_TO_ID(index) (index + CONTROL_ID_FIRST)
#define GUI_ID_TO_INDEX(id) (id - CONTROL_ID_FIRST)
#define GUI_HWND_TO_INDEX(hwnd) GUI_ID_TO_INDEX(GetDlgCtrlID(hwnd)) // Returns -1 on failure (e.g. HWND not found)	
// Testing shows that GetDlgCtrlID() is dramatically faster than looping through a GUI window's control
// array to find a matching HWND.


#define ERR_ABORT_NO_SPACES "The current thread will exit."
#define ERR_ABORT "  " ERR_ABORT_NO_SPACES
#define WILL_EXIT "The program will exit."
#define OLD_STILL_IN_EFFECT "The script was not reloaded; the old version will remain in effect."
#define ERR_UNRECOGNIZED_ACTION "This line does not contain a recognized action."
#define ERR_MISSING_OUTPUT_VAR "This command requires that at least one of its output variables be provided."
#define ERR_ELSE_WITH_NO_IF "This ELSE doesn't appear to belong to any IF-statement."
#define ERR_OUTOFMEM "Out of memory."
#define ERR_MEM_LIMIT_REACHED "Memory limit reached (see #MaxMem in the help file)." ERR_ABORT
#define ERR_SETTIMER "This timer's target label does not exist."
#define ERR_ONEXIT_LABEL "Parameter #1 is not a valid label."
#define ERR_HOTKEY_LABEL "Parameter #2 is not a valid label or action."
#define ERR_MENU "Menu does not exist."
#define ERR_SUBMENU "Submenu does not exist."
#define ERR_MENULABEL "This menu item's target label does not exist."
#define ERR_CONTROLLABEL "This control's target label does not exist."
#define ERR_GROUPADD_LABEL "The target label in parameter #4 does not exist."
#define ERR_WINDOW_PARAM "This command requires that at least one of its window parameters be non-blank."
#define ERR_LOOP_FILE_MODE "If not blank or a variable reference, parameter #2 must be either 0, 1, 2."
#define ERR_LOOP_REG_MODE  "If not blank or a variable reference, parameter #3 must be either 0, 1, 2."
#define ERR_ON_OFF "If not blank or a variable reference, this parameter must be either ON or OFF."
#define ERR_ON_OFF_ALWAYS "If not blank or a variable reference, this parameter must be either ON, OFF, ALWAYSON, or ALWAYSOFF."
#define ERR_ON_OFF_TOGGLE "If not blank or a variable reference, this parameter must be either ON, OFF, or TOGGLE."
#define ERR_ON_OFF_TOGGLE_PERMIT "If not blank or a variable reference, this parameter must be either ON, OFF, TOGGLE, or PERMIT."
#define ERR_BLOCKINPUT "Parameter #1 is not a valid BlockInput mode."
#define ERR_TITLEMATCHMODE "TitleMatchMode must be either 1, 2, 3, slow, fast, or a variable reference."
#define ERR_TITLEMATCHMODE2 "The variable does not contain a valid TitleMatchMode." ERR_ABORT
#define ERR_TRANSFORMCOMMAND "Parameter #2 is not a valid transform command."
#define ERR_MENUCOMMAND "Parameter #2 is not a valid menu command."
#define ERR_MENUCOMMAND2 "Parameter #2's variable does not contain a valid menu command."
#define ERR_GUICOMMAND "Parameter #1 is not a valid GUI command."
#define ERR_GUICONTROL "Parameter #2 is not a valid GUI control type."
#define ERR_THREADCOMMAND "Parameter #1 is not a valid thread command."
#define ERR_MENUTRAY "Supported only for the tray menu."
#define ERR_CONTROLCOMMAND "Parameter #1 is not a valid Control command."
#define ERR_CONTROLGETCOMMAND "Parameter #2 is not a valid ControlGet command."
#define ERR_GUICONTROLCOMMAND "Parameter #1 is not a valid GuiControl command. Be sure to include a +/- sign if required."
#define ERR_GUICONTROLGETCOMMAND "Parameter #2 is not a valid GuiControlGet command."
#define ERR_DRIVECOMMAND "Parameter #1 is not a valid Drive command."
#define ERR_DRIVEGETCOMMAND "Parameter #2 is not a valid DriveGet command."
#define ERR_PROCESSCOMMAND "Parameter #1 is not a valid Process command."
#define ERR_WINSET "Parameter #1 is not a valid WinSet attribute."
#define ERR_WINGET "Parameter #2 is not a valid WinGet command."
#define ERR_SYSGET "Parameter #2 is not a valid SysGet command."
#define ERR_COORDMODE "Parameter #1 is not valid."
#define ERR_IFMSGBOX "This line specifies an invalid MsgBox result."
#define ERR_REG_KEY "Invalid registry root key."
#define ERR_REG_VALUE_TYPE "Invalid registry value type."
#define ERR_COMPARE_TIMES "Parameter #3 must be either blank, a variable reference, or one of these words: Seconds, Minutes, Hours, Days."
#define ERR_INVALID_DATETIME "This date-time string contains at least one invalid component."
#define ERR_FILE_TIME "Parameter #3 must be either blank, M, C, A, or a variable reference."
#define ERR_MOUSE_BUTTON "This line specifies an invalid mouse button."
#define ERR_MOUSE_COORD "The X & Y coordinates must be either both absent or both present."
#define ERR_MOUSE_UPDOWN "Parameter #6 must be either blank, D, U, or a variable reference."
#define ERR_DIVIDEBYZERO "This line would attempt to divide by zero."
#define ERR_PERCENT "Parameter #1 must be a number between -100 and 100 (inclusive), or a variable reference."
#define ERR_MOUSE_SPEED "Mouse speed must be between 0 and " MAX_MOUSE_SPEED_STR "."
#define ERR_VAR_IS_RESERVED "This variable is reserved and cannot be assigned to."
#define ERR_DEFINE_CHAR "The character being defined must not be identical to another special or reserved character."
#define ERR_INCLUDE_FILE "A filename must be specified for #Include."
#define ERR_DEFINE_COMMENT "Comment flag must not be one of the hotkey definition symbols (e.g. ! ^ + $ ~ * < >)."

//----------------------------------------------------------------------------------

bool Util_Shutdown(int nFlag);
BOOL Util_ShutdownHandler(HWND hwnd, DWORD lParam);
void Util_WinKill(HWND hWnd);

enum MainWindowModes {MAIN_MODE_NO_CHANGE, MAIN_MODE_LINES, MAIN_MODE_VARS
	, MAIN_MODE_HOTKEYS, MAIN_MODE_KEYHISTORY, MAIN_MODE_REFRESH};
ResultType ShowMainWindow(MainWindowModes aMode = MAIN_MODE_NO_CHANGE, bool aRestricted = true);
ResultType GetAHKInstallDir(char *aBuf);


struct InputBoxType
{
	char *title;
	char *text;
	int width;
	int height;
	int xpos;
	int ypos;
	Var *output_var;
	char password_char;
	char *default_string;
	DWORD timeout;
	HWND hwnd;
};

struct SplashType
{
	int width;
	int height;
	int bar_pos;  // The amount of progress of the bar (it's position).
	int margin_x; // left/right margin
	int margin_y; // top margin
	int text1_height; // Height of main text control.
	int object_width;   // Width of image.
	int object_height;  // Height of the progress bar or image.
	HWND hwnd;
	LPPICTURE pic; // For SplashImage.
	HWND hwnd_bar;
	HWND hwnd_text1;  // MainText
	HWND hwnd_text2;  // SubText
	HFONT hfont1; // Main
	HFONT hfont2; // Sub
	HBRUSH hbrush; // Window background color brush.
	COLORREF color_bk; // The background color itself.
	COLORREF color_text; // Color of the font.
};

// Use GetClientRect() to determine the available width so that control's can be centered.
#define SPLASH_CALC_YPOS \
	int bar_y = splash.margin_y + (splash.text1_height ? (splash.text1_height + splash.margin_y) : 0);\
	int sub_y = bar_y + splash.object_height + (splash.object_height ? splash.margin_y : 0); // i.e. don't include margin_y twice if there's no bar.
#define PROGRESS_MAIN_POS splash.margin_x, splash.margin_y, control_width, splash.text1_height
#define PROGRESS_BAR_POS  splash.margin_x, bar_y, control_width, splash.object_height
#define PROGRESS_SUB_POS  splash.margin_x, sub_y, control_width, (client_rect.bottom - client_rect.top) - sub_y

// From AutoIt3's inputbox:
template <class T>
inline void swap(T &v1, T &v2) {
	T tmp=v1;
	v1=v2;
	v2=tmp;
}

#define INPUTBOX_DEFAULT INT_MIN
ResultType InputBox(Var *aOutputVar, char *aTitle, char *aText, bool aHideInput
	, int aWidth, int aHeight, int aX, int aY, double aTimeout, char *aDefault);
BOOL CALLBACK InputBoxProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK InputBoxTimeout(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
BOOL CALLBACK EnumChildFindSeqNum(HWND aWnd, LPARAM lParam);
BOOL CALLBACK EnumChildFindPoint(HWND aWnd, LPARAM lParam);
BOOL CALLBACK EnumChildGetControlList(HWND aWnd, LPARAM lParam);
BOOL CALLBACK EnumMonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM lParam);
BOOL CALLBACK EnumChildGetText(HWND aWnd, LPARAM lParam);
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
bool HandleMenuItem(WORD aMenuItemID, WPARAM aGuiIndex);


typedef UINT LineNumberType;
#define LOADING_FAILED UINT_MAX

// -2 for the beginning and ending g_DerefChars:
#define MAX_VAR_NAME_LENGTH (UCHAR_MAX - 2)
#define MAX_DEREFS_PER_ARG 512
typedef UINT DerefLengthType;  // Formerly UCHAR, but since UCHAR doesn't actually save memory yet, a UINT for performance reasons.
struct DerefType
{
	char *marker;
	Var *var;
	// Keep any fields that aren't an even multiple of 4 adjacent to each other.  This conserves memory
	// due to byte-alignment:
	DerefLengthType length;
};

typedef UCHAR ArgTypeType;  // UCHAR vs. an enum, to save memory.
#define ARG_TYPE_NORMAL     (UCHAR)0  // Formerly (UCHAR)0
#define ARG_TYPE_INPUT_VAR  (UCHAR)1  // Formerly (UCHAR)1
#define ARG_TYPE_OUTPUT_VAR (UCHAR)2  // Formerly (UCHAR)2
struct ArgStruct
{
	ArgTypeType type;
	bool is_expression; // Whether this ARG is known to contain an expression.
	// Above are kept adjacent to each other to conserve memory (any fields that aren't an even
	// multiple of 4, if adjacent to each other, consume less memory due to default byte alignment
	// setting [which helps performance]).
	char *text;
	DerefType *deref;  // Will hold a NULL-terminated array of var-deref locations within <text>.
};


// Some of these lengths and such are based on the MSDN example at
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/enumerating_registry_subkeys.asp:
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define REG_SUBKEY -2 // Custom type, not standard in Windows.
struct RegItemStruct
{
	HKEY root_key_type, root_key;  // root_key_type is always a local HKEY, whereas root_key can be a remote handle.
	char subkey[MAX_PATH];  // The branch of the registry where this subkey or value is located.
	char name[MAX_VALUE_NAME]; // The subkey or value name.
	DWORD type; // Value Type (e.g REG_DWORD). This is the length used by MSDN in their example code.
	FILETIME ftLastWriteTime; // Non-initialized.
	void InitForValues() {ftLastWriteTime.dwHighDateTime = ftLastWriteTime.dwLowDateTime = 0;}
	void InitForSubkeys() {type = REG_SUBKEY;}  // To distinguish REG_DWORD and such from the subkeys themselves.
	RegItemStruct(HKEY aRootKeyType, HKEY aRootKey, char *aSubKey)
		: root_key_type(aRootKeyType), root_key(aRootKey), type(REG_NONE)
	{
		*name = '\0';
		// Make a local copy on the caller's stack so that if the current script subroutine is
		// interrupted to allow another to run, the contents of the deref buffer is saved here:
		strlcpy(subkey, aSubKey, sizeof(subkey));
		// Even though the call may work with a trailing backslash, it's best to remove it
		// so that consistent results are delivered to the user.  For example, if the script
		// is enumerating recursively into a subkey, subkeys deeper down will not include the
		// trailing backslash when they are reported.  So the user's own subkey should not
		// have one either so that when A_ScriptSubKey is referenced in the script, it will
		// always show up as the value without a trailing backslash:
		size_t length = strlen(subkey);
		if (length && subkey[length - 1] == '\\')
			subkey[length - 1] = '\0';
	}
};

struct LoopReadFileStruct
{
	FILE *mReadFile, *mWriteFile;
	char mWriteFileName[MAX_PATH];
	#define READ_FILE_LINE_SIZE (64 * 1024)  // This is also used by FileReadLine().
	char mCurrentLine[READ_FILE_LINE_SIZE];
	LoopReadFileStruct(FILE *aReadFile, char *aWriteFileName)
		: mReadFile(aReadFile), mWriteFile(NULL) // mWriteFile is opened by FileAppend() only upon first use.
	{
		// Use our own buffer because caller's is volatile due to possibly being in the deref buffer:
		strlcpy(mWriteFileName, aWriteFileName, sizeof(mWriteFileName));
		*mCurrentLine = '\0';
	}
};


typedef UCHAR ArgCountType;
#define MAX_ARGS 20


// Note that currently this value must fit into a sc_type variable because that is how TextToKey()
// stores it in the hotkey class.  sc_type is currently a UINT, and will always be at least a
// WORD in size, so it shouldn't be much of an issue:
#define MAX_JOYSTICKS 16  // The maximum allowed by any Windows operating system.
#define MAX_JOY_BUTTONS 32 // Also the max that Windows supports.
enum JoyControls {JOYCTRL_INVALID, JOYCTRL_XPOS, JOYCTRL_YPOS, JOYCTRL_ZPOS
, JOYCTRL_RPOS, JOYCTRL_UPOS, JOYCTRL_VPOS, JOYCTRL_POV
, JOYCTRL_NAME, JOYCTRL_BUTTONS, JOYCTRL_AXES, JOYCTRL_INFO
, JOYCTRL_1, JOYCTRL_2, JOYCTRL_3, JOYCTRL_4, JOYCTRL_5, JOYCTRL_6, JOYCTRL_7, JOYCTRL_8  // Buttons.
, JOYCTRL_9, JOYCTRL_10, JOYCTRL_11, JOYCTRL_12, JOYCTRL_13, JOYCTRL_14, JOYCTRL_15, JOYCTRL_16
, JOYCTRL_17, JOYCTRL_18, JOYCTRL_19, JOYCTRL_20, JOYCTRL_21, JOYCTRL_22, JOYCTRL_23, JOYCTRL_24
, JOYCTRL_25, JOYCTRL_26, JOYCTRL_27, JOYCTRL_28, JOYCTRL_29, JOYCTRL_30, JOYCTRL_31, JOYCTRL_32
, JOYCTRL_BUTTON_MAX = JOYCTRL_32
};
#define IS_JOYSTICK_BUTTON(joy) (joy >= JOYCTRL_1 && joy <= JOYCTRL_BUTTON_MAX)

enum WinGetCmds {WINGET_CMD_INVALID, WINGET_CMD_ID, WINGET_CMD_IDLAST, WINGET_CMD_PID, WINGET_CMD_PROCESSNAME
	, WINGET_CMD_COUNT, WINGET_CMD_LIST, WINGET_CMD_MINMAX, WINGET_CMD_CONTROLLIST
	, WINGET_CMD_STYLE, WINGET_CMD_EXSTYLE, WINGET_CMD_TRANSPARENT, WINGET_CMD_TRANSCOLOR
};

enum SysGetCmds {SYSGET_CMD_INVALID, SYSGET_CMD_METRICS, SYSGET_CMD_MONITORCOUNT, SYSGET_CMD_MONITORPRIMARY
	, SYSGET_CMD_MONITORAREA, SYSGET_CMD_MONITORWORKAREA, SYSGET_CMD_MONITORNAME
};

enum TransformCmds {TRANS_CMD_INVALID, TRANS_CMD_ASC, TRANS_CMD_CHR, TRANS_CMD_DEREF
	, TRANS_CMD_UNICODE, TRANS_CMD_HTML
	, TRANS_CMD_MOD, TRANS_CMD_POW, TRANS_CMD_EXP, TRANS_CMD_SQRT, TRANS_CMD_LOG, TRANS_CMD_LN
	, TRANS_CMD_ROUND, TRANS_CMD_CEIL, TRANS_CMD_FLOOR, TRANS_CMD_ABS
	, TRANS_CMD_SIN, TRANS_CMD_COS, TRANS_CMD_TAN, TRANS_CMD_ASIN, TRANS_CMD_ACOS, TRANS_CMD_ATAN
	, TRANS_CMD_BITAND, TRANS_CMD_BITOR, TRANS_CMD_BITXOR, TRANS_CMD_BITNOT
	, TRANS_CMD_BITSHIFTLEFT, TRANS_CMD_BITSHIFTRIGHT
};

enum MenuCommands {MENU_CMD_INVALID, MENU_CMD_SHOW, MENU_CMD_USEERRORLEVEL
	, MENU_CMD_ADD, MENU_CMD_RENAME, MENU_CMD_CHECK, MENU_CMD_UNCHECK, MENU_CMD_TOGGLECHECK
	, MENU_CMD_ENABLE, MENU_CMD_DISABLE, MENU_CMD_TOGGLEENABLE
	, MENU_CMD_STANDARD, MENU_CMD_NOSTANDARD, MENU_CMD_COLOR, MENU_CMD_DEFAULT, MENU_CMD_NODEFAULT
	, MENU_CMD_DELETE, MENU_CMD_DELETEALL, MENU_CMD_TIP, MENU_CMD_ICON, MENU_CMD_NOICON
	, MENU_CMD_CLICK, MENU_CMD_MAINWINDOW, MENU_CMD_NOMAINWINDOW
};

enum GuiCommands {GUI_CMD_INVALID, GUI_CMD_OPTIONS, GUI_CMD_ADD, GUI_CMD_MENU, GUI_CMD_SHOW
	, GUI_CMD_SUBMIT, GUI_CMD_CANCEL, GUI_CMD_MINIMIZE, GUI_CMD_MAXIMIZE, GUI_CMD_RESTORE
	, GUI_CMD_DESTROY, GUI_CMD_FONT, GUI_CMD_TAB, GUI_CMD_DEFAULT
	, GUI_CMD_COLOR, GUI_CMD_FLASH
};

enum GuiControlCmds {GUICONTROL_CMD_INVALID, GUICONTROL_CMD_OPTIONS, GUICONTROL_CMD_CONTENTS, GUICONTROL_CMD_TEXT
	, GUICONTROL_CMD_MOVE, GUICONTROL_CMD_FOCUS, GUICONTROL_CMD_ENABLE, GUICONTROL_CMD_DISABLE
	, GUICONTROL_CMD_SHOW, GUICONTROL_CMD_HIDE, GUICONTROL_CMD_CHOOSE, GUICONTROL_CMD_CHOOSESTRING
	, GUICONTROL_CMD_FONT
};

enum GuiControlGetCmds {GUICONTROLGET_CMD_INVALID, GUICONTROLGET_CMD_CONTENTS, GUICONTROLGET_CMD_POS
	, GUICONTROLGET_CMD_FOCUS, GUICONTROLGET_CMD_ENABLED, GUICONTROLGET_CMD_VISIBLE
};

// Not done as an enum so that it can be a UCHAR type, which saves memory in the arrays of controls:
typedef UCHAR GuiControls;
#define GUI_CONTROL_INVALID      0  // This should be zero due to use of things like ZeroMemory() on the struct.
#define GUI_CONTROL_TEXT         1
#define GUI_CONTROL_PIC          2
#define GUI_CONTROL_GROUPBOX     3
#define GUI_CONTROL_BUTTON       4
#define GUI_CONTROL_CHECKBOX     5
#define GUI_CONTROL_RADIO        6
#define GUI_CONTROL_DROPDOWNLIST 7
#define GUI_CONTROL_COMBOBOX     8
#define GUI_CONTROL_LISTBOX      9
#define GUI_CONTROL_EDIT         10
#define GUI_CONTROL_HOTKEY       11
#define GUI_CONTROL_SLIDER       12
#define GUI_CONTROL_PROGRESS     13
#define GUI_CONTROL_TAB          14 // Keep below flush with above as a reminder to keep it in sync:
#define GUI_CONTROL_TYPE_CAN_BE_FOCUSED(type) !(type == GUI_CONTROL_TEXT || type == GUI_CONTROL_PIC \
	|| type == GUI_CONTROL_GROUPBOX || type == GUI_CONTROL_PROGRESS)

enum ThreadCommands {THREAD_CMD_INVALID, THREAD_CMD_PRIORITY, THREAD_CMD_INTERRUPT};

#define PROCESS_PRIORITY_LETTERS "LBNAHR"
enum ProcessCmds {PROCESS_CMD_INVALID, PROCESS_CMD_EXIST, PROCESS_CMD_CLOSE, PROCESS_CMD_PRIORITY
	, PROCESS_CMD_WAIT, PROCESS_CMD_WAITCLOSE};

enum ControlCmds {CONTROL_CMD_INVALID, CONTROL_CMD_CHECK, CONTROL_CMD_UNCHECK
	, CONTROL_CMD_ENABLE, CONTROL_CMD_DISABLE, CONTROL_CMD_SHOW, CONTROL_CMD_HIDE
	, CONTROL_CMD_STYLE, CONTROL_CMD_EXSTYLE
	, CONTROL_CMD_SHOWDROPDOWN, CONTROL_CMD_HIDEDROPDOWN
	, CONTROL_CMD_TABLEFT, CONTROL_CMD_TABRIGHT
	, CONTROL_CMD_ADD, CONTROL_CMD_DELETE, CONTROL_CMD_CHOOSE
	, CONTROL_CMD_CHOOSESTRING, CONTROL_CMD_EDITPASTE};

enum ControlGetCmds {CONTROLGET_CMD_INVALID, CONTROLGET_CMD_CHECKED, CONTROLGET_CMD_ENABLED
	, CONTROLGET_CMD_VISIBLE, CONTROLGET_CMD_TAB, CONTROLGET_CMD_FINDSTRING
	, CONTROLGET_CMD_CHOICE, CONTROLGET_CMD_LIST, CONTROLGET_CMD_LINECOUNT, CONTROLGET_CMD_CURRENTLINE
	, CONTROLGET_CMD_CURRENTCOL, CONTROLGET_CMD_LINE, CONTROLGET_CMD_SELECTED
	, CONTROLGET_CMD_STYLE, CONTROLGET_CMD_EXSTYLE};

enum DriveCmds {DRIVE_CMD_INVALID, DRIVE_CMD_EJECT, DRIVE_CMD_LOCK, DRIVE_CMD_UNLOCK, DRIVE_CMD_LABEL};

enum DriveGetCmds {DRIVEGET_CMD_INVALID, DRIVEGET_CMD_LIST, DRIVEGET_CMD_FILESYSTEM, DRIVEGET_CMD_LABEL
	, DRIVEGET_CMD_SETLABEL, DRIVEGET_CMD_SERIAL, DRIVEGET_CMD_TYPE, DRIVEGET_CMD_STATUS
	, DRIVEGET_CMD_STATUSCD, DRIVEGET_CMD_CAPACITY};

enum WinSetAttributes {WINSET_INVALID, WINSET_TRANSPARENT, WINSET_TRANSCOLOR, WINSET_ALWAYSONTOP
	, WINSET_BOTTOM, WINSET_STYLE, WINSET_EXSTYLE, WINSET_REDRAW};



class Line
{
public:
	// AutoIt3 functions:
	static bool Util_CopyDir (const char *szInputSource, const char *szInputDest, bool bOverwrite);
	static bool Util_MoveDir (const char *szInputSource, const char *szInputDest, bool bOverwrite);
	static bool Util_RemoveDir (const char *szInputSource, bool bRecurse);
	static int Util_CopyFile(const char *szInputSource, const char *szInputDest, bool bOverwrite, bool bMove);
	static void Util_ExpandFilenameWildcard(const char *szSource, const char *szDest, char *szExpandedDest);
	static void Util_ExpandFilenameWildcardPart(const char *szSource, const char *szDest, char *szExpandedDest);
	static bool Util_CreateDir(const char *szDirName);
	static bool Util_DoesFileExist(const char *szFilename);
	static bool Util_IsDir(const char *szPath);
	static void Util_GetFullPathName(const char *szIn, char *szOut);
	static void Util_StripTrailingDir(char *szPath);
	static bool Util_IsDifferentVolumes(const char *szPath1, const char *szPath2);

private:
	static char *sDerefBuf;  // Buffer to hold the values of any args that need to be dereferenced.
	static char *sDerefBufMarker;  // Location of next free byte.
	static size_t sDerefBufSize;

	// Static because only one line can be Expanded at a time (not to mention the fact that we
	// wouldn't want the size of each line to be expanded by this size):
	static char *sArgDeref[MAX_ARGS];

	ResultType EvaluateCondition();
	ResultType PerformLoop(WIN32_FIND_DATA *apCurrentFile, RegItemStruct *apCurrentRegItem
		, LoopReadFileStruct *apCurrentReadFile, char *aCurrentField, bool &aContinueMainLoop, Line *&aJumpToLine
		, AttributeType aAttr, FileLoopModeType aFileLoopMode, bool aRecurseSubfolders, char *aFilePattern
		, __int64 aIterationLimit, bool aIsInfinite, __int64 &aIndex);
	ResultType PerformLoopReg(WIN32_FIND_DATA *apCurrentFile, LoopReadFileStruct *apCurrentReadFile
		, char *aCurrentField, bool &aContinueMainLoop, Line *&aJumpToLine, FileLoopModeType aFileLoopMode
		, bool aRecurseSubfolders, HKEY aRootKeyType, HKEY aRootKey, char *aRegSubkey, __int64 &aIndex);
	ResultType PerformLoopParse(WIN32_FIND_DATA *apCurrentFile, RegItemStruct *apCurrentRegItem
		, LoopReadFileStruct *apCurrentReadFile, bool &aContinueMainLoop, Line *&aJumpToLine, __int64 &aIndex);
	ResultType Line::PerformLoopParseCSV(WIN32_FIND_DATA *apCurrentFile, RegItemStruct *apCurrentRegItem
		, LoopReadFileStruct *apCurrentReadFile, bool &aContinueMainLoop, Line *&aJumpToLine, __int64 &aIndex);
	ResultType PerformLoopReadFile(WIN32_FIND_DATA *apCurrentFile, RegItemStruct *apCurrentRegItem
		, char *aCurrentField, bool &aContinueMainLoop, Line *&aJumpToLine, FILE *aReadFile, char *aWriteFileName
		, __int64 &aIndex);
	ResultType Perform(WIN32_FIND_DATA *aCurrentFile, RegItemStruct *aCurrentRegItem, LoopReadFileStruct *aCurrentReadFile);
	ResultType FormatTime(char *aYYYYMMDD, char *aFormat);
	ResultType PerformAssign();
	ResultType StringSplit(char *aArrayName, char *aInputString, char *aDelimiterList, char *aOmitList);
	ResultType SplitPath(char *aFileSpec);
	ResultType PerformSort(char *aContents, char *aOptions);

	ResultType GetKeyJoyState(char *aKeyName, char *aOption);
	bool ScriptGetKeyState(vk_type aVK, KeyStateTypes aKeyStateType);
	double ScriptGetJoyState(JoyControls aJoy, int aJoystickID, Var *aOutputVar);

	ResultType DriveSpace(char *aPath, bool aGetFreeSpace);
	ResultType Drive(char *aCmd, char *aValue, char *aValue2);
	ResultType DriveLock(char aDriveLetter, bool aLockIt);
	ResultType DriveGet(char *aCmd, char *aValue);
	ResultType SoundSetGet(char *aSetting, DWORD aComponentType, int aComponentInstance
		, DWORD aControlType, UINT aMixerID);
	ResultType SoundGetWaveVolume(HWAVEOUT aDeviceID);
	ResultType SoundSetWaveVolume(char *aVolume, HWAVEOUT aDeviceID);
	ResultType SoundPlay(char *aFilespec, bool aSleepUntilDone);
	ResultType URLDownloadToFile(char *aURL, char *aFilespec);
	ResultType FileSelectFile(char *aOptions, char *aWorkingDir, char *aGreeting, char *aFilter);

	// Bitwise flags:
	#define FSF_ALLOW_CREATE 0x01
	#define FSF_EDITBOX      0x02
	ResultType FileSelectFolder(char *aRootDir, DWORD aOptions, char *aGreeting);

	ResultType FileGetShortcut(char *aShortcutFile);
	ResultType FileCreateShortcut(char *aTargetFile, char *aShortcutFile, char *aWorkingDir, char *aArgs
		, char *aDescription, char *aIconFile, char *aHotkey, char *aIconNumber, char *aRunState);
	ResultType FileCreateDir(char *aDirSpec);
	ResultType FileRead(char *aFilespec);
	ResultType FileReadLine(char *aFilespec, char *aLineNumber);
	ResultType FileAppend(char *aFilespec, char *aBuf, LoopReadFileStruct *aCurrentReadFile);
	ResultType FileDelete(char *aFilePattern);
	ResultType FileRecycle(char *aFilePattern);
	ResultType FileRecycleEmpty(char *aDriveLetter);
	ResultType FileInstall(char *aSource, char *aDest, char *aFlag);

	ResultType FileGetAttrib(char *aFilespec);
	int FileSetAttrib(char *aAttributes, char *aFilePattern, FileLoopModeType aOperateOnFolders
		, bool aDoRecurse, bool aCalledRecursively = false);
	ResultType FileGetTime(char *aFilespec, char aWhichTime);
	int FileSetTime(char *aYYYYMMDD, char *aFilePattern, char aWhichTime
		, FileLoopModeType aOperateOnFolders, bool aDoRecurse, bool aCalledRecursively = false);
	ResultType FileGetSize(char *aFilespec, char *aGranularity);
	ResultType FileGetVersion(char *aFilespec);

	ResultType IniRead(char *aFilespec, char *aSection, char *aKey, char *aDefault);
	ResultType IniWrite(char *aValue, char *aFilespec, char *aSection, char *aKey);
	ResultType IniDelete(char *aFilespec, char *aSection, char *aKey);
	ResultType RegRead(HKEY aRootKey, char *aRegSubkey, char *aValueName);
	ResultType RegWrite(DWORD aValueType, HKEY aRootKey, char *aRegSubkey, char *aValueName, char *aValue);
	ResultType RegDelete(HKEY aRootKey, char *aRegSubkey, char *aValueName);
	static bool RegRemoveSubkeys(HKEY hRegKey);

	ResultType PerformShowWindow(ActionTypeType aActionType, char *aTitle = "", char *aText = ""
		, char *aExcludeTitle = "", char *aExcludeText = "");
	ResultType Splash(char *aOptions, char *aSubText, char *aMainText, char *aTitle, char *aFontName
		, char *aImageFile, bool aSplashImage);
	ResultType ToolTip(char *aText, char *aX, char *aY, char *aID);
	ResultType TrayTip(char *aTitle, char *aText, char *aTimeout, char *aOptions);
	ResultType Transform(char *aCmd, char *aValue1, char *aValue2);
	ResultType Input(char *aOptions, char *aEndKeys, char *aMatchList);
	ResultType WinMove(char *aTitle, char *aText, char *aX, char *aY
		, char *aWidth = "", char *aHeight = "", char *aExcludeTitle = "", char *aExcludeText = "");
	ResultType WinMenuSelectItem(char *aTitle, char *aText, char *aMenu1, char *aMenu2
		, char *aMenu3, char *aMenu4, char *aMenu5, char *aMenu6, char *aMenu7
		, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlSend(char *aControl, char *aKeysToSend, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText, bool aSendRaw);
	ResultType ControlClick(vk_type aVK, int aClickCount, char *aOptions, char *aControl
		, char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlMove(char *aControl, char *aX, char *aY, char *aWidth, char *aHeight
		, char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlGetPos(char *aControl, char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlGetFocus(char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlFocus(char *aControl, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlSetText(char *aControl, char *aNewText, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlGetText(char *aControl, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType Control(char *aCmd, char *aValue, char *aControl, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType ControlGet(char *aCommand, char *aValue, char *aControl, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType GuiControl(char *aCommand, char *aControlID, char *aParam3);
	ResultType GuiControlGet(char *aCommand, char *aControlID, char *aParam3);
	ResultType StatusBarGetText(char *aPart, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType StatusBarWait(char *aTextToWaitFor, char *aSeconds, char *aPart, char *aTitle, char *aText
		, char *aInterval, char *aExcludeTitle, char *aExcludeText);
	ResultType ScriptPostMessage(char *aMsg, char *awParam, char *alParam, char *aControl
		, char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType ScriptSendMessage(char *aMsg, char *awParam, char *alParam, char *aControl
		, char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType ScriptProcess(char *aCmd, char *aProcess, char *aParam3);
	ResultType WinSet(char *aAttrib, char *aValue, char *aTitle, char *aText
		, char *aExcludeTitle, char *aExcludeText);
	ResultType WinSetTitle(char *aTitle, char *aText, char *aNewTitle
		, char *aExcludeTitle = "", char *aExcludeText = "");
	ResultType WinGetTitle(char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType WinGetClass(char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType WinGet(char *aCmd, char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType WinGetControlList(Var *aOutputVar, HWND aTargetWindow);
	ResultType WinGetText(char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType WinGetPos(char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);
	ResultType SysGet(char *aCmd, char *aValue);
	ResultType PixelSearch(int aLeft, int aTop, int aRight, int aBottom, COLORREF aColorBGR, int aVariation, char *aOptions);
	ResultType ImageSearch(int aLeft, int aTop, int aRight, int aBottom, char *aImageFile);
	ResultType PixelGetColor(int aX, int aY, bool aUseRGB);

	static ResultType SetToggleState(vk_type aVK, ToggleValueType &ForceLock, char *aToggleText);
public:
	// Keep any fields that aren't an even multiple of 4 adjacent to each other.  This conserves memory
	// due to byte-alignment:
	ActionTypeType mActionType; // What type of line this is.
	UCHAR mFileNumber;  // Which file the line came from.  0 is the first, and it's the main script file.
	ArgCountType mArgc; // How many arguments exist in mArg[].

	ArgStruct *mArg; // Will be used to hold a dynamic array of dynamic Args.
	LineNumberType mLineNumber;  // The line number in the file from which the script was loaded, for debugging.
	AttributeType mAttribute;
	Line *mPrevLine, *mNextLine; // The prev & next lines adjacent to this one in the linked list; NULL if none.
	Line *mRelatedLine;  // e.g. the "else" that belongs to this "if"
	Line *mParentLine; // Indicates the parent (owner) of this line.
	// Probably best to always use ARG1 even if other things have supposedly verfied
	// that it exists, since it's count-check should make the dereference of a NULL
	// pointer (or accessing non-existent array elements) virtually impossible.
	// Empty-string is probably more universally useful than NULL, since some
	// API calls and other functions might not appreciate receiving NULLs.  In addition,
	// always remembering to have to check for NULL makes things harder to maintain
	// and more bug-prone.  The below macros rely upon the fact that the individual
	// elements of mArg cannot be NULL (because they're explicitly set to be blank
	// when the user has omitted an arg in between two non-blank args).  Later, might
	// want to review if any of the API calls used expect a string whose contents are
	// modifiable.
	#define RAW_ARG1 (mArgc > 0 ? mArg[0].text : "")
	#define RAW_ARG2 (mArgc > 1 ? mArg[1].text : "")
	#define RAW_ARG3 (mArgc > 2 ? mArg[2].text : "")
	#define RAW_ARG4 (mArgc > 3 ? mArg[3].text : "")
	#define RAW_ARG5 (mArgc > 4 ? mArg[4].text : "")
	#define RAW_ARG6 (mArgc > 5 ? mArg[5].text : "")
	#define RAW_ARG7 (mArgc > 6 ? mArg[6].text : "")
	#define RAW_ARG8 (mArgc > 7 ? mArg[7].text : "")
	#define LINE_RAW_ARG1 (line->mArgc > 0 ? line->mArg[0].text : "")
	#define LINE_RAW_ARG2 (line->mArgc > 1 ? line->mArg[1].text : "")
	#define LINE_RAW_ARG3 (line->mArgc > 2 ? line->mArg[2].text : "")
	#define LINE_RAW_ARG4 (line->mArgc > 3 ? line->mArg[3].text : "")
	#define LINE_RAW_ARG5 (line->mArgc > 4 ? line->mArg[4].text : "")
	#define LINE_RAW_ARG6 (line->mArgc > 5 ? line->mArg[5].text : "")
	#define LINE_RAW_ARG7 (line->mArgc > 6 ? line->mArg[6].text : "")
	#define LINE_RAW_ARG8 (line->mArgc > 7 ? line->mArg[7].text : "")
	#define LINE_RAW_ARG9 (line->mArgc > 8 ? line->mArg[8].text : "")
	#define NEW_RAW_ARG1 (aArgc > 0 ? new_arg[0].text : "") // Helps performance to use this vs. LINE_RAW_ARG where possible.
	#define NEW_RAW_ARG2 (aArgc > 1 ? new_arg[1].text : "")
	#define NEW_RAW_ARG3 (aArgc > 2 ? new_arg[2].text : "")
	#define NEW_RAW_ARG4 (aArgc > 3 ? new_arg[3].text : "")
	#define NEW_RAW_ARG5 (aArgc > 4 ? new_arg[4].text : "")
	#define NEW_RAW_ARG6 (aArgc > 5 ? new_arg[5].text : "")
	#define NEW_RAW_ARG7 (aArgc > 6 ? new_arg[6].text : "")
	#define NEW_RAW_ARG8 (aArgc > 7 ? new_arg[7].text : "")
	#define NEW_RAW_ARG9 (aArgc > 8 ? new_arg[8].text : "")
	#define LINE_ARG1 (line->mArgc > 0 ? line->sArgDeref[0] : "")
	#define LINE_ARG2 (line->mArgc > 1 ? line->sArgDeref[1] : "")
	#define LINE_ARG3 (line->mArgc > 2 ? line->sArgDeref[2] : "")
	#define LINE_ARG4 (line->mArgc > 3 ? line->sArgDeref[3] : "")
	#define SAVED_ARG1 (mArgc > 0 ? arg[0] : "")
	#define SAVED_ARG2 (mArgc > 1 ? arg[1] : "")
	#define SAVED_ARG3 (mArgc > 2 ? arg[2] : "")
	#define SAVED_ARG4 (mArgc > 3 ? arg[3] : "")
	#define SAVED_ARG5 (mArgc > 4 ? arg[4] : "")
	#define ARG1 (mArgc > 0 ? sArgDeref[0] : "")
	#define ARG2 (mArgc > 1 ? sArgDeref[1] : "")
	#define ARG3 (mArgc > 2 ? sArgDeref[2] : "")
	#define ARG4 (mArgc > 3 ? sArgDeref[3] : "")
	#define ARG5 (mArgc > 4 ? sArgDeref[4] : "")
	#define ARG6 (mArgc > 5 ? sArgDeref[5] : "")
	#define ARG7 (mArgc > 6 ? sArgDeref[6] : "")
	#define ARG8 (mArgc > 7 ? sArgDeref[7] : "")
	#define ARG9 (mArgc > 8 ? sArgDeref[8] : "")
	#define ARG10 (mArgc > 9 ? sArgDeref[9] : "")
	#define ARG11 (mArgc > 10 ? sArgDeref[10] : "")
	#define TWO_ARGS ARG1, ARG2
	#define THREE_ARGS  ARG1, ARG2, ARG3
	#define FOUR_ARGS   ARG1, ARG2, ARG3, ARG4
	#define FIVE_ARGS   ARG1, ARG2, ARG3, ARG4, ARG5
	#define SIX_ARGS    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6
	#define SEVEN_ARGS  ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7
	#define EIGHT_ARGS  ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8
	#define NINE_ARGS   ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9
	#define TEN_ARGS    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10
	#define ELEVEN_ARGS ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11
	// If the arg's text is non-blank, it means the variable is a dynamic name such as array%i%
	// that must be resolved at runtime rather than load-time.  Therefore, this macro must not
	// be used without first having checked that arg.text is blank:
	#define VAR(arg) ((Var *)arg.deref)
	// Uses arg number (i.e. the first arg is 1, not 0).  Caller must ensure that ArgNum >= 1 and that
	// the arg in question is an input or output variable (since that isn't checked there):
	#define ARG_HAS_VAR(ArgNum) (mArgc >= ArgNum && (*mArg[ArgNum-1].text || mArg[ArgNum-1].deref))

	// Shouldn't go much higher than 200 since the main window's Edit control is currently limited
	// to 32K to be compatible with the Win9x limit.  Avg. line length is probably under 100 for
	// the vast majority of scripts, so 200 seems unlikely to exceed the buffer size.  Even in the
	// worst case where the buffer size is exceeded, the text is simply truncated, so it's not too
	// bad:
	#define LINE_LOG_SIZE 200
	static Line *sLog[LINE_LOG_SIZE];
	static DWORD sLogTick[LINE_LOG_SIZE];
	static int sLogNext;

	#define MAX_SCRIPT_FILES (UCHAR_MAX + 1)
	static char *sSourceFile[MAX_SCRIPT_FILES];
	static int nSourceFiles; // An int vs. UCHAR so that it can be exactly 256 without overflowing.

	static void FreeDerefBuf()
	{
		if (sDerefBuf)
		{
			free(sDerefBuf);
			sDerefBuf = NULL;
			sDerefBufSize = 0;
		}
	}

	static ResultType MouseClickDrag(vk_type aVK // Which button.
		, int aX1, int aY1, int aX2, int aY2, int aSpeed, bool aMoveRelative);
	static ResultType MouseClick(vk_type aVK // Which button.
		, int aX = COORD_UNSPECIFIED, int aY = COORD_UNSPECIFIED // These values signal us not to move the mouse.
		, int aClickCount = 1, int aSpeed = DEFAULT_MOUSE_SPEED, KeyEventTypes aEventType = KEYDOWNANDUP, bool aMoveRelative = false);
	static void MouseMove(int aX, int aY, int aSpeed = DEFAULT_MOUSE_SPEED, bool aMoveRelative = false);
	ResultType MouseGetPos(bool aSimpleMode);
	static void MouseEvent(DWORD aEventFlags, DWORD aX = 0, DWORD aY = 0, DWORD aData = 0)
	// A small inline to help us remember to use KEY_IGNORE so that our own mouse
	// events won't be falsely detected as hotkeys by the hooks (if they are installed).
	{
		mouse_event(aEventFlags, aX, aY, aData, KEY_IGNORE);
	}

	ResultType ExecUntil(ExecUntilMode aMode, Line **apJumpToLine = NULL
		, WIN32_FIND_DATA *aCurrentFile = NULL, RegItemStruct *aCurrentRegItem = NULL
		, LoopReadFileStruct *aCurrentReadFile = NULL, char *aCurrentField = NULL
		, __int64 aCurrentLoopIteration = 0); // Use signed, since script/ITOA64 aren't designed to work with unsigned.

	// The characters common to both EXP_TELLTALES and EXP_OPERAND_TERMINATORS:
	#define EXP_COMMON "<>=*/&|^~!()"
	// Characters whose presence in a mandatory-numeric param make it an expression for certain.
	// + and - are not included here because legacy numeric parameters can contain unary plus or minus,
	// e.g. WinMove, -%x%, -%y%:
	#define EXP_TELLTALES "\"" EXP_COMMON
	// Characters that mark the end of an operand inside an expression.  Double-quote must not be included:
	#define EXP_OPERAND_TERMINATORS "\t +-" EXP_COMMON
	#define EXP_ILLEGAL_CHARS ",'\\:;`{}" // Characters illegal in an expression.

	Var *ResolveVarOfArg(int aArgIndex, bool aCreateIfNecessary = true);
	ResultType ExpandArgs();
	VarSizeType GetExpandedArgSize(bool aCalcDerefBufSize);
	char *ExpandArg(char *aBuf, int aArgIndex);
	char *ExpandExpression(char *aBuf, int aArgIndex);
	ResultType Deref(Var *aOutputVar, char *aBuf);

	bool FileIsFilteredOut(WIN32_FIND_DATA &aCurrentFile, FileLoopModeType aFileLoopMode, char *aFilePath);

	Line *GetJumpTarget(bool aIsDereferenced);
	ResultType IsJumpValid(Line *aDestination);

	HWND DetermineTargetWindow(char *aTitle, char *aText, char *aExcludeTitle, char *aExcludeText);

#ifndef AUTOHOTKEYSC
	static int ConvertEscapeChar(char *aFilespec, char aOldChar, char aNewChar, bool aFromAutoIt2 = false);
	static size_t ConvertEscapeCharGetLine(char *aBuf, int aMaxCharsToRead, FILE *fp);
#endif  // The functions above are not needed by the self-contained version.

	
	// This is in the .h file so that it's more likely the compiler's cost/benefit estimate will
	// make it inline (since it is called from only one place).  Inline would be good since it
	// is called frequently during script loading and is difficult to macro-ize in a way that
	// retains readability.
	static ArgTypeType ArgIsVar(ActionTypeType aActionType, int aArgIndex)
	{
		switch(aArgIndex)
		{
		case 0:  // Arg #1
			switch(aActionType)
			{
			case ACT_ASSIGN:
			case ACT_ASSIGNEXPR:
			case ACT_ADD:
			case ACT_SUB:
			case ACT_MULT:
			case ACT_DIV:
			case ACT_TRANSFORM:
			case ACT_STRINGLEFT:
			case ACT_STRINGRIGHT:
			case ACT_STRINGMID:
			case ACT_STRINGTRIMLEFT:
			case ACT_STRINGTRIMRIGHT:
			case ACT_STRINGLOWER:
			case ACT_STRINGUPPER:
			case ACT_STRINGLEN:
			case ACT_STRINGREPLACE:
			case ACT_STRINGGETPOS:
			case ACT_GETKEYSTATE:
			case ACT_CONTROLGETFOCUS:
			case ACT_CONTROLGETTEXT:
			case ACT_CONTROLGET:
			case ACT_GUICONTROLGET:
			case ACT_STATUSBARGETTEXT:
			case ACT_INPUTBOX:
			case ACT_RANDOM:
			case ACT_INIREAD:
			case ACT_REGREAD:
			case ACT_DRIVESPACEFREE:
			case ACT_DRIVEGET:
			case ACT_SOUNDGET:
			case ACT_SOUNDGETWAVEVOLUME:
			case ACT_FILEREAD:
			case ACT_FILEREADLINE:
			case ACT_FILEGETATTRIB:
			case ACT_FILEGETTIME:
			case ACT_FILEGETSIZE:
			case ACT_FILEGETVERSION:
			case ACT_FILESELECTFILE:
			case ACT_FILESELECTFOLDER:
			case ACT_MOUSEGETPOS:
			case ACT_WINGETTITLE:
			case ACT_WINGETCLASS:
			case ACT_WINGET:
			case ACT_WINGETTEXT:
			case ACT_WINGETPOS:
			case ACT_SYSGET:
			case ACT_CONTROLGETPOS:
			case ACT_PIXELGETCOLOR:
			case ACT_PIXELSEARCH:
			case ACT_IMAGESEARCH:
			case ACT_INPUT:
			case ACT_FORMATTIME:
				return ARG_TYPE_OUTPUT_VAR;

			case ACT_SORT:
			case ACT_SPLITPATH:
			case ACT_IFINSTRING:
			case ACT_IFNOTINSTRING:
			case ACT_IFEQUAL:
			case ACT_IFNOTEQUAL:
			case ACT_IFGREATER:
			case ACT_IFGREATEROREQUAL:
			case ACT_IFLESS:
			case ACT_IFLESSOREQUAL:
			case ACT_IFBETWEEN:
			case ACT_IFNOTBETWEEN:
			case ACT_IFIN:
			case ACT_IFNOTIN:
			case ACT_IFCONTAINS:
			case ACT_IFNOTCONTAINS:
			case ACT_IFIS:
			case ACT_IFISNOT:
				return ARG_TYPE_INPUT_VAR;
			}
			break;

		case 1:  // Arg #2
			switch(aActionType)
			{
			case ACT_STRINGLEFT:
			case ACT_STRINGRIGHT:
			case ACT_STRINGMID:
			case ACT_STRINGTRIMLEFT:
			case ACT_STRINGTRIMRIGHT:
			case ACT_STRINGLOWER:
			case ACT_STRINGUPPER:
			case ACT_STRINGLEN:
			case ACT_STRINGREPLACE:
			case ACT_STRINGGETPOS:
			case ACT_STRINGSPLIT:
				return ARG_TYPE_INPUT_VAR;

			case ACT_MOUSEGETPOS:
			case ACT_WINGETPOS:
			case ACT_CONTROLGETPOS:
			case ACT_PIXELSEARCH:
			case ACT_IMAGESEARCH:
			case ACT_SPLITPATH:
			case ACT_FILEGETSHORTCUT:
				return ARG_TYPE_OUTPUT_VAR;
			}
			break;

		case 2:  // Arg #3
			switch(aActionType)
			{
			case ACT_WINGETPOS:
			case ACT_CONTROLGETPOS:
			case ACT_MOUSEGETPOS:
			case ACT_SPLITPATH:
			case ACT_FILEGETSHORTCUT:
				return ARG_TYPE_OUTPUT_VAR;
			}
			break;

		case 3:  // Arg #4
			switch(aActionType)
			{
			case ACT_WINGETPOS:
			case ACT_CONTROLGETPOS:
			case ACT_MOUSEGETPOS:
			case ACT_SPLITPATH:
			case ACT_FILEGETSHORTCUT:
			case ACT_RUN:
				return ARG_TYPE_OUTPUT_VAR;
			}
			break;

		case 4:  // Arg #5
		case 5:  // Arg #6
			if (aActionType == ACT_SPLITPATH || aActionType == ACT_FILEGETSHORTCUT)
				return ARG_TYPE_OUTPUT_VAR;
			break;

		case 6:  // Arg #7
		case 7:  // Arg #8
			if (aActionType == ACT_FILEGETSHORTCUT)
				return ARG_TYPE_OUTPUT_VAR;
		}
		// Otherwise:
		return ARG_TYPE_NORMAL;
	}



	ResultType ArgMustBeDereferenced(Var *aVar, int aArgIndexToExclude);
	bool ArgHasDeref(int aArgNum)
	// This function should always be called in lieu of doing something like "strchr(arg.text, g_DerefChar)"
	// because that method is unreliable due to the possible presence of literal (escaped) g_DerefChars
	// in the text.
	// Caller must ensure that aArgNum should be 1 or greater.
	{
#ifdef _DEBUG
		if (!aArgNum)
		{
			LineError("BAD", WARN);
			++aArgNum;  // But let it continue.
		}
#endif
		if (aArgNum > mArgc) // Arg doesn't exist.
			return false;
		ArgStruct &arg = mArg[aArgNum - 1]; // For performance.
		// Return false if it's not of a type caller wants deemed to have derefs.
		// Relies on short-circuit boolean evaluation order to prevent NULL-deref:
		return (arg.type == ARG_TYPE_NORMAL) ? arg.deref && arg.deref[0].marker : false;
	}



	static HKEY RegConvertRootKey(char *aBuf, bool *aIsRemoteRegistry = NULL)
	{
		// Even if the computer name is a single letter, it seems like using a colon as delimiter is ok
		// (e.g. a:HKEY_LOCAL_MACHINE), since we wouldn't expect the root key to be used as a filename
		// in that exact way, i.e. a drive letter should be followed by a backslash 99% of the time in
		// this context.
		// Research indicates that colon is an illegal char in a computer name (at least for NT,
		// and by extension probably all other OSes).  So it should be safe to use it as a delimiter
		// for the remote registry feature.  But just in case, get the right-most one,
		// e.g. Computer:01:HKEY_LOCAL_MACHINE  ; the first colon is probably illegal on all OSes.
		// Additional notes from the Internet:
		// "A Windows NT computer name can be up to 15 alphanumeric characters with no blank spaces
		// and must be unique on the network. It can contain the following special characters:
		// ! @ # $ % ^ & ( ) -   ' { } .
		// It may not contain:
		// \ * + = | : ; " ? ,
		// The following is a list of illegal characters in a computer name:
		// regEx.Pattern = "`|~|!|@|#|\$|\^|\&|\*|\(|\)|\=|\+|{|}|\\|;|:|'|<|>|/|\?|\||%"

		char *colon_pos = strrchr(aBuf, ':');
		char *key_name = colon_pos ? omit_leading_whitespace(colon_pos + 1) : aBuf;
		if (aIsRemoteRegistry) // Caller wanted the below put into the output parameter.
			*aIsRemoteRegistry = (colon_pos != NULL);
		HKEY root_key = NULL; // Set default.
		if (!stricmp(key_name, "HKEY_LOCAL_MACHINE") || !stricmp(key_name, "HKLM"))  root_key = HKEY_LOCAL_MACHINE;
		if (!stricmp(key_name, "HKEY_CLASSES_ROOT") || !stricmp(key_name, "HKCR"))   root_key = HKEY_CLASSES_ROOT;
		if (!stricmp(key_name, "HKEY_CURRENT_CONFIG") || !stricmp(key_name, "HKCC")) root_key = HKEY_CURRENT_CONFIG;
		if (!stricmp(key_name, "HKEY_CURRENT_USER") || !stricmp(key_name, "HKCU"))   root_key = HKEY_CURRENT_USER;
		if (!stricmp(key_name, "HKEY_USERS") || !stricmp(key_name, "HKU"))           root_key = HKEY_USERS;
		if (!root_key)  // Invalid or unsupported root key name.
			return NULL;
		if (!aIsRemoteRegistry || !colon_pos) // Either caller didn't want it opened, or it doesn't need to be.
			return root_key; // If it's a remote key, this value should only be used by the caller as an indicator.
		// Otherwise, it's a remote computer whose registry the caller wants us to open:
		// It seems best to require the two leading backslashes in case the computer name contains
		// spaces (just in case spaces are allowed on some OSes or perhaps for Unix interoperability, etc.).
		// Therefore, make no attempt to trim leading and trailing spaces from the computer name:
		char computer_name[128];
		strlcpy(computer_name, aBuf, sizeof(computer_name));
		computer_name[colon_pos - aBuf] = '\0';
		HKEY remote_key;
		return (RegConnectRegistry(computer_name, root_key, &remote_key) == ERROR_SUCCESS) ? remote_key : NULL;
	}

	static char *RegConvertRootKey(char *aBuf, size_t aBufSize, HKEY aRootKey)
	{
		// switch() doesn't directly support expression of type HKEY:
		if (aRootKey == HKEY_LOCAL_MACHINE)       strlcpy(aBuf, "HKEY_LOCAL_MACHINE", aBufSize);
		else if (aRootKey == HKEY_CLASSES_ROOT)   strlcpy(aBuf, "HKEY_CLASSES_ROOT", aBufSize);
		else if (aRootKey == HKEY_CURRENT_CONFIG) strlcpy(aBuf, "HKEY_CURRENT_CONFIG", aBufSize);
		else if (aRootKey == HKEY_CURRENT_USER)   strlcpy(aBuf, "HKEY_CURRENT_USER", aBufSize);
		else if (aRootKey == HKEY_USERS)          strlcpy(aBuf, "HKEY_USERS", aBufSize);
		else if (aBufSize)                        *aBuf = '\0'; // Make it be the empty string for anything else.
		// These are either unused or so rarely used (DYN_DATA on Win9x) that they aren't supported:
		// HKEY_PERFORMANCE_DATA, HKEY_PERFORMANCE_TEXT, HKEY_PERFORMANCE_NLSTEXT, HKEY_DYN_DATA
		return aBuf;
	}
	static int RegConvertValueType(char *aValueType)
	{
		if (!stricmp(aValueType, "REG_SZ")) return REG_SZ;
		if (!stricmp(aValueType, "REG_EXPAND_SZ")) return REG_EXPAND_SZ;
		if (!stricmp(aValueType, "REG_MULTI_SZ")) return REG_MULTI_SZ;
		if (!stricmp(aValueType, "REG_DWORD")) return REG_DWORD;
		if (!stricmp(aValueType, "REG_BINARY")) return REG_BINARY;
		return REG_NONE; // Unknown or unsupported type.
	}
	static char *RegConvertValueType(char *aBuf, size_t aBufSize, DWORD aValueType)
	{
		switch(aValueType)
		{
		case REG_SZ: strlcpy(aBuf, "REG_SZ", aBufSize); return aBuf;
		case REG_EXPAND_SZ: strlcpy(aBuf, "REG_EXPAND_SZ", aBufSize); return aBuf;
		case REG_BINARY: strlcpy(aBuf, "REG_BINARY", aBufSize); return aBuf;
		case REG_DWORD: strlcpy(aBuf, "REG_DWORD", aBufSize); return aBuf;
		case REG_DWORD_BIG_ENDIAN: strlcpy(aBuf, "REG_DWORD_BIG_ENDIAN", aBufSize); return aBuf;
		case REG_LINK: strlcpy(aBuf, "REG_LINK", aBufSize); return aBuf;
		case REG_MULTI_SZ: strlcpy(aBuf, "REG_MULTI_SZ", aBufSize); return aBuf;
		case REG_RESOURCE_LIST: strlcpy(aBuf, "REG_RESOURCE_LIST", aBufSize); return aBuf;
		case REG_FULL_RESOURCE_DESCRIPTOR: strlcpy(aBuf, "REG_FULL_RESOURCE_DESCRIPTOR", aBufSize); return aBuf;
		case REG_RESOURCE_REQUIREMENTS_LIST: strlcpy(aBuf, "REG_RESOURCE_REQUIREMENTS_LIST", aBufSize); return aBuf;
		case REG_QWORD: strlcpy(aBuf, "REG_QWORD", aBufSize); return aBuf;
		case REG_SUBKEY: strlcpy(aBuf, "KEY", aBufSize); return aBuf;  // Custom (non-standard) type.
		default: if (aBufSize) *aBuf = '\0'; return aBuf;  // Make it be the empty string for REG_NONE and anything else.
		}
	}

	static DWORD SoundConvertComponentType(char *aBuf, int *aInstanceNumber = NULL)
	{
		char *colon_pos = strchr(aBuf, ':');
		UINT length_to_check = (UINT)(colon_pos ? colon_pos - aBuf : strlen(aBuf));
		if (aInstanceNumber) // Caller wanted the below put into the output parameter.
		{
			if (colon_pos)
			{
				*aInstanceNumber = ATOI(colon_pos + 1);
				if (*aInstanceNumber < 1)
					*aInstanceNumber = 1;
			}
			else
				*aInstanceNumber = 1;
		}
		if (!strlicmp(aBuf, "Master", length_to_check)) return MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
		if (!strlicmp(aBuf, "Speakers", length_to_check)) return MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
		if (!strlicmp(aBuf, "Digital", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_DIGITAL;
		if (!strlicmp(aBuf, "Line", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_LINE;
		if (!strlicmp(aBuf, "Microphone", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
		if (!strlicmp(aBuf, "Synth", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER;
		if (!strlicmp(aBuf, "CD", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;
		if (!strlicmp(aBuf, "Telephone", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE;
		if (!strlicmp(aBuf, "PCSpeaker", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER;
		if (!strlicmp(aBuf, "Wave", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
		if (!strlicmp(aBuf, "Aux", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY;
		if (!strlicmp(aBuf, "Analog", length_to_check)) return MIXERLINE_COMPONENTTYPE_SRC_ANALOG;
		return MIXERLINE_COMPONENTTYPE_DST_UNDEFINED;
	}
	static DWORD SoundConvertControlType(char *aBuf)
	{
		// These are the types that seem to correspond to actual sound attributes.  Some of the values
		// are not included here, such as MIXERCONTROL_CONTROLTYPE_FADER, which seems to be a type of
		// sound control rather than a quality of the sound itself.  For performance, put the most
		// often used ones up top:
		if (!stricmp(aBuf, "Vol")) return MIXERCONTROL_CONTROLTYPE_VOLUME;
		if (!stricmp(aBuf, "Volume")) return MIXERCONTROL_CONTROLTYPE_VOLUME;
		if (!stricmp(aBuf, "OnOff")) return MIXERCONTROL_CONTROLTYPE_ONOFF;
		if (!stricmp(aBuf, "Mute")) return MIXERCONTROL_CONTROLTYPE_MUTE;
		if (!stricmp(aBuf, "Mono")) return MIXERCONTROL_CONTROLTYPE_MONO;
		if (!stricmp(aBuf, "Loudness")) return MIXERCONTROL_CONTROLTYPE_LOUDNESS;
		if (!stricmp(aBuf, "StereoEnh")) return MIXERCONTROL_CONTROLTYPE_STEREOENH;
		if (!stricmp(aBuf, "BassBoost")) return MIXERCONTROL_CONTROLTYPE_BASS_BOOST;
		if (!stricmp(aBuf, "Pan")) return MIXERCONTROL_CONTROLTYPE_PAN;
		if (!stricmp(aBuf, "QSoundPan")) return MIXERCONTROL_CONTROLTYPE_QSOUNDPAN;
		if (!stricmp(aBuf, "Bass")) return MIXERCONTROL_CONTROLTYPE_BASS;
		if (!stricmp(aBuf, "Treble")) return MIXERCONTROL_CONTROLTYPE_TREBLE;
		if (!stricmp(aBuf, "Equalizer")) return MIXERCONTROL_CONTROLTYPE_EQUALIZER;
		#define MIXERCONTROL_CONTROLTYPE_INVALID 0 // 0 seems like a safe "undefined" indicator for this type.
		return MIXERCONTROL_CONTROLTYPE_INVALID;
	}

	static int ConvertJoy(char *aBuf, int *aJoystickID = NULL, bool aAllowOnlyButtons = false)
	{
		if (aJoystickID)
			*aJoystickID = 0;  // Set default output value for the caller.
		if (!aBuf || !*aBuf) return JOYCTRL_INVALID;
		char *aBuf_orig = aBuf;
		for (; *aBuf >= '0' && *aBuf <= '9'; ++aBuf); // self-contained loop to find the first non-digit.
		if (aBuf > aBuf_orig) // The string starts with a number.
		{
			int joystick_id = ATOI(aBuf_orig) - 1;
			if (joystick_id < 0 || joystick_id >= MAX_JOYSTICKS)
				return JOYCTRL_INVALID;
			if (aJoystickID)
				*aJoystickID = joystick_id;  // Use ATOI vs. atoi even though hex isn't supported yet.
		}

		if (!strnicmp(aBuf, "Joy", 3))
		{
			if (IsPureNumeric(aBuf + 3, false, false))
			{
				int offset = ATOI(aBuf + 3);
				if (offset < 1 || offset > MAX_JOY_BUTTONS)
					return JOYCTRL_INVALID;
				return JOYCTRL_1 + offset - 1;
			}
		}
		if (aAllowOnlyButtons)
			return JOYCTRL_INVALID;

		// Otherwise:
		if (!stricmp(aBuf, "JoyX")) return JOYCTRL_XPOS;
		if (!stricmp(aBuf, "JoyY")) return JOYCTRL_YPOS;
		if (!stricmp(aBuf, "JoyZ")) return JOYCTRL_ZPOS;
		if (!stricmp(aBuf, "JoyR")) return JOYCTRL_RPOS;
		if (!stricmp(aBuf, "JoyU")) return JOYCTRL_UPOS;
		if (!stricmp(aBuf, "JoyV")) return JOYCTRL_VPOS;
		if (!stricmp(aBuf, "JoyPOV")) return JOYCTRL_POV;
		if (!stricmp(aBuf, "JoyName")) return JOYCTRL_NAME;
		if (!stricmp(aBuf, "JoyButtons")) return JOYCTRL_BUTTONS;
		if (!stricmp(aBuf, "JoyAxes")) return JOYCTRL_AXES;
		if (!stricmp(aBuf, "JoyInfo")) return JOYCTRL_INFO;
		return JOYCTRL_INVALID;
	}

	static TitleMatchModes ConvertTitleMatchMode(char *aBuf)
	{
		if (!aBuf || !*aBuf) return MATCHMODE_INVALID;
		if (*aBuf == '1' && !*(aBuf + 1)) return FIND_IN_LEADING_PART;
		if (*aBuf == '2' && !*(aBuf + 1)) return FIND_ANYWHERE;
		if (*aBuf == '3' && !*(aBuf + 1)) return FIND_EXACT;
		if (!stricmp(aBuf, "FAST")) return FIND_FAST;
		if (!stricmp(aBuf, "SLOW")) return FIND_SLOW;
		return MATCHMODE_INVALID;
	}

	static WinGetCmds ConvertWinGetCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return WINGET_CMD_ID;  // If blank, return the default command.
		if (!stricmp(aBuf, "ID")) return WINGET_CMD_ID;
		if (!stricmp(aBuf, "IDLast")) return WINGET_CMD_IDLAST;
		if (!stricmp(aBuf, "PID")) return WINGET_CMD_PID;
		if (!stricmp(aBuf, "ProcessName")) return WINGET_CMD_PROCESSNAME;
		if (!stricmp(aBuf, "Count")) return WINGET_CMD_COUNT;
		if (!stricmp(aBuf, "List")) return WINGET_CMD_LIST;
		if (!stricmp(aBuf, "MinMax")) return WINGET_CMD_MINMAX;
		if (!stricmp(aBuf, "ControlList")) return WINGET_CMD_CONTROLLIST;
		if (!stricmp(aBuf, "Style")) return WINGET_CMD_STYLE;
		if (!stricmp(aBuf, "ExStyle")) return WINGET_CMD_EXSTYLE;
		if (!stricmp(aBuf, "Transparent")) return WINGET_CMD_TRANSPARENT;
		if (!stricmp(aBuf, "TransColor")) return WINGET_CMD_TRANSCOLOR;
		return WINGET_CMD_INVALID;
	}

	static SysGetCmds ConvertSysGetCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return SYSGET_CMD_INVALID;
		if (IsPureNumeric(aBuf)) return SYSGET_CMD_METRICS;
		if (!stricmp(aBuf, "MonitorCount")) return SYSGET_CMD_MONITORCOUNT;
		if (!stricmp(aBuf, "MonitorPrimary")) return SYSGET_CMD_MONITORPRIMARY;
		if (!stricmp(aBuf, "Monitor")) return SYSGET_CMD_MONITORAREA; // Called "Monitor" vs. "MonitorArea" to make it easier to remember.
		if (!stricmp(aBuf, "MonitorWorkArea")) return SYSGET_CMD_MONITORWORKAREA;
		if (!stricmp(aBuf, "MonitorName")) return SYSGET_CMD_MONITORNAME;
		return SYSGET_CMD_INVALID;
	}

	static TransformCmds ConvertTransformCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return TRANS_CMD_INVALID;
		if (!stricmp(aBuf, "Asc")) return TRANS_CMD_ASC;
		if (!stricmp(aBuf, "Chr")) return TRANS_CMD_CHR;
		if (!stricmp(aBuf, "Deref")) return TRANS_CMD_DEREF;
		if (!stricmp(aBuf, "Unicode")) return TRANS_CMD_UNICODE;
		if (!stricmp(aBuf, "HTML")) return TRANS_CMD_HTML;
		if (!stricmp(aBuf, "Mod")) return TRANS_CMD_MOD;
		if (!stricmp(aBuf, "Pow")) return TRANS_CMD_POW;
		if (!stricmp(aBuf, "Exp")) return TRANS_CMD_EXP;
		if (!stricmp(aBuf, "Sqrt")) return TRANS_CMD_SQRT;
		if (!stricmp(aBuf, "Log")) return TRANS_CMD_LOG;
		if (!stricmp(aBuf, "Ln")) return TRANS_CMD_LN;  // Natural log.
		if (!stricmp(aBuf, "Round")) return TRANS_CMD_ROUND;
		if (!stricmp(aBuf, "Ceil")) return TRANS_CMD_CEIL;
		if (!stricmp(aBuf, "Floor")) return TRANS_CMD_FLOOR;
		if (!stricmp(aBuf, "Abs")) return TRANS_CMD_ABS;
		if (!stricmp(aBuf, "Sin")) return TRANS_CMD_SIN;
		if (!stricmp(aBuf, "Cos")) return TRANS_CMD_COS;
		if (!stricmp(aBuf, "Tan")) return TRANS_CMD_TAN;
		if (!stricmp(aBuf, "ASin")) return TRANS_CMD_ASIN;
		if (!stricmp(aBuf, "ACos")) return TRANS_CMD_ACOS;
		if (!stricmp(aBuf, "ATan")) return TRANS_CMD_ATAN;
		if (!stricmp(aBuf, "BitAnd")) return TRANS_CMD_BITAND;
		if (!stricmp(aBuf, "BitOr")) return TRANS_CMD_BITOR;
		if (!stricmp(aBuf, "BitXOr")) return TRANS_CMD_BITXOR;
		if (!stricmp(aBuf, "BitNot")) return TRANS_CMD_BITNOT;
		if (!stricmp(aBuf, "BitShiftLeft")) return TRANS_CMD_BITSHIFTLEFT;
		if (!stricmp(aBuf, "BitShiftRight")) return TRANS_CMD_BITSHIFTRIGHT;
		return TRANS_CMD_INVALID;
	}

	static MenuCommands ConvertMenuCommand(char *aBuf)
	{
		if (!aBuf || !*aBuf) return MENU_CMD_INVALID;
		if (!stricmp(aBuf, "Show")) return MENU_CMD_SHOW;
		if (!stricmp(aBuf, "UseErrorLevel")) return MENU_CMD_USEERRORLEVEL;
		if (!stricmp(aBuf, "Add")) return MENU_CMD_ADD;
		if (!stricmp(aBuf, "Rename")) return MENU_CMD_RENAME;
		if (!stricmp(aBuf, "Check")) return MENU_CMD_CHECK;
		if (!stricmp(aBuf, "Uncheck")) return MENU_CMD_UNCHECK;
		if (!stricmp(aBuf, "ToggleCheck")) return MENU_CMD_TOGGLECHECK;
		if (!stricmp(aBuf, "Enable")) return MENU_CMD_ENABLE;
		if (!stricmp(aBuf, "Disable")) return MENU_CMD_DISABLE;
		if (!stricmp(aBuf, "ToggleEnable")) return MENU_CMD_TOGGLEENABLE;
		if (!stricmp(aBuf, "Standard")) return MENU_CMD_STANDARD;
		if (!stricmp(aBuf, "NoStandard")) return MENU_CMD_NOSTANDARD;
		if (!stricmp(aBuf, "Color")) return MENU_CMD_COLOR;
		if (!stricmp(aBuf, "Default")) return MENU_CMD_DEFAULT;
		if (!stricmp(aBuf, "NoDefault")) return MENU_CMD_NODEFAULT;
		if (!stricmp(aBuf, "Delete")) return MENU_CMD_DELETE;
		if (!stricmp(aBuf, "DeleteAll")) return MENU_CMD_DELETEALL;
		if (!stricmp(aBuf, "Tip")) return MENU_CMD_TIP;
		if (!stricmp(aBuf, "Icon")) return MENU_CMD_ICON;
		if (!stricmp(aBuf, "NoIcon")) return MENU_CMD_NOICON;
		if (!stricmp(aBuf, "Click")) return MENU_CMD_CLICK;
		if (!stricmp(aBuf, "MainWindow")) return MENU_CMD_MAINWINDOW;
		if (!stricmp(aBuf, "NoMainWindow")) return MENU_CMD_NOMAINWINDOW;
		return MENU_CMD_INVALID;
	}

	static GuiCommands ConvertGuiCommand(char *aBuf, int *aWindowIndex = NULL, char **aOptions = NULL)
	{
		// Notes about the below macro:
		// "< 3" avoids ambiguity with a future use such as "gui +cmd:whatever" while still allowing
		// up to 99 windows, e.g. "gui 99:add"
		// omit_leading_whitespace(): Move the buf pointer to the location of the sub-command.
		#define DETERMINE_WINDOW_INDEX \
			char *colon_pos = strchr(aBuf, ':');\
			if (colon_pos && colon_pos - aBuf < 3)\
			{\
				if (aWindowIndex)\
					*aWindowIndex = ATOI(aBuf) - 1;\
				aBuf = omit_leading_whitespace(colon_pos + 1);\
			}
			//else leave it set to the default already put in it by the caller.
		DETERMINE_WINDOW_INDEX
		if (aOptions)
			*aOptions = aBuf; // Return position where options start to the caller.
		if (!*aBuf || *aBuf == '+' || *aBuf == '-') // Assume a var ref that resolves to blank is "options" (for runtime flexibility).
			return GUI_CMD_OPTIONS;
		if (!stricmp(aBuf, "Add")) return GUI_CMD_ADD;
		if (!stricmp(aBuf, "Show")) return GUI_CMD_SHOW;
		if (!stricmp(aBuf, "Submit")) return GUI_CMD_SUBMIT;
		if (!stricmp(aBuf, "Cancel") || !stricmp(aBuf, "Hide")) return GUI_CMD_CANCEL;
		if (!stricmp(aBuf, "Minimize")) return GUI_CMD_MINIMIZE;
		if (!stricmp(aBuf, "Maximize")) return GUI_CMD_MAXIMIZE;
		if (!stricmp(aBuf, "Restore")) return GUI_CMD_RESTORE;
		if (!stricmp(aBuf, "Destroy")) return GUI_CMD_DESTROY;
		if (!stricmp(aBuf, "Menu")) return GUI_CMD_MENU;
		if (!stricmp(aBuf, "Font")) return GUI_CMD_FONT;
		if (!stricmp(aBuf, "Tab")) return GUI_CMD_TAB;
		if (!stricmp(aBuf, "Default")) return GUI_CMD_DEFAULT;
		if (!stricmp(aBuf, "Color")) return GUI_CMD_COLOR;
		if (!stricmp(aBuf, "Flash")) return GUI_CMD_FLASH;
		return GUI_CMD_INVALID;
	}

	GuiControlCmds ConvertGuiControlCmd(char *aBuf, int *aWindowIndex = NULL, char **aOptions = NULL)
	{
		DETERMINE_WINDOW_INDEX
		if (aOptions)
			*aOptions = aBuf; // Return position where options start to the caller.
		// If it's blank without a deref, that's CONTENTS.  Otherwise, assume it's OPTIONS for better
		// runtime flexibility (i.e. user can leave the variable blank to make the command do nothing):
		if (!*aBuf && !ArgHasDeref(1))
			return GUICONTROL_CMD_CONTENTS;
		if (!*aBuf || *aBuf == '+' || *aBuf == '-') // Assume a var ref that resolves to blank is "options" (for runtime flexibility).
			return GUICONTROL_CMD_OPTIONS;
		if (!stricmp(aBuf, "Text")) return GUICONTROL_CMD_TEXT;
		if (!stricmp(aBuf, "Move")) return GUICONTROL_CMD_MOVE;
		if (!stricmp(aBuf, "Focus")) return GUICONTROL_CMD_FOCUS;
		if (!stricmp(aBuf, "Enable")) return GUICONTROL_CMD_ENABLE;
		if (!stricmp(aBuf, "Disable")) return GUICONTROL_CMD_DISABLE;
		if (!stricmp(aBuf, "Show")) return GUICONTROL_CMD_SHOW;
		if (!stricmp(aBuf, "Hide")) return GUICONTROL_CMD_HIDE;
		if (!stricmp(aBuf, "Choose")) return GUICONTROL_CMD_CHOOSE;
		if (!stricmp(aBuf, "ChooseString")) return GUICONTROL_CMD_CHOOSESTRING;
		if (!stricmp(aBuf, "Font")) return GUICONTROL_CMD_FONT;
		return GUICONTROL_CMD_INVALID;
	}

	static GuiControlGetCmds ConvertGuiControlGetCmd(char *aBuf, int *aWindowIndex = NULL)
	{
		DETERMINE_WINDOW_INDEX
		if (!*aBuf) return GUICONTROLGET_CMD_CONTENTS; // The implicit command when nothing was specified.
		if (!stricmp(aBuf, "Pos")) return GUICONTROLGET_CMD_POS;
		if (!stricmp(aBuf, "Focus")) return GUICONTROLGET_CMD_FOCUS;
		if (!stricmp(aBuf, "Enabled")) return GUICONTROLGET_CMD_ENABLED;
		if (!stricmp(aBuf, "Visible")) return GUICONTROLGET_CMD_VISIBLE;
		return GUICONTROLGET_CMD_INVALID;
	}

	static GuiControls ConvertGuiControl(char *aBuf)
	{
		if (!aBuf || !*aBuf) return GUI_CONTROL_INVALID;
		if (!stricmp(aBuf, "Text")) return GUI_CONTROL_TEXT;
		if (!stricmp(aBuf, "Button")) return GUI_CONTROL_BUTTON;
		if (!stricmp(aBuf, "Checkbox")) return GUI_CONTROL_CHECKBOX;
		if (!stricmp(aBuf, "Radio")) return GUI_CONTROL_RADIO;
		if (!stricmp(aBuf, "DDL") || !stricmp(aBuf, "DropDownList")) return GUI_CONTROL_DROPDOWNLIST;
		if (!stricmp(aBuf, "ComboBox")) return GUI_CONTROL_COMBOBOX;
		if (!stricmp(aBuf, "ListBox")) return GUI_CONTROL_LISTBOX;
		if (!stricmp(aBuf, "Edit")) return GUI_CONTROL_EDIT;
		// Keep those seldom used at the bottom for performance:
		if (!stricmp(aBuf, "Slider")) return GUI_CONTROL_SLIDER;
		if (!stricmp(aBuf, "Progress")) return GUI_CONTROL_PROGRESS;
		if (!stricmp(aBuf, "Tab")) return GUI_CONTROL_TAB;
		if (!stricmp(aBuf, "GroupBox")) return GUI_CONTROL_GROUPBOX;
		if (!stricmp(aBuf, "Pic") || !stricmp(aBuf, "Picture")) return GUI_CONTROL_PIC;
		if (!stricmp(aBuf, "Hotkey")) return GUI_CONTROL_HOTKEY;
		return GUI_CONTROL_INVALID;
	}

	static ThreadCommands ConvertThreadCommand(char *aBuf)
	{
		if (!aBuf || !*aBuf) return THREAD_CMD_INVALID;
		if (!stricmp(aBuf, "Priority")) return THREAD_CMD_PRIORITY;
		if (!stricmp(aBuf, "Interrupt")) return THREAD_CMD_INTERRUPT;
		return THREAD_CMD_INVALID;
	}
	
	static ProcessCmds ConvertProcessCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return PROCESS_CMD_INVALID;
		if (!stricmp(aBuf, "Exist")) return PROCESS_CMD_EXIST;
		if (!stricmp(aBuf, "Close")) return PROCESS_CMD_CLOSE;
		if (!stricmp(aBuf, "Priority")) return PROCESS_CMD_PRIORITY;
		if (!stricmp(aBuf, "Wait")) return PROCESS_CMD_WAIT;
		if (!stricmp(aBuf, "WaitClose")) return PROCESS_CMD_WAITCLOSE;
		return PROCESS_CMD_INVALID;
	}

	static ControlCmds ConvertControlCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return CONTROL_CMD_INVALID;
		if (!stricmp(aBuf, "Check")) return CONTROL_CMD_CHECK;
		if (!stricmp(aBuf, "Uncheck")) return CONTROL_CMD_UNCHECK;
		if (!stricmp(aBuf, "Enable")) return CONTROL_CMD_ENABLE;
		if (!stricmp(aBuf, "Disable")) return CONTROL_CMD_DISABLE;
		if (!stricmp(aBuf, "Show")) return CONTROL_CMD_SHOW;
		if (!stricmp(aBuf, "Hide")) return CONTROL_CMD_HIDE;
		if (!stricmp(aBuf, "Style")) return CONTROL_CMD_STYLE;
		if (!stricmp(aBuf, "ExStyle")) return CONTROL_CMD_EXSTYLE;
		if (!stricmp(aBuf, "ShowDropDown")) return CONTROL_CMD_SHOWDROPDOWN;
		if (!stricmp(aBuf, "HideDropDown")) return CONTROL_CMD_HIDEDROPDOWN;
		if (!stricmp(aBuf, "TabLeft")) return CONTROL_CMD_TABLEFT;
		if (!stricmp(aBuf, "TabRight")) return CONTROL_CMD_TABRIGHT;
		if (!stricmp(aBuf, "Add")) return CONTROL_CMD_ADD;
		if (!stricmp(aBuf, "Delete")) return CONTROL_CMD_DELETE;
		if (!stricmp(aBuf, "Choose")) return CONTROL_CMD_CHOOSE;
		if (!stricmp(aBuf, "ChooseString")) return CONTROL_CMD_CHOOSESTRING;
		if (!stricmp(aBuf, "EditPaste")) return CONTROL_CMD_EDITPASTE;
		return CONTROL_CMD_INVALID;
	}

	static ControlGetCmds ConvertControlGetCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return CONTROLGET_CMD_INVALID;
		if (!stricmp(aBuf, "Checked")) return CONTROLGET_CMD_CHECKED;
		if (!stricmp(aBuf, "Enabled")) return CONTROLGET_CMD_ENABLED;
		if (!stricmp(aBuf, "Visible")) return CONTROLGET_CMD_VISIBLE;
		if (!stricmp(aBuf, "Tab")) return CONTROLGET_CMD_TAB;
		if (!stricmp(aBuf, "FindString")) return CONTROLGET_CMD_FINDSTRING;
		if (!stricmp(aBuf, "Choice")) return CONTROLGET_CMD_CHOICE;
		if (!stricmp(aBuf, "List")) return CONTROLGET_CMD_LIST;
		if (!stricmp(aBuf, "LineCount")) return CONTROLGET_CMD_LINECOUNT;
		if (!stricmp(aBuf, "CurrentLine")) return CONTROLGET_CMD_CURRENTLINE;
		if (!stricmp(aBuf, "CurrentCol")) return CONTROLGET_CMD_CURRENTCOL;
		if (!stricmp(aBuf, "Line")) return CONTROLGET_CMD_LINE;
		if (!stricmp(aBuf, "Selected")) return CONTROLGET_CMD_SELECTED;
		if (!stricmp(aBuf, "Style")) return CONTROLGET_CMD_STYLE;
		if (!stricmp(aBuf, "ExStyle")) return CONTROLGET_CMD_EXSTYLE;
		return CONTROLGET_CMD_INVALID;
	}

	static DriveCmds ConvertDriveCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return DRIVE_CMD_INVALID;
		if (!stricmp(aBuf, "Eject")) return DRIVE_CMD_EJECT;
		if (!stricmp(aBuf, "Lock")) return DRIVE_CMD_LOCK;
		if (!stricmp(aBuf, "Unlock")) return DRIVE_CMD_UNLOCK;
		if (!stricmp(aBuf, "Label")) return DRIVE_CMD_LABEL;
		return DRIVE_CMD_INVALID;
	}

	static DriveGetCmds ConvertDriveGetCmd(char *aBuf)
	{
		if (!aBuf || !*aBuf) return DRIVEGET_CMD_INVALID;
		if (!stricmp(aBuf, "List")) return DRIVEGET_CMD_LIST;
		if (!stricmp(aBuf, "FileSystem") || !stricmp(aBuf, "FS")) return DRIVEGET_CMD_FILESYSTEM;
		if (!stricmp(aBuf, "Label")) return DRIVEGET_CMD_LABEL;
		if (!strnicmp(aBuf, "SetLabel:", 9)) return DRIVEGET_CMD_SETLABEL;  // Uses strnicmp() vs. stricmp().
		if (!stricmp(aBuf, "Serial")) return DRIVEGET_CMD_SERIAL;
		if (!stricmp(aBuf, "Type")) return DRIVEGET_CMD_TYPE;
		if (!stricmp(aBuf, "Status")) return DRIVEGET_CMD_STATUS;
		if (!stricmp(aBuf, "StatusCD")) return DRIVEGET_CMD_STATUSCD;
		if (!stricmp(aBuf, "Capacity") || !stricmp(aBuf, "Cap")) return DRIVEGET_CMD_CAPACITY;
		return DRIVEGET_CMD_INVALID;
	}

	static WinSetAttributes ConvertWinSetAttribute(char *aBuf)
	{
		if (!aBuf || !*aBuf) return WINSET_INVALID;
		if (!stricmp(aBuf, "Trans") || !stricmp(aBuf, "Transparent")) return WINSET_TRANSPARENT;
		if (!stricmp(aBuf, "TransColor")) return WINSET_TRANSCOLOR;
		if (!stricmp(aBuf, "AlwaysOnTop") || !stricmp(aBuf, "Topmost")) return WINSET_ALWAYSONTOP;
		if (!stricmp(aBuf, "Bottom")) return WINSET_BOTTOM;
		if (!stricmp(aBuf, "Style")) return WINSET_STYLE;
		if (!stricmp(aBuf, "ExStyle")) return WINSET_EXSTYLE;
		if (!stricmp(aBuf, "Redraw")) return WINSET_REDRAW;
		return WINSET_INVALID;
	}


	static ToggleValueType ConvertOnOff(char *aBuf, ToggleValueType aDefault = TOGGLE_INVALID)
	// Returns aDefault if aBuf isn't either ON, OFF, or blank.
	{
		if (!aBuf || !*aBuf) return NEUTRAL;
		if (!stricmp(aBuf, "ON")) return TOGGLED_ON;
		if (!stricmp(aBuf, "OFF")) return TOGGLED_OFF;
		return aDefault;
	}

	static ToggleValueType ConvertOnOffAlways(char *aBuf, ToggleValueType aDefault = TOGGLE_INVALID)
	// Returns aDefault if aBuf isn't either ON, OFF, ALWAYSON, ALWAYSOFF, or blank.
	{
		if (!aBuf || !*aBuf) return NEUTRAL;
		if (!stricmp(aBuf, "On")) return TOGGLED_ON;
		if (!stricmp(aBuf, "Off")) return TOGGLED_OFF;
		if (!stricmp(aBuf, "AlwaysOn")) return ALWAYS_ON;
		if (!stricmp(aBuf, "AlwaysOff")) return ALWAYS_OFF;
		return aDefault;
	}

	static ToggleValueType ConvertOnOffToggle(char *aBuf, ToggleValueType aDefault = TOGGLE_INVALID)
	// Returns aDefault if aBuf isn't either ON, OFF, TOGGLE, or blank.
	{
		if (!aBuf || !*aBuf) return NEUTRAL;
		if (!stricmp(aBuf, "On")) return TOGGLED_ON;
		if (!stricmp(aBuf, "Off")) return TOGGLED_OFF;
		if (!stricmp(aBuf, "Toggle")) return TOGGLE;
		return aDefault;
	}

	static ToggleValueType ConvertOnOffTogglePermit(char *aBuf, ToggleValueType aDefault = TOGGLE_INVALID)
	// Returns aDefault if aBuf isn't either ON, OFF, TOGGLE, PERMIT, or blank.
	{
		if (!aBuf || !*aBuf) return NEUTRAL;
		if (!stricmp(aBuf, "On")) return TOGGLED_ON;
		if (!stricmp(aBuf, "Off")) return TOGGLED_OFF;
		if (!stricmp(aBuf, "Toggle")) return TOGGLE;
		if (!stricmp(aBuf, "Permit")) return TOGGLE_PERMIT;
		return aDefault;
	}

	static ToggleValueType ConvertBlockInput(char *aBuf)
	{
		if (!aBuf || !*aBuf) return NEUTRAL;  // For backward compatibility, blank is not considered INVALID.
		if (!stricmp(aBuf, "On")) return TOGGLED_ON;
		if (!stricmp(aBuf, "Off")) return TOGGLED_OFF;
		if (!stricmp(aBuf, "Send")) return TOGGLE_SEND;
		if (!stricmp(aBuf, "Mouse")) return TOGGLE_MOUSE;
		if (!stricmp(aBuf, "SendAndMouse")) return TOGGLE_SENDANDMOUSE;
		if (!stricmp(aBuf, "Default")) return TOGGLE_DEFAULT;
		return TOGGLE_INVALID;
	}

	static FileLoopModeType ConvertLoopMode(char *aBuf)
	// Returns the file loop mode, or FILE_LOOP_INVALID if aBuf contains an invalid mode.
	{
		if (!aBuf || !*aBuf)
			return FILE_LOOP_FILES_ONLY; // The default mode is used if the param is blank.
		if (strlen(aBuf) > 1)
			return FILE_LOOP_INVALID;
		if (*aBuf == '0')
			return FILE_LOOP_FILES_ONLY;
		if (*aBuf == '1')
			return FILE_LOOP_FILES_AND_FOLDERS;
		if (*aBuf == '2')
			return FILE_LOOP_FOLDERS_ONLY;
		return FILE_LOOP_INVALID;
	}

	static int ConvertMsgBoxResult(char *aBuf)
	// Returns the matching ID, or zero if none.
	{
		if (!aBuf || !*aBuf) return 0;
		// Keeping the most oft-used ones up top helps perf. a little:
		if (!stricmp(aBuf, "YES")) return IDYES;
		if (!stricmp(aBuf, "NO")) return IDNO;
		if (!stricmp(aBuf, "OK")) return IDOK;
		if (!stricmp(aBuf, "CANCEL")) return IDCANCEL;
		if (!stricmp(aBuf, "ABORT")) return IDABORT;
		if (!stricmp(aBuf, "IGNORE")) return IDIGNORE;
		if (!stricmp(aBuf, "RETRY")) return IDRETRY;
		if (!stricmp(aBuf, "TIMEOUT")) return AHK_TIMEOUT;  // Our custom result value.
		return 0;
	}

	static int ConvertRunMode(char *aBuf)
	// Returns the matching WinShow mode, or SW_SHOWNORMAL if none.
	// These are also the modes that AutoIt3 uses.
	{
		// For v1.0.19, this was made more permissive (the use of strcasestr vs. stricmp) to support
		// the optional word ErrorLevel inside this parameter:
		if (!aBuf || !*aBuf) return SW_SHOWNORMAL;
		if (strcasestr(aBuf, "MIN")) return SW_MINIMIZE;
		if (strcasestr(aBuf, "MAX")) return SW_MAXIMIZE;
		if (strcasestr(aBuf, "HIDE")) return SW_HIDE;
		return SW_SHOWNORMAL;
	}

	static int ConvertMouseButton(char *aBuf, bool aAllowWheel = true)
	// Returns the matching VK, or zero if none.
	{
		if (!aBuf || !*aBuf) return 0;
		if (!stricmp(aBuf, "LEFT") || !stricmp(aBuf, "L")) return VK_LBUTTON;
		if (!stricmp(aBuf, "RIGHT") || !stricmp(aBuf, "R")) return VK_RBUTTON;
		if (!stricmp(aBuf, "MIDDLE") || !stricmp(aBuf, "M")) return VK_MBUTTON;
		if (!stricmp(aBuf, "X1")) return VK_XBUTTON1;
		if (!stricmp(aBuf, "X2")) return VK_XBUTTON2;
		if (aAllowWheel)
		{
			if (!stricmp(aBuf, "WheelUp") || !stricmp(aBuf, "WU")) return VK_WHEEL_UP;
			if (!stricmp(aBuf, "WheelDown") || !stricmp(aBuf, "WD")) return VK_WHEEL_DOWN;
		}
		return 0;
	}

	static CoordModeAttribType ConvertCoordModeAttrib(char *aBuf)
	{
		if (!aBuf || !*aBuf) return 0;
		if (!stricmp(aBuf, "Pixel")) return COORD_MODE_PIXEL;
		if (!stricmp(aBuf, "Mouse")) return COORD_MODE_MOUSE;
		if (!stricmp(aBuf, "ToolTip")) return COORD_MODE_TOOLTIP;
		if (!stricmp(aBuf, "Caret")) return COORD_MODE_CARET;
		if (!stricmp(aBuf, "Menu")) return COORD_MODE_MENU;
		return 0;
	}

	static VariableTypeType ConvertVariableTypeName(char *aBuf)
	// Returns the matching type, or zero if none.
	{
		if (!aBuf || !*aBuf) return VAR_TYPE_INVALID;
		if (!stricmp(aBuf, "integer")) return VAR_TYPE_INTEGER;
		if (!stricmp(aBuf, "float")) return VAR_TYPE_FLOAT;
		if (!stricmp(aBuf, "number")) return VAR_TYPE_NUMBER;
		if (!stricmp(aBuf, "time")) return VAR_TYPE_TIME;
		if (!stricmp(aBuf, "date")) return VAR_TYPE_TIME;  // "date" is just an alias for "time".
		if (!stricmp(aBuf, "digit")) return VAR_TYPE_DIGIT;
		if (!stricmp(aBuf, "xdigit")) return VAR_TYPE_XDIGIT;
		if (!stricmp(aBuf, "alnum")) return VAR_TYPE_ALNUM;
		if (!stricmp(aBuf, "alpha")) return VAR_TYPE_ALPHA;
		if (!stricmp(aBuf, "upper")) return VAR_TYPE_UPPER;
		if (!stricmp(aBuf, "lower")) return VAR_TYPE_LOWER;
		if (!stricmp(aBuf, "space")) return VAR_TYPE_SPACE;
		return VAR_TYPE_INVALID;
	}

	static ResultType ValidateMouseCoords(char *aX, char *aY)
	{
		if (!aX) aX = "";  if (!aY) aY = "";
		if (!*aX && !*aY) return OK;  // Both are absent, which is the signal to use the current position.
		if (*aX && *aY) return OK; // Both are present (that they are numeric is validated elsewhere).
		return FAIL; // Otherwise, one is blank but the other isn't, which is not allowed.
	}

	static char *LogToText(char *aBuf, size_t aBufSize);
	char *VicinityToText(char *aBuf, size_t aBufSize, int aMaxLines = 15);
	char *ToText(char *aBuf, size_t aBufSize, bool aCRLF, DWORD aElapsed = 0);

	static void ToggleSuspendState();
	ResultType ChangePauseState(ToggleValueType aChangeTo);
	static ResultType ScriptBlockInput(bool aEnable);

	Line *PreparseError(char *aErrorText);
	// Call this LineError to avoid confusion with Script's error-displaying functions:
	ResultType LineError(char *aErrorText, ResultType aErrorType = FAIL, char *aExtraInfo = "");

	Line(UCHAR aFileNumber, LineNumberType aFileLineNumber, ActionTypeType aActionType
		, ArgStruct aArg[], ArgCountType aArgc) // Constructor
		: mFileNumber(aFileNumber), mLineNumber(aFileLineNumber), mActionType(aActionType)
		, mAttribute(ATTR_NONE), mArgc(aArgc), mArg(aArg)
		, mPrevLine(NULL), mNextLine(NULL), mRelatedLine(NULL), mParentLine(NULL)
		{}
	void *operator new(size_t aBytes) {return SimpleHeap::Malloc(aBytes);}
	void *operator new[](size_t aBytes) {return SimpleHeap::Malloc(aBytes);}
	void operator delete(void *aPtr) {}  // Intentionally does nothing because we're using SimpleHeap for everything.
	void operator delete[](void *aPtr) {}
};



class Label
{
public:
	char *mName;
	Line *mJumpToLine;
	Label *mPrevLabel, *mNextLabel;  // Prev & Next items in linked list.

	bool IsExemptFromSuspend()
	{
		// Hotkey and Hotstring subroutines whose first line is the Suspend command are exempt from
		// being suspended themselves except when their first parameter is the literal
		// word "on":
		return mJumpToLine->mActionType == ACT_SUSPEND && (!mJumpToLine->mArgc || mJumpToLine->ArgHasDeref(1)
			|| stricmp(mJumpToLine->mArg[0].text, "on"));
	}

	Label(char *aLabelName)
		: mName(aLabelName) // Caller gave us a pointer to dynamic memory for this.
		, mJumpToLine(NULL)
		, mPrevLabel(NULL), mNextLabel(NULL)
	{}
	void *operator new(size_t aBytes) {return SimpleHeap::Malloc(aBytes);}
	void *operator new[](size_t aBytes) {return SimpleHeap::Malloc(aBytes);}
	void operator delete(void *aPtr) {}
	void operator delete[](void *aPtr) {}
};



class ScriptTimer
{
public:
	Label *mLabel;
	int mPeriod;
	int mPriority;  // Thread priority relative to other threads, default 0.
	UCHAR mExistingThreads;  // Whether this timer is already running its subroutine.
	DWORD mTimeLastRun;  // TickCount
	bool mEnabled;
	ScriptTimer *mNextTimer;  // Next items in linked list
	ScriptTimer(Label *aLabel)
		#define DEFAULT_TIMER_PERIOD 250
		: mLabel(aLabel), mPeriod(DEFAULT_TIMER_PERIOD), mPriority(0) // Default is always 0.
		, mExistingThreads(0), mTimeLastRun(0)
		, mEnabled(false), mNextTimer(NULL)  // Note that mEnabled must default to false for the counts to be right.
	{}
	void *operator new(size_t aBytes) {return SimpleHeap::Malloc(aBytes);}
	void *operator new[](size_t aBytes) {return SimpleHeap::Malloc(aBytes);}
	void operator delete(void *aPtr) {}
	void operator delete[](void *aPtr) {}
};



#define MAX_MENU_NAME_LENGTH MAX_PATH // For both menu and menu item names.
class UserMenuItem;  // Forward declaration since classes use each other (i.e. a menu *item* can have a submenu).
class UserMenu
{
public:
	char *mName;  // Dynamically allocated.
	UserMenuItem *mFirstMenuItem, *mLastMenuItem, *mDefault;
	// Keep any fields that aren't an even multiple of 4 adjacent to each other.  This conserves memory
	// due to byte-alignment:
	bool mIncludeStandardItems;
	int mClickCount; // How many clicks it takes to trigger the default menu item.  2 = double-click
	UINT mMenuItemCount;  // The count of user-defined menu items (doesn't include the standard items, if present).
	UserMenu *mNextMenu;  // Next item in linked list
	HMENU mMenu;
	MenuTypeType mMenuType; // MENU_TYPE_POPUP (via CreatePopupMenu) vs. MENU_TYPE_BAR (via CreateMenu).
	HBRUSH mBrush;   // Background color to apply to menu.
	COLORREF mColor; // The color that corresponds to the above.

	// Don't overload new and delete operators in this case since we want to use real dynamic memory
	// (since menus can be read in from a file, destroyed and recreated, over and over).

	UserMenu(char *aName) // Constructor
		: mName(aName), mFirstMenuItem(NULL), mLastMenuItem(NULL), mDefault(NULL)
		, mIncludeStandardItems(false), mClickCount(2), mMenuItemCount(0), mNextMenu(NULL), mMenu(NULL)
		, mMenuType(MENU_TYPE_POPUP) // The MENU_TYPE_NONE flag is not used in this context.  Default = POPUP.
		, mBrush(NULL), mColor(CLR_DEFAULT)
	{
	}

	ResultType AddItem(char *aName, UINT aMenuID, Label *aLabel, UserMenu *aSubmenu, char *aOptions);
	ResultType DeleteItem(UserMenuItem *aMenuItem, UserMenuItem *aMenuItemPrev);
	ResultType DeleteAllItems();
	ResultType ModifyItem(UserMenuItem *aMenuItem, Label *aLabel, UserMenu *aSubmenu, char *aOptions);
	void UpdateOptions(UserMenuItem *aMenuItem, char *aOptions);
	ResultType RenameItem(UserMenuItem *aMenuItem, char *aNewName);
	ResultType UpdateName(UserMenuItem *aMenuItem, char *aNewName);
	ResultType CheckItem(UserMenuItem *aMenuItem);
	ResultType UncheckItem(UserMenuItem *aMenuItem);
	ResultType ToggleCheckItem(UserMenuItem *aMenuItem);
	ResultType EnableItem(UserMenuItem *aMenuItem);
	ResultType DisableItem(UserMenuItem *aMenuItem);
	ResultType ToggleEnableItem(UserMenuItem *aMenuItem);
	ResultType SetDefault(UserMenuItem *aMenuItem = NULL);
	ResultType IncludeStandardItems();
	ResultType ExcludeStandardItems();
	ResultType Create(MenuTypeType aMenuType = MENU_TYPE_NONE); // NONE means UNSPECIFIED in this context.
	void SetColor(char *aColorName, bool aApplyToSubmenus);
	void ApplyColor(bool aApplyToSubmenus);
	ResultType AppendStandardItems();
	ResultType Destroy();
	ResultType Display(bool aForceToForeground = true, int aX = COORD_UNSPECIFIED, int aY = COORD_UNSPECIFIED);
	UINT GetSubmenuPos(HMENU ahMenu);
	UINT GetItemPos(char *aMenuItemName);
	bool ContainsMenu(UserMenu *aMenu);
};



class UserMenuItem
{
public:
	char *mName;  // Dynamically allocated.
	size_t mNameCapacity;
	UINT mMenuID;
	Label *mLabel;
	UserMenu *mSubmenu;
	UserMenu *mMenu;  // The menu to which this item belongs.  Needed to support script var A_ThisMenu.
	int mPriority;
	// Keep any fields that aren't an even multiple of 4 adjacent to each other.  This conserves memory
	// due to byte-alignment:
	bool mEnabled, mChecked;
	UserMenuItem *mNextMenuItem;  // Next item in linked list

	// Constructor:
	UserMenuItem(char *aName, size_t aNameCapacity, UINT aMenuID, Label *aLabel, UserMenu *aSubmenu, UserMenu *aMenu);

	// Don't overload new and delete operators in this case since we want to use real dynamic memory
	// (since menus can be read in from a file, destroyed and recreated, over and over).
};



struct FontType
{
	#define MAX_FONT_NAME_LENGTH 63  // Longest name I've seen is 29 chars, "Franklin Gothic Medium Italic".
	char name[MAX_FONT_NAME_LENGTH + 1];
	// Keep any fields that aren't an even multiple of 4 adjacent to each other.  This conserves memory
	// due to byte-alignment:
	bool italic;
	bool underline;
	bool strikeout;
	int point_size; // Decided to use int vs. float to simplify the code in many places. Fractional sizes seem rarely needed.
	int weight;
	HFONT hfont;
};

typedef UCHAR TabControlIndexType;
typedef UCHAR TabIndexType;
// Keep the below in sync with the size of the types above:
#define MAX_TAB_CONTROLS 255  // i.e. the value 255 itself is reserved to mean "doesn't belong to a tab".
#define MAX_TABS_PER_CONTROL 256
struct GuiControlType
{
	HWND hwnd;
	// Keep any fields that are smaller than 4 bytes adjacent to each other.  This conserves memory
	// due to byte-alignment.  It has been verified to save 4 bytes per struct in this case:
	GuiControls type;
	#define GUI_CONTROL_ATTRIB_IMPLICIT_CANCEL     0x01
	#define GUI_CONTROL_ATTRIB_ALTSUBMIT           0x02
	#define GUI_CONTROL_ATTRIB_LABEL_IS_RUNNING    0x04
	#define GUI_CONTROL_ATTRIB_EXPLICITLY_HIDDEN   0x08
	#define GUI_CONTROL_ATTRIB_EXPLICITLY_DISABLED 0x10
	#define GUI_CONTROL_ATTRIB_BACKGROUND_DEFAULT  0x20 // i.e. Don't conform to window/control background color; use default instead.
	#define GUI_CONTROL_ATTRIB_BACKGROUND_TRANS    0x40 // i.e. Leave this control's background transparent.
	#define GUI_CONTROL_ATTRIB_ALTBEHAVIOR         0x80 // For sliders: Reverse/Invert the value.
	UCHAR attrib; // A field of option flags/bits defined above.
	TabControlIndexType tab_control_index; // Which tab control this control belongs to, if any.
	TabIndexType tab_index; // For type==TAB, this stores the tab control's index.  For other types, it stores the page.
	Var *output_var;
	Label *jump_to_label;
	union
	{
		COLORREF union_color;  // Color of the control's text.
		HBITMAP union_hbitmap; // For PIC controls, stores the bitmap.
		// Note: Pic controls cannot obey the text color, but they can obey the window's background
		// color if the picture's background is transparent (at least in the case of icons on XP).
	};
	#define USES_FONT_AND_TEXT_COLOR(type) !(type == GUI_CONTROL_PIC || type == GUI_CONTROL_SLIDER || type == GUI_CONTROL_PROGRESS)
};

struct GuiControlOptionsType
{
	DWORD style_add, style_remove, exstyle_add, exstyle_remove;
	int x, y, width, height;  // Position info.
	float row_count;
	int choice;  // Which item of a DropDownList/ComboBox/ListBox to initially choose.
	int range_min, range_max;  // Allowable range, such as for a slider control.
	int tick_interval; // The interval at which to draw tickmarks for a slider control.
	int line_size, page_size; // Also for slider.
	int thickness;  // Thickness of slider's thumb.
	int tip_side; // Which side of the control to display the tip on (0 to use default side).
	GuiControlType *buddy1, *buddy2;
	COLORREF progress_color_bk;
	bool color_changed; // To discern when a control has been put back to the default color. [v1.0.26]
	int limit;   // The max number of characters to permit in an edit or combobox's edit.
	int hscroll_pixels;  // The number of pixels for a listbox's horizontal scrollbar to be able to scroll.
	int checked; // When zeroed, struct contains default starting state of checkbox/radio, i.e. BST_UNCHECKED.
	int icon_number; // Which icon of a multi-icon file to use.  Zero means use-default, i.e. the first icon.
	char password_char; // When zeroed, indicates "use default password" for an edit control with the password style.
	#define MAX_EDIT_TABSTOPS 50
	UINT tabstop[MAX_EDIT_TABSTOPS]; // Array of tabstops for the interior of a multi-line edit control.
	UINT tabstop_count;  // The number of entries in the above array.
	bool start_new_section;
};

LRESULT CALLBACK GuiWindowProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabWindowProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

class GuiType
{
public:
	#define GUI_STANDARD_WIDTH_MULTIPLIER 15 // This times font size = width, if all other means of determining it are exhausted.
	#define GUI_STANDARD_WIDTH (GUI_STANDARD_WIDTH_MULTIPLIER * sFont[mCurrentFontIndex].point_size)
	// Update for v1.0.21: Reduced it to 8 vs. 9 because 8 causes the height each edit (with the
	// default style) to exactly match that of a Combo or DropDownList.  This type of spacing seems
	// to be what other apps use too, and seems to make edits stand out a little nicer:
	#define GUI_CTL_VERTICAL_DEADSPACE 8
	HWND mHwnd;
	// Control IDs are higher than their index in the array by the below amount.  This offset is
	// necessary because windows that behave like dialogs automatically return IDOK and IDCANCEL in
	// response to certain types of standard actions:
	GuiIndexType mWindowIndex;
	GuiIndexType mControlCount;
	GuiControlType mControl[MAX_CONTROLS_PER_GUI];
	GuiIndexType mDefaultButtonIndex; // Index vs. pointer is needed for some things.
	Label *mLabelForClose, *mLabelForEscape, *mLabelForSize, *mLabelForDropFiles;
	bool mLabelForCloseIsRunning, mLabelForEscapeIsRunning, mLabelForSizeIsRunning; // DropFiles doesn't need one of these.
	WPARAM mSizeType; // And it does not need to be initalized since its contents are documented as being defined only in the GuiSize subroutine.
	LPARAM mSizeWidthHeight; // Same comment as above.
	DWORD mStyle, mExStyle; // Style of window.
	bool mInRadioGroup; // Whether the control currently being created is inside a prior radio-group.
	bool mUseTheme;  // Whether XP theme and styles should be applied to the parent window and subsequently added controls.
	HWND mOwner;  // The window that owns this one, if any.  Note that Windows provides no way to change owners after window creation.
	int mCurrentFontIndex;
	TabControlIndexType mTabControlCount;
	TabControlIndexType mCurrentTabControlIndex; // Which tab control of the window.
	TabIndexType mCurrentTabIndex;// Which tab of a tab control is currently the default for newly added controls.
	COLORREF mCurrentColor;       // The default color of text in controls.
	COLORREF mBackgroundColorWin; // The window's background color itself.
	HBRUSH mBackgroundBrushWin;   // Brush corresponding to the above.
	COLORREF mBackgroundColorCtl; // Background color for controls.
	HBRUSH mBackgroundBrushCtl;   // Brush corresponding to the above.
	HDROP mHdrop;                 // Used for drag and drop operations.
	int mMarginX, mMarginY, mPrevX, mPrevY, mPrevWidth, mPrevHeight, mMaxExtentRight, mMaxExtentDown
		, mSectionX, mSectionY, mMaxExtentRightSection, mMaxExtentDownSection;
	bool mFirstGuiShowCmd, mFirstActivation, mShowIsInProgress, mDestroyWindowHasBeenCalled;

	#define MAX_GUI_FONTS 100
	static FontType sFont[MAX_GUI_FONTS];
	static int sFontCount;
	static int sObjectCount; // The number of non-NULL items in the g_gui array. Maintained only for performance reasons.

	// Don't overload new and delete operators in this case since we want to use real dynamic memory
	// (since GUIs can be destroyed and recreated, over and over).

	// Keep the default destructor to avoid entering the "Law of the Big Three": If your class requires a
	// copy constructor, copy assignment operator, or a destructor, then it very likely will require all three.

	GuiType(int aWindowIndex) // Constructor
		: mHwnd(NULL), mWindowIndex(aWindowIndex), mControlCount(0)
		, mDefaultButtonIndex(-1), mLabelForClose(NULL), mLabelForEscape(NULL), mLabelForSize(NULL), mLabelForDropFiles(NULL)
		, mLabelForCloseIsRunning(false), mLabelForEscapeIsRunning(false), mLabelForSizeIsRunning(false)
		// The styles DS_CENTER and DS_3DLOOK appear to be ineffectual in this case.
		// Also note that WS_CLIPSIBLINGS winds up on the window even if unspecified, which is a strong hint
		// that it should always be used for top level windows across all OSes.  Usenet posts confirm this.
		// Also, it seems safer to have WS_POPUP under a vague feeling that it seems to apply to dialog
		// style windows such as this one, and the fact that it also allows the window's caption to be
		// removed, which implies that POPUP windows are more flexible than OVERLAPPED windows.
		, mStyle(WS_POPUP|WS_CLIPSIBLINGS|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX) // WS_CLIPCHILDREN (doesn't seem helpful currently)
		, mExStyle(0)
		, mInRadioGroup(false), mUseTheme(true), mOwner(NULL)
		, mCurrentFontIndex(FindOrCreateFont()) // Omit params to tell it to find or create DEFAULT_GUI_FONT.
		, mTabControlCount(0), mCurrentTabControlIndex(MAX_TAB_CONTROLS), mCurrentTabIndex(0)
		, mCurrentColor(CLR_DEFAULT)
		, mBackgroundColorWin(CLR_DEFAULT), mBackgroundBrushWin(NULL)
		, mBackgroundColorCtl(CLR_DEFAULT), mBackgroundBrushCtl(NULL)
		, mHdrop(NULL)
		, mMarginX(COORD_UNSPECIFIED), mMarginY(COORD_UNSPECIFIED) // These will be set when the first control is added.
		, mPrevX(0), mPrevY(0)
		, mPrevWidth(0), mPrevHeight(0) // Needs to be zero for first control to start off at right offset.
		, mMaxExtentRight(0), mMaxExtentDown(0)
		, mSectionX(COORD_UNSPECIFIED), mSectionY(COORD_UNSPECIFIED)
		, mMaxExtentRightSection(COORD_UNSPECIFIED), mMaxExtentDownSection(COORD_UNSPECIFIED)
		, mFirstGuiShowCmd(true), mFirstActivation(true), mShowIsInProgress(false)
		, mDestroyWindowHasBeenCalled(false)
	{
		// The array of controls is left unitialized to catch bugs.  Each control's attributes should be
		// fully populated when it is created.
		//ZeroMemory(mControl, sizeof(mControl));
	}

	static ResultType Destroy(GuiIndexType aWindowIndex);
	ResultType Create();
	static void UpdateMenuBars(HMENU aMenu);
	ResultType AddControl(GuiControls aControlType, char *aOptions, char *aText);
	ResultType ParseOptions(char *aOptions, bool &aSetLastFoundWindow);
	ResultType ControlParseOptions(char *aOptions, GuiControlOptionsType &aOpt, GuiControlType &aControl
		, GuiIndexType aControlIndex = -1); // aControlIndex is not needed upon control creation.
	void ControlInitOptions(GuiControlOptionsType &aOpt, GuiControlType &aControl);
	void ControlAddContents(GuiControlType &aControl, char *aContent, int aChoice);
	ResultType Show(char *aOptions, char *aTitle);
	ResultType Clear();
	ResultType Cancel();
	ResultType Close(); // Due to SC_CLOSE, etc.
	ResultType Escape(); // Similar to close, except typically called when the user presses ESCAPE.
	ResultType Submit(bool aHideIt);
	ResultType ControlGetContents(Var &aOutputVar, GuiControlType &aControl, char *aMode = "");

	static GuiType *FindGui(HWND aHwnd) // Find which GUI object owns the specified window.
	{
		#define EXTERN_GUI extern GuiType *g_gui[MAX_GUI_WINDOWS]
		EXTERN_GUI;
		if (!sObjectCount)
			return NULL;
		// The loop will usually find it on the first iteration since the #1 window is default
		// and thus most commonly used.
		for (int i = 0; i < MAX_GUI_WINDOWS; ++i)
			if (g_gui[i] && g_gui[i]->mHwnd == aHwnd)
				return g_gui[i];
		return NULL;
	}

	GuiIndexType FindControl(char *aControlID);
	GuiControlType *FindControl(HWND aHwnd)
	{
		GuiIndexType index = GUI_HWND_TO_INDEX(aHwnd); // Retrieves -1 on failure, which will be out of bounds when converted to unsigned.
		if (index < mControlCount)
			return &mControl[index];
		// Otherwise: Since ComboBoxes (and possibly other future control types) have children, try looking
		// up aHwnd's parent to see if its a known control of this dialog.  Some callers rely on us making
		// this extra effort:
		if (   !(aHwnd = GetParent(aHwnd))   )
			return NULL;
		index = GUI_HWND_TO_INDEX(aHwnd);
		return (index < mControlCount) ? &mControl[index] : NULL;
	}
	int FindGroup(GuiIndexType aControlIndex, GuiIndexType &aGroupStart, GuiIndexType &aGroupEnd);

	ResultType SetCurrentFont(char *aOptions, char *aFontName);
	static int FindOrCreateFont(char *aOptions = "", char *aFontName = "", FontType *aFoundationFont = NULL
		, COLORREF *aColor = NULL);
	static int FindFont(FontType &aFont);

	void Event(GuiIndexType aControlIndex, UINT aNotifyCode);

	static WORD TextToHotkey(char *aText);
	static char *HotkeyToText(WORD aHotkey, char *aBuf);
	void ControlCheckRadioButton(GuiControlType &aControl, GuiIndexType aControlIndex, WPARAM aCheckType);
	int ControlGetDefaultSliderThickness(DWORD aStyle, int aThumbThickness);
	void ControlSetSliderOptions(GuiControlType &aControl, GuiControlOptionsType &aOpt);
	int ControlInvertSliderIfNeeded(GuiControlType &aControl, int aPosition);
	void ControlSetProgressOptions(GuiControlType &aControl, GuiControlOptionsType &aOpt, DWORD aStyle);
	bool ControlOverrideBkColor(GuiControlType &aControl);

	void ControlUpdateCurrentTab(GuiControlType &aTabControl, bool aFocusFirstControl);
	GuiControlType *FindTabControl(TabControlIndexType aTabControlIndex);
	int FindTabIndexByName(GuiControlType &aTabControl, char *aName);
	int GetControlCountOnTabPage(TabControlIndexType aTabControlIndex, TabIndexType aTabIndex);
	POINT GetPositionOfTabClientArea(GuiControlType &aTabControl);
	ResultType SelectAdjacentTab(GuiControlType &aTabControl, bool aMoveToRight, bool aFocusFirstControl
		, bool aWrapAround);
};



class Script
{
private:
	friend class Hotkey;
	Line *mFirstLine, *mLastLine;     // The first and last lines in the linked list.
	Label *mFirstLabel, *mLastLabel;  // The first and last labels in the linked list.
	Var *mFirstVar, *mLastVar;  // The first and last variables in the linked list.
	WinGroup *mFirstGroup, *mLastGroup;  // The first and last variables in the linked list.
	UINT mLineCount, mLabelCount, mVarCount, mGroupCount;

#ifdef AUTOHOTKEYSC
	bool mCompiledHasCustomIcon; // Whether the compiled script uses a custom icon.
#endif;

	// These two track the file number and line number in that file of the line currently being loaded,
	// which simplifies calls to ScriptError() and LineError() (reduces the number of params that must be passed):
	UCHAR mCurrFileNumber;
	LineNumberType mCurrLineNumber;
	bool mNoHotkeyLabels;
	bool mMenuUseErrorLevel;  // Whether runtime errors should be displayed by the Menu command, vs. ErrorLevel.

	#define UPDATE_TIP_FIELD strlcpy(mNIC.szTip, (mTrayIconTip && *mTrayIconTip) ? mTrayIconTip \
		: (mFileName ? mFileName : NAME_P), sizeof(mNIC.szTip));
	NOTIFYICONDATA mNIC; // For ease of adding and deleting our tray icon.

#ifdef AUTOHOTKEYSC
	ResultType CloseAndReturn(HS_EXEArc_Read *fp, UCHAR *aBuf, ResultType return_value);
	size_t GetLine(char *aBuf, int aMaxCharsToRead, UCHAR *&aMemFile);
#else
	ResultType CloseAndReturn(FILE *fp, UCHAR *aBuf, ResultType return_value);
	size_t GetLine(char *aBuf, int aMaxCharsToRead, FILE *fp);
#endif
	ResultType IsPreprocessorDirective(char *aBuf);

	ResultType ParseAndAddLine(char *aLineText, char *aActionName = NULL
		, char *aEndMarker = NULL, char *aLiteralMap = NULL, size_t aLiteralMapLength = 0
		, ActionTypeType aActionType = ACT_INVALID, ActionTypeType aOldActionType = OLD_INVALID);
	ResultType ParseDerefs(char *aArgText, char *aArgMap, DerefType *aDeref, int &aDerefCount);
	char *ParseActionType(char *aBufTarget, char *aBufSource, bool aDisplayErrors);
	static ActionTypeType ConvertActionType(char *aActionTypeString);
	static ActionTypeType ConvertOldActionType(char *aActionTypeString);
	ResultType AddLabel(char *aLabelName);
	ResultType AddLine(ActionTypeType aActionType, char *aArg[] = NULL, ArgCountType aArgc = 0, char *aArgMap[] = NULL);
	Var *AddVar(char *aVarName, size_t aVarNameLength, Var *aVarPrev);

	// These aren't in the Line class because I think they're easier to implement
	// if aStartingLine is allowed to be NULL (for recursive calls).  If they
	// were member functions of class Line, a check for NULL would have to
	// be done before dereferencing any line's mNextLine, for example:
	Line *PreparseBlocks(Line *aStartingLine, bool aFindBlockEnd = false, Line *aParentLine = NULL);
	Line *PreparseIfElse(Line *aStartingLine, ExecUntilMode aMode = NORMAL_MODE, AttributeType aLoopTypeFile = ATTR_NONE
		, AttributeType aLoopTypeReg = ATTR_NONE, AttributeType aLoopTypeRead = ATTR_NONE
		, AttributeType aLoopTypeParse = ATTR_NONE);

public:
	Line *mCurrLine;  // Seems better to make this public than make Line our friend.
	char mThisMenuItemName[MAX_MENU_NAME_LENGTH + 1];
	char mThisMenuName[MAX_MENU_NAME_LENGTH + 1];
	char *mThisHotkeyName, *mPriorHotkeyName;
	Label *mOnExitLabel;  // The label to run when the script terminates (NULL if none).
	ExitReasons mExitReason;

	ScriptTimer *mFirstTimer, *mLastTimer;  // The first and last script timers in the linked list.
	UINT mTimerCount, mTimerEnabledCount;

	UserMenu *mFirstMenu, *mLastMenu;
	UINT mMenuCount;

	WIN32_FIND_DATA *mLoopFile;  // The file of the current file-loop, if applicable.
	RegItemStruct *mLoopRegItem; // The registry subkey or value of the current registry enumeration loop.
	LoopReadFileStruct *mLoopReadFile;  // The file whose contents are currently being read by a File-Read Loop.
	char *mLoopField;  // The field of the current string-parsing loop.
	__int64 mLoopIteration; // Signed, since script/ITOA64 aren't designed to handle unsigned.
	DWORD mThisHotkeyStartTime, mPriorHotkeyStartTime;  // Tickcount timestamp of when its subroutine began.
	char mEndChar;  // The ending character pressed to trigger the most recent non-auto-replace hotstring.
	modLR_type mThisHotkeyModifiersLR;
	char *mFileSpec; // Will hold the full filespec, for convenience.
	char *mFileDir;  // Will hold the directory containing the script file.
	char *mFileName; // Will hold the script's naked file name.
	char *mOurEXE; // Will hold this app's module name (e.g. C:\Program Files\AutoHotkey\AutoHotkey.exe).
	char *mOurEXEDir;  // Same as above but just the containing diretory (for convenience).
	char *mMainWindowTitle; // Will hold our main window's title, for consistency & convenience.
	bool mIsReadyToExecute;
	bool AutoExecSectionIsRunning;
	bool mIsRestart; // The app is restarting rather than starting from scratch.
	bool mIsAutoIt2; // Whether this script is considered to be an AutoIt2 script.
	bool mErrorStdOut; // true if load-time syntax errors should be sent to stdout vs. a MsgBox.
	__int64 mLinesExecutedThisCycle; // Use 64-bit to match the type of g.LinesPerCycle
	int mUninterruptedLineCountMax; // 32-bit for performance (since huge values seem unnecessary here).
	int mUninterruptibleTime;
	DWORD mLastScriptRest, mLastPeekTime;

	#define RUNAS_ITEM_SIZE (257 * sizeof(wchar_t))
	wchar_t *mRunAsUser, *mRunAsPass, *mRunAsDomain; // Memory is allocated at runtime, upon first use.

	HICON mCustomIcon;  // NULL unless the script has loaded a custom icon during its runtime.
	char *mCustomIconFile; // Filename of icon.  Allocated on first use.
	char *mTrayIconTip;  // Custom tip text for tray icon.  Allocated on first use.
	UINT mCustomIconNumber; // The number of the icon inside the above file.

	UserMenu *mTrayMenu; // Our tray menu, which should be destroyed upon exiting the program.
    
	ResultType Init(char *aScriptFilename, bool aIsRestart);
	ResultType CreateWindows();
	void CreateTrayIcon();
	void UpdateTrayIcon(bool aForceUpdate = false);
	ResultType AutoExecSection();
	ResultType Edit();
	ResultType Reload(bool aDisplayErrors);
	ResultType ExitApp(ExitReasons aExitReason, char *aBuf = NULL, int ExitCode = 0);
	void TerminateApp(int aExitCode);
	LineNumberType LoadFromFile();
	ResultType LoadIncludedFile(char *aFileSpec, bool aAllowDuplicateInclude);
	ResultType UpdateOrCreateTimer(Label *aLabel, char *aPeriod, char *aPriority, bool aEnable
		, bool aUpdatePriorityOnly);

	#define VAR_NAME_LENGTH_DEFAULT 0
	Var *FindOrAddVar(char *aVarName, size_t aVarNameLength = VAR_NAME_LENGTH_DEFAULT, Var *aSearchStart = NULL);
	Var *FindVar(char *aVarName, size_t aVarNameLength = 0, Var **apVarPrev = NULL, Var *aSearchStart = NULL);
	static VarTypes GetVarType(char *aVarName);

	WinGroup *FindOrAddGroup(char *aGroupName);
	ResultType AddGroup(char *aGroupName);
	Label *FindLabel(char *aLabelName);
	ResultType ActionExec(char *aAction, char *aParams = NULL, char *aWorkingDir = NULL
		, bool aDisplayErrors = true, char *aRunShowMode = NULL, HANDLE *aProcess = NULL
		, bool aUseRunAs = false, Var *aOutputVar = NULL);
	char *ListVars(char *aBuf, size_t aBufSize);
	char *ListKeyHistory(char *aBuf, size_t aBufSize);

	ResultType PerformMenu(char *aMenu, char *aCommand, char *aParam3, char *aParam4, char *aOptions);
	UserMenu *FindMenu(char *aMenuName);
	UserMenu *AddMenu(char *aMenuName);
	ResultType ScriptDeleteMenu(UserMenu *aMenu);
	UserMenuItem *FindMenuItemByID(UINT aID)
	{
		UserMenuItem *mi;
		for (UserMenu *m = mFirstMenu; m; m = m->mNextMenu)
			for (mi = m->mFirstMenuItem; mi; mi = mi->mNextMenuItem)
				if (mi->mMenuID == aID)
					return mi;
		return NULL;
	}

	ResultType PerformGui(char *aCommand, char *aControlType, char *aOptions, char *aParam4);

	// Call this SciptError to avoid confusion with Line's error-displaying functions:
	ResultType ScriptError(char *aErrorText, char *aExtraInfo = ""); // , ResultType aErrorType = FAIL);

	#define SOUNDPLAY_ALIAS "AHK_PlayMe"  // Used by destructor and SoundPlay().

	VarSizeType GetBatchLines(char *aBuf = NULL);
	VarSizeType GetTitleMatchMode(char *aBuf = NULL);
	VarSizeType GetTitleMatchModeSpeed(char *aBuf = NULL);
	VarSizeType GetDetectHiddenWindows(char *aBuf = NULL);
	VarSizeType GetDetectHiddenText(char *aBuf = NULL);
	VarSizeType GetAutoTrim(char *aBuf = NULL);
	VarSizeType GetStringCaseSense(char *aBuf = NULL);
	VarSizeType GetFormatInteger(char *aBuf = NULL);
	VarSizeType GetFormatFloat(char *aBuf = NULL);
	VarSizeType GetKeyDelay(char *aBuf = NULL);
	VarSizeType GetWinDelay(char *aBuf = NULL);
	VarSizeType GetControlDelay(char *aBuf = NULL);
	VarSizeType GetMouseDelay(char *aBuf = NULL);
	VarSizeType GetDefaultMouseSpeed(char *aBuf = NULL);
	VarSizeType GetIconHidden(char *aBuf = NULL);
	VarSizeType GetIconTip(char *aBuf = NULL);
	VarSizeType GetIconFile(char *aBuf = NULL);
	VarSizeType GetIconNumber(char *aBuf = NULL);
	VarSizeType GetExitReason(char *aBuf = NULL);
	VarSizeType GetTimeIdlePhysical(char *aBuf = NULL);
	VarSizeType GetSpace(VarTypeType aType, char *aBuf = NULL);
	VarSizeType GetAhkVersion(char *aBuf = NULL);
	VarSizeType GetMMMM(char *aBuf = NULL);
	VarSizeType GetMMM(char *aBuf = NULL);
	VarSizeType GetDDDD(char *aBuf = NULL);
	VarSizeType GetDDD(char *aBuf = NULL);
	VarSizeType MyGetTickCount(char *aBuf = NULL);
	VarSizeType GetNow(char *aBuf = NULL);
	VarSizeType GetNowUTC(char *aBuf = NULL);
	VarSizeType GetOSType(char *aBuf = NULL);
	VarSizeType GetOSVersion(char *aBuf = NULL);
	VarSizeType GetLanguage(char *aBuf = NULL);
	VarSizeType GetUserOrComputer(bool aGetUser, char *aBuf = NULL);
	VarSizeType GetProgramFiles(char *aBuf = NULL);
	VarSizeType GetDesktop(bool aGetCommon, char *aBuf = NULL);
	VarSizeType GetStartMenu(bool aGetCommon, char *aBuf = NULL);
	VarSizeType GetPrograms(bool aGetCommon, char *aBuf = NULL);
	VarSizeType GetStartup(bool aGetCommon, char *aBuf = NULL);
	VarSizeType GetMyDocuments(char *aBuf = NULL);
	VarSizeType GetIsAdmin(char *aBuf = NULL);
	VarSizeType ScriptGetCursor(char *aBuf = NULL);
	VarSizeType ScriptGetCaret(VarTypeType aVarType, char *aBuf = NULL);
	VarSizeType GetScreenWidth(char *aBuf = NULL);
	VarSizeType GetScreenHeight(char *aBuf = NULL);
	VarSizeType GetIP(int aAdapterIndex, char *aBuf = NULL);
	VarSizeType GetFilename(char *aBuf = NULL);
	VarSizeType GetFileDir(char *aBuf = NULL);
	VarSizeType GetFilespec(char *aBuf = NULL);
	VarSizeType GetLoopFileName(char *aBuf = NULL);
	VarSizeType GetLoopFileShortName(char *aBuf = NULL);
	VarSizeType GetLoopFileDir(char *aBuf = NULL);
	VarSizeType GetLoopFileFullPath(char *aBuf = NULL);
	VarSizeType GetLoopFileLongPath(char *aBuf = NULL);
	VarSizeType GetLoopFileShortPath(char *aBuf = NULL);
	VarSizeType GetLoopFileTimeModified(char *aBuf = NULL);
	VarSizeType GetLoopFileTimeCreated(char *aBuf = NULL);
	VarSizeType GetLoopFileTimeAccessed(char *aBuf = NULL);
	VarSizeType GetLoopFileAttrib(char *aBuf = NULL);
	VarSizeType GetLoopFileSize(char *aBuf, int aDivider);
	VarSizeType GetLoopRegType(char *aBuf = NULL);
	VarSizeType GetLoopRegKey(char *aBuf = NULL);
	VarSizeType GetLoopRegSubKey(char *aBuf = NULL);
	VarSizeType GetLoopRegName(char *aBuf = NULL);
	VarSizeType GetLoopRegTimeModified(char *aBuf = NULL);
	VarSizeType GetLoopReadLine(char *aBuf = NULL);
	VarSizeType GetLoopField(char *aBuf = NULL);
	VarSizeType GetLoopIndex(char *aBuf = NULL);
	VarSizeType GetThisMenuItem(char *aBuf = NULL);
	VarSizeType GetThisMenuItemPos(char *aBuf = NULL);
	VarSizeType GetThisMenu(char *aBuf = NULL);
	VarSizeType GetThisHotkey(char *aBuf = NULL);
	VarSizeType GetPriorHotkey(char *aBuf = NULL);
	VarSizeType GetTimeSinceThisHotkey(char *aBuf = NULL);
	VarSizeType GetTimeSincePriorHotkey(char *aBuf = NULL);
	VarSizeType GetEndChar(char *aBuf = NULL);
	VarSizeType GetGui(VarTypeType aVarType, char *aBuf = NULL);
	VarSizeType GetGuiControl(char *aBuf = NULL);
	VarSizeType GetGuiControlEvent(char *aBuf = NULL);
	VarSizeType GetTimeIdle(char *aBuf = NULL);

	Script();
	~Script();
	// Note that the anchors to any linked lists will be lost when this
	// object goes away, so for now, be sure the destructor is only called
	// when the program is about to be exited, which will thereby reclaim
	// the memory used by the abandoned linked lists (otherwise, a memory
	// leak will result).
};

#endif

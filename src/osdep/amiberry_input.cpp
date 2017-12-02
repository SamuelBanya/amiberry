#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "options.h"
#include "inputdevice.h"
#include <SDL.h>

#include "zfile.h"  /// Horace added

#include <fstream>  /// Horace added 
#include <string> /// Horace added (to remove)
#include <stdlib.h>   /// Horace added   /* atol */


static struct host_input_button default_controller_map;
struct host_input_button host_input_buttons[MAX_INPUT_DEVICES];

static struct host_keyboard_button default_keyboard_map;
struct host_keyboard_button host_keyboard_buttons[4];


static int joyXviaCustom = 0;
static int joyYviaCustom = 0;
static int joyButXviaCustom[7] = {0, 0, 0, 0, 0, 0, 0};
static int mouseBut1viaCustom = 0;
static int mouseBut2viaCustom = 0;


const int REMAP_BUTTONS = 16;
#define REMAP_BUTTONS             16
#define MAX_MOUSE_BUTTONS	  2
#define MAX_MOUSE_AXES            2
#define FIRST_MOUSE_AXIS	  0
#define FIRST_MOUSE_BUTTON	MAX_MOUSE_AXES
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos) (var |= 1 << pos)

static int numMice = 0;
static int numKeysAsJoys;

static void fill_default_controller(void)
{
	default_controller_map.hotkey_button = -1;
	default_controller_map.quit_button = -1;
	default_controller_map.reset_button = -1;
	default_controller_map.menu_button = -1;

	default_controller_map.number_of_hats = 1;

	default_controller_map.north_button = 3; // yellow
	default_controller_map.east_button = 1; // blue
	default_controller_map.south_button = 0; // red
	default_controller_map.west_button = 2; // green

	default_controller_map.dpad_left = -1;
	default_controller_map.dpad_right = -1;
	default_controller_map.dpad_up = -1;
	default_controller_map.dpad_down = -1;

	default_controller_map.select_button = -1; //                          
	default_controller_map.start_button = 4; // play
	default_controller_map.left_shoulder = 6; // rwd
	default_controller_map.right_shoulder = 5; // fwd

	default_controller_map.left_shoulder = -1; // rwd
	default_controller_map.right_shoulder = -1; // fwd
	default_controller_map.left_trigger = -1; // alt. fire
	default_controller_map.right_trigger = -1; // esc?

	default_controller_map.lstick_button = -1;
	default_controller_map.lstick_left = -1;
	default_controller_map.lstick_right = -1;
	default_controller_map.lstick_up = -1;
	default_controller_map.lstick_down = -1;
	default_controller_map.lstick_axis_y = 1;
	default_controller_map.lstick_axis_x = 0;

	default_controller_map.rstick_button = -1;
	default_controller_map.rstick_left = -1;
	default_controller_map.rstick_right = -1;
	default_controller_map.rstick_up = -1;
	default_controller_map.rstick_down = -1;
	default_controller_map.rstick_axis_y = 3;
	default_controller_map.rstick_axis_x = 2;

	default_controller_map.is_retroarch = false;
}

static void fill_blank_controller(void)
{
	default_controller_map.hotkey_button = -1;
	default_controller_map.quit_button = -1;
	default_controller_map.reset_button = -1;
	default_controller_map.menu_button = -1;

	default_controller_map.number_of_hats = -1;

	default_controller_map.north_button = -1; // yellow
	default_controller_map.east_button = -1; // blue
	default_controller_map.south_button = -1; // red
	default_controller_map.west_button = -1; // green

	default_controller_map.dpad_left = -1;
	default_controller_map.dpad_right = -1;
	default_controller_map.dpad_up = -1;
	default_controller_map.dpad_down = -1;

	default_controller_map.select_button = -1; //                 
	default_controller_map.start_button = -1; // play
	default_controller_map.left_shoulder = -1; // rwd
	default_controller_map.right_shoulder = -1; // fwd

	default_controller_map.left_shoulder = -1; // rwd
	default_controller_map.right_shoulder = -1; // fwd
	default_controller_map.left_trigger = -1; // alt. fire
	default_controller_map.right_trigger = -1; // esc?

	default_controller_map.lstick_button = -1;
	default_controller_map.lstick_left = -1;
	default_controller_map.lstick_right = -1;
	default_controller_map.lstick_up = -1;
	default_controller_map.lstick_down = -1;
	default_controller_map.lstick_axis_y = -1;
	default_controller_map.lstick_axis_x = -1;

	default_controller_map.rstick_button = -1;
	default_controller_map.rstick_left = -1;
	default_controller_map.rstick_right = -1;
	default_controller_map.rstick_up = -1;
	default_controller_map.rstick_down = -1;
	default_controller_map.rstick_axis_y = -1;
	default_controller_map.rstick_axis_x = -1;

	default_controller_map.is_retroarch = false;
}


static void fill_default_keyboard(void)
{
	// test using iPac layout 
	default_keyboard_map.north_button = SDLK_LSHIFT;
	default_keyboard_map.east_button = SDLK_LCTRL;
	default_keyboard_map.south_button = SDLK_LALT;
	default_keyboard_map.west_button = SDLK_SPACE;
	default_keyboard_map.dpad_left = SDLK_LEFT;
	default_keyboard_map.dpad_right = SDLK_RIGHT;
	default_keyboard_map.dpad_up = SDLK_UP;
	default_keyboard_map.dpad_down = SDLK_DOWN;
	default_keyboard_map.left_shoulder = SDLK_z;
	default_keyboard_map.right_shoulder = SDLK_x;
	default_keyboard_map.select_button = SDLK_1;
	default_keyboard_map.start_button = SDLK_2;
	default_keyboard_map.lstick_button = SDLK_F1;
	default_keyboard_map.rstick_button = SDLK_F2;

	default_keyboard_map.is_retroarch = false;
}

//# Keyboard input. Will recognize letters (a to z) and the following special keys (where kp_
//# is for keypad keys):
//#
//#   "left", right, up, down, enter, kp_enter, tab, insert, del, end, home,
//#   rshift, shift, ctrl, alt, space, escape, add, subtract, kp_plus, kp_minus,
//#   f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
//#   num0, num1, num2, num3, num4, num5, num6, num7, num8, num9, pageup, pagedown,
//#   keypad0, keypad1, keypad2, keypad3, keypad4, keypad5, keypad6, keypad7, keypad8, keypad9,
//#   period, capslock, numlock, backspace, multiply, divide, print_screen, scroll_lock,
//#   tilde, backquote, pause, quote, comma, minus, slash, semicolon, equals, leftbracket,
//#   backslash, rightbracket, kp_period, kp_equals, rctrl, ralt

const int RemapKeyMapList[] = {
	-1,
	SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e,
	SDLK_f, SDLK_g, SDLK_h, SDLK_i, SDLK_j,
	SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o,
	SDLK_p, SDLK_q, SDLK_r, SDLK_s, SDLK_t,
	SDLK_u, SDLK_v, SDLK_w, SDLK_x, SDLK_y, SDLK_z,
	SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN, SDLK_RETURN,
	SDLK_KP_ENTER, SDLK_TAB, SDLK_INSERT, SDLK_DELETE, SDLK_END, SDLK_HOME,
	SDLK_RSHIFT, SDLK_LSHIFT, SDLK_LCTRL, SDLK_LALT, SDLK_SPACE,
	SDLK_ESCAPE, SDLK_PLUS, SDLK_MINUS, SDLK_KP_PLUS, SDLK_KP_MINUS,
	SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6,
	SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10, SDLK_F11, SDLK_F12,
	SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
	SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_PAGEUP, SDLK_PAGEDOWN,
	SDLK_KP_0, SDLK_KP_1, SDLK_KP_2, SDLK_KP_3, SDLK_KP_4,
	SDLK_KP_5, SDLK_KP_6, SDLK_KP_7, SDLK_KP_8, SDLK_KP_9,
	SDLK_PERIOD, SDLK_CAPSLOCK, SDLK_NUMLOCKCLEAR, SDLK_BACKSPACE,
	//              divide
	SDLK_ASTERISK, -1, SDLK_PRINTSCREEN, SDLK_SCROLLLOCK,
	// tilde
	-1, SDLK_BACKQUOTE,SDLK_PAUSE, SDLK_QUOTE, SDLK_COMMA,
	SDLK_MINUS, SDLK_SLASH, SDLK_SEMICOLON,SDLK_EQUALS, SDLK_LEFTPAREN,
	SDLK_BACKSLASH, SDLK_RIGHTPAREN,
	SDLK_KP_PERIOD, SDLK_KP_EQUALS,SDLK_RCTRL, SDLK_RALT
};


const char* RemapKeyMapListStrings[] = {
	"nul",
	"a", "b", "c", "d", "e",
	"f", "g", "h", "i", "j",
	"k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t",
	"u", "v", "w", "x", "y", "z",
	"left", "right", "up", "down", "enter",
	"kp_enter", "tab", "insert", "del", "end", "home",
	"rshift", "shift", "ctrl", "alt", "space",
	"escape", "add", "subtract", "kp_plus", "kp_minus",
	"f1", "f2", "f3", "f4", "f5", "f6",
	"f7", "f8", "f9", "f10", "f11", "f12",
	"num0", "num1", "num2", "num3", "num4", "num5",
	"num6", "num7", "num8", "num9", "pageup", "pagedown",
	"keypad0", "keypad1", "keypad2", "keypad3", "keypad4",
	"keypad5", "keypad6", "keypad7", "keypad8", "keypad9",
	"period", "capslock", "numlock", "backspace",
	"multiply", "divide", "print_screen", "scroll_lock",
	"tilde", "backquote","pause", "quote", "comma",
	"minus", "slash", "semicolon","equals", "leftbracket",
	"backslash", "rightbracket",
	"kp_period", "kp_equals","rctrl", "ralt"

};

const int RemapKeyMapListSize = sizeof(RemapKeyMapList) / sizeof(RemapKeyMapList[0]);


static int init_mouse(void)
{
	const int mouse = open("/dev/input/mouse0", O_RDONLY);
	if (mouse != -1)
	{
		numMice++;
		close(mouse);
	}
	return numMice;
}

static void close_mouse(void)
{
}

static int acquire_mouse(int num, int flags)
{
	if (num >= 0 && num < numMice)
		return 1;
	return 0;
}

static void unacquire_mouse(int num)
{
}

static int get_mouse_num(void)
{
	return numMice;
}

static const TCHAR* get_mouse_friendlyname(int mouse)
{
	if (numMice > 0 && mouse == 0)
		return "Mouse";
	return "";
}

static const TCHAR* get_mouse_uniquename(int mouse)
{
	if (numMice > 0 && mouse == 0)
		return "MOUSE0";
	return "";
}

static int get_mouse_widget_num(int mouse)
{
	if (numMice > 0 && mouse == 0)
	{
		return MAX_MOUSE_AXES + MAX_MOUSE_BUTTONS;
	}
	return 0;
}

static int get_mouse_widget_first(int mouse, int type)
{
	if (numMice > 0 && mouse == 0)
	{
		switch (type)
		{
		case IDEV_WIDGET_BUTTON:
			return FIRST_MOUSE_BUTTON;
		case IDEV_WIDGET_AXIS:
			return FIRST_MOUSE_AXIS;
		case IDEV_WIDGET_BUTTONAXIS:
			return MAX_MOUSE_AXES + MAX_MOUSE_BUTTONS;
		}
	}
	return -1;
}

static int get_mouse_widget_type(int mouse, int num, TCHAR* name, uae_u32* code)
{
	if (numMice > 0 && mouse == 0)
	{
		if (num >= MAX_MOUSE_AXES && num < MAX_MOUSE_AXES + MAX_MOUSE_BUTTONS)
		{
			if (name)
				sprintf(name, "Button %d", num + 1 - MAX_MOUSE_AXES);
			return IDEV_WIDGET_BUTTON;
		}
		if (num < MAX_MOUSE_AXES)
		{
			if (name)
			{
				if (num == 0)
					sprintf(name, "X Axis");
				else if (num == 1)
					sprintf(name, "Y Axis");
				else
					sprintf(name, "Axis %d", num + 1);
			}
			return IDEV_WIDGET_AXIS;
		}
	}
	return IDEV_WIDGET_NONE;
}

static void read_mouse(void)
{
	if (currprefs.input_tablet > TABLET_OFF)
	{
		// Mousehack active
		int x, y;
		SDL_GetMouseState(&x, &y);
		setmousestate(0, 0, x, 1);
		setmousestate(0, 1, y, 1);
	}
}


static int get_mouse_flags(int num)
{
	return 0;
}

struct inputdevice_functions inputdevicefunc_mouse = {
	init_mouse, close_mouse, acquire_mouse, unacquire_mouse, read_mouse,
	get_mouse_num, get_mouse_friendlyname, get_mouse_uniquename,
	get_mouse_widget_num, get_mouse_widget_type,
	get_mouse_widget_first,
	get_mouse_flags
};

static void setid(struct uae_input_device* uid, int i, int slot, int sub, int port, int evt, bool gp)
{
	if (gp)
		inputdevice_sparecopy(&uid[i], slot, 0);
	uid[i].eventid[slot][sub] = evt;
	uid[i].port[slot][sub] = port + 1;
}

static void setid_af(struct uae_input_device* uid, int i, int slot, int sub, int port, int evt, int af, bool gp)
{
	setid(uid, i, slot, sub, port, evt, gp);
	uid[i].flags[slot][sub] &= ~ID_FLAG_AUTOFIRE_MASK;
	if (af >= JPORT_AF_NORMAL)
		uid[i].flags[slot][sub] |= ID_FLAG_AUTOFIRE;
}

int input_get_default_mouse(struct uae_input_device* uid, int i, int port, int af, bool gp, bool wheel,
                            bool joymouseswap)
{
	setid(uid, i, ID_AXIS_OFFSET + 0, 0, port, port ? INPUTEVENT_MOUSE2_HORIZ : INPUTEVENT_MOUSE1_HORIZ, gp);
	setid(uid, i, ID_AXIS_OFFSET + 1, 0, port, port ? INPUTEVENT_MOUSE2_VERT : INPUTEVENT_MOUSE1_VERT, gp);
	setid_af(uid, i, ID_BUTTON_OFFSET + 0, 0, port, port ? INPUTEVENT_JOY2_FIRE_BUTTON : INPUTEVENT_JOY1_FIRE_BUTTON, af,
	         gp);
	setid(uid, i, ID_BUTTON_OFFSET + 1, 0, port, port ? INPUTEVENT_JOY2_2ND_BUTTON : INPUTEVENT_JOY1_2ND_BUTTON, gp);

	if (i == 0)
		return 1;
	return 0;
}


static int init_kb(void)
{
	return 1;
}

static void close_kb(void)
{
}

static int acquire_kb(int num, int flags)
{
	return 1;
}

static void unacquire_kb(int num)
{
}

static void read_kb(void)
{
}

static int get_kb_num(void)
{
	return 1;
}

static const TCHAR* get_kb_friendlyname(int kb)
{
	return strdup("Default Keyboard");
}


static const TCHAR* get_kb_uniquename(int kb)
{
	return strdup("KEYBOARD0");
}

static int get_kb_widget_num(int kb)
{
	return 255;
}

static int get_kb_widget_first(int kb, int type)
{
	return 0;
}

static int get_kb_widget_type(int kb, int num, TCHAR* name, uae_u32* code)
{
	if (code)
		*code = num;
	return IDEV_WIDGET_KEY;
}

static int get_kb_flags(int num)
{
	return 0;
}

struct inputdevice_functions inputdevicefunc_keyboard = {
	init_kb, close_kb, acquire_kb, unacquire_kb, read_kb,
	get_kb_num, get_kb_friendlyname, get_kb_uniquename,
	get_kb_widget_num, get_kb_widget_type,
	get_kb_widget_first,
	get_kb_flags
};

int input_get_default_keyboard(int num)
{
	if (num == 0)
	{
		return 1;
	}
	return 0;
}

#define MAX_JOY_BUTTONS	  16
#define MAX_JOY_AXES	    8
#define FIRST_JOY_AXIS	  0
#define FIRST_JOY_BUTTON	MAX_JOY_AXES

static int nr_joysticks = 0;
static char JoystickName[MAX_INPUT_DEVICES][80];

static char IsPS3Controller[MAX_INPUT_DEVICES];

static SDL_Joystick* Joysticktable[MAX_INPUT_DEVICES];


int find_retroarch(const TCHAR* find_setting, char* retroarch_file, host_input_button current_hostinput)
{
	// opening file and parsing
	ifstream readFile(retroarch_file);
	string line;
	string delimiter = " = ";

	int tempbutton = -1;

	// read each line in
	while (std::getline(readFile, line))
	{
		const string option = line.substr(0, line.find(delimiter));

		if (option != line) // exit if we got no result from splitting the string
		{
			// using the " = " to work out whis is the option, and which is the parameter.
			string param = line.substr(line.find(delimiter) + delimiter.length(), line.length());

			// remove leading "
			if (param.at(0) == '"')
			{
				param.erase(0, 1);
			}

			// remove trailing "
			if (param.at(param.length() - 1) == '"')
			{
				param.erase(param.length() - 1, 1);
			}

			//  time to get the output number
			if (param.at(0) != 'h') // check it isnt some kind of hat starting 'h' (so if D-pad uses buttons)
			{
				tempbutton = abs(atol(param.c_str()));
			} // gets the parameter

			// this will need something separate to pull out the number of hats 
			//  use SET_BIT on hX numbers 

			if (find_setting == "count_hats" && param.at(0) == 'h')
			{
				if (param.at(1) == '0') { SET_BIT (tempbutton,0); }
				if (param.at(1) == '1') { SET_BIT (tempbutton,1); }
				if (param.at(1) == '2') { SET_BIT (tempbutton,2); }
				if (param.at(1) == '3') { SET_BIT (tempbutton,3); }
			}

			// ok, this is the 'normal' storing of values
			if (option == find_setting)
			{
				break;
			}
			if (find_setting != "count_hats")
			{
				tempbutton = -1;
			}
		}
	}
	readFile.close();

	return tempbutton;
}


const TCHAR* find_retroarch_key(const TCHAR* find_setting, char* retroarch_file)
{
	// opening file and parsing
	ifstream readFile(retroarch_file);
	string line;
	string delimiter = " = ";

	const TCHAR* output = "nul";

	// read each line in
	while (std::getline(readFile, line))
	{
		const string option = line.substr(0, line.find(delimiter));

		if (option != line) // exit if we got no result from splitting the string
		{
			// using the " = " to work out whis is the option, and which is the parameter.
			string param = line.substr(line.find(delimiter) + delimiter.length(), line.length());

			// remove leading "
			if (param.at(0) == '"')
			{
				param.erase(0, 1);
			}

			// remove trailing "
			if (param.at(param.length() - 1) == '"')
			{
				param.erase(param.length() - 1, 1);
			}

			output = &param[0u];

			// ok, this is the 'normal' storing of values
			if (option == find_setting)
			{
				break;
			}
			output = "nul";
		}
	}
	readFile.close();

	return output;
}

int find_string_in_array(const char* arr[], int n, const char* key)
{
	int index = -1;

	for (int i = 0; i < n; i++)
	{
		// _tcscmp  // strcmp
		if (!_tcsicmp(arr[i], key))
		{
			index = i;
			break;
		}
	}

	return index;
}

static int get_joystick_num(void)
{
	// Add the Keyboard Arrow keys as Joystick 0, or retroarch keyboards (1-4)
	return nr_joysticks + numKeysAsJoys;
}

//
static int init_joystick(void)
{
	// we will also use this routine to grab the retroarch buttons

	// set up variables / paths etc.
	char retroarch_file[MAX_PATH];
	extern void fetch_retroarchfile(char* out, int size);
	fetch_retroarchfile(retroarch_file, MAX_PATH);

	if (zfile_exists(retroarch_file))
	{
		struct host_keyboard_button temp_keyboard_buttons;

		const char* tempkey = find_retroarch_key("input_player1_y", retroarch_file);
		int x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.north_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_a", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.east_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_b", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.south_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_x", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.west_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_left", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.dpad_left = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_right", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.dpad_right = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_up", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.dpad_up = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_down", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.dpad_down = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_l", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.left_shoulder = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_r", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.right_shoulder = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_select", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.select_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_start", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.start_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_l3", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.lstick_button = RemapKeyMapList[x];
		tempkey = find_retroarch_key("input_player1_r3", retroarch_file);
		x = find_string_in_array(RemapKeyMapListStrings, RemapKeyMapListSize, tempkey);
		temp_keyboard_buttons.rstick_button = RemapKeyMapList[x];

		temp_keyboard_buttons.is_retroarch = true;
		host_keyboard_buttons[0] = temp_keyboard_buttons;
		numKeysAsJoys = 1;
	}

	else
	{
		fill_default_keyboard();
		host_keyboard_buttons[0] = default_keyboard_map;
		numKeysAsJoys = 1;
	}


	//This function is called too many times... we can filter if number of joy is good...
	// this code 
	if (nr_joysticks == SDL_NumJoysticks())
		return 1;

	// cap the number of joysticks etc
	nr_joysticks = SDL_NumJoysticks();
	if (nr_joysticks > MAX_INPUT_DEVICES)
		nr_joysticks = MAX_INPUT_DEVICES;

	// set up variables / paths etc.
	char tmp[MAX_PATH];
	extern void fetch_controllerspath(char* out, int size);
	fetch_controllerspath(tmp, MAX_PATH);

	// do the loop
	for (int cpt = 0; cpt < nr_joysticks; cpt++)
	{
		Joysticktable[cpt] = SDL_JoystickOpen(cpt);

		if (Joysticktable[cpt] != nullptr)
		{
			if (SDL_JoystickNameForIndex(cpt) != nullptr)
				strncpy(JoystickName[cpt], SDL_JoystickNameForIndex(cpt), sizeof JoystickName[cpt] - 1);
			else
				sprintf(JoystickName[cpt], "Joystick%d", cpt);

			//this now uses controllers path (in tmp)
			char ControlConfig[255];
			strcpy(ControlConfig, tmp);
			strcat(ControlConfig, JoystickName[cpt]);
			strcat(ControlConfig, ".cfg");

			if (zfile_exists(ControlConfig))
			{
				fill_blank_controller();
				host_input_buttons[cpt] = default_controller_map;

				host_input_buttons[cpt].is_retroarch = true;

				host_input_buttons[cpt].hotkey_button = find_retroarch("input_enable_hotkey_btn", ControlConfig,
				                                                       host_input_buttons[cpt]);

				host_input_buttons[cpt].quit_button = find_retroarch("input_exit_emulator_btn", ControlConfig,
				                                                     host_input_buttons[cpt]);
				host_input_buttons[cpt].menu_button = find_retroarch("input_menu_toggle_btn", ControlConfig,
				                                                     host_input_buttons[cpt]);
				host_input_buttons[cpt].reset_button = find_retroarch("input_reset_btn", ControlConfig, host_input_buttons[cpt]);

				host_input_buttons[cpt].east_button = find_retroarch("input_a_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].south_button = find_retroarch("input_b_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].north_button = find_retroarch("input_x_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].west_button = find_retroarch("input_y_btn", ControlConfig, host_input_buttons[cpt]);

				host_input_buttons[cpt].dpad_left = find_retroarch("input_left_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].dpad_right = find_retroarch("input_right_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].dpad_up = find_retroarch("input_up_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].dpad_down = find_retroarch("input_down_btn", ControlConfig, host_input_buttons[cpt]);

				host_input_buttons[cpt].select_button = find_retroarch("input_select_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].start_button = find_retroarch("input_start_btn", ControlConfig, host_input_buttons[cpt]);

				host_input_buttons[cpt].left_shoulder = find_retroarch("input_l_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].right_shoulder = find_retroarch("input_r_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].left_trigger = find_retroarch("input_l2_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].right_trigger = find_retroarch("input_r2_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].lstick_button = find_retroarch("input_l3_btn", ControlConfig, host_input_buttons[cpt]);
				host_input_buttons[cpt].rstick_button = find_retroarch("input_r3_btn", ControlConfig, host_input_buttons[cpt]);

				host_input_buttons[cpt].lstick_axis_x = find_retroarch("input_l_x_plus_axis", ControlConfig,
				                                                       host_input_buttons[cpt]);
				if (host_input_buttons[cpt].lstick_axis_x == -1)
				{
					host_input_buttons[cpt].lstick_axis_x = find_retroarch("input_right_axis", ControlConfig, host_input_buttons[cpt]);
				}

				host_input_buttons[cpt].lstick_axis_y = find_retroarch("input_l_y_plus_axis", ControlConfig,
				                                                       host_input_buttons[cpt]);
				if (host_input_buttons[cpt].lstick_axis_y == -1)
				{
					host_input_buttons[cpt].lstick_axis_y = find_retroarch("input_down_axis", ControlConfig, host_input_buttons[cpt]);
				}

				host_input_buttons[cpt].rstick_axis_x = find_retroarch("input_r_x_plus_axis", ControlConfig,
				                                                       host_input_buttons[cpt]);
				host_input_buttons[cpt].rstick_axis_y = find_retroarch("input_r_y_plus_axis", ControlConfig,
				                                                       host_input_buttons[cpt]);

				host_input_buttons[cpt].number_of_hats = find_retroarch("count_hats", ControlConfig, host_input_buttons[cpt]);

				//input_state_slot_increase_axis
				//input_state_slot_decrease_axis
			} // end of .cfg file found

			else // do manual checks if there is no .cfg
			{
				//printf("    No .cfg config file found.\n");
				//printf("    Using default controller layout: with n/s/e/w mapping.\n");
				fill_default_controller();
				host_input_buttons[cpt] = default_controller_map;
			}
		}
	}

	return 1;
}


void import_joysticks(void)
{
	init_joystick();
}

static void close_joystick(void)
{
	for (int cpt = 0; cpt < nr_joysticks; cpt++)
	{
		SDL_JoystickClose(Joysticktable[cpt]);
	}
	nr_joysticks = 0;
}

static int acquire_joystick(int num, int flags)
{
	if (num >= 0 && num < nr_joysticks)
		return 1;
	return 0;
}

static void unacquire_joystick(int num)
{
}

static const TCHAR* get_joystick_friendlyname(int joy)
{
	TCHAR* tmp1 = new char[255];
	for (int n = 0; n < numKeysAsJoys; ++n)
	{
		if (joy == n)
		{
			if (host_keyboard_buttons[n].is_retroarch == true)
			{
				sprintf(tmp1, "RetroArch Keyboard as Joystick [Input #%d]", n + 1);
				return tmp1;
			}
			return "Keyboard as Joystick [Default]";
		}
	}
	return JoystickName[joy - numKeysAsJoys];
}

static const TCHAR* get_joystick_uniquename(int joy)
{
	if (joy == 0)
		return "JOY0";
	if (joy == 1)
		return "JOY1";
	if (joy == 2)
		return "JOY2";
	if (joy == 3)
		return "JOY3";
	if (joy == 4)
		return "JOY4";
	if (joy == 5)
		return "JOY5";
	if (joy == 6)
		return "JOY6";

	return "JOY7";
}

static int get_joystick_widget_num(int joy)
{
	if (joy >= 0 && joy < nr_joysticks)
	{
		return SDL_JoystickNumAxes(Joysticktable[joy]) + SDL_JoystickNumButtons(Joysticktable[joy]);
	}
	return 0;
}

static int get_joystick_widget_first(int joy, int type)
{
	if (joy >= 0 && joy < nr_joysticks)
	{
		switch (type)
		{
		case IDEV_WIDGET_BUTTON:
			return FIRST_JOY_BUTTON;
		case IDEV_WIDGET_AXIS:
			return FIRST_JOY_AXIS;
		case IDEV_WIDGET_BUTTONAXIS:
			return MAX_JOY_AXES + MAX_JOY_BUTTONS;
		}
	}
	return -1;
}

static int get_joystick_widget_type(int joy, int num, TCHAR* name, uae_u32* code)
{
	if (joy >= 0 && joy < nr_joysticks)
	{
		if (num >= MAX_JOY_AXES && num < MAX_JOY_AXES + MAX_JOY_BUTTONS)
		{
			if (name)
			{
				switch (num)
				{
				case FIRST_JOY_BUTTON:
					sprintf(name, "Button X/CD32 red");
					break;
				case FIRST_JOY_BUTTON + 1:
					sprintf(name, "Button B/CD32 blue");
					break;
				case FIRST_JOY_BUTTON + 2:
					sprintf(name, "Button A/CD32 green");
					break;
				case FIRST_JOY_BUTTON + 3:
					sprintf(name, "Button Y/CD32 yellow");
					break;
				case FIRST_JOY_BUTTON + 4:
					sprintf(name, "CD32 start");
					break;
				case FIRST_JOY_BUTTON + 5:
					sprintf(name, "CD32 ffw");
					break;
				case FIRST_JOY_BUTTON + 6:
					sprintf(name, "CD32 rwd");
					break;
				}
			}
			return IDEV_WIDGET_BUTTON;
		}
		if (num < MAX_JOY_AXES)
		{
			if (name)
			{
				if (num == 0)
					sprintf(name, "X Axis");
				else if (num == 1)
					sprintf(name, "Y Axis");
				else
					sprintf(name, "Axis %d", num + 1);
			}
			return IDEV_WIDGET_AXIS;
		}
	}
	return IDEV_WIDGET_NONE;
}

static int get_joystick_flags(int num)
{
	return 0;
}


static void read_joystick(void)
{
	for (int joyid = 0; joyid < MAX_JPORTS; joyid++)
	{
		// First handle retroarch (or default) keys as Joystick...
		if (currprefs.jports[joyid].id >= JSEM_JOYS && currprefs.jports[joyid].id < JSEM_JOYS + numKeysAsJoys)
		{
			const int hostkeyid = currprefs.jports[joyid].id - JSEM_JOYS;

			const Uint8* keystate = SDL_GetKeyboardState(nullptr);

			// cd32 red, blue, green, yellow
			setjoybuttonstate(0, 0, keystate[host_keyboard_buttons[hostkeyid].south_button]); // b
			setjoybuttonstate(0, 1, keystate[host_keyboard_buttons[hostkeyid].east_button]); // a
			setjoybuttonstate(0, 2, keystate[host_keyboard_buttons[hostkeyid].north_button]); //y 
			setjoybuttonstate(0, 3, keystate[host_keyboard_buttons[hostkeyid].west_button]); // x

			setjoybuttonstate(0, 4, keystate[host_keyboard_buttons[hostkeyid].left_shoulder]); // z 
			setjoybuttonstate(0, 5, keystate[host_keyboard_buttons[hostkeyid].right_shoulder]); // x
			setjoybuttonstate(0, 6, keystate[host_keyboard_buttons[hostkeyid].start_button]); //num1
			// up down left right     
			setjoybuttonstate(0, 7, keystate[host_keyboard_buttons[hostkeyid].dpad_up]);
			setjoybuttonstate(0, 8, keystate[host_keyboard_buttons[hostkeyid].dpad_down]);
			setjoybuttonstate(0, 9, keystate[host_keyboard_buttons[hostkeyid].dpad_left]);
			setjoybuttonstate(0, 10, keystate[host_keyboard_buttons[hostkeyid].dpad_right]);

			//stick left/right     
			setjoybuttonstate(0, 11, keystate[host_keyboard_buttons[hostkeyid].lstick_button]);
			setjoybuttonstate(0, 12, keystate[host_keyboard_buttons[hostkeyid].rstick_button]);
			setjoybuttonstate(0, 13, keystate[host_keyboard_buttons[hostkeyid].select_button]); // num2
		}

		// this is what we actually use on the Pi (for joysticks :)
		else if (jsem_isjoy(joyid, &currprefs) != -1)
		{
			// Now we handle real SDL joystick...
			const int hostjoyid = currprefs.jports[joyid].id - JSEM_JOYS - numKeysAsJoys;

			static struct host_input_button current_controller_map;

			current_controller_map = host_input_buttons[hostjoyid];

			if (current_controller_map.east_button == current_controller_map.hotkey_button)
			{
				current_controller_map.east_button = -1;
			}
			if (current_controller_map.south_button == current_controller_map.hotkey_button)
			{
				current_controller_map.south_button = -1;
			}
			if (current_controller_map.north_button == current_controller_map.hotkey_button)
			{
				current_controller_map.north_button = -1;
			}
			if (current_controller_map.west_button == current_controller_map.hotkey_button)
			{
				current_controller_map.west_button = -1;
			}
			if (current_controller_map.dpad_left == current_controller_map.hotkey_button)
			{
				current_controller_map.dpad_left = -1;
			}
			if (current_controller_map.dpad_right == current_controller_map.hotkey_button)
			{
				current_controller_map.dpad_right = -1;
			}
			if (current_controller_map.dpad_up == current_controller_map.hotkey_button) { current_controller_map.dpad_up = -1; }
			if (current_controller_map.dpad_down == current_controller_map.hotkey_button)
			{
				current_controller_map.dpad_down = -1;
			}
			if (current_controller_map.select_button == current_controller_map.hotkey_button)
			{
				current_controller_map.select_button = -1;
			}
			if (current_controller_map.start_button == current_controller_map.hotkey_button)
			{
				current_controller_map.start_button = -1;
			}
			if (current_controller_map.left_trigger == current_controller_map.hotkey_button)
			{
				current_controller_map.left_trigger = -1;
			}
			if (current_controller_map.right_trigger == current_controller_map.hotkey_button)
			{
				current_controller_map.right_trigger = -1;
			}
			if (current_controller_map.lstick_button == current_controller_map.hotkey_button)
			{
				current_controller_map.lstick_button = -1;
			}
			if (current_controller_map.rstick_button == current_controller_map.hotkey_button)
			{
				current_controller_map.rstick_button = -1;
			}

			if (current_controller_map.left_shoulder == current_controller_map.hotkey_button)
			{
				current_controller_map.left_shoulder = -1;
			}
			if (current_controller_map.right_shoulder == current_controller_map.hotkey_button)
			{
				current_controller_map.right_shoulder = -1;
			}


			// left stick   
			// handle the X axis  (left stick) and d-pad (Hat)
			int val = SDL_JoystickGetAxis(Joysticktable[hostjoyid], current_controller_map.lstick_axis_x);
			setjoystickstate(hostjoyid + 1, 0, val, 32767);

			// handle the Y axis   
			val = SDL_JoystickGetAxis(Joysticktable[hostjoyid], current_controller_map.lstick_axis_y);
			setjoystickstate(hostjoyid + 1, 1, val, 32767);

			// right stick
			val = SDL_JoystickGetAxis(Joysticktable[hostjoyid], current_controller_map.rstick_axis_x);
			setjoystickstate(hostjoyid + 1, 2, val, 32767);

			val = SDL_JoystickGetAxis(Joysticktable[hostjoyid], current_controller_map.rstick_axis_y);
			setjoystickstate(hostjoyid + 1, 3, val, 32767);


			int held_offset = 0;


			// temporary solution for retroarch buttons inc. HOTKEY 
			if (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.hotkey_button) & 1)
			{
				held_offset = REMAP_BUTTONS;
				setjoybuttonstate(hostjoyid + 1, 14,
				                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.menu_button) & 1));
				// menu button
				setjoybuttonstate(hostjoyid + 1, 15,
				                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.quit_button) & 1));
				// quit button
				setjoybuttonstate(hostjoyid + 1, 30,
				                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.reset_button) & 1));
				// reest button
			}

				// this *should* allow us to handle function buttons (l2/r2/select)  <<<  except there were issues this work, picking a fixed number!!                            
				// these two cannot be used whilst we are limtied to 32 buttons, since 'REMAP_BUTTONS' = 14
				// else if (SDL_JoystickGetButton(Joysticktable[hostjoyid], host_input_buttons[hostjoyid].left_trigger) & 1)
				//     held_offset = REMAP_BUTTONS * 2;
				// else if (SDL_JoystickGetButton(Joysticktable[hostjoyid], host_input_buttons[hostjoyid].right_trigger) & 1)
				//     held_offset = REMAP_BUTTONS * 3;

			else
				held_offset = 0;

			// cd32 red, blue, green, yellow
			// south 
			setjoybuttonstate(hostjoyid + 1, 0 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.south_button) & 1));
			// east                    
			setjoybuttonstate(hostjoyid + 1, 1 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.east_button) & 1));
			// west
			setjoybuttonstate(hostjoyid + 1, 2 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.west_button) & 1));
			// north
			setjoybuttonstate(hostjoyid + 1, 3 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.north_button) & 1));


			// cd32  rwd, ffw, start
			setjoybuttonstate(hostjoyid + 1, 4 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.left_shoulder) & 1));
			// left shoulder
			setjoybuttonstate(hostjoyid + 1, 5 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.right_shoulder) & 1));
			// right shoulder
			setjoybuttonstate(hostjoyid + 1, 6 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid], current_controller_map.start_button) & 1));
			// start

			// up down left right
			// HAT Handling *or* D-PAD buttons     
			const int hat = SDL_JoystickGetHat(Joysticktable[hostjoyid], 0);

			setjoybuttonstate(hostjoyid + 1, 7 + held_offset, current_controller_map.dpad_up + 1
				                                                  ? (SDL_JoystickGetButton(
					                                                  Joysticktable[hostjoyid],
					                                                  current_controller_map.dpad_up) & 1)
				                                                  : hat & SDL_HAT_UP);
			setjoybuttonstate(hostjoyid + 1, 8 + held_offset, current_controller_map.dpad_down + 1
				                                                  ? (SDL_JoystickGetButton(
					                                                  Joysticktable[hostjoyid],
					                                                  current_controller_map.dpad_down) & 1)
				                                                  : hat & SDL_HAT_DOWN);
			setjoybuttonstate(hostjoyid + 1, 9 + held_offset, current_controller_map.dpad_left + 1
				                                                  ? (SDL_JoystickGetButton(
					                                                  Joysticktable[hostjoyid],
					                                                  current_controller_map.dpad_left) & 1)
				                                                  : hat & SDL_HAT_LEFT);
			setjoybuttonstate(hostjoyid + 1, 10 + held_offset, current_controller_map.dpad_right + 1
				                                                   ? (SDL_JoystickGetButton(
					                                                   Joysticktable[hostjoyid],
					                                                   current_controller_map.dpad_right) & 1)
				                                                   : hat & SDL_HAT_RIGHT);

			// stick left/right/select
			setjoybuttonstate(hostjoyid + 1, 11 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid],
			                                         current_controller_map.lstick_button) & 1)); // left stick
			setjoybuttonstate(hostjoyid + 1, 12 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid],
			                                         current_controller_map.rstick_button) & 1)); // right stick

			setjoybuttonstate(hostjoyid + 1, 13 + held_offset,
			                  (SDL_JoystickGetButton(Joysticktable[hostjoyid],
			                                         current_controller_map.select_button) & 1)); // select button
		}
	}
}


struct inputdevice_functions inputdevicefunc_joystick = {
	init_joystick, close_joystick, acquire_joystick, unacquire_joystick,
	read_joystick, get_joystick_num, get_joystick_friendlyname, get_joystick_uniquename,
	get_joystick_widget_num, get_joystick_widget_type,
	get_joystick_widget_first,
	get_joystick_flags
};

int input_get_default_joystick(struct uae_input_device* uid, int num, int port, int af, int mode, bool gp,
                               bool joymouseswap)
{
	// DEAL WITH AXIS INPUT EVENTS
	int h, v;

	if (port < 2) // ports 0, 1 ... both sticks, with mousemap
	{
		for (int n = 0; n < 2; ++n)
		{
			if (CHECK_BIT(currprefs.jports[port].mousemap, n))
			{
				h = port ? INPUTEVENT_MOUSE2_HORIZ : INPUTEVENT_MOUSE1_HORIZ;
				v = port ? INPUTEVENT_MOUSE2_VERT : INPUTEVENT_MOUSE1_VERT;
			}
			else
			{
				h = port ? INPUTEVENT_JOY2_HORIZ : INPUTEVENT_JOY1_HORIZ;
				v = port ? INPUTEVENT_JOY2_VERT : INPUTEVENT_JOY1_VERT;
			}

			setid(uid, num, ID_AXIS_OFFSET + (n * 2) + 0, 0, port, h, gp);
			setid(uid, num, ID_AXIS_OFFSET + (n * 2) + 1, 0, port, v, gp);
		}
	}
	else // ports 2, 3 (parallel ports) ... both sticks,
	{
		for (int n = 0; n < 2; ++n)
		{
			h = port - 2 ? INPUTEVENT_PAR_JOY1_HORIZ : INPUTEVENT_PAR_JOY2_HORIZ;
			v = port - 2 ? INPUTEVENT_PAR_JOY1_VERT : INPUTEVENT_PAR_JOY2_VERT;
			setid(uid, num, ID_AXIS_OFFSET + (n * 2) + 0, 0, port, h, gp);
			setid(uid, num, ID_AXIS_OFFSET + (n * 2) + 1, 0, port, v, gp);
		}
	}

	//

	// ASSIGN ALL INPUT EVENT ACTIONS, EITHER CUSTOM OR DEFAULT
	//
	// set up a temporary control layout/ called 'thismap'
	struct joypad_map_layout thismap[4];


	// here, we will fill thismap with defaults, if a custom value is not set. 
	// this will do a lot of the 'logic' of the original code.

	thismap[0] = currprefs.jports[port].amiberry_custom_none; // grab the 'no selection' options for the current map

	// directions

	if (port < 2) // ports 0, 1 ... 
	{
		if (CHECK_BIT(currprefs.jports[port].mousemap,0))
		{
			thismap[0].dpad_up_action = (thismap[0].dpad_up_action
				                             ? thismap[0].dpad_up_action
				                             : (port ? INPUTEVENT_MOUSE2_UP : INPUTEVENT_MOUSE1_UP));
			thismap[0].dpad_down_action = (thismap[0].dpad_down_action
				                               ? thismap[0].dpad_down_action
				                               : (port ? INPUTEVENT_MOUSE2_DOWN : INPUTEVENT_MOUSE1_DOWN));
			thismap[0].dpad_left_action = (thismap[0].dpad_left_action
				                               ? thismap[0].dpad_left_action
				                               : (port ? INPUTEVENT_MOUSE2_LEFT : INPUTEVENT_MOUSE1_LEFT));
			thismap[0].dpad_right_action = (thismap[0].dpad_right_action
				                                ? thismap[0].dpad_right_action
				                                : (port ? INPUTEVENT_MOUSE2_RIGHT : INPUTEVENT_MOUSE1_RIGHT));
		}
		else
		{
			thismap[0].dpad_up_action = (thismap[0].dpad_up_action
				                             ? thismap[0].dpad_up_action
				                             : (port ? INPUTEVENT_JOY2_UP : INPUTEVENT_JOY1_UP));
			thismap[0].dpad_down_action = (thismap[0].dpad_down_action
				                               ? thismap[0].dpad_down_action
				                               : (port ? INPUTEVENT_JOY2_DOWN : INPUTEVENT_JOY1_DOWN));
			thismap[0].dpad_left_action = (thismap[0].dpad_left_action
				                               ? thismap[0].dpad_left_action
				                               : (port ? INPUTEVENT_JOY2_LEFT : INPUTEVENT_JOY1_LEFT));
			thismap[0].dpad_right_action = (thismap[0].dpad_right_action
				                                ? thismap[0].dpad_right_action
				                                : (port ? INPUTEVENT_JOY2_RIGHT : INPUTEVENT_JOY1_RIGHT));
		}
		// standard fire buttons
		if (mode == JSEM_MODE_JOYSTICK_CD32) // CD32 joypad  
		{
			thismap[0].south_action = (thismap[0].south_action
				                           ? thismap[0].south_action
				                           : (port ? INPUTEVENT_JOY2_CD32_RED : INPUTEVENT_JOY1_CD32_RED));
			thismap[0].east_action = (thismap[0].east_action
				                          ? thismap[0].east_action
				                          : (port ? INPUTEVENT_JOY2_CD32_BLUE : INPUTEVENT_JOY1_CD32_BLUE));
			thismap[0].west_action = (thismap[0].west_action
				                          ? thismap[0].west_action
				                          : (port ? INPUTEVENT_JOY2_CD32_GREEN : INPUTEVENT_JOY1_CD32_GREEN));
			thismap[0].north_action = (thismap[0].north_action
				                           ? thismap[0].north_action
				                           : (port ? INPUTEVENT_JOY2_CD32_YELLOW : INPUTEVENT_JOY1_CD32_YELLOW));
			thismap[0].start_action = (thismap[0].start_action
				                           ? thismap[0].start_action
				                           : (port ? INPUTEVENT_JOY2_CD32_PLAY : INPUTEVENT_JOY1_CD32_PLAY));
		}
		else // default, normal joystick  
		{
			thismap[0].south_action = (thismap[0].south_action
				                           ? thismap[0].south_action
				                           : (port ? INPUTEVENT_JOY2_FIRE_BUTTON : INPUTEVENT_JOY1_FIRE_BUTTON));
			thismap[0].east_action = (thismap[0].east_action
				                          ? thismap[0].east_action
				                          : (port ? INPUTEVENT_JOY2_2ND_BUTTON : INPUTEVENT_JOY1_2ND_BUTTON));
			thismap[0].west_action = (thismap[0].west_action
				                          ? thismap[0].west_action
				                          : (port ? INPUTEVENT_JOY2_UP : INPUTEVENT_JOY1_UP));
			thismap[0].north_action = (thismap[0].north_action
				                           ? thismap[0].north_action
				                           : (port ? INPUTEVENT_JOY2_3RD_BUTTON : INPUTEVENT_JOY1_3RD_BUTTON));

			thismap[0].start_action = (thismap[0].start_action ? thismap[0].start_action : INPUTEVENT_KEY_P);
		}


		// shoulder buttons
		if (CHECK_BIT(currprefs.jports[port].mousemap,1))
			// if we use right-analogue as mouse, then we will use shoulder buttons as LMB/RMB
			//if (1==0)
		{
			thismap[0].left_shoulder_action = (thismap[0].left_shoulder_action
				                                   ? thismap[0].left_shoulder_action
				                                   : port
				                                   ? INPUTEVENT_JOY2_FIRE_BUTTON
				                                   : INPUTEVENT_JOY1_FIRE_BUTTON);
			thismap[0].right_shoulder_action = (thismap[0].right_shoulder_action
				                                    ? thismap[0].right_shoulder_action
				                                    : port
				                                    ? INPUTEVENT_JOY2_2ND_BUTTON
				                                    : INPUTEVENT_JOY1_2ND_BUTTON);
		}

		else if (mode == JSEM_MODE_JOYSTICK_CD32) // CD32 joypad, use RWD/FWD
		{
			thismap[0].left_shoulder_action = (thismap[0].left_shoulder_action
				                                   ? thismap[0].left_shoulder_action
				                                   : (port ? INPUTEVENT_JOY2_CD32_RWD : INPUTEVENT_JOY1_CD32_RWD));

			thismap[0].right_shoulder_action = (thismap[0].right_shoulder_action
				                                    ? thismap[0].right_shoulder_action
				                                    : (port ? INPUTEVENT_JOY2_CD32_FFW : INPUTEVENT_JOY1_CD32_FFW));
		}
		else // default, normal joystick
		{
			thismap[0].left_shoulder_action = (thismap[0].left_shoulder_action
				                                   ? thismap[0].left_shoulder_action
				                                   : INPUTEVENT_KEY_SPACE);
			thismap[0].right_shoulder_action = (thismap[0].right_shoulder_action
				                                    ? thismap[0].right_shoulder_action
				                                    : INPUTEVENT_KEY_RETURN);
		}
	}
	else // ports 2, 3 ... parallel ports 
	{
		thismap[0].dpad_up_action = (thismap[0].dpad_up_action
			                             ? thismap[0].dpad_up_action
			                             : (port - 2 ? INPUTEVENT_PAR_JOY2_UP : INPUTEVENT_PAR_JOY1_UP));
		thismap[0].dpad_down_action = (thismap[0].dpad_down_action
			                               ? thismap[0].dpad_down_action
			                               : (port - 2 ? INPUTEVENT_PAR_JOY2_DOWN : INPUTEVENT_PAR_JOY1_DOWN));
		thismap[0].dpad_left_action = (thismap[0].dpad_left_action
			                               ? thismap[0].dpad_left_action
			                               : (port - 2 ? INPUTEVENT_PAR_JOY2_LEFT : INPUTEVENT_PAR_JOY1_LEFT));
		thismap[0].dpad_right_action = (thismap[0].dpad_right_action
			                                ? thismap[0].dpad_right_action
			                                : (port - 2 ? INPUTEVENT_PAR_JOY2_RIGHT : INPUTEVENT_PAR_JOY1_RIGHT));

		thismap[0].south_action = (thismap[0].south_action
			                           ? thismap[0].south_action
			                           : (port - 2 ? INPUTEVENT_PAR_JOY2_FIRE_BUTTON : INPUTEVENT_PAR_JOY1_FIRE_BUTTON));
		thismap[0].east_action = (thismap[0].east_action
			                          ? thismap[0].east_action
			                          : (port - 2 ? INPUTEVENT_PAR_JOY2_2ND_BUTTON : INPUTEVENT_PAR_JOY1_2ND_BUTTON));

		thismap[0].start_action = (thismap[0].start_action ? thismap[0].start_action : INPUTEVENT_KEY_P);
		thismap[0].left_shoulder_action = (thismap[0].left_shoulder_action
			                                   ? thismap[0].left_shoulder_action
			                                   : INPUTEVENT_KEY_SPACE);
		thismap[0].right_shoulder_action = (thismap[0].right_shoulder_action
			                                    ? thismap[0].right_shoulder_action
			                                    : INPUTEVENT_KEY_RETURN);
	}


	thismap[1] = currprefs.jports[port].amiberry_custom_hotkey; // grab the 'select button' options for the current map

	// currently disabled
	//	thismap[2] = currprefs.jports[port].amiberry_custom_left_trigger; // grab the 'left trigger'  options for the current map        
	//	thismap[3] = currprefs.jports[port].amiberry_custom_right_trigger; // grab the 'right trigger' options for the current map

	//  Now assign the actual buttons VALUES (TRUE/FALSE) to trigger the EVENTS
	int function_offset = 0;


	for (int n = 0; n < 2; ++n) /// temporarily limited to '2' only
	{
		function_offset = n * REMAP_BUTTONS;

		// s/e/w/n      
		setid_af(uid, num, ID_BUTTON_OFFSET + 0 + function_offset, 0, port, thismap[n].south_action, af, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 1 + function_offset, 0, port, thismap[n].east_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 2 + function_offset, 0, port, thismap[n].west_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 3 + function_offset, 0, port, thismap[n].north_action, gp);

		// left shoulder / right shoulder / start  
		setid(uid, num, ID_BUTTON_OFFSET + 4 + function_offset, 0, port, thismap[n].left_shoulder_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 5 + function_offset, 0, port, thismap[n].right_shoulder_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 6 + function_offset, 0, port, thismap[n].start_action, gp);

		// directions  
		setid(uid, num, ID_BUTTON_OFFSET + 7 + function_offset, 0, port, thismap[n].dpad_up_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 8 + function_offset, 0, port, thismap[n].dpad_down_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 9 + function_offset, 0, port, thismap[n].dpad_left_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 10 + function_offset, 0, port, thismap[n].dpad_right_action, gp);

		// stick buttons
		setid(uid, num, ID_BUTTON_OFFSET + 11 + function_offset, 0, port, thismap[n].lstick_select_action, gp);
		setid(uid, num, ID_BUTTON_OFFSET + 12 + function_offset, 0, port, thismap[n].rstick_select_action, gp);

		setid(uid, num, ID_BUTTON_OFFSET + 13 + function_offset, 0, port, thismap[n].select_action, gp);
	}

	// if using retroarch options
	if (currprefs.amiberry_use_retroarch_menu == true)
	{
		setid(uid, num, ID_BUTTON_OFFSET + 14, 0, port, INPUTEVENT_SPC_ENTERGUI, gp);
	}
	if (currprefs.amiberry_use_retroarch_quit == true)
	{
		setid(uid, num, ID_BUTTON_OFFSET + 15, 0, port, INPUTEVENT_SPC_QUIT, gp);
	}
	if (currprefs.amiberry_use_retroarch_reset == true)
	{
		setid(uid, num, ID_BUTTON_OFFSET + 30, 0, port, INPUTEVENT_SPC_SOFTRESET, gp);
	}

	if (num >= 0 && num < nr_joysticks)
	{
		return 1;
	}
	return 0;
}

int input_get_default_joystick_analog(struct uae_input_device* uid, int num, int port, int af, bool joymouseswap)
{
	return 0;
}


void SimulateMouseOrJoy(int code, int keypressed)
{
	switch (code)
	{
	case REMAP_MOUSEBUTTON_LEFT:
		mouseBut1viaCustom = keypressed;
		setmousebuttonstate(0, 0, keypressed);
		setmousebuttonstate(1, 0, keypressed);
		break;

	case REMAP_MOUSEBUTTON_RIGHT:
		mouseBut2viaCustom = keypressed;
		setmousebuttonstate(0, 1, keypressed);
		setmousebuttonstate(1, 1, keypressed);
		break;

	case REMAP_JOYBUTTON_ONE:
		joyButXviaCustom[0] = keypressed;
		setjoybuttonstate(0, 0, keypressed);
		break;

	case REMAP_JOYBUTTON_TWO:
		joyButXviaCustom[1] = keypressed;
		setjoybuttonstate(0, 1, keypressed);
		break;

	case REMAP_JOY_UP:
		joyYviaCustom = keypressed;
		setjoystickstate(0, 1, keypressed ? -32767 : 0, 32767);
		break;

	case REMAP_JOY_DOWN:
		joyYviaCustom = keypressed;
		setjoystickstate(0, 1, keypressed ? 32767 : 0, 32767);
		break;

	case REMAP_JOY_LEFT:
		joyXviaCustom = keypressed;
		setjoystickstate(0, 0, keypressed ? -32767 : 0, 32767);
		break;

	case REMAP_JOY_RIGHT:
		joyXviaCustom = keypressed;
		setjoystickstate(0, 0, keypressed ? 32767 : 0, 32767);
		break;

	case REMAP_CD32_GREEN:
		joyButXviaCustom[2] = keypressed;
		setjoybuttonstate(0, 2, keypressed);
		break;

	case REMAP_CD32_YELLOW:
		joyButXviaCustom[3] = keypressed;
		setjoybuttonstate(0, 3, keypressed);
		break;

	case REMAP_CD32_PLAY:
		joyButXviaCustom[6] = keypressed;
		setjoybuttonstate(0, 6, keypressed);
		break;

	case REMAP_CD32_FFW:
		joyButXviaCustom[5] = keypressed;
		setjoybuttonstate(0, 5, keypressed);
		break;

	case REMAP_CD32_RWD:
		joyButXviaCustom[4] = keypressed;
		setjoybuttonstate(0, 4, keypressed);
		break;
	}
}

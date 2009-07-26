/**
* Copyright (c) 2006-2009 LOVE Development Team
* 
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
**/

#include "constants.h"

#include <filesystem/File.h>
#include <event/Event.h>
#include <mouse/Mouse.h>
#include <keyboard/Keyboard.h>
#include <joystick/Joystick.h>
#include <graphics/Graphics.h>
#include <graphics/Image.h>
#include <physics/Shape.h>
#include <physics/Joint.h>

namespace love
{
	// Constants.
	const lua_constant_entry lua_constants[] = 
	{
		{ "key_unknown", 0 },
		{ "key_first", 0 },
		{ "key_backspace", 8 },
		{ "key_tab", 9 },
		{ "key_clear", 12 },
		{ "key_return", 13 },
		{ "key_pause", 19 },
		{ "key_escape", 27 },
		{ "key_space", 32 },
		{ "key_exclaim", 33 },
		{ "key_quotedbl", 34 },
		{ "key_hash", 35 },
		{ "key_dollar", 36 },
		{ "key_ampersand", 38 },
		{ "key_quote", 39 },
		{ "key_leftparen", 40 },
		{ "key_rightparen", 41 },
		{ "key_asterisk", 42 },
		{ "key_plus", 43 },
		{ "key_comma", 44 },
		{ "key_minus", 45 },
		{ "key_period", 46 },
		{ "key_slash", 47 },

		{ "key_0", 48 },
		{ "key_1", 49 },
		{ "key_2", 50 },
		{ "key_3", 51 },
		{ "key_4", 52 },
		{ "key_5", 53 },
		{ "key_6", 54 },
		{ "key_7", 55 },
		{ "key_8", 56 },
		{ "key_9", 57 },

		{ "key_colon", 58 },
		{ "key_semicolon", 59 },
		{ "key_less", 60 },
		{ "key_equals", 61 },
		{ "key_greater", 62 },
		{ "key_question", 63 },
		{ "key_at", 64 },

		{ "key_leftbracket", 91 },
		{ "key_backslash", 92 },
		{ "key_rightbracket", 93 },
		{ "key_caret", 94 },
		{ "key_underscore", 95 },
		{ "key_backquote", 96 },

		{ "key_a", 97 },
		{ "key_b", 98 },
		{ "key_c", 99 },
		{ "key_d", 100 },
		{ "key_e", 101 },
		{ "key_f", 102 },
		{ "key_g", 103 },
		{ "key_h", 104 },
		{ "key_i", 105 },
		{ "key_j", 106 },
		{ "key_k", 107 },
		{ "key_l", 108 },
		{ "key_m", 109 },
		{ "key_n", 110 },
		{ "key_o", 111 },
		{ "key_p", 112 },
		{ "key_q", 113 },
		{ "key_r", 114 },
		{ "key_s", 115 },
		{ "key_t", 116 },
		{ "key_u", 117 },
		{ "key_v", 118 },
		{ "key_w", 119 },
		{ "key_x", 120 },
		{ "key_y", 121 },
		{ "key_z", 122 },
		{ "key_delete", 127 },

		{ "key_kp0", 256 },
		{ "key_kp1", 257 },
		{ "key_kp2", 258 },
		{ "key_kp3", 259 },
		{ "key_kp4", 260 },
		{ "key_kp5", 261 },
		{ "key_kp6", 262 },
		{ "key_kp7", 263 },
		{ "key_kp8", 264 },
		{ "key_kp9", 265 },
		{ "key_kp_period", 266 },
		{ "key_kp_divide", 267 },
		{ "key_kp_multiply", 268 },
		{ "key_kp_minus", 269 },
		{ "key_kp_plus", 270 },
		{ "key_kp_enter", 271 },
		{ "key_kp_equals", 272 },

		{ "key_up", 273 },
		{ "key_down", 274 },
		{ "key_right", 275 },
		{ "key_left", 276 },
		{ "key_insert", 277 },
		{ "key_home", 278 },
		{ "key_end", 279 },
		{ "key_pageup", 280 },
		{ "key_pagedown", 281 },

		{ "key_f1", 282 },
		{ "key_f2", 283 },
		{ "key_f3", 284 },
		{ "key_f4", 285 },
		{ "key_f5", 286 },
		{ "key_f6", 287 },
		{ "key_f7", 288 },
		{ "key_f8", 289 },
		{ "key_f9", 290 },
		{ "key_f10", 291 },
		{ "key_f11", 292 },
		{ "key_f12", 293 },
		{ "key_f13", 294 },
		{ "key_f14", 295 },
		{ "key_f15", 296 },

		{ "key_numlock", 300 },
		{ "key_capslock", 301 },
		{ "key_scrollock", 302 },
		{ "key_rshift", 303 },
		{ "key_lshift", 304 },
		{ "key_rctrl", 305 },
		{ "key_lctrl", 306 },
		{ "key_ralt", 307 },
		{ "key_lalt", 308 },
		{ "key_rmeta", 309 },
		{ "key_lmeta", 310 },
		{ "key_lsuper", 311 },
		{ "key_rsuper", 312 },
		{ "key_mode", 313 },
		{ "key_compose", 314 },

		{ "key_help", 315 },
		{ "key_print", 316 },
		{ "key_sysreq", 317 },
		{ "key_break", 318 },
		{ "key_menu", 319 },
		{ "key_power", 320 },
		{ "key_euro", 321 },
		{ "key_undo", 322 },

		{ "key_repeat_delay", keyboard::Keyboard::KEY_REPEAT_DELAY },
		{ "key_repeat_interval", keyboard::Keyboard::KEY_REPEAT_INTERVAL },

		{ "mouse_left", mouse::Mouse::MOUSE_LEFT },
		{ "mouse_middle", mouse::Mouse::MOUSE_MIDDLE },
		{ "mouse_right", mouse::Mouse::MOUSE_RIGHT },
		{ "mouse_wheelup", mouse::Mouse::MOUSE_WHEELUP },
		{ "mouse_wheeldown", mouse::Mouse::MOUSE_WHEELDOWN },

		{ "align_left", graphics::Graphics::ALIGN_LEFT },
		{ "align_right", graphics::Graphics::ALIGN_RIGHT },
		{ "align_center", graphics::Graphics::ALIGN_CENTER },

		{ "blend_alpha", graphics::Graphics::BLEND_ALPHA },
		{ "blend_additive", graphics::Graphics::BLEND_ADDITIVE },
		{ "color_replace", graphics::Graphics::COLOR_REPLACE },
		{ "color_modulate", graphics::Graphics::COLOR_MODULATE },

		{ "file_closed", filesystem::File::CLOSED },
		{ "file_read", filesystem::File::READ },
		{ "file_write", filesystem::File::WRITE },
		{ "file_append", filesystem::File::APPEND },

		{ "draw_line", graphics::Graphics::DRAW_LINE },
		{ "draw_fill", graphics::Graphics::DRAW_FILL },

		{ "line_smooth", graphics::Graphics::LINE_SMOOTH },
		{ "line_rough", graphics::Graphics::LINE_ROUGH },

		{ "point_smooth", graphics::Graphics::POINT_SMOOTH },
		{ "point_rough", graphics::Graphics::POINT_ROUGH },

		{ "shape_circle", physics::Shape::SHAPE_CIRCLE },
		{ "shape_polygon", physics::Shape::SHAPE_POLYGON },

		{ "joint_distance", physics::Joint::JOINT_DISTANCE },
		{ "joint_revolute", physics::Joint::JOINT_REVOLUTE },
		{ "joint_prismatic", physics::Joint::JOINT_PRISMATIC },
		{ "joint_mouse", physics::Joint::JOINT_MOUSE },	
		{ "joint_pulley", physics::Joint::JOINT_PULLEY },	
		{ "joint_gear", physics::Joint::JOINT_GEAR },

		{ "joystick_axis_horizontal", joystick::Joystick::JOYSTICK_AXIS_HORIZONTAL },
		{ "joystick_axis_vertical", joystick::Joystick::JOYSTICK_AXIS_VERITCAL },

		{ "joystick_hat_centered", joystick::Joystick::JOYSTICK_HAT_CENTERED },
		{ "joystick_hat_up", joystick::Joystick::JOYSTICK_HAT_UP },
		{ "joystick_hat_right", joystick::Joystick::JOYSTICK_HAT_RIGHT },
		{ "joystick_hat_down", joystick::Joystick::JOYSTICK_HAT_DOWN },
		{ "joystick_hat_left", joystick::Joystick::JOYSTICK_HAT_LEFT },
		{ "joystick_hat_rightup", joystick::Joystick::JOYSTICK_HAT_RIGHTUP },
		{ "joystick_hat_rightdown", joystick::Joystick::JOYSTICK_HAT_RIGHTDOWN },
		{ "joystick_hat_leftup", joystick::Joystick::JOYSTICK_HAT_LEFTUP },
		{ "joystick_hat_leftdown", joystick::Joystick::JOYSTICK_HAT_LEFTDOWN },

		{ "event_keypressed", event::Event::EVENT_KEYDOWN },
		{ "event_keyreleased", event::Event::EVENT_KEYUP },
		{ "event_mousepressed", event::Event::EVENT_MOUSEBUTTONDOWN },
		{ "event_mousereleased", event::Event::EVENT_MOUSEBUTTONUP },
		{ "event_joystickpressed", event::Event::EVENT_JOYBUTTONDOWN },
		{ "event_joystickreleased", event::Event::EVENT_JOYBUTTONUP },
		{ "event_quit", event::Event::EVENT_QUIT },

		{ "filter_linear", graphics::Image::FILTER_LINEAR },
		{ "filter_nearest", graphics::Image::FILTER_NEAREST },

		{ "wrap_clamp", graphics::Image::WRAP_CLAMP },
		{ "wrap_repeat", graphics::Image::WRAP_REPEAT },

		/**

		// Vertex buffer geometry types.

		{ "type_points", TYPE_POINTS },
		{ "type_lines", TYPE_LINES },
		{ "type_line_strip", TYPE_LINE_STRIP },
		{ "type_triangles", TYPE_TRIANGLES },
		{ "type_triangle_strip", TYPE_TRIANGLE_STRIP },
		{ "type_triangle_fan", TYPE_TRIANGLE_FAN },
		{ "type_num", TYPE_NUM },
		
		// Vertex buffer usage hints.

		{ "usage_array", USAGE_ARRAY },
		{ "usage_dynamic", USAGE_DYNAMIC },
		{ "usage_static", USAGE_STATIC },
		{ "usage_stream", USAGE_STREAM },
		{ "usage_num", USAGE_NUM },
		**/

		{0, 0}, // Indicates the end

	}; // lua constants

} // love

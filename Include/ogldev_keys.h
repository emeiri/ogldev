/*

	Copyright 2014 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OGLDEV_KEYS_H__
#define OGLDEV_KEYS_H__

enum OGLDEV_KEY
{
    OGLDEV_KEY_SPACE =             32,
    OGLDEV_KEY_APOSTROPHE =        39,
    OGLDEV_KEY_PLUS =              43,
    OGLDEV_KEY_COMMA =             44,
    OGLDEV_KEY_MINUS =             45,
    OGLDEV_KEY_PERIOD =            46,
    OGLDEV_KEY_SLASH =             47,
    OGLDEV_KEY_0 =                 48,
    OGLDEV_KEY_1 =                 49,
    OGLDEV_KEY_2 =                 50,
    OGLDEV_KEY_3 =                 51,
    OGLDEV_KEY_4 =                 52,
    OGLDEV_KEY_5 =                 53,
    OGLDEV_KEY_6 =                 54,
    OGLDEV_KEY_7 =                 55,
    OGLDEV_KEY_8 =                 56,
    OGLDEV_KEY_9 =                 57,
    OGLDEV_KEY_SEMICOLON =         58,
    OGLDEV_KEY_EQUAL =             61,
    OGLDEV_KEY_A =                 65,
    OGLDEV_KEY_B =                 66,
    OGLDEV_KEY_C =                 67,
    OGLDEV_KEY_D =                 68,
    OGLDEV_KEY_E =                 69,
    OGLDEV_KEY_F =                 70,
    OGLDEV_KEY_G =                 71,
    OGLDEV_KEY_H =                 72,
    OGLDEV_KEY_I =                 73,
    OGLDEV_KEY_J =                 74,
    OGLDEV_KEY_K =                 75,
    OGLDEV_KEY_L =                 76,
    OGLDEV_KEY_M =                 77,
    OGLDEV_KEY_N =                 78,
    OGLDEV_KEY_O =                 79,
    OGLDEV_KEY_P =                 80,
    OGLDEV_KEY_Q =                 81,
    OGLDEV_KEY_R =                 82,
    OGLDEV_KEY_S =                 83,
    OGLDEV_KEY_T =                 84,
    OGLDEV_KEY_U =                 85,
    OGLDEV_KEY_V =                 86,
    OGLDEV_KEY_W =                 87,
    OGLDEV_KEY_X =                 88,
    OGLDEV_KEY_Y =                 89,
    OGLDEV_KEY_Z =                 90,
    OGLDEV_KEY_LEFT_BRACKET =      91,
    OGLDEV_KEY_BACKSLASH =         92,
    OGLDEV_KEY_RIGHT_BRACKET =     93,
    OGLDEV_KEY_a =                 97,
    OGLDEV_KEY_b =                 98,
    OGLDEV_KEY_c =                 99,
    OGLDEV_KEY_d =                 100,
    OGLDEV_KEY_e =                 101,
    OGLDEV_KEY_f =                 102,
    OGLDEV_KEY_g =                 103,
    OGLDEV_KEY_h =                 104,
    OGLDEV_KEY_i =                 105,
    OGLDEV_KEY_j =                 106,
    OGLDEV_KEY_k =                 107,
    OGLDEV_KEY_l =                 108,
    OGLDEV_KEY_m =                 109,
    OGLDEV_KEY_n =                 110,
    OGLDEV_KEY_o =                 111,
    OGLDEV_KEY_p =                 112,
    OGLDEV_KEY_q =                 113,
    OGLDEV_KEY_r =                 114,
    OGLDEV_KEY_s =                 115,
    OGLDEV_KEY_t =                 116,
    OGLDEV_KEY_u =                 117,
    OGLDEV_KEY_v =                 118,
    OGLDEV_KEY_w =                 119,
    OGLDEV_KEY_x =                 120,
    OGLDEV_KEY_y =                 121,
    OGLDEV_KEY_z =                 122,
    OGLDEV_KEY_ESCAPE,
    OGLDEV_KEY_ENTER,          
    OGLDEV_KEY_TAB,            
    OGLDEV_KEY_BACKSPACE,      
    OGLDEV_KEY_INSERT,         
    OGLDEV_KEY_DELETE,            
    OGLDEV_KEY_RIGHT,             
    OGLDEV_KEY_LEFT,               
    OGLDEV_KEY_DOWN,               
    OGLDEV_KEY_UP,         
    OGLDEV_KEY_PAGE_UP,   
    OGLDEV_KEY_PAGE_DOWN,      
    OGLDEV_KEY_HOME,    
    OGLDEV_KEY_END,     
    OGLDEV_KEY_F1,        
    OGLDEV_KEY_F2,          
    OGLDEV_KEY_F3,       
    OGLDEV_KEY_F4,   
    OGLDEV_KEY_F5,      
    OGLDEV_KEY_F6,     
    OGLDEV_KEY_F7,     
    OGLDEV_KEY_F8,     
    OGLDEV_KEY_F9,    
    OGLDEV_KEY_F10,    
    OGLDEV_KEY_F11,    
    OGLDEV_KEY_F12,
    OGLDEV_KEY_UNDEFINED = 999,
};

enum OGLDEV_MOUSE {
    OGLDEV_MOUSE_BUTTON_LEFT,
    OGLDEV_MOUSE_BUTTON_MIDDLE,
    OGLDEV_MOUSE_BUTTON_RIGHT,
    OGLDEV_MOUSE_UNDEFINED = 999
};

enum OGLDEV_KEY_STATE {
    OGLDEV_KEY_STATE_PRESS,
    OGLDEV_KEY_STATE_RELEASE
};

#endif
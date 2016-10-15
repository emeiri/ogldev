/*
        Copyright 2015 Etay Meiri

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

#include "ogldev_atb.h"
#include "ogldev_lights_common.h"

TwType TW_TYPE_OGLDEV_VECTOR3F;
TwType TW_TYPE_OGLDEV_ATTENUATION;



ATB::ATB()
{
    
}


bool ATB::Init()
{
    bool ret = false;
    
    if (TwInit(TW_OPENGL_CORE, NULL) == 1)
    {
        TwStructMember Vector3fMembers[] = {
            { "x", TW_TYPE_FLOAT, offsetof(Vector3f, x), "" },
            { "y", TW_TYPE_FLOAT, offsetof(Vector3f, y), "" },
            { "z", TW_TYPE_FLOAT, offsetof(Vector3f, z), "" }
        };
        
        TW_TYPE_OGLDEV_VECTOR3F = TwDefineStruct("Vector3f", Vector3fMembers, 3, sizeof(Vector3f), NULL, NULL);

        TwStructMember AttenuationMembers[] = {
            { "Const", TW_TYPE_FLOAT, offsetof(LightAttenuation, Constant), "" },
            { "Linear", TW_TYPE_FLOAT, offsetof(LightAttenuation, Linear), "" },
            { "Exp", TW_TYPE_FLOAT, offsetof(LightAttenuation, Exp), "" }
        };
        
        TW_TYPE_OGLDEV_ATTENUATION = TwDefineStruct("Attenuation", AttenuationMembers, 3, sizeof(LightAttenuation), NULL, NULL);
        
        ret = true;
    }
    
    return ret;
}


static int OgldevKeyToATBKey(OGLDEV_KEY OgldevKey)
{
    if (OgldevKey >= OGLDEV_KEY_SPACE && OgldevKey <= OGLDEV_KEY_RIGHT_BRACKET) {
        return OgldevKey;
    }

    switch(OgldevKey) {

        case OGLDEV_KEY_BACKSPACE:
            return TW_KEY_BACKSPACE;
        case OGLDEV_KEY_TAB:
            return TW_KEY_TAB;
//            return TW_KEY_CLEAR;
        case OGLDEV_KEY_ENTER:
            return TW_KEY_RETURN;
            
            //return TW_KEY_PAUSE;
        case OGLDEV_KEY_ESCAPE:
            return TW_KEY_ESCAPE;
        case OGLDEV_KEY_DELETE:
            return TW_KEY_DELETE;
        case OGLDEV_KEY_UP:
            return TW_KEY_UP;
        case OGLDEV_KEY_DOWN:
            return TW_KEY_DOWN;
        case OGLDEV_KEY_RIGHT:
            return TW_KEY_RIGHT;
        case OGLDEV_KEY_LEFT:
            return TW_KEY_LEFT;
        case OGLDEV_KEY_INSERT:
            return TW_KEY_INSERT;
        case OGLDEV_KEY_HOME:
            return TW_KEY_HOME;
        case OGLDEV_KEY_END:
            return TW_KEY_END;
        case OGLDEV_KEY_PAGE_UP:
            return TW_KEY_PAGE_UP;
        case OGLDEV_KEY_PAGE_DOWN:
            return TW_KEY_PAGE_DOWN;
        case OGLDEV_KEY_F1:
            return TW_KEY_F1;
        case OGLDEV_KEY_F2:            
            return TW_KEY_F2;
        case OGLDEV_KEY_F3:
            return TW_KEY_F3;
        case OGLDEV_KEY_F4:
            return TW_KEY_F4;
        case OGLDEV_KEY_F5:
            return TW_KEY_F5;
        case OGLDEV_KEY_F6:
            return TW_KEY_F6;
        case OGLDEV_KEY_F7:
            return TW_KEY_F7;
        case OGLDEV_KEY_F8:
            return TW_KEY_F8;
        case OGLDEV_KEY_F9:
            return TW_KEY_F9;
        case OGLDEV_KEY_F10:
            return TW_KEY_F10;
        case OGLDEV_KEY_F11:
            return TW_KEY_F11;
        case OGLDEV_KEY_F12:
            return TW_KEY_F12;
        default:
            OGLDEV_ERROR0("Unimplemented OGLDEV to ATB key");
    }
    
    return TW_KEY_LAST;
}

bool ATB::KeyboardCB(OGLDEV_KEY OgldevKey)
{
    int ATBKey = OgldevKeyToATBKey(OgldevKey);
    
    if (ATBKey == TW_KEY_LAST) {
        return false;
    }
    
    return (TwKeyPressed(ATBKey, TW_KMOD_NONE) == 1);
}


bool ATB::PassiveMouseCB(int x, int y)
{
    return (TwMouseMotion(x, y) == 1);
}


bool ATB::MouseCB(OGLDEV_MOUSE Button, OGLDEV_KEY_STATE State, int x, int y)
{    
    TwMouseButtonID btn = (Button == OGLDEV_MOUSE_BUTTON_LEFT) ? TW_MOUSE_LEFT : TW_MOUSE_RIGHT;
    TwMouseAction ma = (State == OGLDEV_KEY_STATE_PRESS) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
    
    return (TwMouseButton(ma, btn) == 1);
}


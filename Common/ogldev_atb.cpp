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

bool ATB::KeyboardCB(OGLDEV_KEY OgldevKey)
{
    return (TwKeyPressed(OgldevKey, TW_KMOD_NONE) == 1);
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


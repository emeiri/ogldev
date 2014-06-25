/*

	Copyright 2011 Etay Meiri

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

#ifndef CALLBACKS_H
#define	CALLBACKS_H

class ICallbacks
{
public:

    virtual void SpecialKeyboardCB(int Key, int x, int y) {};

    virtual void KeyboardCB(unsigned char Key, int x, int y) {};

    virtual void PassiveMouseCB(int x, int y) {};

    virtual void RenderSceneCB() {};

    virtual void IdleCB() {};
    
    virtual void MouseCB(int Button, int State, int x, int y) {};
};

#endif	/* I3DAPPLICATION_H */


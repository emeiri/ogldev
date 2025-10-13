/*

        Copyright 2025 Etay Meiri

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


#pragma once

#include <vector>
#include "point_mass.h"

namespace Physics {

class System {

public:

    System() {}

    ~System() {}

    void Init(int NumPointMasses, UpdateListener pUpdateListener);

    void SetListener(UpdateListener pUpdateListener) { m_pUpdateListener = pUpdateListener; }

    void Update(int DeltaTimeMillis);

    PointMass* AllocPointMass();

private:

    std::vector<PointMass> m_pointMasses;
    UpdateListener m_pUpdateListener = NULL;
    int m_numPointMasses = 0;
};

}
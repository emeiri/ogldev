/*

        Copyright 2023 Etay Meiri

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

#include "demolition_scene.h"


void Scene::AddObject(DemolitionMesh* pObject)
{
    std::list<DemolitionMesh*>::const_iterator it = std::find(m_objects.begin(), m_objects.end(), pObject);

    if (it == m_objects.end()) {
        m_objects.push_back(pObject);
    }
}


bool Scene::RemoveObject(DemolitionMesh* pObject)
{
    std::list<DemolitionMesh*>::const_iterator it = std::find(m_objects.begin(), m_objects.end(), pObject);

    bool ret = false;

    if (it != m_objects.end()) {
        m_objects.erase(it);
        ret = true;
    }

    return ret;
}

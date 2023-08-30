/*

        Copyright 2022 Etay Meiri

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

#include <list>

#include "demolition_forward_lighting.h"
#include "demolition_model.h"


class BaseRenderingSubsystem;

class Scene {
public:
    Scene(BaseRenderingSubsystem* pRenderingSystem) { m_pRenderingSystem = pRenderingSystem; }

    virtual ~Scene() {}

    void SetMainModel(int ModelHandle);
    void AddObject(int ModelHandle);
    bool RemoveObject(int ModelHandle);
    DemolitionModel* GetMainModel() const { return m_pMainModel; }

    void SetClearColor(const Vector4f& Color) { m_clearColor = Color; m_clearFrame = true; }
    void DisableClear() { m_clearFrame = false;  }
    bool IsClearFrame() const { return m_clearFrame; }
    const Vector4f& GetClearColor() { return m_clearColor;  }  

    void SetRotation(int ModelHandle, float x, float y, float z);
    void SetPosition(int ModelHandle, float x, float y, float z);

    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<DirectionalLight> m_dirLights;

protected:
    BaseRenderingSubsystem* m_pRenderingSystem = NULL;
    DemolitionModel* m_pMainModel = NULL;
    std::list<DemolitionModel*> m_objects;
    bool m_clearFrame = false;
    Vector4f m_clearColor;
};

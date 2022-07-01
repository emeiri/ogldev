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

#ifndef OGLDEV_BASIC_GLFW_CAMERA_H
#define OGLDEV_BASIC_GLFW_CAMERA_H

#include "ogldev_math_3d.h"

class BasicCamera
{
public:

    BasicCamera(int WindowWidth, int WindowHeight);

    BasicCamera(const PersProjInfo& persProjInfo, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up);

    void SetPosition(float x, float y, float z);

    void SetPosition(const Vector3f& pos);

    void SetTarget(float x, float y, float z);

    void SetTarget(const Vector3f& target);

    void OnKeyboard(int key);

    void OnMouse(int x, int y);

    void OnRender();

    Matrix4f GetMatrix() const;

    const Vector3f& GetPos() const { return m_pos; }

    const Vector3f& GetTarget() const { return m_target; }

    const Vector3f& GetUp() const { return m_up; }

    const Matrix4f& GetProjectionMat() const { return ProjectionMat; }

    PersProjInfo m_persProjInfo;

private:

    void Init();
    void Update();

    Vector3f m_pos;
    Vector3f m_target;
    Vector3f m_up;

    float m_speed = 0.1f;
    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    Vector2i m_mousePos;

    Matrix4f ProjectionMat;
};

#endif

/*

        Copyright 2021 Etay Meiri

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

#include <GL/freeglut.h>

#include "camera.h"

const static float STEP_SCALE = 1.0f;
const static float EDGE_STEP = 0.5f;
const static int MARGIN = 10;

Camera::Camera()
{
    m_pos          = Vector3f(0.0f, 0.0f, 0.0f);
    m_target       = Vector3f(0.0f, 0.0f, 1.0f);
    m_up           = Vector3f(0.0f, 1.0f, 0.0f);
}


Camera::Camera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
{
    m_pos = Pos;
    m_target = Target;
    m_target.Normalize();

    m_up = Up;
    m_up.Normalize();
}


void Camera::SetPosition(float x, float y, float z)
{
    m_pos.x = x;
    m_pos.y = y;
    m_pos.z = z;
}


bool Camera::OnKeyboard(unsigned char Key)
{
    bool Ret = false;

    switch (Key) {

    case GLUT_KEY_UP:
        {
            m_pos += (m_target * STEP_SCALE);
            Ret = true;
        }
        break;

    case GLUT_KEY_DOWN:
        {
            m_pos -= (m_target * STEP_SCALE);
            Ret = true;
        }
        break;

    case GLUT_KEY_LEFT:
        {
            Vector3f Left = m_target.Cross(m_up);
            Left.Normalize();
            Left *= STEP_SCALE;
            m_pos += Left;
            Ret = true;
        }
        break;

    case GLUT_KEY_RIGHT:
        {
            Vector3f Right = m_up.Cross(m_target);
            Right.Normalize();
            Right *= STEP_SCALE;
            m_pos += Right;
            Ret = true;
        }
        break;

    case GLUT_KEY_PAGE_UP:
        m_pos.y += STEP_SCALE;
        break;

    case GLUT_KEY_PAGE_DOWN:
        m_pos.y -= STEP_SCALE;
        break;

    default:
        break;
    }

    return Ret;
}


Matrix4f Camera::GetMatrix()
{
    Matrix4f Translation;
    Translation.InitTranslationTransform(-m_pos.x, -m_pos.y, -m_pos.z);

    Matrix4f Rotation;
    Rotation.InitCameraTransform(m_target, m_up);

    Matrix4f CameraTransformation = Rotation * Translation;

    return CameraTransformation;
}

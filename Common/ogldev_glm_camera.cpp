/*

		Copyright 2024 Etay Meiri

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

	This camera implementation is based on the "3D Graphics Rendering Cookbook"
*/

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "ogldev_glm_camera.h"

GLMCameraFirstPerson::GLMCameraFirstPerson(const glm::vec3& Pos, const glm::vec3& Target,
										   const glm::vec3& Up, PersProjInfo& persProjInfo)
{
	Init(Pos, Target, Up, persProjInfo);
}

void GLMCameraFirstPerson::Init(const glm::vec3& Pos, const glm::vec3& Target)
{
	if (!m_initialized) {
        printf("Error: GLMCameraFirstPerson::Init(Pos, Target) called before full initialization. Call the other Init() method first.\n");
		assert(0);
		exit(1);
	}

    Init(Pos, Target, m_up, m_persProjInfo);
}


void GLMCameraFirstPerson::Init(const glm::vec3& Pos, const glm::vec3& Target,
								const glm::vec3& Up, PersProjInfo& persProjInfo)
{
	m_cameraPos = Pos;
	m_up = glm::normalize(Up);
	m_persProjInfo = persProjInfo;
	m_oldMousePos = m_mouseState.m_pos;

	glm::vec3 senseDir = (glm::abs(m_up.y) > 0.9f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

	if (CAMERA_LEFT_HANDED) {
		// LH: Cross(Up, Forward-Sense)
		m_worldRight = glm::normalize(glm::cross(m_up, senseDir));
	} else {
		// RH: Cross(Forward-Sense, Up)
		m_worldRight = glm::normalize(glm::cross(senseDir, m_up));
	}

	// 1. Calculate Initial Yaw and Pitch from Target direction
	glm::vec3 Dir = glm::normalize(Target);
	m_pitch = glm::asin(Dir.y);

	if (CAMERA_LEFT_HANDED) {
		m_yaw = glm::atan2(Dir.x, Dir.z);
	} else {
		m_yaw = glm::atan2(Dir.x, -Dir.z);
	}

//	printf("Yaw: %f, Pitch: %f\n", m_yaw, m_pitch);

	// 2. Build the initial m_cameraOrientation quaternion
	CalcCameraOrientation();

	// 3. Set up Projection Matrix	
	float ar = (float)persProjInfo.Width / (float)persProjInfo.Height;

	if (CAMERA_LEFT_HANDED) {
#ifdef OGLDEV_VULKAN
		m_persProjection = glm::perspectiveLH_ZO(glm::radians(persProjInfo.FOV), ar,
			persProjInfo.zNear, persProjInfo.zFar);
#else
		m_persProjection = glm::perspectiveLH(glm::radians(persProjInfo.FOV), ar,
			persProjInfo.zNear, persProjInfo.zFar);
#endif
	} else {
#ifdef OGLDEV_VULKAN
		m_persProjection = glm::perspectiveRH_ZO(glm::radians(persProjInfo.FOV), ar,
			persProjInfo.zNear, persProjInfo.zFar);
#else
		m_persProjection = glm::perspectiveRH(glm::radians(persProjInfo.FOV), ar,
			persProjInfo.zNear, persProjInfo.zFar);
#endif
	}

#ifdef OGLDEV_VULKAN
	//m_persProjection[1][1] *= -1; // Flip the Y-axis for Vulkan - currently disabled because it flips the entire world
#endif

    m_initialized = true;
}


void GLMCameraFirstPerson::Update(float dt)
{
	if (m_mouseState.m_buttonPressed) {
		CalcCameraOrientation();
	}

	m_oldMousePos = m_mouseState.m_pos;

	CalcVelocity(dt);

	m_cameraPos += m_velocity * dt;
}


void GLMCameraFirstPerson::SetMousePos(float x, float y)
{
	m_mouseState.m_pos.x = x / (float)m_persProjInfo.Width;
	m_mouseState.m_pos.y = y / (float)m_persProjInfo.Height;

    //printf("MousePos: %f, %f\n", m_mouseState.m_pos.x, m_mouseState.m_pos.y);
}


void GLMCameraFirstPerson::HandleMouseButton(int Button, int Action, int Mods)
{
	if (Button == GLFW_MOUSE_BUTTON_LEFT) {
		if (Action == GLFW_PRESS) {
			m_mouseState.m_buttonPressed = true;
			m_isFirstClick = true;
		} else if (Action == GLFW_RELEASE) {
			m_mouseState.m_buttonPressed = false;			
		}
	}
}


void GLMCameraFirstPerson::CalcCameraOrientation()
{
	if (m_isFirstClick) {
		m_oldMousePos = m_mouseState.m_pos;
		m_isFirstClick = false;
	}

	glm::vec2 DeltaMouse = m_mouseState.m_pos - m_oldMousePos;

//    printf("DeltaMouse: %f, %f\n", DeltaMouse.x, DeltaMouse.y);

	// 1. Update angles dynamically based on Handedness
	if (CAMERA_LEFT_HANDED) {
		// In Left-Handed space, dragging right should decrease the yaw angle
		m_yaw -= DeltaMouse.x * m_mouseSpeed;
	} else {
		// In Right-Handed space, dragging right should increase the yaw angle
		m_yaw += DeltaMouse.x * m_mouseSpeed;
	}

	m_pitch += DeltaMouse.y * m_mouseSpeed;

//    printf("Yaw: %f, Pitch: %f\n", m_yaw, m_pitch);

	// 2. Clamp Pitch to 89 degrees (avoids the "sticky" apex lock)
	if (m_pitch > 1.5f) m_pitch = 1.5f;
	if (m_pitch < -1.5f) m_pitch = -1.5f;

	glm::quat qYaw;
	glm::quat qPitch;

	// 3. Calculate the direction vector from angles
	if (CAMERA_LEFT_HANDED) {
		// Left-handed: Invert yaw angle application to match standard LH space
		qYaw = glm::angleAxis(-m_yaw, m_up);
		qPitch = glm::angleAxis(m_pitch, m_worldRight);
	} else {
		// Right-handed: Standard Euler tracking
		qYaw = glm::angleAxis(m_yaw, m_up);
		qPitch = glm::angleAxis(m_pitch, m_worldRight);
	}

//	GLM_PRINT_QUAT("Yaw Quaternion: ", qYaw);
//	GLM_PRINT_QUAT("Pitch Quaternion: ", qPitch);

//	GLM_PRINT_QUAT("Camera Orientation before: ", m_cameraOrientation);

	// 4. Combine - Yaw first, then Pitch
	m_cameraOrientation = qPitch * qYaw;
	
	// Important: Normalize to prevent float drift
	m_cameraOrientation = glm::normalize(m_cameraOrientation);

 //   GLM_PRINT_QUAT("Camera Orientation after: ", m_cameraOrientation);
}


void GLMCameraFirstPerson::CalcVelocity(float dt)
{
	glm::vec3 Acceleration = CalcAcceleration();

	if (Acceleration == glm::vec3(0.0f)) {
		m_velocity -= m_velocity * std::min(dt * m_damping, 1.0f);
	} else {
		m_velocity += Acceleration * m_acceleration * dt;
		float MaxSpeed = m_movement.FastSpeed ? m_maxSpeed * m_fastCoef : m_maxSpeed;
		//printf("MaxSpeed %f\n", MaxSpeed);
		if (glm::length(m_velocity) > MaxSpeed) {
			m_velocity = glm::normalize(m_velocity) * MaxSpeed;
		}

	//	GLM_PRINT_VEC3("Velocity: ", m_velocity);
	}
}


glm::vec3 GLMCameraFirstPerson::CalcAcceleration()
{
	glm::mat4 v = glm::mat4_cast(m_cameraOrientation);

	glm::vec3 Forward, Up;

	glm::vec3 Right = glm::vec3(v[0][0], v[1][0], v[2][0]);

	if (CAMERA_LEFT_HANDED) {
		Forward = glm::vec3(v[0][2], v[1][2], v[2][2]) * 0.1f;
		Up = glm::cross(Forward, Right);
	} else {
		Forward = -glm::vec3(v[0][2], v[1][2], v[2][2]) * 0.1f;
		Up = glm::cross(Right, Forward);
	}
	
	glm::vec3 Acceleration = glm::vec3(0.0f);

	if (m_movement.Forward) { 
		if (m_movement.Ctrl) {
			Forward.y = 0;
		}
		Acceleration += Forward; 
	}

	if (m_movement.Backward) { 
		if (m_movement.Ctrl) {
			Forward.y = 0;
		}
		Acceleration += -Forward; 
	}

	if (m_movement.StrafeLeft) { 
		Acceleration += -Right; 
	}

	if (m_movement.StrafeRight) { 
		Acceleration += Right; 
	}

	if (m_movement.Up) { 
		Acceleration += Up;
	}

	if (m_movement.Down) { 
		Acceleration -= Up;
	}

	if (m_movement.FastSpeed) { 
		Acceleration *= m_fastCoef; 
	}

	if (m_movement.Plus) {
		m_maxSpeed += 0.1f;
		m_acceleration += 0.01f;
	}

	if (m_movement.Minus) {
		m_maxSpeed -= 0.1f;

		if (m_maxSpeed <= 0.1f) {
			m_maxSpeed = 0.1f;
		}

		m_acceleration -= 0.01f;

		if (m_acceleration <= 10.0f) {
			m_acceleration = 10.0f;
		}
	}

	return Acceleration;
}


glm::mat4 GLMCameraFirstPerson::GetViewMatrix() const
{
	glm::mat4 t = glm::translate(glm::mat4(1.0), -m_cameraPos);

	glm::mat4 r = glm::mat4_cast(m_cameraOrientation);

	glm::mat4 res = r * t;

	return res;
}


glm::mat4 GLMCameraFirstPerson::GetVPMatrix() const
{
	glm::mat4 View = GetViewMatrix();

	glm::mat4 VP = m_persProjection * View;

	//std::cout << glm::to_string(m_persProjection) << std::endl;

	return VP;
}


glm::mat4 GLMCameraFirstPerson::GetVPMatrixNoTranslate() const
{
	glm::mat4 View = glm::mat4_cast(m_cameraOrientation);

	glm::mat4 VP = m_persProjection * View;

	return VP;
}


void GLMCameraFirstPerson::SetTarget(const glm::vec3& Target)
{
    Init(m_cameraPos, Target, m_up, m_persProjInfo);
}


void GLMCameraFirstPerson::SetCenter(const glm::vec3& Center)
{
    glm::vec3 Target = Center - m_cameraPos;
    Init(m_cameraPos, Target, m_up, m_persProjInfo);
}


glm::vec3 GLMCameraFirstPerson::GetTarget() const
{
	glm::mat4 View = GetViewMatrix();

	glm::vec3 Forward = glm::vec3(View[0][2], View[1][2], View[2][2]);

	return Forward;
}


void GLMCameraFirstPerson::Print() const
{
	glm::vec3 Target = GetTarget();

	printf("Camera: pos (%f,%f,%f) target (%f,%f,%f)\n", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z, Target.x, Target.y, Target.z);
}


const Vector3f GLMCameraFirstPerson::GetPos() const
{
	Vector3f Ret = m_cameraPos;
	return Ret;
}

Matrix4f GLMCameraFirstPerson::GetViewportMatrix() const
{
	Matrix4f Ret(GetViewMatrix());
	return Ret;
}

Matrix4f GLMCameraFirstPerson::GetMatrix() const
{
	Matrix4f Ret(GetVPMatrix());
	return Ret;
}

const Matrix4f GLMCameraFirstPerson::GetProjectionMat() const
{
	Matrix4f Ret(GetProjMatrixGLM());
	return Ret;
}

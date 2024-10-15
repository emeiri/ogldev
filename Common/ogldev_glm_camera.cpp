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
#include <iostream>
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "ogldev_glm_camera.h"

GLMCameraFirstPerson::GLMCameraFirstPerson(const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up, PersProjInfo& persProjInfo)
{
	m_cameraPos = Pos;
	
	float ar = (float)persProjInfo.Width / (float)persProjInfo.Height;

	if (CAMERA_LEFT_HANDED) {
		m_cameraOrientation = glm::lookAtLH(Pos, Target, Up);
		m_persProjection = glm::perspectiveLH(persProjInfo.FOV, ar, persProjInfo.zNear, persProjInfo.zFar);
	} else {
		m_cameraOrientation = glm::lookAtRH(Pos, Target, Up);
		m_persProjection = glm::perspectiveRH(persProjInfo.FOV, ar, persProjInfo.zNear, persProjInfo.zFar);
	}	
}


void GLMCameraFirstPerson::Update(float dt)
{
	CalcVelocity(dt);

	m_cameraPos += m_velocity * dt;
}


void GLMCameraFirstPerson::CalcVelocity(float dt)
{
	glm::vec3 Acceleration = CalcAcceleration();

	if (Acceleration == glm::vec3(0.0f)) {
		m_velocity -= m_velocity * std::min(dt * 1.0f / m_damping, 1.0f);
	} else {
		m_velocity += Acceleration * m_acceleration * dt;
		float MaxSpeed = m_movement.FastSpeed ? m_maxSpeed * m_fastCoef : m_maxSpeed;

		if (glm::length(m_velocity) > m_maxSpeed) {
			m_velocity = glm::normalize(m_velocity) * m_maxSpeed;
		}
	}
}


glm::vec3 GLMCameraFirstPerson::CalcAcceleration()
{
	glm::mat4 v = glm::mat4_cast(m_cameraOrientation);

	glm::vec3 Forward, Up;

	glm::vec3 Right = glm::vec3(v[0][0], v[1][0], v[2][0]);

	if (CAMERA_LEFT_HANDED) {
		Forward = glm::vec3(v[0][2], v[1][2], v[2][2]);
		Up = glm::cross(Forward, Right);
	} else {
		Forward = -glm::vec3(v[0][2], v[1][2], v[2][2]);
		Up = glm::cross(Right, Forward);
	}
	
	glm::vec3 Acceleration = glm::vec3(0.0f);

	//printf("2 %d\n", m_movement.Forward);

	if (m_movement.Forward) { 
		Acceleration = Forward; 
		//std::cout << glm::to_string(Acceleration) << std::endl;
	}

	if (m_movement.Backward) { 
		Acceleration = -Forward; 
	}

	if (m_movement.Left) { 
		Acceleration = -Right; 
	}

	if (m_movement.Right) { 
		Acceleration = Right; 
	}

	if (m_movement.Up) { 
		Acceleration = Up; 
	}

	if (m_movement.Down) { 
		Acceleration = -Up; 
	}

	if (m_movement.FastSpeed) { 
		Acceleration *= m_fastCoef; 
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

	return VP;
}


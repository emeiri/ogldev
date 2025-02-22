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

#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ogldev_math_3d.h"
#include "ogldev_glfw_camera_handler.h"
#include "ogldev_camera_api.h"

static bool constexpr CAMERA_LEFT_HANDED = true;

struct MouseState {
	glm::vec2 m_pos = glm::vec2(0.0f);
	bool m_buttonPressed = false;
};

class GLMCameraFirstPerson : public CameraAPI {

public:

	CameraMovement m_movement;
	float m_acceleration = 150.0f;
	float m_damping = 5.0f;
	float m_maxSpeed = 10.0f;
	float m_fastCoef = 10.0f;
	float m_mouseSpeed = 4.0f;	

	GLMCameraFirstPerson() {}

	GLMCameraFirstPerson(const glm::vec3& Pos, const glm::vec3& Target,
		                 const glm::vec3& Up, PersProjInfo& persProjInfo);

	void Init(const glm::vec3& Pos, const glm::vec3& Target,
			  const glm::vec3& Up, PersProjInfo& persProjInfo);

	void Update(float dt);

	void SetMousePos(float x, float y);

	void HandleMouseButton(int Button, int Action, int Mods);

	const glm::mat4& GetProjMatrix() const { return m_persProjection; }

	glm::vec3 GetPosition() const { return m_cameraPos; }

	glm::vec3 GetTarget() const;

	glm::vec3 GetUp() const { return m_up; }

	glm::mat4 GetViewMatrix() const;

	glm::mat4 GetVPMatrix() const;

	glm::mat4 GetVPMatrixNoTranslate() const;

	const PersProjInfo& GetPersProjInfo() const { return m_persProjInfo; }

	void SetPos(const glm::vec3& Pos) { m_cameraPos = Pos; }

	void SetUp(const glm::vec3& Up) { m_up = Up; }

	void SetTarget(const glm::vec3& Target);

	void Print() const;

	// Implementation of CameraAPI

	virtual const Vector3f GetPos() const;

	virtual Matrix4f GetViewportMatrix() const;

	virtual Matrix4f GetMatrix() const;

	virtual const Matrix4f GetProjectionMat() const;

private:

	glm::vec3 CalcAcceleration();
	void CalcVelocity(float dt);
	void CalcCameraOrientation();
	void SetUpVector();	

	glm::mat4 m_persProjection = glm::mat4(0.0);
	glm::vec3 m_cameraPos = glm::vec3(0.0f);
	glm::quat m_cameraOrientation = glm::quat(glm::vec3(0.0f));
	glm::vec3 m_velocity = glm::vec3(0.0f);
	glm::vec2 m_oldMousePos = glm::vec2(0.0f);
	glm::vec3 m_up = glm::vec3(0.0f);
	MouseState m_mouseState;
	PersProjInfo m_persProjInfo;
};

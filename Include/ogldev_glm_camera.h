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

static bool constexpr CAMERA_LEFT_HANDED = true;

class GLMCameraFirstPerson {
public:

	GLMCameraFirstPerson() {}

	GLMCameraFirstPerson(const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up, PersProjInfo& persProjInfo);

	glm::mat4 GetViewMatrix() const;

	const glm::mat4& GetProjMatrix() const { return m_persProjection; }

	glm::mat4 GetVPMatrix() const;

	glm::vec3 GetPosition() const { return m_cameraPos; }

private:

	glm::mat4 m_persProjection = glm::mat4(0.0);
	glm::vec3 m_cameraPos = glm::vec3(0.0f);
	glm::quat m_cameraOrientation = glm::quat(glm::vec3(0.0f));
};

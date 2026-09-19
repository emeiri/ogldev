/*

		Copyright 2026 Etay Meiri

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

    Pong In Vulkan
*/

#include "prebaked_renderer.h"
#include "pong.h"

#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

#define NAME_OF_THE_GAME "Pong"

class VulkanPong : public GameCallbacks {

public:
    
    VulkanPong(int WindowWidth, int WindowHeight) 
		: m_renderer(WindowWidth, WindowHeight, NAME_OF_THE_GAME, this)
    {        
        m_config.BallSpeed = 5.0f;        
        m_config.PaddleOffset = 0.5f;
        m_config.PaddleMaxSpeed = 15.0f;
        m_config.PaddleAcceleration = 20.0f;
        m_config.PaddleDeceleration = 15.0f;
    }

    void Execute()
    {
		std::string AssetPath = "../../Games/Pong/Pong2.glb";
		m_renderer.Init(AssetPath);

        OgldevVK::VkModel& PongModel = m_renderer.GetModel();

        const MeshDims& BallDims = PongModel.GetNodeDims("Ball");
        const MeshDims& PaddleLDims = PongModel.GetNodeDims("PaddleL");
        const MeshDims& TopWallDims = PongModel.GetNodeDims("TopWall");
        const MeshDims& BottomWallDims = PongModel.GetNodeDims("BottomWall");
        const MeshDims& PaddleRDims = PongModel.GetNodeDims("PaddleR");

        m_config.BallSize = BallDims.Size.x;
        
        m_config.PaddleWidth = PaddleLDims.Size.x;
        m_config.PaddleHeight = PaddleLDims.Size.z;

        float TopWallBottomSide = TopWallDims.Pos.z - TopWallDims.Size.z / 2.0f;
        float BottomWallTopSide = BottomWallDims.Pos.z + BottomWallDims.Size.z / 2.0f;

        m_config.BaseWindowPosition.x = PaddleLDims.Pos.x - PaddleLDims.Size.x / 2.0f;        
        m_config.BaseWindowPosition.y = -TopWallBottomSide;     // The Pong core goes from zero down to full size

        float PaddleRRightSide = PaddleRDims.Pos.x + PaddleRDims.Size.x / 2.0f;
        float PaddleLLeftSide = PaddleLDims.Pos.x - PaddleLDims.Size.x / 2.0f;

        m_config.WindowSize.x = PaddleRRightSide - PaddleLLeftSide;
        m_config.WindowSize.y = TopWallBottomSide - BottomWallTopSide;

        m_game.Init(m_config);

        m_renderer.Execute();
    }

	void UpdateGameState(float dt)
	{
        //printf("DeltaTime: %f\n", dt);
        m_game.Update(m_inputState.PaddleLUp, m_inputState.PaddleLDown, 
                      m_inputState.PaddleRUp, m_inputState.PaddleRDown, dt);

        glm::vec2 BallPos, PaddleLPos, PaddleRPos;

        m_game.GetPositions(BallPos, PaddleLPos, PaddleRPos);

        OgldevVK::VkModel& PongModel = m_renderer.GetModel();

        std::vector<glm::mat4>& Transformations = PongModel.GetTransformationsMutable();

        PongModel.SetNodePositionXZ("Ball", BallPos.x, BallPos.y);

        PongModel.SetNodePositionXZ("PaddleL", PaddleLPos.x, PaddleLPos.y);

        PongModel.SetNodePositionXZ("PaddleR", PaddleRPos.x, PaddleRPos.y);
	}


    virtual void OnKey(int Key, int Scancode, int Action, int Mods)
    {
        bool KeyPressed = (Action == GLFW_PRESS || Action == GLFW_REPEAT);

        switch (Key) {
        case GLFW_KEY_R:
            m_inputState.PaddleLUp = KeyPressed;
            break;

        case GLFW_KEY_F:
            m_inputState.PaddleLDown = KeyPressed;
            break;

        case GLFW_KEY_O:
            m_inputState.PaddleRUp = KeyPressed;
            break;

        case GLFW_KEY_L:
            m_inputState.PaddleRDown = KeyPressed;
            break;
        }
    }


private:
    PreBakedRenderer m_renderer;
	GameConfig m_config;
    Pong m_game;
    PongInputState m_inputState;
};


int main(int argc, char* argv[])
{
    VulkanPong Game(WINDOW_WIDTH, WINDOW_HEIGHT);

	Game.Execute();

	return 0;
}
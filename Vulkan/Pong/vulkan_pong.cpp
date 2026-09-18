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
        m_config.WindowSize = { 20.0f, 20.0f };// { (float)WindowWidth, (float)WindowHeight };
        m_config.BallInitialVelocity = { -1.0f, 0.0f };
        m_config.BallSize = 0.75f;
        m_config.BaseWindowPosition = { -10.0f, -10.0f };
        m_config.PaddleHeight = 4.0f;
        m_config.PaddleWidth = 1.0f;
        m_config.PaddleOffset = 0.5f;
        m_config.PaddleSpeed = 5.0f;
    }

    void Execute()
    {
        m_game.Init(m_config);

		std::string AssetPath = "../../Games/Pong/Pong2.glb";
		m_renderer.Init(AssetPath);
        m_renderer.Execute();
    }

	void UpdateGameState(float dt)
	{
        //printf("DeltaTime: %f\n", dt);
        m_game.Update(m_inputState.PaddleLUp, m_inputState.PaddleLDown, 
                      m_inputState.PaddleRUp, m_inputState.PaddleRDown, dt);

        Rect BallRect, PaddleLRect, PaddleRRect;

        m_game.GetRects(BallRect, PaddleLRect, PaddleRRect);

        OgldevVK::VkModel& PongModel = m_renderer.GetModel();

        std::vector<glm::mat4>& Transformations = PongModel.GetTransformationsMutable();

        glm::mat4& BallTransform = Transformations[0];

        BallTransform[3][0] = BallRect.x;
        BallTransform[3][2] = BallRect.y;

     //   printf("BallRect.x %f Ratio %f BallX: %f\n", BallRect.x, (BallRect.x / m_config.WindowSize.x), BallX);

        glm::mat4& PaddleLTransform = Transformations[1];
        glm::mat4& PaddleRTransform = Transformations[2];
        
        
        //printf("Ball: x=%f, y=%f, w=%f, h=%f\n", BallRect.x, BallRect.y, BallRect.w, BallRect.h);
        //m_renderer.SetEntityPositions(BallRect, PaddleLRect, PaddleRRect);
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
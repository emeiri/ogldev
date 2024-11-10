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

    Tutorial 57 - Bindless Textures
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app.h"
#include "ogldev_infinite_grid.h"
#include "ogldev_glm_camera.h"
#include "bindless_tex_technique.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


#define NUM_INSTANCES 1000

class Tutorial57 : public OgldevBaseApp
{
public:

    Tutorial57()
    {
    }


    virtual ~Tutorial57()
    {
    }


    void Init()
    {
        DefaultCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 57");

        DefaultInitCallbacks();

        InitCamera();

        InitInfiniteGrid();

        DefaultInitGUI();

        m_bindlessTexTech.Init();

        InitTexture();

      //  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }


	virtual bool KeyboardCB(int Key, int Action, int Mods)
	{
        bool Handled = GLFWCameraHandler(m_pCamera->m_movement, Key, Action, Mods);

        if (Handled) {
            return true;
        } else {
            return OgldevBaseApp::KeyboardCB(Key, Action, Mods);
        }
	}


	void MouseMoveCB(int xpos, int ypos)
	{
		m_pCamera->m_mouseState.m_pos.x = (float)xpos / (float)WINDOW_WIDTH;
        m_pCamera->m_mouseState.m_pos.y = (float)ypos / (float)WINDOW_HEIGHT;
	}


	virtual void MouseButtonCB(int Button, int Action, int x, int y)
	{
		if (Button == GLFW_MOUSE_BUTTON_LEFT) {
            m_pCamera->m_mouseState.m_buttonPressed = (Action == GLFW_PRESS);
		}
	}


    virtual void RenderSceneCB(float dt)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		m_pCamera->Update(dt);		

        glm::mat4 VP = m_pCamera->GetVPMatrix();

     //   m_infiniteGrid.Render(m_config, VP, m_pCamera->GetPosition());

        m_bindlessTexTech.Enable();
       // glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, NUM_INSTANCES, 0);

        static int TextureIndex = 0;
        m_bindlessTexTech.SetTextureIndex(TextureIndex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        TextureIndex++;
        if (TextureIndex == NUM_INSTANCES) {
            TextureIndex = 0;
        }
    }


    void RenderGui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    private:

    void InitTexture()
    {
        std::vector<GLuint> textures;
        std::vector<GLuint64> textureHandles;

        textures.resize(NUM_INSTANCES);
        textureHandles.resize(NUM_INSTANCES);

        unsigned char limit = unsigned char(rand() % 231 + 25);
        const size_t textureSize = 32 * 32 * 3;
        unsigned char textureData[textureSize];

        for (int i = 0; i < NUM_INSTANCES; i++) {
            // Randomly generate an unsigned char per RGB channel
            for (int j = 0; j < textureSize; ++j) {
                textureData[j] = unsigned char(rand() % limit);
            }

            GLuint texture;
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);
            glTextureStorage2D(texture, 1, GL_RGB8, 32, 32);
            glTextureSubImage2D(texture,
                // level, xoffset, yoffset, width, height
                0, 0, 0, 32, 32,
                GL_RGB, GL_UNSIGNED_BYTE,
                (const void*)&textureData[0]);
            glGenerateTextureMipmap(texture);

            // Retrieve the texture handle after we finish creating the texture
            const GLuint64 handle = glGetTextureHandleARB(texture);
            if (handle == 0) {
                printf("glGetTextureHandleARB failed\n");
                exit(-1);
            }

            textures[i] = texture;
            textureHandles[i] = handle;

            glMakeTextureHandleResidentARB(handle);
        }

        GLuint textureBuffer;
        glCreateBuffers(1, &textureBuffer);
        glNamedBufferStorage(textureBuffer, sizeof(GLuint64) * textureHandles.size(), (const void*)textureHandles.data(), GL_DYNAMIC_STORAGE_BIT);        

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureBuffer);
    }

    void InitCamera()
    {
        float FOV = 45.0f;
        float zNear = 1.0f;
        float zFar = 1000.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

        glm::vec3 Pos(0.0f, 2.1f, 0.0f);
        glm::vec3 Target(0.0f, 2.1f, 1.0f);
        glm::vec3 Up(0.0, 1.0f, 0.0f);

        m_pCamera = new GLMCameraFirstPerson(Pos, Target, Up, persProjInfo);   
    }


    void InitInfiniteGrid()
    {
        m_infiniteGrid.Init(); 
    }

    InfiniteGrid m_infiniteGrid;
    InfiniteGridConfig m_config;
    GLMCameraFirstPerson* m_pCamera = NULL;    
    BindlessTextureTechnique m_bindlessTexTech;
};


int main(int argc, char** argv)
{
    Tutorial57* app = new Tutorial57();

    app->Init();

    app->Run();

    delete app;

    return 0;
}

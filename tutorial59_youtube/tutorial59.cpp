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

    Tutorial 59 - Bindless Textures
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_util.h"
#include "ogldev_vertex_buffer.h"
#include "ogldev_base_app2.h"
#include "ogldev_glm_camera.h"
#include "bindless_tex_technique.h"
#include "3rdparty/stb_image.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define NUM_DIRS 3
#define NUM_FILES_IN_DIR 100
#define NUM_TOTAL_FILES (NUM_DIRS * NUM_FILES_IN_DIR)


class Tutorial59 : public OgldevBaseApp2
{
public:

    Tutorial59()
    {
    }


    virtual ~Tutorial59()
    {
    }


    void Init()
    {
        InitBaseApp(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 59");

        m_bindlessTexTech.Init();

        InitTextures();

        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
//        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }



    virtual void RenderSceneCB(float dt)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        m_bindlessTexTech.Enable();

        static int TextureIndex = 0;
        m_bindlessTexTech.SetTextureIndex(TextureIndex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        TextureIndex++;
        if (TextureIndex == NUM_TOTAL_FILES) {
            TextureIndex = 0;
        }
    }

    private:

    void InitTextures()
    {
        std::vector<GLuint> textures;
        std::vector<GLuint64> textureHandles;
        std::vector<std::string> textureFiles;

        textures.resize(NUM_TOTAL_FILES);
        textureHandles.resize(NUM_TOTAL_FILES);
        textureFiles.resize(NUM_TOTAL_FILES);

        for (uint32_t j = 0; j < NUM_DIRS; j++) {
            for (uint32_t i = 0; i != NUM_FILES_IN_DIR; i++) {
                char fname[1024];
                snprintf(fname, sizeof(fname), "G://emeir/Books/3D-Graphics-Rendering-Cookbook-2/deps/src/explosion%01u/explosion%02u-frame%03u.tga", j, j, i + 1);
                int Index = j * NUM_FILES_IN_DIR + i;
                textureFiles[Index] = fname;
            }
        }

        for (int i = 0; i < NUM_TOTAL_FILES; i++) {
            int texWidth, texHeight, texChannels;
            printf("Loading '%s'\n", textureFiles[i].c_str());
            stbi_uc* pixels = stbi_load(textureFiles[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

            if (!pixels) {
                printf("Failed to load [%s] texture\n", textureFiles[i].c_str()); fflush(stdout);
                exit(1);
            }

            GLuint texture;
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);
            glTextureStorage2D(texture, 1, GL_RGBA8, texWidth, texHeight);
            glTextureSubImage2D(texture, 0, 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)pixels);
            glGenerateTextureMipmap(texture);

            stbi_image_free(pixels);

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

    BindlessTextureTechnique m_bindlessTexTech;
};


int main(int argc, char** argv)
{
    Tutorial59* app = new Tutorial59();

    app->Init();

    app->Run();

    delete app;

    return 0;
}

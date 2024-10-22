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

    Tutorial 56 - Cubemaps
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
#include "ogldev_skybox.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

template <typename T>
T clamp(T v, T a, T b)
{
    if (v < a) return a;
    if (v > b) return b;
    return v;
}


glm::vec3 FaceCoordsToXYZ(int i, int j, int FaceID, int FaceSize)
{
    float A = 2.0f * float(i) / FaceSize;
    float B = 2.0f * float(j) / FaceSize;

    glm::vec3 Ret;

    switch (FaceID) {
    case 0:
        Ret = glm::vec3(-1.0f, A - 1.0f, B - 1.0f);
        break;
    
    case 1:
        Ret = glm::vec3(A - 1.0f, -1.0f, 1.0f - B);
        break;
    
    case 2:
        Ret = glm::vec3(1.0f, A - 1.0f, 1.0f - B);
        break;

    case 3:
        Ret = glm::vec3(1.0f - A, 1.0f, 1.0f - B);
        break;

    case 4:
        Ret = glm::vec3(B - 1.0f, A - 1.0f, 1.0f);
        break;

    case 5:
        Ret = glm::vec3(1.0f - B, A - 1.0f, -1.0f);
        break;

    default:
        assert(0);
    } 

    return Ret;
}

enum eBitmapType
{
    eBitmapType_2D,
    eBitmapType_Cube
};

enum eBitmapFormat
{
    eBitmapFormat_UnsignedByte,
    eBitmapFormat_Float,
};


struct Bitmap
{
    Bitmap() = default;
    Bitmap(int w, int h, int comp, eBitmapFormat fmt)
        :w_(w), h_(h), comp_(comp), fmt_(fmt), data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }
    Bitmap(int w, int h, int d, int comp, eBitmapFormat fmt)
        :w_(w), h_(h), d_(d), comp_(comp), fmt_(fmt), data_(w* h* d* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
    }
    Bitmap(int w, int h, int comp, eBitmapFormat fmt,  void* ptr)
        :w_(w), h_(h), comp_(comp), fmt_(fmt), data_(w* h* comp* getBytesPerComponent(fmt))
    {
        initGetSetFuncs();
        memcpy(data_.data(), ptr, data_.size());
    }
    int w_ = 0;
    int h_ = 0;
    int d_ = 1;
    int comp_ = 3;
    eBitmapFormat fmt_ = eBitmapFormat_UnsignedByte;
    eBitmapType type_ = eBitmapType_2D;
    std::vector<uint8_t> data_;

    static int getBytesPerComponent(eBitmapFormat fmt)
    {
        if (fmt == eBitmapFormat_UnsignedByte) return 1;
        if (fmt == eBitmapFormat_Float) return 4;
        return 0;
    }

    void setPixel(int x, int y,  glm::vec4& c)
    {
        (*this.*setPixelFunc)(x, y, c);
    }
    glm::vec4 getPixel(int x, int y) 
    {
        return ((*this.*getPixelFunc)(x, y));
    }
private:
    using setPixel_t = void(Bitmap::*)(int, int,  glm::vec4&);
    using getPixel_t = glm::vec4(Bitmap::*)(int, int) ;
    setPixel_t setPixelFunc = &Bitmap::setPixelUnsignedByte;
    getPixel_t getPixelFunc = &Bitmap::getPixelUnsignedByte;

    void initGetSetFuncs()
    {
        switch (fmt_)
        {
        case eBitmapFormat_UnsignedByte:
            setPixelFunc = &Bitmap::setPixelUnsignedByte;
            getPixelFunc = &Bitmap::getPixelUnsignedByte;
            break;
        case eBitmapFormat_Float:
            setPixelFunc = &Bitmap::setPixelFloat;
            getPixelFunc = &Bitmap::getPixelFloat;
            break;
        }
    }

    void setPixelFloat(int x, int y,  glm::vec4& c)
    {
         int ofs = comp_ * (y * w_ + x);
        float* data = reinterpret_cast<float*>(data_.data());
        if (comp_ > 0) data[ofs + 0] = c.x;
        if (comp_ > 1) data[ofs + 1] = c.y;
        if (comp_ > 2) data[ofs + 2] = c.z;
        if (comp_ > 3) data[ofs + 3] = c.w;
    }
    glm::vec4 getPixelFloat(int x, int y) 
    {
         int ofs = comp_ * (y * w_ + x);
         float* data = reinterpret_cast< float*>(data_.data());
        return glm::vec4(
            comp_ > 0 ? data[ofs + 0] : 0.0f,
            comp_ > 1 ? data[ofs + 1] : 0.0f,
            comp_ > 2 ? data[ofs + 2] : 0.0f,
            comp_ > 3 ? data[ofs + 3] : 0.0f);
    }

    void setPixelUnsignedByte(int x, int y,  glm::vec4& c)
    {
         int ofs = comp_ * (y * w_ + x);
        if (comp_ > 0) data_[ofs + 0] = uint8_t(c.x * 255.0f);
        if (comp_ > 1) data_[ofs + 1] = uint8_t(c.y * 255.0f);
        if (comp_ > 2) data_[ofs + 2] = uint8_t(c.z * 255.0f);
        if (comp_ > 3) data_[ofs + 3] = uint8_t(c.w * 255.0f);
    }
    glm::vec4 getPixelUnsignedByte(int x, int y) 
    {
         int ofs = comp_ * (y * w_ + x);
        return glm::vec4(
            comp_ > 0 ? float(data_[ofs + 0]) / 255.0f : 0.0f,
            comp_ > 1 ? float(data_[ofs + 1]) / 255.0f : 0.0f,
            comp_ > 2 ? float(data_[ofs + 2]) / 255.0f : 0.0f,
            comp_ > 3 ? float(data_[ofs + 3]) / 255.0f : 0.0f);
    }
};


Bitmap convertEquirectangularMapToVerticalCross( Bitmap& b)
{
    if (b.type_ != eBitmapType_2D) return Bitmap();

    int faceSize = b.w_ / 4;

    int w = faceSize * 3;
    int h = faceSize * 4;

    Bitmap result(w, h, b.comp_, b.fmt_);

    glm::ivec2 kFaceOffsets[] =
    {
        glm::ivec2(faceSize, faceSize * 3),
        glm::ivec2(0, faceSize),
        glm::ivec2(faceSize, faceSize),
        glm::ivec2(faceSize * 2, faceSize),
        glm::ivec2(faceSize, 0),
        glm::ivec2(faceSize, faceSize * 2)
    };

    int clampW = b.w_ - 1;
    int clampH = b.h_ - 1;

    for (int face = 0; face != 6; face++)
    {
        for (int i = 0; i != faceSize; i++)
        {
            for (int j = 0; j != faceSize; j++)
            {
                glm::vec3 P = FaceCoordsToXYZ(i, j, face, faceSize);
                float R = hypot(P.x, P.y);
                float theta = atan2f(P.y, P.x);
                float phi = atan2f(P.z, R);
                //	float point source coordinates
                float Uf = float(2.0f * faceSize * (theta + M_PI) / M_PI);
                float Vf = float(2.0f * faceSize * (M_PI / 2.0f - phi) / M_PI);
                // 4-samples for bilinear interpolation
                int U1 = clamp(int(floor(Uf)), 0, clampW);
                int V1 = clamp(int(floor(Vf)), 0, clampH);
                int U2 = clamp(U1 + 1, 0, clampW);
                int V2 = clamp(V1 + 1, 0, clampH);
                // fractional part
                float s = Uf - U1;
                float t = Vf - V1;
                // fetch 4-samples
                glm::vec4 A = b.getPixel(U1, V1);
                glm::vec4 B = b.getPixel(U2, V1);
                glm::vec4 C = b.getPixel(U1, V2);
                glm::vec4 D = b.getPixel(U2, V2);
                // bilinear interpolation
                glm::vec4 color = A * (1 - s) * (1 - t) + B * (s) * (1 - t) + C * (1 - s) * t + D * (s) * (t);
                result.setPixel(i + kFaceOffsets[face].x, j + kFaceOffsets[face].y, color);
            }
        };
    }

    return result;
}




class Tutorial56 : public OgldevBaseApp
{
public:

    Tutorial56()
    {
    }


    virtual ~Tutorial56()
    {
    }


    void Init()
    {
        DefaultCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 55");

        DefaultInitCallbacks();

        InitCamera();

        InitInfiniteGrid();

        DefaultInitGUI();

        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

        if (!m_skybox.Init("../Content/envmap_interstellar",
            "interstellar_lf.tga",
            "interstellar_rt.tga",
            "interstellar_up.tga",
            "interstellar_dn.tga",
            "interstellar_ft.tga",
            "interstellar_bk.tga")) {
            exit(0);
        }

      //  glFrontFace(GL_CCW);
      //  glEnable(GL_CULL_FACE);
        //glEnable(GL_DEPTH_TEST);
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

        glm::mat4 VP = m_pCamera->GetVPMatrixNoTranslate();

        m_skybox.Render(VP);

        //m_infiniteGrid.Render(m_config, VP, m_pCamera->GetPosition());
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

#define STEP 0.01f

    private:

    void InitCamera()
    {
        float FOV = 45.0f;
        float zNear = 1.0f;
        float zFar = 1000.0f;
        PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, 
                                      zNear, zFar };

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
    SkyBox m_skybox;
};


int main(int argc, char** argv)
{
    Tutorial56* app = new Tutorial56();

    app->Init();

    app->Run();

    delete app;

    return 0;
}

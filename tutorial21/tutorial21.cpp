/*

	Copyright 2011 Etay Meiri

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

    Tutorial 21 - Spot Light
*/

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_util.h"
#include "pipeline.h"
#include "camera.h"
#include "ogldev_texture.h"
#include "lighting_technique.h"
#include "glut_backend.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1200

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;

    Vertex() {}

    Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
    }
};

static const float FieldDepth = 20.0f;
static const float FieldWidth = 10.0f;

class Tutorial21 : public ICallbacks
{
public:

    Tutorial21()
    {
        m_pGameCamera = NULL;
        m_pTexture = NULL;
        m_pEffect = NULL;
        m_scale = 0.0f;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.0f;
        m_directionalLight.DiffuseIntensity = 0.01f;
        m_directionalLight.Direction = Vector3f(1.0f, -1.0, 0.0);
    }

    ~Tutorial21()
    {
        delete m_pEffect;
        delete m_pGameCamera;
        delete m_pTexture;
    }

    bool Init()
    {
        Vector3f Pos(5.0f, 1.0f, -3.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);

        CreateVertexBuffer();
        
        m_pEffect = new LightingTechnique();

        if (!m_pEffect->Init())
        {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        m_pEffect->Enable();

        m_pEffect->SetTextureUnit(0);

        m_pTexture = new Texture(GL_TEXTURE_2D, "../Content/test.png");

        if (!m_pTexture->Load()) {
            return false;
        }

        return true;
    }

    void Run()
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB()
    {
        m_scale += 0.0057f;

        m_pGameCamera->OnRender();

        glClear(GL_COLOR_BUFFER_BIT);

        PointLight pl[2];
        pl[0].DiffuseIntensity = 0.25f;
        pl[0].Color = Vector3f(1.0f, 0.5f, 0.0f);
        pl[0].Position = Vector3f(3.0f, 1.0f, FieldDepth * (cosf(m_scale) + 1.0f) / 2.0f);
        pl[0].Attenuation.Linear = 0.1f;
        pl[1].DiffuseIntensity = 0.25f;
        pl[1].Color = Vector3f(0.0f, 0.5f, 1.0f);
        pl[1].Position = Vector3f(7.0f, 1.0f, FieldDepth * (sinf(m_scale) + 1.0f) / 2.0f);
        pl[1].Attenuation.Linear = 0.1f;
        m_pEffect->SetPointLights(2, pl);

        SpotLight sl[2];
        sl[0].DiffuseIntensity = 0.9f;
        sl[0].Color = Vector3f(0.0f, 1.0f, 1.0f);
        sl[0].Position = m_pGameCamera->GetPos();
        sl[0].Direction = m_pGameCamera->GetTarget();
        sl[0].Attenuation.Linear = 0.1f;
        sl[0].Cutoff = 10.0f;

        sl[1].DiffuseIntensity = 0.9f;
        sl[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        sl[1].Position = Vector3f(5.0f, 3.0f, 10.0f);
        sl[1].Direction = Vector3f(0.0f, -1.0f, 0.0f);
        sl[1].Attenuation.Linear = 0.1f;
        sl[1].Cutoff = 20.0f;
        m_pEffect->SetSpotLights(2, sl);

        Pipeline p;
        p.WorldPos(0.0f, 0.0f, 1.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f);
        m_pEffect->SetWVP(p.GetWVPTrans());
        const Matrix4f& WorldTransformation = p.GetWorldTrans();
        m_pEffect->SetWorldMatrix(WorldTransformation);
        m_pEffect->SetDirectionalLight(m_directionalLight);
        m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());
        m_pEffect->SetMatSpecularIntensity(0.0f);
        m_pEffect->SetMatSpecularPower(0);
        
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);
        m_pTexture->Bind(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2); 

        glutSwapBuffers();
    }

    virtual void IdleCB()
    {
        RenderSceneCB();
    }

    virtual void SpecialKeyboardCB(int Key, int x, int y)
    {
        m_pGameCamera->OnKeyboard(Key);
    }


    virtual void KeyboardCB(unsigned char Key, int x, int y)
    {
        switch (Key) {
            case 'q':
                glutLeaveMainLoop();
                break;

            case 'a':
                m_directionalLight.AmbientIntensity += 0.05f;
                break;

            case 's':
                m_directionalLight.AmbientIntensity -= 0.05f;
                break;

            case 'z':
                m_directionalLight.DiffuseIntensity += 0.05f;
                break;

            case 'x':
                m_directionalLight.DiffuseIntensity -= 0.05f;
                break;
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

private:

    void CreateVertexBuffer()
    {
        const Vector3f Normal = Vector3f(0.0, 1.0f, 0.0f);

        Vertex Vertices[6] = {
                                Vertex(Vector3f(0.0f, 0.0f, 0.0f),             Vector2f(0.0f, 0.0f), Normal),
                                Vertex(Vector3f(0.0f, 0.0f, FieldDepth),       Vector2f(0.0f, 1.0f), Normal),
                                Vertex(Vector3f(FieldWidth, 0.0f, 0.0f),       Vector2f(1.0f, 0.0f), Normal),

                                Vertex(Vector3f(FieldWidth, 0.0f, 0.0f),       Vector2f(1.0f, 0.0f), Normal),
                                Vertex(Vector3f(0.0f, 0.0f, FieldDepth),       Vector2f(0.0f, 1.0f), Normal),
                                Vertex(Vector3f(FieldWidth, 0.0f, FieldDepth), Vector2f(1.0f, 1.0f), Normal)
                             };

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    }

    GLuint m_VBO;
    LightingTechnique* m_pEffect;
    Texture* m_pTexture;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_directionalLight;
};


int main(int argc, char** argv)
{
    Magick::InitializeMagick(*argv);
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 21")) {
        return 1;
    }

    Tutorial21* pApp = new Tutorial21();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;
 
    return 0;
}
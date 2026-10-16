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

    DemoLITION - Carbonara - AnimationDemo
*/

#include "../carbonara/carbonara.h"


class AnimationDemo : public Carbonara {

public:

    AnimationDemo() : m_accelorObject(6.0f, 4.0f, 5.0f) {}

    void InitChild()
    {
        m_pScene->GetConfig()->ControlShadowMapping(false);
        m_pScene->GetConfig()->GetInfiniteGrid().Enabled = true;

        m_pScene->SetCamera(Vector3f(0.0f, 4.0f, 0.0f), Vector3f(0.0, -0.4f, 1.0f));
       // PhysicsSceneObject CObject = LoadAndAddModel("../Content/iclone-7-raptoid-mascot/scene.gltf", false, 0.01f);
      //  CObject.pSceneObject->SetRotation(90.0f, 0.0f, 0.0f);
		
        m_pModel = m_pRenderingSystem->LoadModel("../Content/Mixamo/Running/Running.dae");
      // m_pModel = m_pRenderingSystem->LoadModel("../Content/box.obj");
        m_pSceneObject = m_pScene->CreateSceneObject(m_pModel);
      //  m_pSceneObject->SetScale(0.1f);
     //   m_pSceneObject->SetRotation(0.0f, 180.0f, 0.0f);
        m_pSceneObject->SetPosition(0.0f, 0.0f, 10.0f);

        m_pScene->AddToRenderList(m_pSceneObject);
    }


    bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = Carbonara::OnKeyboard(key, action);

        return HandledByMe;
    }

protected:

    virtual void OnFrameChild(double dt)
    {
       // printf("%f\n", dt);

        const InputState& is = m_pScene->GetInputState();

        bool isMovingForward = (is.Keys[KEY_UP].Pressed > is.Keys[KEY_UP].Released);

        float Speed = m_accelorObject.Update((float)dt, isMovingForward, false);

        // Move the character using the calculated current speed
        if (Speed > 0.0f) {
            Vector3f ForwardDir = m_pSceneObject->GetForwardDir();
            m_pSceneObject->TranslateBy(ForwardDir * Speed);
        }


      /*  if (is.Keys[KEY_UP].Pressed > is.Keys[KEY_UP].Released) {
            Vector3f ForwardDir = m_pSceneObject->GetForwardDir();
            m_pSceneObject->TranslateBy(ForwardDir * m_speed * (float)dt);
           // printf("move forward\n");
            //m_oldUpPressed = m_pScene->GetInputState().Keys[KEY_UP].Pressed;
        } //else printf("stop\n");*/
               
        if (is.Keys[KEY_DOWN].Pressed > is.Keys[KEY_DOWN].Released) {
            Vector3f BackwardDir = m_pSceneObject->GetForwardDir() * -1.0f;
         //   m_pSceneObject->TranslateBy(BackwardDir * m_speed * (float)dt);
            //  m_oldDownPressed = m_pScene->GetInputState().Keys[KEY_DOWN].Pressed;
        }

        if (is.Keys[KEY_LEFT].Pressed > is.Keys[KEY_LEFT].Released) {
          //  Speed = std::min(Speed, 1.0f);
            m_pSceneObject->RotateBy(0.0f, 2.0f * Speed, 0.0f);
        }

        if (is.Keys[KEY_RIGHT].Pressed > is.Keys[KEY_RIGHT].Released) {
        //    Speed = std::min(Speed, 1.0f);
            m_pSceneObject->RotateBy(0.0f, -2.0f * Speed, 0.0f);
        }

        GLMCameraFirstPerson* pCamera = m_pScene->GetCurrentCamera();
        pCamera->SetCenter(m_pSceneObject->GetGLMPos());
        Vector3f ForwardDir = m_pSceneObject->GetForwardDir();
        Vector3f ModelPos = m_pSceneObject->GetPosition();
        Vector3f CameraPosCenter = ModelPos - ForwardDir * 10.0f;
        CameraPosCenter.y = ModelPos.y + 5.0f;
        pCamera->SetPos(CameraPosCenter);
#if 0
        static float x = 0.0f;
        if (isMovingForward) {
            m_curCameraPos = CameraPosCenter;
        } else {
            if (m_curCameraPos.IsZero()) {
                m_curCameraPos = m_endCameraPos = CameraPosCenter;
            } else {
                float CurDistance = m_endCameraPos.Distance(m_curCameraPos);
                printf("CurDistance %f\n", CurDistance);
                if (CurDistance >= 10.0f) {
                    m_curCameraPos = CameraPosCenter;
                } else if (CurDistance < 0.001f) {
                    float x = RandomFloatRange(-1.0f, 1.0f);
                    float y = RandomFloatRange(-1.0f, 1.0f);
                    float z = RandomFloatRange(-1.0f, 1.0f);
                    Vector3f v(x, y, z);
                    float SwayRange = RandomFloatRange(0.0f, 1.0f) * m_maxAmbientSwayRange;
                    m_endCameraPos = CameraPosCenter + v.Normalize() * SwayRange;
                    m_cameraTargetDistance = m_endCameraPos.Distance(m_curCameraPos);
                } else {
                    Vector3f Dir = (m_endCameraPos - m_curCameraPos).Normalize();
                    m_curCameraPos += Dir * m_ambientSwaySpeed;
                }
            }
        }

//        m_curCameraPos.Print();
        pCamera->SetPos(m_curCameraPos);
#endif
    }

private:

    Model* m_pModel = NULL;
    SceneObject* m_pSceneObject = NULL;
    AccelorObject m_accelorObject;
    u64 m_oldUpPressed = 0;
    u64 m_oldDownPressed = 0;
    Vector3f m_curCameraPos = Vector3f(0.0f);
    Vector3f m_endCameraPos = Vector3f(0.0f);
    float m_cameraTargetDistance = 0.0f;
    float m_maxAmbientSwayRange = 1.0f;
    float m_ambientSwaySpeed = 0.004f;
};


int main(int argc, char* arg[])
{
    AnimationDemo demo;

    demo.Start();
}

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

    DemoLITION - Carbonara - BridgeDemo
*/

#include "../carbonara/carbonara.h"


static void DirToRotation(const Vector3f& Dir, SceneObject& o)
{
    Vector3f DirN = Dir;
    DirN.Normalize();

    glm::vec3 Start(1.0f, 0.0f, 0.0f);
    glm::vec3 Dest(Dir.x, Dir.y, Dir.z);

    glm::quat q = RotationBetweenVectors(Start, Dest);
    o.SetQuaternion(q);
}


class BridgeDemo : public Carbonara {

public:

#define NUM_SPHERES 12
#define NUM_CABLES (NUM_SPHERES-2)
#define NUM_PLANKS (NUM_SPHERES / 2)

#define PLANK_LENGTH 2.0f

#define BASE_MASS 1
#define EXTRA_MASS 10

    BridgeDemo() 
    {
        m_physicsSystem.Init(NUM_PSOs, NUM_SPHERES * 10, 0);
    }

    void InitChild()
    {
       // glDisable(GL_CULL_FACE);
        m_pScene->SetCamera(Vector3f(0.0f, 4.5f, -14.0f), Vector3f(0.0, -0.25f, 1.0f));

        m_pRod = m_pRenderingSystem->LoadModel("../Content/demolition/rod.obj");
        int SphereTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/brickwall.jpg");
        m_pRod->SetColorTexture(SphereTexture);

        m_pSphere = m_pRenderingSystem->LoadModel("../Content/demolition/sphere8.obj");
        m_pSphere->SetColorTexture(SphereTexture);

        m_PSOs.resize(NUM_SPHERES);
        m_particles.resize(NUM_SPHERES);
        m_supports.resize(NUM_SPHERES);
        m_vLinks.resize(NUM_SPHERES);
        m_hLinks.resize(NUM_CABLES);
        m_cables.resize(NUM_CABLES);
        m_planks.resize(NUM_PLANKS);
        m_rods.resize(NUM_PLANKS);

        InitPlankEnds();

        InitPlanks();
        
        InitVLinks();

        InitHLinks();

        InitBall();

        m_groundContactGenerator.Init(&m_particles);

        m_physicsSystem.AddContactGenerator(&m_groundContactGenerator);
    }


    bool OnKeyboard(int key, int action)
    {
        bool HandledByMe = false;

        switch (key) {
        case GLFW_KEY_U:
            m_ballBasePos.z += 0.1f;
            HandledByMe = true;
            break;

        case GLFW_KEY_M:
            m_ballBasePos.z -= 0.1f;
            HandledByMe = true;
            break;

        case GLFW_KEY_H:
            m_ballBasePos.x -= 0.1f;
            HandledByMe = true;
            break;

        case GLFW_KEY_K:
            m_ballBasePos.x += 0.1f;
            HandledByMe = true;
            break;

        default:
            HandledByMe = Carbonara::OnKeyboard(key, action);
        }

        m_ballBasePos.x = std::clamp(m_ballBasePos.x, 0.0f, 5.0f);
        m_ballBasePos.z = std::clamp(m_ballBasePos.z, -1.0f, 1.0f);
        return HandledByMe;
    }


protected:

    virtual void OnFrameChild(double DeltaTime)
    {
        // Links from above
        for (int i = 0; i < NUM_SPHERES; i++) {
            Vector3f Dir = m_supports[i].m_anchor - m_PSOs[i].pSceneObject->GetPosition();
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            m_vLinks[i]->SetPosition(Pos);
            m_vLinks[i]->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *m_vLinks[i]);
        }

        for (int i = 0; i < NUM_CABLES; i++) {
            Vector3f Dir = (m_PSOs[i + 2].pSceneObject->GetPosition() - m_PSOs[i].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            m_hLinks[i]->SetPosition(Pos);
            m_hLinks[i]->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *m_hLinks[i]);
        }

        for (int i = 0; i < NUM_PLANKS; i++) {
            int BaseIndex = i * 2;
            Vector3f Dir = (m_PSOs[BaseIndex + 1].pSceneObject->GetPosition() - m_PSOs[BaseIndex].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[BaseIndex].pSceneObject->GetPosition() + Dir / 2.0f;
            m_planks[i]->SetPosition(Pos);
            m_planks[i]->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *m_planks[i]);
        }

        UpdateBallPosition();
    }

    
    void UpdateBallPosition()
    {
        int x = int(m_ballBasePos.x);

        float xp = fmod(m_ballBasePos.x, 1.0f);

        if (x < 0) {
            x = 0;
            xp = 0;
        } 
        
        if (x >= 5) {
            x = 5;
            xp = 0;
        }

        int z = int(m_ballBasePos.z);
        float zp = fmod(m_ballBasePos.z, 1.0f);

        if (z < 0) {
            z = 0;
            zp = 0.0f;
        }

        if (z >= 1) {
            z = 1;
            zp = 0;
        }


        // Calculate where to draw the mass
        m_ballDisplayPos.SetZero();

        // Add the proportion to the correct masses
        m_particles[x * 2 + z]->SetMass(BASE_MASS + EXTRA_MASS * (1 - xp) * (1 - zp));

        m_ballDisplayPos += (m_particles[x * 2 + z]->GetPosition() * (1 - xp) * (1 - zp));

        if (xp > 0) {
            m_particles[x * 2 + z + 2]->SetMass(BASE_MASS + EXTRA_MASS * xp * (1 - zp));
            m_ballDisplayPos += (m_particles[x * 2 + z + 2]->GetPosition() * xp * (1 - zp));

            if (zp > 0) {
                m_particles[x * 2 + z + 3]->SetMass(BASE_MASS + EXTRA_MASS * xp * zp);
                m_ballDisplayPos += (m_particles[x * 2 + z + 3]->GetPosition() * xp * zp);
            }
        }

        if (zp > 0) {
            m_particles[x * 2 + z + 1]->SetMass(BASE_MASS + EXTRA_MASS * (1 - xp) * zp);
            m_ballDisplayPos += (m_particles[x * 2 + z + 1]->GetPosition() * (1 - xp) * zp);
        }

        m_ballDisplayPos.y += 0.5f;

        m_ballPSObject.InitPosition(m_ballDisplayPos);
    }

private:

    void InitPlankEnds()
    {
        // Base spheres
        for (int i = 0; i < NUM_SPHERES; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pSphere, true, 0.05f);
            Vector3f Pos((i / 2.0f) * 2.0f - 5.0f, 1.0f, (i % 2) * 2.0f - 1.0f);
            PSObject.InitPosition(Pos);
            PSObject.pParticle->SetMass(1.0f);
            PSObject.pParticle->SetDamping(0.9f);
            PSObject.pParticle->SetAcceleration(OgldevPhysics::GRAVITY);
            m_PSOs[i] = PSObject;
            m_particles[i] = PSObject.pParticle;
        }
    }

    void InitPlanks()
    {
        // Rods that connect each opposing spheres
        for (int i = 0; i < NUM_PLANKS; i++) {
            int BaseIndex = i * 2;
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f Dir = (m_PSOs[BaseIndex + 1].pSceneObject->GetPosition() - m_PSOs[BaseIndex].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[BaseIndex].pSceneObject->GetPosition() + Dir / 2.0f;
            m_planks[i] = PSObject.pSceneObject;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *PSObject.pSceneObject);
            m_rods[i].m_pParticles[0] = m_PSOs[BaseIndex].pParticle;
            m_rods[i].m_pParticles[1] = m_PSOs[BaseIndex + 1].pParticle;
            m_rods[i].m_len = PLANK_LENGTH;
            m_physicsSystem.AddContactGenerator(&m_rods[i]);
        }
    }

    void InitVLinks()
    {
        // Links from above
        for (int i = 0; i < NUM_SPHERES; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            Vector3f AnchorPos((i / 2.0f) * 2.2f - 5.5f, 6, (i % 2) * 1.6f - 0.8f);
            Vector3f Dir = AnchorPos - m_PSOs[i].pSceneObject->GetPosition();
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            m_vLinks[i] = PSObject.pSceneObject;
            DirToRotation(Dir, *PSObject.pSceneObject);
            m_supports[i].m_pParticle = m_PSOs[i].pParticle;
            m_supports[i].m_anchor = AnchorPos;
            float MaxLength = 0.0f;
            
            if (i < 6) {
                MaxLength = (i / 2.0f) * 0.5f + 3.0f;
            } else {
                MaxLength = 5.5f - (i / 2.0f) * 0.5f;
            }

            m_supports[i].m_maxLength = MaxLength;
            m_supports[i].m_restitution = 0.5f;
            m_physicsSystem.AddContactGenerator(&m_supports[i]);
        }
    }

    void InitHLinks()
    {
        // Links between consecutive spheres
        for (int i = 0; i < NUM_CABLES ; i++) {
            PhysicsSceneObject PSObject = AddPhysicsSceneObject(m_pRod, false);
            m_cables[i].m_pParticles[0] = m_PSOs[i].pParticle;
            m_cables[i].m_pParticles[1] = m_PSOs[i+2].pParticle;
            m_cables[i].m_maxLength = 1.9f;
            m_cables[i].m_restituion = 0.3f;
            m_physicsSystem.AddContactGenerator(&m_cables[i]);
            Vector3f Dir = (m_PSOs[i + 2].pSceneObject->GetPosition() - m_PSOs[i].pSceneObject->GetPosition());
            float RodLen = Dir.Length();
            Vector3f Pos = m_PSOs[i].pSceneObject->GetPosition() + Dir / 2.0f;
            m_hLinks[i] = PSObject.pSceneObject;
            PSObject.pSceneObject->SetPosition(Pos);
            PSObject.pSceneObject->SetScale(RodLen / 2.0f);
            DirToRotation(Dir, *PSObject.pSceneObject);
        }
    }

    void InitBall()
    {
        int BallTexture = m_pRenderingSystem->LoadTexture2D("../Content/textures/gutsy.png");
        m_pBall = m_pRenderingSystem->LoadModel("../Content/demolition/sphere8.obj");
        m_pBall->SetColorTexture(BallTexture);
        m_ballPSObject = AddPhysicsSceneObject(m_pBall, true, 0.5f);
    }

    std::vector<PhysicsSceneObject> m_PSOs;
    std::vector<SceneObject*> m_vLinks;
    std::vector<SceneObject*> m_hLinks;
    std::vector<SceneObject*> m_planks;
    std::vector<OgldevPhysics::Particle*> m_particles;
    std::vector<OgldevPhysics::ParticleCable> m_cables;
    std::vector<OgldevPhysics::ParticleCableConstraint> m_supports;
    std::vector<OgldevPhysics::ParticleRod> m_rods;
    OgldevPhysics::GroundContacts m_groundContactGenerator;
    Model* m_pSphere = NULL;
    Model* m_pBall = NULL;
    Model* m_pRod = NULL;
    PhysicsSceneObject m_ballPSObject;
    Vector3f m_ballBasePos;
    Vector3f m_ballDisplayPos;
};


int main(int argc, char* arg[])
{
    BridgeDemo demo;

    demo.Start();
}

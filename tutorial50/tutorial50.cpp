/*

	Copyright 2016 Etay Meiri

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

    Tutorial 50 - Vulkan
*/

#include <cfloat>
#include <math.h>
#include <GL/glew.h>
#include <string>
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
//#include <xcb/xcb.h>
#endif
#include <sys/types.h>

#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_util.h"
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "ogldev_backend.h"
#include "ogldev_camera.h"
#include "ogldev_basic_mesh.h"
#include "ogldev_lights_common.h"
#include "ogldev_shadow_map_fbo.h"
#include "ogldev_atb.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include <vulkan/vulkan.h>

#include <vulkan/vk_sdk_platform.h>

#define WINDOW_WIDTH  1024  
#define WINDOW_HEIGHT 1024

#if 0

#define NUM_MESHES 5
#define NUM_FRUSTUM_CORNERS 8

Quaternion g_Rotation = Quaternion(0.707f, 0.0f, 0.0f, 0.707f);

int gGLMajorVersion = 0;



class Tutorial50 : public ICallbacks, public OgldevApp
{
public:

    Tutorial50() 
    {
        m_pGameCamera = NULL;
        m_pGroundTex = NULL;
        
        m_dirLight.Name = "DirLight1";
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.AmbientIntensity = 0.5f;
        m_dirLight.DiffuseIntensity = 0.9f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);

        m_persProjInfo.FOV    = 90.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width  = WINDOW_WIDTH;
        m_persProjInfo.zNear  = 1.0f;
        m_persProjInfo.zFar   = 200.0f;  
        
        m_cascadeEnd[0] = m_persProjInfo.zNear;
        m_cascadeEnd[1] = 25.0f,
        m_cascadeEnd[2] = 90.0f,
        m_cascadeEnd[3] = m_persProjInfo.zFar;
                                     
        m_quad.GetOrientation().m_scale    = Vector3f(50.0f, 100.0f, 100.0f);
        m_quad.GetOrientation().m_pos      = Vector3f(0.0f, 0.0f, 90.0f);
        m_quad.GetOrientation().m_rotation = Vector3f(90.0f, 0.0f, 0.0f);

        for (int i = 0; i < NUM_MESHES ; i++) {
            m_meshOrientation[i].m_scale    = Vector3f(1.0f, 1.0f, 1.0f);
            m_meshOrientation[i].m_pos      = Vector3f(0.0f, 0.0f, 3.0f + i * 30.0f);
        }                    
               
        glGetIntegerv(GL_MAJOR_VERSION, &gGLMajorVersion);
    }

    ~Tutorial50()
    {
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pGroundTex);
    }    

    bool Init()
    {
        if (!m_atb.Init()) {
            return false;
        }
        
        if (!m_csmFBO.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
            return false;
        }
		
        Vector3f Pos(8.0, 21.0, -23.0);
        //Vector3f Pos(0.0, 0.0, 0.0);
        Vector3f Target(-0.07f, -0.44f, 0.9f);
        //Vector3f Target(0.0f, 0.0f, 0.1f);
        Vector3f Up(0.0, 1.0f, 0.0f);

        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);
              
        if (!m_LightingTech.Init()) {
            OGLDEV_ERROR("Error initializing the lighting technique\n");
            return false;
        }

        m_LightingTech.Enable();               

        m_LightingTech.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        m_LightingTech.SetShadowMapTextureUnit();		
        m_LightingTech.SetDirectionalLight(m_dirLight);
        m_LightingTech.SetMatSpecularIntensity(0.0f);
        m_LightingTech.SetMatSpecularPower(0);

        for (uint i = 0 ; i < NUM_CASCADES ; i++) {
            Matrix4f Proj;
            Proj.InitPersProjTransform(m_persProjInfo);
            Vector4f vView(0.0f, 0.0f, m_cascadeEnd[i + 1], 1.0f);
            Vector4f vClip = Proj * vView;
            vClip.Print();
            m_LightingTech.SetCascadeEndClipSpace(i, vClip.z);
        }
              
        if (!m_mesh.LoadMesh("../Content/dragon.obj")) {
            return false;            
        }                
       
        if (!m_ShadowMapEffect.Init()) {
            printf("Error initializing the shadow map technique\n");
            return false;
        }        
         
#ifndef WIN32
        // Disabled for now because it somehow clashes with the regular rendering...
 //       if (!m_fontRenderer.InitFontRenderer()) {
   //         return false;
   //     }
#endif        	    

		if (!m_quad.LoadMesh("../Content/quad.obj")) {
            return false;
        }

		m_pGroundTex = new Texture(GL_TEXTURE_2D, "../Content/wal67ar_small.jpg");

        if (!m_pGroundTex->Load()) {
            return false;
        }

        bar = TwNewBar("OGLDEV");
        
        m_pGameCamera->AddToATB(bar);
        
        TwAddSeparator(bar, "", NULL);
        
        TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation, " axisz=-z ");
        
        TwAddSeparator(bar, "", NULL);
        
        m_dirLight.AddToATB(bar);
        
        float refresh = 0.1f;
        TwSetParam(bar, NULL, "refresh", TW_PARAM_FLOAT, 1, &refresh);                
            
        TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with OGLDEV.' "); // Message added to the help bar.
                                                        
        TwAddVarRO(bar, "GL Major Version", TW_TYPE_INT32, &gGLMajorVersion, " label='Major version of GL' ");
        
        return true;
    }

    void Run()
    {
        OgldevBackendRun(this);
    }
    

    virtual void RenderSceneCB()
    {   
        for (int i = 0; i < NUM_MESHES ; i++) {
            m_meshOrientation[i].m_rotation.y += 0.5f;
        }
        
        m_pGameCamera->OnRender();
        
        ShadowMapPass();
        RenderPass();
        OgldevBackendSwapBuffers();
    }
    
	
    void ShadowMapPass()
    {      
        CalcOrthoProjs();

        m_ShadowMapEffect.Enable();        
        
        Pipeline p;
                
        p.SetCamera(Vector3f(0.0f, 0.0f, 0.0f), m_dirLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
               
        for (uint i = 0 ; i < NUM_CASCADES ; i++) {
            m_csmFBO.BindForWriting(i);
            glClear(GL_DEPTH_BUFFER_BIT);            
                        
            p.SetOrthographicProj(m_shadowOrthoProjInfo[i]);                    

            for (int i = 0; i < NUM_MESHES ; i++) {
                p.Orient(m_meshOrientation[i]);
                m_ShadowMapEffect.SetWVP(p.GetWVOrthoPTrans());
                m_mesh.Render();
            }
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

        
    void RenderPass()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_LightingTech.Enable();

        m_LightingTech.SetEyeWorldPos(m_pGameCamera->GetPos());
       
        m_csmFBO.BindForReading();

        Pipeline p;        
        p.Orient(m_quad.GetOrientation());        
        
        p.SetCamera(Vector3f(0.0f, 0.0f, 0.0f), m_dirLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        for (uint i = 0 ; i < NUM_CASCADES ; i++) {
            p.SetOrthographicProj(m_shadowOrthoProjInfo[i]);        
            m_LightingTech.SetLightWVP(i, p.GetWVOrthoPTrans());
        }
                
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);                
        m_LightingTech.SetWVP(p.GetWVPTrans());
        m_LightingTech.SetWorldMatrix(p.GetWorldTrans());        
        m_pGroundTex->Bind(COLOR_TEXTURE_UNIT);

        m_quad.Render();
           
        for (int i = 0; i < NUM_MESHES ; i++) {
            p.Orient(m_meshOrientation[i]);                     
            m_LightingTech.SetWVP(p.GetWVPTrans());
            m_LightingTech.SetWorldMatrix(p.GetWorldTrans());
            m_mesh.Render();
        }
    }
	
       
    virtual void KeyboardCB(OGLDEV_KEY OgldevKey, OGLDEV_KEY_STATE OgldevKeyState)
    {
        if (OgldevKeyState == OGLDEV_KEY_STATE_PRESS) {
            if (m_atb.KeyboardCB(OgldevKey)) {
                return;
            }
        }
        
        switch (OgldevKey) {
            case OGLDEV_KEY_A:
            {
                int Pos[2], Size[2];
                TwGetParam(bar, NULL, "position", TW_PARAM_INT32, 2, Pos);
                TwGetParam(bar, NULL, "size", TW_PARAM_INT32, 2, Size);
                OgldevBackendSetMousePos(Pos[0] + Size[0]/2, 
                                         Pos[1] + Size[1]/2);
                break;
            }
            case OGLDEV_KEY_ESCAPE:
            case OGLDEV_KEY_q:
                OgldevBackendLeaveMainLoop();
                break;
            default:
                m_pGameCamera->OnKeyboard(OgldevKey);
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        if (!m_atb.PassiveMouseCB(x, y)) {
            m_pGameCamera->OnMouse(x, y);
        }        
    }
    
    
    virtual void MouseCB(OGLDEV_MOUSE Button, OGLDEV_KEY_STATE State, int x, int y)
    {
        m_atb.MouseCB(Button, State, x, y);
    }

private:
    
    void CalcOrthoProjs()
    {                                   
        Pipeline p;
        
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        Matrix4f Cam = p.GetViewTrans();
        Matrix4f CamInv = Cam.Inverse();
        
        p.SetCamera(Vector3f(0.0f, 0.0f, 0.0f), m_dirLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        Matrix4f LightM = p.GetViewTrans();
        
        float ar = m_persProjInfo.Height / m_persProjInfo.Width;
        float tanHalfHFOV = tanf(ToRadian(m_persProjInfo.FOV / 2.0f));
        float tanHalfVFOV = tanf(ToRadian((m_persProjInfo.FOV * ar) / 2.0f));
                        
        printf("ar %f tanHalfHFOV %f tanHalfVFOV %f\n", ar, tanHalfHFOV, tanHalfVFOV);
        
        for (uint i = 0 ; i < NUM_CASCADES ; i++) {
            float xn = m_cascadeEnd[i]     * tanHalfHFOV;
            float xf = m_cascadeEnd[i + 1] * tanHalfHFOV;
            float yn = m_cascadeEnd[i]     * tanHalfVFOV;
            float yf = m_cascadeEnd[i + 1] * tanHalfVFOV;

            printf("xn %f xf %f\n", xn, xf);
            printf("yn %f yf %f\n", yn, yf);

            Vector4f frustumCorners[NUM_FRUSTUM_CORNERS] = { 
                // near face
                Vector4f(xn,   yn, m_cascadeEnd[i], 1.0),
                Vector4f(-xn,  yn, m_cascadeEnd[i], 1.0),
                Vector4f(xn,  -yn, m_cascadeEnd[i], 1.0),
                Vector4f(-xn, -yn, m_cascadeEnd[i], 1.0),
                
                // far face
                Vector4f(xf,   yf, m_cascadeEnd[i + 1], 1.0),
                Vector4f(-xf,  yf, m_cascadeEnd[i + 1], 1.0),
                Vector4f(xf,  -yf, m_cascadeEnd[i + 1], 1.0),
                Vector4f(-xf, -yf, m_cascadeEnd[i + 1], 1.0)            
            };

            Vector4f frustumCornersL[NUM_FRUSTUM_CORNERS];

            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::min();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::min();
            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::min();

            for (uint j = 0 ; j < NUM_FRUSTUM_CORNERS ; j++) {
                printf("Frustum: ");
                Vector4f vW = CamInv * frustumCorners[j];
                vW.Print();
                printf("Light space: ");
                frustumCornersL[j] = LightM * vW;
                frustumCornersL[j].Print();
                printf("\n");

                minX = min(minX, frustumCornersL[j].x);
                maxX = max(maxX, frustumCornersL[j].x);
                minY = min(minY, frustumCornersL[j].y);
                maxY = max(maxY, frustumCornersL[j].y);
                minZ = min(minZ, frustumCornersL[j].z);
                maxZ = max(maxZ, frustumCornersL[j].z);
            }

            printf("BB: %f %f %f %f %f %f\n", minX, maxX, minY, maxY, minZ, maxZ);
            
            m_shadowOrthoProjInfo[i].r = maxX;
            m_shadowOrthoProjInfo[i].l = minX;
            m_shadowOrthoProjInfo[i].b = minY;
            m_shadowOrthoProjInfo[i].t = maxY;
            m_shadowOrthoProjInfo[i].f = maxZ;
            m_shadowOrthoProjInfo[i].n = minZ;
		}
	}
        
    LightingTechnique m_LightingTech;
    CSMTechnique m_ShadowMapEffect;
    Camera* m_pGameCamera;
    DirectionalLight m_dirLight;
    BasicMesh m_mesh;
    Orientation m_meshOrientation[NUM_MESHES];
	BasicMesh m_quad;
    Texture* m_pGroundTex;
    CascadedShadowMapFBO m_csmFBO;
    PersProjInfo m_persProjInfo;
    OrthoProjInfo m_shadowOrthoProjInfo[NUM_CASCADES];
    float m_cascadeEnd[NUM_CASCADES + 1];
    ATB m_atb;
    TwBar *bar;
};


/*int main(int argc, char** argv)
{
    OgldevBackendInit(OGLDEV_BACKEND_TYPE_GLFW, argc, argv, true, false);

    if (!OgldevBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false, "Tutorial 49")) {
        OgldevBackendTerminate();
		return 1;
    }

    SRANDOM;
    
    Tutorial50* pApp = new Tutorial50();

    if (!pApp->Init()) {
		delete pApp;
		OgldevBackendTerminate();
        return 1;
    }

    pApp->Run();

    delete pApp;

	OgldevBackendTerminate();
 
    return 0;
}*/
#endif



class OgldevVulkanApp
{
public:
    OgldevVulkanApp(const char* pAppName);
    
    ~OgldevVulkanApp();
    
    bool Init();       
    
private:
#ifndef WIN32    
    void CreateWindow();
#endif    
    void EnumExt();
    void EnumDevices();
    void CreateInstance();
    void CreateDevice();
    void CreateSurface();
    
    VkInstance m_inst;
    std::string m_appName;
    std::vector<VkPhysicalDevice> m_physDevices;
    int m_gfxDevIndex;
    VkDevice m_device;
    std::vector<std::string> m_instExt;
    std::vector<std::string> m_devExt;
    VkSurfaceKHR m_surface;
    VkFormat m_colorFormat;
#ifdef WIN32
    fsdfs
#else
    xcb_connection_t* m_pXCBConn;
    xcb_screen_t* m_pXCBScreen;
    xcb_window_t m_xcbWindow;           
    xcb_intern_atom_reply_t* m_pXCBDelWin;
#endif    
};


OgldevVulkanApp::OgldevVulkanApp(const char* pAppName)
{
    m_appName = std::string(pAppName);
    m_gfxDevIndex = -1;
}


OgldevVulkanApp::~OgldevVulkanApp()
{
    
}

#ifndef WIN32
void OgldevVulkanApp::CreateWindow()
{
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    m_pXCBConn = xcb_connect(NULL, &scr);
    
    if (m_pXCBConn == NULL) {
        printf("Error opening xcb connection\n");
        assert(0);
    }
    
    printf("XCB connection opened\n");

    setup = xcb_get_setup(m_pXCBConn);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);

    m_pXCBScreen = iter.data;    
    
    printf("XCB screen %p\n", m_pXCBScreen);
    
    uint32_t value_mask, value_list[32];

    m_xcbWindow = xcb_generate_id(m_pXCBConn);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = m_pXCBScreen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(m_pXCBConn, 
                      XCB_COPY_FROM_PARENT, 
                      m_xcbWindow,
                      m_pXCBScreen->root, 
                      0, 
                      0, 
                      WINDOW_WIDTH, 
                      WINDOW_HEIGHT, 
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, 
                      m_pXCBScreen->root_visual,
                      value_mask, 
                      value_list);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_pXCBConn, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply =  xcb_intern_atom_reply(m_pXCBConn, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_pXCBConn, 0, 16, "WM_DELETE_WINDOW");
    m_pXCBDelWin = xcb_intern_atom_reply(m_pXCBConn, cookie2, 0);

    xcb_change_property(m_pXCBConn, 
                        XCB_PROP_MODE_REPLACE, 
                        m_xcbWindow,
                        (*reply).atom, 
                        4, 
                        32, 
                        1,
                        &(m_pXCBDelWin->atom));
    free(reply);

    xcb_map_window(m_pXCBConn, m_xcbWindow);    
    
    printf("Window %x created\n", m_xcbWindow);
}
#endif

void OgldevVulkanApp::EnumExt()
{
    uint NumExt = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, NULL);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating extensions");
        assert(0);
    }
    
    printf("Found %d extensions\n", NumExt);
    
    std::vector<VkExtensionProperties> ExtProps(NumExt);

    res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, &ExtProps[0]);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating extensions");
        assert(0);
    }
        
    for (uint i = 0 ; i < NumExt ; i++) {
        printf("Instance extension %d - %s\n", i, ExtProps[i].extensionName);
        m_instExt.push_back(std::string(ExtProps[i].extensionName));
    }
}

void OgldevVulkanApp::EnumDevices()
{
    uint NumDevices = 0;
    
    VkResult res = vkEnumeratePhysicalDevices(m_inst, &NumDevices, NULL);
    
    if (res != VK_SUCCESS) {
        OGLDEV_ERROR("vkEnumeratePhysicalDevices error");
    }
    
    printf("Num physical devices %d\n", NumDevices);
    
    m_physDevices.reserve(NumDevices);
    
    res = vkEnumeratePhysicalDevices(m_inst, &NumDevices, &m_physDevices[0]);
    
    if (res != VK_SUCCESS) {
        OGLDEV_ERROR("vkEnumeratePhysicalDevices");
    }
    
    VkPhysicalDeviceProperties DeviceProperties;
    
    for (uint i = 0 ; i < NumDevices ; i++) {
        memset(&DeviceProperties, 0, sizeof(DeviceProperties));
        vkGetPhysicalDeviceProperties(m_physDevices[i], &DeviceProperties);
        
        printf("Device name: %s\n", DeviceProperties.deviceName);
        uint32_t apiVer = DeviceProperties.apiVersion;
        printf("API version: %d.%d.%d\n", VK_VERSION_MAJOR(apiVer),
                                          VK_VERSION_MINOR(apiVer),
                                          VK_VERSION_PATCH(apiVer));
        uint NumQFamily = 0;         
        vkGetPhysicalDeviceQueueFamilyProperties(m_physDevices[i], &NumQFamily, NULL);
        printf("Num of family queues: %d\n", NumQFamily);
        
        std::vector<VkQueueFamilyProperties> QFamilyProp(NumQFamily);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physDevices[i], &NumQFamily, &QFamilyProp[0]);
        
        for (uint j = 0 ; j < NumQFamily ; j++) {
            printf("Family %d Num queues: %d\n", j, QFamilyProp[j].queueCount);
            VkQueueFlags flags = QFamilyProp[j].queueFlags;
            printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                    (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                    (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");
            
            if ((flags & VK_QUEUE_GRAPHICS_BIT) && (m_gfxDevIndex == -1)) {
                m_gfxDevIndex = i;
                printf("Using GFX device %d\n", m_gfxDevIndex);
            }
        }
    }
    
    if (m_gfxDevIndex == -1) {
        printf("No GFX device found!\n");
        assert(0);
    }    
    
    uint NumExt = 0;
    
    VkPhysicalDevice& gfxPhysDev = m_physDevices[m_gfxDevIndex];
    
    res = vkEnumerateDeviceExtensionProperties(gfxPhysDev, NULL, &NumExt, NULL);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating device extensions %x\n", res);
        assert(0);
    }
    
    std::vector<VkExtensionProperties> ExtProps(NumExt);

    res = vkEnumerateDeviceExtensionProperties(gfxPhysDev, NULL, &NumExt, &ExtProps[0]);
    
    if (res != VK_SUCCESS) {
        printf("Error enumerating extensions");
        assert(0);
    }
        
    for (uint i = 0 ; i < NumExt ; i++) {
        printf("Device extension %d - %s\n", i, ExtProps[i].extensionName);
        m_devExt.push_back(std::string(ExtProps[i].extensionName));
    }            
}


void OgldevVulkanApp::CreateInstance()
{
    VkApplicationInfo appInfo;
    VkInstanceCreateInfo instCreateInfo;
        
    ZERO_MEM_VAR(appInfo);
    ZERO_MEM_VAR(instCreateInfo);
    
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_appName.c_str();
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const char* pInstExt[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
    #ifdef _WIN32    
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #else    
        VK_KHR_XCB_SURFACE_EXTENSION_NAME
    #endif            
    };
    
    instCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instCreateInfo.pApplicationInfo = &appInfo;
    instCreateInfo.enabledExtensionCount = ARRAY_SIZE_IN_ELEMENTS(pInstExt);
    instCreateInfo.ppEnabledExtensionNames = pInstExt;         

    VkResult res = vkCreateInstance(&instCreateInfo, NULL, &m_inst);
    
    if (res != VK_SUCCESS) {
        OGLDEV_ERROR("Failed to create instance");
        assert(0);
    }    
}


void OgldevVulkanApp::CreateDevice()
{
    VkDeviceQueueCreateInfo qInfo;
    ZERO_MEM_VAR(qInfo);
    qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    
    float qPriorities = 1.0f;
    qInfo.queueCount = 1;
    qInfo.pQueuePriorities = &qPriorities;

    const char* pDevExt[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    VkDeviceCreateInfo devInfo;
    ZERO_MEM_VAR(devInfo);
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.enabledExtensionCount = 1;
    devInfo.ppEnabledExtensionNames = pDevExt;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &qInfo;
    
    printf("%d\n",devInfo.enabledExtensionCount );
    
    VkResult res = vkCreateDevice(m_physDevices[m_gfxDevIndex], &devInfo, NULL, &m_device);
    
    if (res != VK_SUCCESS) {
        printf("Error creating device\n");
        assert(0);
    }
   
    printf("Device created\n");
}


void OgldevVulkanApp::CreateSurface()
{
#ifdef WIN32
    sfsdfsdfsd
#else
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
    ZERO_MEM_VAR(surfaceCreateInfo);
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = m_pXCBConn;
    surfaceCreateInfo.window = m_xcbWindow;
    
    VkResult res = vkCreateXcbSurfaceKHR(m_inst, &surfaceCreateInfo, NULL, &m_surface);
    
    if (res != VK_SUCCESS) {
        printf("Error creating surface\n");
        assert(0);
    }
    
    printf("Surface created\n");
#endif
}


bool OgldevVulkanApp::Init()
{
#ifndef WIN32
    CreateWindow();
#endif    
    EnumExt();
    CreateInstance();
    EnumDevices();
    CreateDevice();
    CreateSurface();
    
    return true;
}


class Tutorial50 : public OgldevVulkanApp
{
public:
    Tutorial50(const char* pAppName);
    
    ~Tutorial50();
};


Tutorial50::Tutorial50(const char* pAppName) : OgldevVulkanApp(pAppName)
{
    
}

Tutorial50::~Tutorial50()
{
    
}

int main(int argc, char** argv)
{
    Tutorial50 app("Tutorial 50");
    
    if (!app.Init()) {
        return 1;
    }        
    
    return 0;
}


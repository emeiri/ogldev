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
*/

#include <string>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "Int/assimp_model_loader.h"
#include "3rdparty/meshoptimizer/src/meshoptimizer.h"

static bool UseMeshOptimizer = false;

#define DEMOLITION_ASSIMP_LOAD_FLAGS (aiProcess_JoinIdenticalVertices | \
                                      aiProcess_Triangulate | \
                                      aiProcess_GenBoundingBoxes | \
                                      aiProcess_GenSmoothNormals | \
                                      aiProcess_LimitBoneWeights | \
                                      aiProcess_SplitLargeMeshes | \
                                      aiProcess_ImproveCacheLocality | \
                                      aiProcess_RemoveRedundantMaterials | \
                                      aiProcess_FindDegenerates | \
                                      aiProcess_FindInvalidData | \
                                      aiProcess_GenUVCoords | \
                                      aiProcess_CalcTangentSpace)

int traverse(int depth, aiNode* pNode)
{
    if (!pNode) {
        return 0;
    }

#ifdef DEBUG_SCENE_HIERARCHY
    for (int i = 0; i < depth; i++) {
        printf(" ");
    }

    printf("%s\n", pNode->mName.C_Str());
#endif

#ifdef DEBUG_SCENE_HIERARCHY
    Matrix4f NodeTransformation(pNode->mTransformation);
    NodeTransformation.Print();
#endif

    int Count = 1;

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        Count += traverse(depth + 1, pNode->mChildren[i]);
    }

    return Count;
}



Vector3f VectorFromAssimpVector(const aiVector3D& v)
{
    Vector3f ret;

    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;

    return ret;
}


uint CountValidFaces(const aiMesh& Mesh)
{
    uint NumValidFaces = 0;

    for (uint i = 0; i < Mesh.mNumFaces; i++) {
        if (Mesh.mFaces[i].mNumIndices == 3) {
            NumValidFaces++;
        }
    }

    return NumValidFaces;
}


bool GetFullTransformation(const aiNode* pRootNode, const char* pName, Matrix4f& Transformation)
{
    Transformation.InitIdentity();

    const aiNode* pNode = pRootNode->FindNode(pName);

    if (!pNode) {
        printf("Warning! Cannot find a node for '%s'\n", pName);
        return false;
    }

    while (pNode) {
        Matrix4f CurTransformation(pNode->mTransformation);
        Transformation = CurTransformation * Transformation;
        pNode = pNode->mParent;
    }

    return true;
}



void AssimpModelLoader::Load(const std::string& Filename, const ModelLoadFlags& Flags, ModelData& OutModelData, int WindowWidth, int WindowHeight)
{
    m_filename = Filename;
    m_pModelData = &OutModelData;
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;

#ifndef OGLDEV_VULKAN
    std::string FileExt = Filename.substr(Filename.find_last_of(".") + 1);
    m_flipTexCoordV = (FileExt == "glb" || FileExt == "gltf");
#endif

    uint LoadFlags = DEMOLITION_ASSIMP_LOAD_FLAGS;

    if (Flags.ConvertToLeftHanded) {
        LoadFlags |= aiProcess_ConvertToLeftHanded;
    }

    m_pImporter = new Assimp::Importer();
    m_pScene = m_pImporter->ReadFile(Filename.c_str(), LoadFlags);

    if (m_pScene) {
        printf("--- START Node Hierarchy ---\n");
        int NumNodes = traverse(0, m_pScene->mRootNode);
        printf("Num nodes: %d\n", NumNodes);
        printf("--- END Node Hierarchy ---\n");
        m_pModelData->m_globalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_pModelData->m_globalInverseTransform = m_pModelData->m_globalInverseTransform.Inverse();
        InitFromScene();
        m_pImporter->FreeScene();
    } else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_pImporter->GetErrorString());
    }

    delete m_pImporter;
    m_pImporter = NULL;
}


void AssimpModelLoader::InitFromScene()
{
    InitGeometry();

    InitCameras();

    InitLights();
}


void AssimpModelLoader::InitGeometry()
{
    printf("\n*** Initializing geometry ***\n");
    m_numAnimations = m_pScene->mNumAnimations;
    m_pModelData->m_meshes.resize(m_pScene->mNumMeshes);
    //m_pModelData->m_materials.resize(m_pScene->mNumMaterials);

    uint NumVertices = 0;
    uint NumIndices = 0;

    CountVerticesAndIndices(NumVertices, NumIndices);

    //    InitMaterials();

    InitMeshes(NumVertices, NumIndices);
}


void AssimpModelLoader::CountVerticesAndIndices(uint& NumVertices, uint& NumIndices)
{
    for (unsigned int i = 0; i < m_pModelData->m_meshes.size(); i++) {
        m_pModelData->m_meshes[i].MaterialIndex = m_pScene->mMeshes[i]->mMaterialIndex;
        m_pModelData->m_meshes[i].ValidFaces = CountValidFaces(*m_pScene->mMeshes[i]);

        NumVertices += m_pScene->mMeshes[i]->mNumVertices;
        NumIndices += m_pModelData->m_meshes[i].NumIndices;
    }
}


void AssimpModelLoader::InitMeshes(unsigned int NumVertices, unsigned int NumIndices)
{
    printf("Num animations %d\n", m_numAnimations);

    m_pModelData->m_numAnimations = m_pScene->mNumAnimations;
    m_pModelData->m_indices.reserve(NumIndices);

    if (m_pScene->mNumAnimations > 0) {
        m_pModelData->m_skinnedVertices.reserve(NumVertices);
        InitializeRequiredNodeMap(m_pScene->mRootNode);
        InitAllMeshes(m_pModelData->m_skinnedVertices);
    } else {
        m_pModelData->m_vertices.reserve(NumVertices);
        InitAllMeshes(m_pModelData->m_vertices);
    }
}


template<typename VertexType>
void AssimpModelLoader::InitAllMeshes(std::vector<VertexType>& Vertices)
{
    for (unsigned int i = 0; i < m_pModelData->m_meshes.size(); i++) {
        const aiMesh* paiMesh = m_pScene->mMeshes[i];

        aiAABB BBox = paiMesh->mAABB;
        m_pModelData->m_meshes[i].Dims.BboxMin = glm::vec3(BBox.mMin.x, BBox.mMin.y, BBox.mMin.z);
        m_pModelData->m_meshes[i].Dims.BboxMax = glm::vec3(BBox.mMax.x, BBox.mMax.y, BBox.mMax.z);

        float Width = BBox.mMax.x - BBox.mMin.x;
        float Height = BBox.mMax.y - BBox.mMin.y;
        float Depth = BBox.mMax.z - BBox.mMin.z;
        m_pModelData->m_meshes[i].Dims.Size = glm::vec3(Width, Height, Depth);

        if (UseMeshOptimizer) {
            InitSingleMeshOpt<VertexType>(Vertices, i, paiMesh);
        } else {
            InitSingleMesh<VertexType>(Vertices, i, paiMesh);
        }
    }
}


template<typename VertexType>
void AssimpModelLoader::InitSingleMesh(std::vector<VertexType>& Vertices, uint MeshIndex, const aiMesh* paiMesh)
{
    printf("Mesh %d: %s\n", MeshIndex, paiMesh->mName.C_Str());
    printf("Size: %f, %f, %f\n", m_pModelData->m_meshes[MeshIndex].Dims.Size.x, m_pModelData->m_meshes[MeshIndex].Dims.Size.y, m_pModelData->m_meshes[MeshIndex].Dims.Size.z);

    // Update exact data tracking directly from current dynamic array size to prevent offset corruption
    m_pModelData->m_meshes[MeshIndex].BaseVertex = (uint)(Vertices.size());
    m_pModelData->m_meshes[MeshIndex].BaseIndex = (uint)(m_pModelData->m_indices.size());

    unsigned int AddedVertices = 0;
    unsigned int AddedIndices = 0;

    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        VertexType v;
        const aiVector3D& Pos = paiMesh->mVertices[i];
        v.Position = Vector3f(Pos.x, Pos.y, Pos.z);

        if (paiMesh->mNormals) {
            v.Normal = Vector3f(paiMesh->mNormals[i].x, paiMesh->mNormals[i].y, paiMesh->mNormals[i].z);
        } else {
            v.Normal = Vector3f(0.0f, 1.0f, 0.0f);
        }

        v.TexCoords0 = paiMesh->HasTextureCoords(0) ? Vector2f(paiMesh->mTextureCoords[0][i].x, paiMesh->mTextureCoords[0][i].y) :
            Vector2f(0.0f);
        v.TexCoords1 = paiMesh->HasTextureCoords(1) ? Vector2f(paiMesh->mTextureCoords[1][i].x, paiMesh->mTextureCoords[1][i].y) :
            Vector2f(0.0f);
        if (m_flipTexCoordV) {
            v.TexCoords0.y = 1.0f - v.TexCoords0.y;
            v.TexCoords1.y = 1.0f - v.TexCoords1.y;
        }

        v.Tangent = paiMesh->mTangents ? Vector3f(paiMesh->mTangents[i].x, paiMesh->mTangents[i].y, paiMesh->mTangents[i].z) :
            Vector3f(0.0f);
        v.Bitangent = paiMesh->mBitangents ? Vector3f(paiMesh->mBitangents[i].x, paiMesh->mBitangents[i].y, paiMesh->mBitangents[i].z) :
            Vector3f(0.0f);
        if (paiMesh->mColors[0]) {
            const aiColor4D& Color = paiMesh->mColors[0][i];
            v.Color = Vector4f(Color.r, Color.g, Color.b, Color.a);
        } else {
            v.Color = Vector4f(1.0f);
        }

        //printf("Pos %d: ", i); v.Position.Print();
      //  printf("Normal: "); v.Normal.Print();
      //  printf("Tangent: "); v.Tangent.Print();
      //  printf("Bitangent: "); v.Bitangent.Print();
     //   printf("Color: "); v.Color.Print();

        Vertices.push_back(v);
        AddedVertices++;
    }

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        //  printf("num indices %d\n", Face.mNumIndices);
        if (Face.mNumIndices != 3) {
            continue; // Safely skip non-triangle data without corrupting pre-calculated indices
        }
        /*   printf("%d: %d\n", i * 3, Face.mIndices[0]);
           printf("%d: %d\n", i * 3 + 1, Face.mIndices[1]);
           printf("%d: %d\n", i * 3 + 2, Face.mIndices[2]);*/
        m_pModelData->m_indices.push_back(Face.mIndices[0]);
        m_pModelData->m_indices.push_back(Face.mIndices[1]);
        m_pModelData->m_indices.push_back(Face.mIndices[2]);
        AddedIndices += 3;
    }

    // Re-assign accurate post-parsing count figures
    m_pModelData->m_meshes[MeshIndex].NumVertices = AddedVertices;
    m_pModelData->m_meshes[MeshIndex].NumIndices = AddedIndices;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        LoadMeshBones(Vertices, MeshIndex, paiMesh);
    }
}


template<typename VertexType>
void AssimpModelLoader::InitSingleMeshOpt(std::vector<VertexType>& AllVertices, uint MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    std::vector<VertexType> Vertices(paiMesh->mNumVertices);
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        VertexType v;
        const aiVector3D& Pos = paiMesh->mVertices[i];

        v.Position = Vector3f(Pos.x, Pos.y, Pos.z);

        if (paiMesh->mNormals) {
            const aiVector3D& pNormal = paiMesh->mNormals[i];
            v.Normal = Vector3f(pNormal.x, pNormal.y, pNormal.z);
        } else {
            v.Normal = Vector3f(0.0f, 1.0f, 0.0f);
        }

        const aiVector3D& pTexCoord0 = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords0 = Vector2f(pTexCoord0.x, pTexCoord0.y);

        const aiVector3D& pTexCoord1 = paiMesh->HasTextureCoords(1) ? paiMesh->mTextureCoords[1][i] : Zero3D;
        v.TexCoords1 = Vector2f(pTexCoord1.x, pTexCoord1.y);

        v.Tangent = paiMesh->mTangents ? Vector3f(paiMesh->mTangents[i].x, paiMesh->mTangents[i].y, paiMesh->mTangents[i].z) : Vector3f(0.0f);
        v.Bitangent = paiMesh->mBitangents ? Vector3f(paiMesh->mBitangents[i].x, paiMesh->mBitangents[i].y, paiMesh->mBitangents[i].z) : Vector3f(0.0f);

        if (paiMesh->mColors[0]) {
            const aiColor4D& Color = paiMesh->mColors[0][i];
            v.Color = Vector4f(Color.r, Color.g, Color.b, Color.a);
        } else {
            v.Color = Vector4f(1.0f);
        }

        /*   printf("Pos %d: ", i); v.Position.Print();
           printf("Normal: "); v.Normal.Print();
           printf("Tangent: "); v.Tangent.Print();
           printf("Bitangent: "); v.Bitangent.Print();*/

        Vertices[i] = v;
    }

    m_pModelData->m_meshes[MeshIndex].BaseVertex = (uint)AllVertices.size();
    m_pModelData->m_meshes[MeshIndex].BaseIndex = (uint)m_pModelData->m_indices.size();

    std::vector<uint> Indices;
    Indices.reserve(paiMesh->mNumFaces * 3);

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];

        if (Face.mNumIndices != 3) {
            printf("Warning! face %d has %d indices\n", i, Face.mNumIndices);
            continue;
        }

        m_pModelData->m_indices.push_back(Face.mIndices[0]);
        m_pModelData->m_indices.push_back(Face.mIndices[1]);
        m_pModelData->m_indices.push_back(Face.mIndices[2]);
    }

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>) {
        LoadMeshBones(Vertices, MeshIndex, paiMesh);
    }

    OptimizeMesh(MeshIndex, Indices, Vertices, AllVertices);
}


template<typename VertexType>
void AssimpModelLoader::OptimizeMesh(int MeshIndex, std::vector<uint>& Indices, std::vector<VertexType>& Vertices,
    std::vector<VertexType>& AllVertices)
{
    size_t NumIndices = Indices.size();
    size_t NumVertices = Vertices.size();
    if (NumIndices == 0 || NumVertices == 0) return;

    // Create a remap table
    std::vector<unsigned int> remap(NumIndices);
    size_t OptVertexCount = meshopt_generateVertexRemap(remap.data(),    // dst addr
        Indices.data(),  // src indices
        NumIndices,      // ...and size
        Vertices.data(), // src vertices
        NumVertices,     // ...and size
        sizeof(VertexType)); // stride
    // Allocate a local index/vertex arrays
    std::vector<uint> OptIndices(NumIndices);
    std::vector<VertexType> OptVertices(OptVertexCount);

    // Optimization #1: remove duplicate vertices    
    meshopt_remapIndexBuffer(OptIndices.data(), Indices.data(), NumIndices, remap.data());

    meshopt_remapVertexBuffer(OptVertices.data(), Vertices.data(), NumVertices, sizeof(VertexType), remap.data());

    // Optimization #2: improve the locality of the vertices
    meshopt_optimizeVertexCache(OptIndices.data(), OptIndices.data(), NumIndices, OptVertexCount);

    // Optimization #3: reduce pixel overdraw
    meshopt_optimizeOverdraw(OptIndices.data(), OptIndices.data(), NumIndices, &(OptVertices[0].Position.x), OptVertexCount, sizeof(VertexType), 1.05f);

    // Optimization #4: optimize access to the vertex buffer
    meshopt_optimizeVertexFetch(OptVertices.data(), OptIndices.data(), NumIndices, OptVertices.data(), OptVertexCount, sizeof(VertexType));

    m_pModelData->m_meshes[MeshIndex].NumVertices = (uint)OptVertexCount;

    float Threshold = 1.0f;
    if (Threshold < 1.0f) { // Only run simplification code if reduction threshold is requested
        size_t TargetIndexCount = (size_t)(NumIndices * Threshold);
        float TargetError = 0.01f;
        std::vector<unsigned int> SimplifiedIndices(OptIndices.size());
        size_t OptIndexCount = meshopt_simplify(SimplifiedIndices.data(), OptIndices.data(), NumIndices, &OptVertices[0].Position.x, OptVertexCount, sizeof(VertexType), TargetIndexCount, TargetError);
        SimplifiedIndices.resize(OptIndexCount);

        m_pModelData->m_indices.insert(m_pModelData->m_indices.end(), SimplifiedIndices.begin(), SimplifiedIndices.end());
        m_pModelData->m_meshes[MeshIndex].NumIndices = (uint)OptIndexCount;
    } else {
        m_pModelData->m_indices.insert(m_pModelData->m_indices.end(), OptIndices.begin(), OptIndices.end());
        m_pModelData->m_meshes[MeshIndex].NumIndices = (uint)NumIndices;
    }

    AllVertices.insert(AllVertices.end(), OptVertices.begin(), OptVertices.end());
}


void AssimpModelLoader::InitCameras()
{
    printf("\n*** Initializing cameras ***\n");
    printf("Loading %d cameras\n", m_pScene->mNumCameras);

    m_pModelData->m_cameras.resize(m_pScene->mNumCameras);

    for (unsigned int i = 0; i < m_pScene->mNumCameras; i++) {
        InitSingleCamera(i);
    }
}


void AssimpModelLoader::InitSingleCamera(int Index)
{
    const aiCamera* pCamera = m_pScene->mCameras[Index];
    printf("Loading Camera: '%s'\n", pCamera->mName.C_Str());

    // 1. Get the hierarchy matrix (Order: Parent * Child)
    Matrix4f Transformation;
    GetFullTransformation(m_pScene->mRootNode, pCamera->mName.C_Str(), Transformation);

    aiMatrix4x4 aiCameraMatrix;
    pCamera->GetCameraMatrix(aiCameraMatrix);
    Matrix4f CameraMatrix(aiCameraMatrix);
    printf("Camera internal transformation:\n");
    CameraMatrix.Print();

    Matrix4f FinalWorld = Transformation;

    printf("Final camera transformation:\n");
    FinalWorld.Print();

    // 3. Extract World Position
    Vector4f Pos4D(0.0f, 0.0f, 0.0f, 1.0f);
    Pos4D = FinalWorld * Pos4D;
    Vector3f FinalPos(pCamera->mPosition.x, pCamera->mPosition.y, -pCamera->mPosition.z);

    // 4. Extract World Direction (Blender Camera Forward is -Z)
    Vector4f Target4D(0.0f, 0.0f, -1.0f, 0.0f);
    Target4D = FinalWorld * Target4D;
    //Vector3f FinalTargetDir = Vector3f(pCamera->mLookAt.x, pCamera->mLookAt.y, -pCamera->mLookAt.z).Normalize();
    Vector3f FinalTargetDir(Transformation.m[0][2], -Transformation.m[1][2], Transformation.m[2][2]);

    FinalTargetDir = FinalTargetDir.Normalize();

    // 5. Extract World Up (Blender Camera Up is +Y)
    Vector4f Up4D(0.0f, 1.0f, 0.0f, 0.0f);
    Up4D = FinalWorld * Up4D;
    Vector3f FinalUp = Vector3f(Up4D.x, Up4D.y, Up4D.z).Normalize();

    // 6. Setup Perspective Projection
    PersProjInfo persProjInfo;
    persProjInfo.zNear = pCamera->mClipPlaneNear;
    persProjInfo.zFar = pCamera->mClipPlaneFar;
    persProjInfo.Width = (float)m_windowWidth;
    persProjInfo.Height = (float)m_windowHeight;

    float FullHorizontalDeg = ToDegree(pCamera->mHorizontalFOV);

    float AspectRatio = persProjInfo.Width / persProjInfo.Height;

    // 3. Mathematical conversion: Full Horizontal FOV -> Full Vertical FOV
    float FullVerticalRad = 2.0f * atan(tan(ToRadian(FullHorizontalDeg) / 2.0f) / AspectRatio);
    float FullVerticalDeg = ToDegree(FullVerticalRad);

    // 4. Assign the FULL vertical degree angle straight to your projection settings
    // (Do NOT divide by 2! glm::perspective handles halving internally via its cotangent math)
    persProjInfo.FOV = FullVerticalDeg;

    m_pModelData->m_cameras[Index].Init(FinalPos.ToGLM(), FinalTargetDir.ToGLM(), FinalUp.ToGLM(), persProjInfo);

    printf("Final Pos: "); FinalPos.Print();
    printf("Final Dir: "); FinalTargetDir.Print();
    printf("Final Up: "); FinalUp.Print();
}


void AssimpModelLoader::InitLights()
{
    printf("\n*** Initializing lights ***\n");

    for (int i = 0; i < (int)m_pScene->mNumLights; i++) {
        InitSingleLight(*m_pScene->mLights[i]);
    }
}


void AssimpModelLoader::InitSingleLight(const aiLight& light)
{
    printf("Init light '%s'\n", light.mName.C_Str());

    switch (light.mType) {

    case aiLightSource_DIRECTIONAL:
        InitDirectionalLight(light);
        break;

    case aiLightSource_POINT:
        InitPointLight(light);
        break;

    case aiLightSource_SPOT:
        InitSpotLight(light);
        break;

    case aiLightSource_AMBIENT:
        printf("Ambient light is not implemented\n");
        exit(0);

    case aiLightSource_AREA:
        printf("Area light is not implemented\n");
        exit(0);

    case aiLightSource_UNDEFINED:
    default:
        printf("Light type is undefined\n");
        exit(0);
    }
}


void AssimpModelLoader::InitDirectionalLight(const aiLight& light)
{
    DirectionalLight l;
    //l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Matrix4f Transformation;
    GetFullTransformation(m_pScene->mRootNode, light.mName.C_Str(), Transformation);

    /* Vector3f Direction = VectorFromAssimpVector(light.mDirection);
     printf("Original direction: "); Direction.Print();
     Vector4f Dir4D(Direction, 0.0f);
     Dir4D = Transformation * Dir4D;
     l.WorldDirection = Dir4D;
     printf("Transformed direction: "); l.WorldDirection.Print();*/

     // Extract the components directly from your row-major matrix layout
     // to bypass Blender's embedded glTF offset rotation
    Vector3f TrueDirection;
    TrueDirection.x = Transformation.m[0][2];
    TrueDirection.y = Transformation.m[1][2];
    TrueDirection.z = Transformation.m[2][2];

    // Normalize to preserve clean calculations for your lighting equations
    l.WorldDirection = TrueDirection.Normalize();

    printf("Adjusted World Direction: "); l.WorldDirection.Print();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    Vector4f Up4D(Up, 0.0f);
    Up4D = Transformation * Up4D;

    l.Up = Vector3f(Up4D);
    printf("Final up: "); l.Up.Print();

    m_pModelData->m_dirLights.push_back(l);
}


void AssimpModelLoader::InitPointLight(const aiLight& light)
{
    PointLight l;
    l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    printf("Point light color [%f %f %f]\n", l.Color.x, l.Color.y, l.Color.z);
    //l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();

    Matrix4f Transformation;

    GetFullTransformation(m_pScene->mRootNode, light.mName.C_Str(), Transformation);

    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;

    l.WorldPosition = Vector3f(Pos4D);
    printf("Final Position: "); l.WorldPosition.Print();

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic;

    printf("Attenuation: constant %f linear %f exp %f\n", l.Attenuation.Constant, l.Attenuation.Linear, l.Attenuation.Exp);

    m_pModelData->m_pointLights.push_back(l);
}


void AssimpModelLoader::InitSpotLight(const aiLight& light)
{
    SpotLight l;
    //l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Matrix4f Transformation;
    GetFullTransformation(m_pScene->mRootNode, light.mName.C_Str(), Transformation);

    /*    Vector3f Direction = VectorFromAssimpVector(light.mDirection);
        printf("Original direction: "); Direction.Print();
        Vector4f Dir4D(Direction, 0.0f);
        Dir4D = Transformation * Dir4D;
        l.WorldDirection = Dir4D;
        printf("Final direction: "); l.WorldDirection.Print();*/

    Vector3f TrueDirection;
    TrueDirection.x = Transformation.m[0][2];
    TrueDirection.y = Transformation.m[1][2];
    TrueDirection.z = Transformation.m[2][2];

    // Normalize to preserve clean calculations for your lighting equations
    l.WorldDirection = TrueDirection.Normalize();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    if (Up.Length() == 0) {
        // Use clean 3D vector literal dimensions for structural comparisons
        if ((l.WorldDirection == Vector3f(0.0f, 1.0f, 0.0f)) || (l.WorldDirection == Vector3f(0.0f, -1.0f, 0.0f))) {
            l.Up = Vector3f(1.0f, 0.0f, 0.0f);
        } else {
            l.Up = Vector3f(0.0f, 1.0f, 0.0f);
        }
    } else {
        Vector4f Up4D(Up, 0.0f);
        Up4D = Transformation * Up4D;
        l.Up = Vector3f(Up4D);
    }

    printf("Final up: "); l.Up.Print();

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();
    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;

    l.WorldPosition = Vector3f(Pos4D);
    printf("Final Position: "); l.WorldPosition.Print();

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic / 100.0f;

    printf("Attenuation: constant %f linear %f exp (adjusted!) %f\n", l.Attenuation.Constant, l.Attenuation.Linear, l.Attenuation.Exp);

    if (light.mAngleInnerCone != light.mAngleOuterCone) {
        printf("Warning!!! Different values for spot light inner/outer cone angles is not supported\n");
    }

    l.Cutoff = ToDegree(light.mAngleOuterCone);

    printf("Cutoff angle %f\n", l.Cutoff);

    m_pModelData->m_spotLights.push_back(l);
}


void AssimpModelLoader::LoadMeshBones(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiMesh* pMesh)
{
    if (pMesh->mNumBones > MAX_BONES) {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n", pMesh->mNumBones, MAX_BONES);
        printf("Make sure to increase the macro MAX_BONES in the C++ header as well as in the shader to the same value\n");
        assert(0);
    }

    // printf("Loading mesh bones %d\n", MeshIndex);
    for (uint i = 0; i < pMesh->mNumBones; i++) {
        // printf("Bone %d %s\n", i, pMesh->mBones[i]->mName.C_Str());
        LoadSingleBone(SkinnedVertices, MeshIndex, pMesh->mBones[i]);
    }
}


void AssimpModelLoader::LoadSingleBone(std::vector<SkinnedVertex>& SkinnedVertices, uint MeshIndex, const aiBone* pBone)
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_pModelData->m_boneInfo.size()) {
        BoneInfo bi(pBone->mOffsetMatrix);
        // bi.OffsetMatrix.Print();
        m_pModelData->m_boneInfo.push_back(bi);
    }

    for (uint i = 0; i < pBone->mNumWeights; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];

        // Fix: Use the isolated mesh layout offset safely.
        // If optimizing with meshopt, ensure this logic runs BEFORE meshopt_remapVertexBuffer maps the array!
        uint GlobalVertexID = m_pModelData->m_meshes[MeshIndex].BaseVertex + vw.mVertexId;

        if (GlobalVertexID < SkinnedVertices.size()) {
            SkinnedVertices[GlobalVertexID].Bones.AddBoneData(BoneId, vw.mWeight);
        }
    }

    MarkRequiredNodesForBone(pBone);
}


void AssimpModelLoader::MarkRequiredNodesForBone(const aiBone* pBone)
{
    std::string NodeName(pBone->mName.C_Str());

    const aiNode* pParent = NULL;

    do {
        auto it = m_pModelData->m_requiredNodeMap.find(NodeName);
        if (it == m_pModelData->m_requiredNodeMap.end()) {
            printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;

        pParent = it->second.pNode->mParent;

        if (pParent) {
            NodeName = std::string(pParent->mName.C_Str());
        }

    } while (pParent);
}


void AssimpModelLoader::InitializeRequiredNodeMap(const aiNode* pNode)
{
    std::string NodeName(pNode->mName.C_Str());

    NodeInfo info(pNode);

    m_pModelData->m_requiredNodeMap[NodeName] = info;

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
    }
}


int AssimpModelLoader::GetBoneId(const aiBone* pBone)
{
    int BoneIndex = 0;
    std::string BoneName(pBone->mName.C_Str());

    if (m_pModelData->m_boneNameToIndexMap.find(BoneName) == m_pModelData->m_boneNameToIndexMap.end()) {
        // Allocate an index for a new bone
        BoneIndex = (int)m_pModelData->m_boneNameToIndexMap.size();
        m_pModelData->m_boneNameToIndexMap[BoneName] = BoneIndex;
    } else {
        BoneIndex = m_pModelData->m_boneNameToIndexMap[BoneName];
    }

    return BoneIndex;
}

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
*/

#include "ogldev_engine_common.h"
#include "Int/demolition_model.h"

using namespace std;

#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

#define DEMOLITION_ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder)

static void traverse(int depth, aiNode* pNode);
static bool GetFullTransformation(const aiNode* pRootNode, const char* pName, Matrix4f& Transformation);

inline Vector3f VectorFromAssimpVector(const aiVector3D& v)
{
    Vector3f ret;

    ret.x = v.x;
    ret.y = v.y;
    ret.z = v.z;

    return ret;
}


DemolitionModel::~DemolitionModel()
{
    Clear();
}


void DemolitionModel::Clear()
{
    if (m_Buffers[0] != 0) {
        glDeleteBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}


bool DemolitionModel::LoadAssimpModel(const string& Filename, int WindowWidth, int WindowHeight)
{
    // Release the previously loaded mesh (if it exists)
    Clear();

    // Create the VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool Ret = false;

    m_pScene = m_Importer.ReadFile(Filename.c_str(), DEMOLITION_ASSIMP_LOAD_FLAGS);

    if (m_pScene) {
        printf("--- START Node Hierarchy ---\n");
        traverse(0, m_pScene->mRootNode);
        printf("--- END Node Hierarchy ---\n");
        m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform = m_GlobalInverseTransform.Inverse();
        Ret = InitFromScene(m_pScene, Filename, WindowWidth, WindowHeight);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}


bool DemolitionModel::InitFromScene(const aiScene* pScene, const string& Filename, int WindowWidth, int WindowHeight)
{
    if (!InitGeometry(pScene, Filename)) {
        return false;
    }    

    InitCameras(pScene, WindowWidth, WindowHeight);

    InitLights(pScene);

    return true;
}


bool DemolitionModel::InitGeometry(const aiScene* pScene, const string& Filename)
{
    printf("\n*** Initializing geometry ***\n");
    m_Meshes.resize(pScene->mNumMeshes);
    m_Materials.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);

    ReserveSpace(NumVertices, NumIndices);

    InitAllMeshes(pScene);

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }

    PopulateBuffers();   

    CalculateMeshTransformations(pScene);

    return GLCheckError();
}


void DemolitionModel::CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        m_Meshes[i].BaseVertex = NumVertices;
        m_Meshes[i].BaseIndex = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Meshes[i].NumIndices;
    }
}


void DemolitionModel::ReserveSpace(unsigned int NumVertices, unsigned int NumIndices)
{
    m_Positions.reserve(NumVertices);
    m_Normals.reserve(NumVertices);
    m_TexCoords.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
    m_Bones.resize(NumVertices); // TODO: only if there are any bones
    InitializeRequiredNodeMap(m_pScene->mRootNode);	
}


void DemolitionModel::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitSingleMesh(pScene, i, paiMesh);
    }
}


void DemolitionModel::CalculateMeshTransformations(const aiScene* pScene)
{
    printf("----------------------------------------\n");
    printf("Calculating mesh transformations\n");
    Matrix4f Transformation;
    Transformation.InitIdentity();

    TraverseNodeHierarchy(Transformation, pScene->mRootNode);
}


void DemolitionModel::TraverseNodeHierarchy(Matrix4f ParentTransformation, aiNode* pNode)
{
    printf("Traversing node '%s'\n", pNode->mName.C_Str());
    Matrix4f NodeTransformation(pNode->mTransformation);

    Matrix4f CombinedTransformation = ParentTransformation * NodeTransformation;

    printf("Combined transformation:\n");
    CombinedTransformation.Print();
    
    if (pNode->mNumMeshes > 0) {
        printf("Num meshes: %d - ", pNode->mNumMeshes);
        for (int i = 0; i < (int)pNode->mNumMeshes; i++) {
            int MeshIndex = pNode->mMeshes[i];
            printf("%d ", MeshIndex);
            m_Meshes[MeshIndex].Transformation = CombinedTransformation;
        }
        printf("\n");
    }
    else {
        printf("No meshes\n");
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        TraverseNodeHierarchy(CombinedTransformation, pNode->mChildren[i]);
    }

}


void DemolitionModel::InitSingleMesh(const aiScene* pScene, uint MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    printf("Mesh %d %s\n", MeshIndex, paiMesh->mName.C_Str());
    // Populate the vertex attribute vectors
    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D& pPos      = paiMesh->mVertices[i];
        // printf("%d: ", i); Vector3f v(pPos.x, pPos.y, pPos.z); v.Print();
        m_Positions.push_back(Vector3f(pPos.x, pPos.y, pPos.z));

        if (paiMesh->mNormals) {
            const aiVector3D& pNormal   = paiMesh->mNormals[i];
            m_Normals.push_back(Vector3f(pNormal.x, pNormal.y, pNormal.z));
        } else {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            m_Normals.push_back(Vector3f(Normal.x, Normal.y, Normal.z));
        }

        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        m_TexCoords.push_back(Vector2f(pTexCoord.x, pTexCoord.y));
    }

    // Populate the index buffer
    for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        //  printf("num indices %d\n", Face.mNumIndices);
        //  assert(Face.mNumIndices == 3);
     /*   printf("%d: %d\n", i * 3, Face.mIndices[0]);
        printf("%d: %d\n", i * 3 + 1, Face.mIndices[1]);
        printf("%d: %d\n", i * 3 + 2, Face.mIndices[2]);*/
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }

    LoadMeshBones(MeshIndex, paiMesh);
}




bool DemolitionModel::InitMaterials(const aiScene* pScene, const string& Filename)
{
    string Dir = GetDirFromFilename(Filename);

    bool Ret = true;

    printf("Num materials: %d\n", pScene->mNumMaterials);

    // Initialize the materials
    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        LoadTextures(Dir, pMaterial, i);

        LoadColors(pMaterial, i);
    }

    return Ret;
}


void DemolitionModel::LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index)
{
    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}


void DemolitionModel::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pDiffuse = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadDiffuseTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                LoadDiffuseTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void DemolitionModel::LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded diffuse texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pDiffuse = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].pDiffuse->Load(buffer_size, paiTexture->pcData);
}


void DemolitionModel::LoadDiffuseTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string p(Path.data);

    for (int i = 0 ; i < p.length() ; i++) {
        if (p[i] == '\\') {
            p[i] = '/';
        }
    }

    if (p.substr(0, 2) == ".\\") {
        p = p.substr(2, p.size() - 2);
    }

    string FullPath = Dir + "/" + p;

    m_Materials[MaterialIndex].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pDiffuse->Load()) {
        printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded diffuse texture '%s' at index %d\n", FullPath.c_str(), MaterialIndex);
    }
}


void DemolitionModel::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pSpecularExponent = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());

            if (paiTexture) {
                LoadSpecularTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                LoadSpecularTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void DemolitionModel::LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded specular texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pSpecularExponent = new Texture(GL_TEXTURE_2D);
    int buffer_size = paiTexture->mWidth;
    m_Materials[MaterialIndex].pSpecularExponent->Load(buffer_size, paiTexture->pcData);
}


void DemolitionModel::LoadSpecularTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string p(Path.data);

    if (p == "C:\\\\") {
        p = "";
    } else if (p.substr(0, 2) == ".\\") {
        p = p.substr(2, p.size() - 2);
    }

    string FullPath = Dir + "/" + p;

    m_Materials[MaterialIndex].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pSpecularExponent->Load()) {
        printf("Error loading specular texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded specular texture '%s'\n", FullPath.c_str());
    }
}

void DemolitionModel::LoadColors(const aiMaterial* pMaterial, int index)
{
    aiColor3D AmbientColor(0.0f, 0.0f, 0.0f);
    Vector3f AllOnes(1.0f, 1.0f, 1.0f);

    int ShadingModel = 0;
    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
        printf("Shading model %d\n", ShadingModel);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
        m_Materials[index].AmbientColor.r = AmbientColor.r;
        m_Materials[index].AmbientColor.g = AmbientColor.g;
        m_Materials[index].AmbientColor.b = AmbientColor.b;
    } else {
        m_Materials[index].AmbientColor = AllOnes;
    }

    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS) {
        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
        m_Materials[index].DiffuseColor.r = DiffuseColor.r;
        m_Materials[index].DiffuseColor.g = DiffuseColor.g;
        m_Materials[index].DiffuseColor.b = DiffuseColor.b;
    }

    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
        m_Materials[index].SpecularColor.r = SpecularColor.r;
        m_Materials[index].SpecularColor.g = SpecularColor.g;
        m_Materials[index].SpecularColor.b = SpecularColor.b;
    }
}


void DemolitionModel::PopulateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
    glGenBuffers(1, &m_boneBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_boneBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(), &m_Bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
                          (const GLvoid*)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}


void DemolitionModel::Render(DemolitionRenderCallbacks* pRenderCallbacks)
{
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
//if (i == 1)
        RenderMesh(i, pRenderCallbacks);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void DemolitionModel::RenderMesh(int MeshIndex, DemolitionRenderCallbacks* pRenderCallbacks)
{
    unsigned int MaterialIndex = m_Meshes[MeshIndex].MaterialIndex;
    assert(MaterialIndex < m_Materials.size());

    if (m_Materials[MaterialIndex].pDiffuse) {
        m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pSpecularExponent) {
        m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);

        if (pRenderCallbacks) {
            pRenderCallbacks->ControlSpecularExponent_CB(true);
        }
    }
    else {
        if (pRenderCallbacks) {
            pRenderCallbacks->ControlSpecularExponent_CB(false);
        }
    }

    if (pRenderCallbacks) {
        if (m_Materials[MaterialIndex].pDiffuse) {
            pRenderCallbacks->DrawStart_CB(MeshIndex);
            pRenderCallbacks->SetMaterial_CB(m_Materials[MaterialIndex]);
        }
        else {
            pRenderCallbacks->DisableDiffuseTexture_CB();
        }

        pRenderCallbacks->SetWorldMatrix_CB(m_Meshes[MeshIndex].Transformation);
    }

    glDrawElementsBaseVertex(GL_TRIANGLES,
                            m_Meshes[MeshIndex].NumIndices,
                            GL_UNSIGNED_INT,
                            (void*)(sizeof(unsigned int) * m_Meshes[MeshIndex].BaseIndex),
                            m_Meshes[MeshIndex].BaseVertex);
}


void DemolitionModel::Render(unsigned int DrawIndex, unsigned int PrimID)
{
    glBindVertexArray(m_VAO);

    unsigned int MaterialIndex = m_Meshes[DrawIndex].MaterialIndex;
    assert(MaterialIndex < m_Materials.size());

    if (m_Materials[MaterialIndex].pDiffuse) {
        m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pSpecularExponent) {
        m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
    }

    glDrawElementsBaseVertex(GL_TRIANGLES,
                             3,
                             GL_UNSIGNED_INT,
                             (void*)(sizeof(unsigned int) * (m_Meshes[DrawIndex].BaseIndex + PrimID * 3)),
                             m_Meshes[DrawIndex].BaseVertex);

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}



// Used only by instancing
void DemolitionModel::Render(unsigned int NumInstances, const Matrix4f* WVPMats, const Matrix4f* WorldMats)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Matrix4f) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].pDiffuse) {
            m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pSpecularExponent) {
            m_Materials[MaterialIndex].pSpecularExponent->Bind(SPECULAR_EXPONENT_UNIT);
        }

        glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
                                          m_Meshes[i].NumIndices,
                                          GL_UNSIGNED_INT,
                                          (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                                          NumInstances,
                                          m_Meshes[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


void DemolitionModel::GetLeadingVertex(uint DrawIndex, uint PrimID, Vector3f& Vertex)
{
    uint MeshIndex = DrawIndex; // Each mesh is rendered in its own draw call

    assert(MeshIndex < m_pScene->mNumMeshes);
    const aiMesh* paiMesh = m_pScene->mMeshes[MeshIndex];

    assert(PrimID < paiMesh->mNumFaces);
    const aiFace& Face = paiMesh->mFaces[PrimID];

    uint LeadingIndex = Face.mIndices[0];

    assert(LeadingIndex < paiMesh->mNumVertices);
    const aiVector3D& Pos = paiMesh->mVertices[LeadingIndex];
    Vertex.x = Pos.x;
    Vertex.y = Pos.y;
    Vertex.z = Pos.z;
}


static void traverse(int depth, aiNode* pNode)
{
    for (int i = 0 ; i < depth ; i++) {
        printf(" ");
    }

    printf("%s\n", pNode->mName.C_Str());

    Matrix4f NodeTransformation(pNode->mTransformation);
    NodeTransformation.Print(); 

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        traverse(depth + 1, pNode->mChildren[i]);
    }
}


void DemolitionModel::InitCameras(const aiScene* pScene, int WindowWidth, int WindowHeight)
{
    printf("\n*** Initializing cameras ***\n");
    printf("Loading %d cameras\n", pScene->mNumCameras);

    m_cameras.resize(pScene->mNumCameras);

    for (unsigned int i = 0; i < pScene->mNumCameras; i++) {
        InitSingleCamera(i, pScene, WindowWidth, WindowHeight);
    }
}


void DemolitionModel::InitSingleCamera(int Index, const aiScene* pScene, int WindowWidth, int WindowHeight)
{
    const aiCamera* pCamera = pScene->mCameras[Index];
    printf("Camera name: '%s'\n", pCamera->mName.C_Str());

    Matrix4f Transformation;
    GetFullTransformation(pScene->mRootNode, pCamera->mName.C_Str(), Transformation);

    aiMatrix4x4 aiCameraMatrix;
    pCamera->GetCameraMatrix(aiCameraMatrix);
    Matrix4f CameraMatrix(aiCameraMatrix);
    printf("Camera internal transformation:\n");
    CameraMatrix.Print();

    Matrix4f ChangeSystem;
    ChangeSystem.InitIdentity();
    ChangeSystem.m[2][2] = -1;

    printf("Final camera transformation:\n");
    //Transformation = Transformation * ChangeSystem * CameraMatrix;
    Transformation.Print();

  //  Transformation =  CameraMatrix * Transformation;

    Vector3f Pos = VectorFromAssimpVector(pCamera->mPosition);
    Vector3f Target = VectorFromAssimpVector(pCamera->mLookAt);
    Vector3f Up = VectorFromAssimpVector(pCamera->mUp);

    printf("Original pos: "); Pos.Print();
    printf("Original target: "); Target.Print();
    printf("Original up: "); Up.Print();

    Vector4f Pos4D(Pos, 1.0f);
    Pos4D = Transformation * Pos4D;
    Vector3f FinalPos = Pos4D;

    Vector4f Target4D(Target, 0.0f);
    Target4D = Transformation * Target4D;
    Vector3f FinalTarget = Target4D;

    Vector4f Up4D(Up, 0.0f);
    Up4D = Transformation * Up4D;
    Vector3f FinalUp = Up4D;

    printf("Final pos: "); FinalPos.Print();
    printf("Final target: "); FinalTarget.Print();
    printf("Final up: "); FinalUp.Print();

    PersProjInfo persProjInfo;
    persProjInfo.zNear = pCamera->mClipPlaneNear;
    persProjInfo.zFar = pCamera->mClipPlaneFar;
    persProjInfo.Width = (float)WindowWidth;
    persProjInfo.Height = (float)WindowHeight;
    persProjInfo.FOV = ToDegree(pCamera->mHorizontalFOV);

    //exit(0);*/

    float AspectRatio = (float)WindowWidth / (float)WindowHeight;

    if (AspectRatio != pCamera->mAspect) {
        printf("Warning! the aspect ratio of the camera is %f while the aspect ratio of the window is %f\n", pCamera->mAspect, AspectRatio);
    }

    m_cameras[Index] = BasicCamera(persProjInfo, FinalPos, FinalTarget, FinalUp);
}


void DemolitionModel::InitLights(const aiScene* pScene)
{   
    printf("\n*** Initializing lights ***\n");

    for (int i = 0; i < (int)pScene->mNumLights; i++) {
        InitSingleLight(pScene, *pScene->mLights[i]);
    }
}


void DemolitionModel::InitSingleLight(const aiScene* pScene, const aiLight& light)
{
    printf("Init light '%s'\n", light.mName.C_Str());

    switch (light.mType) {

    case aiLightSource_DIRECTIONAL:
        InitDirectionalLight(pScene, light);
        break;

    case aiLightSource_POINT:
        InitPointLight(pScene, light);
        break;

    case aiLightSource_SPOT:
        InitSpotLight(pScene, light);
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


static bool GetFullTransformation(const aiNode* pRootNode, const char* pName, Matrix4f& Transformation)
{
    Transformation.InitIdentity();

    const aiNode* pNode = pRootNode->FindNode(pName);
    
    if (!pNode) {
        printf("Warning! Cannot find a node for '%s'\n", pName);
        return false;
    }    

    while (pNode) {
        Matrix4f NodeTransformation(pNode->mTransformation);
        Transformation = NodeTransformation * Transformation;
        pNode = pNode->mParent;
    }

    return true;
}


void DemolitionModel::InitDirectionalLight(const aiScene* pScene, const aiLight& light)
{
    if (m_dirLights.size() > 0) {
        printf("The lighting shader currently supports only a single directional light!\n");
        exit(0);
    }

    DirectionalLight l;
    //l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Matrix4f Transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), Transformation);

    Vector3f Direction = VectorFromAssimpVector(light.mDirection);
    printf("Original direction: "); Direction.Print();
    Vector4f Dir4D(Direction, 0.0f);
    Dir4D = Transformation * Dir4D;
    l.WorldDirection = Dir4D;
    printf("Final direction: "); l.WorldDirection.Print();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    Vector4f Up4D(Up, 0.0f);
    Up4D = Transformation * Up4D;
    l.Up = Up4D;
    printf("Final up: "); l.Up.Print();

    m_dirLights.push_back(l);
}


void DemolitionModel::InitPointLight(const aiScene* pScene, const aiLight& light)
{
    PointLight l;
    l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    //l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();

    Matrix4f Transformation;

    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), Transformation);

    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;
    Vector3f WorldPosition = Pos4D;
    printf("World Position: "); WorldPosition.Print();
    l.WorldPosition = WorldPosition;

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic;

    m_pointLights.push_back(l);
}


void DemolitionModel::InitSpotLight(const aiScene* pScene, const aiLight& light)
{
    SpotLight l;
    //l.Color = Vector3f(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.Color = Vector3f(1.0f);
    l.DiffuseIntensity = 1.0f; // TODO

    Matrix4f Transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), Transformation);

    Vector3f Direction = VectorFromAssimpVector(light.mDirection);
    printf("Original direction: "); Direction.Print();
    Vector4f Dir4D(Direction, 0.0f);
    Dir4D = Transformation * Dir4D;
    l.WorldDirection = Dir4D;
    printf("Final direction: "); l.WorldDirection.Print();

    Vector3f Up = VectorFromAssimpVector(light.mUp);
    printf("Original up: "); Up.Print();
    Vector4f Up4D(Up, 0.0f);
    Up4D = Transformation * Up4D;
    l.Up = Up4D;
    printf("Final up: "); l.Up.Print();

    Vector3f Position = VectorFromAssimpVector(light.mPosition);
    printf("Original Position: "); Position.Print();
    Vector4f Pos4D(Position, 1.0f);
    Pos4D = Transformation * Pos4D;
    Vector3f WorldPosition = Pos4D;
    printf("World Position: "); WorldPosition.Print();
    l.WorldPosition = WorldPosition;

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic;

    printf("Attenuation: constant %f linear %f exp %f\n", l.Attenuation.Constant, l.Attenuation.Linear, l.Attenuation.Exp);

    if (light.mAngleInnerCone != light.mAngleOuterCone) {
        printf("Warning!!! Different values for spot light inner/outer cone angles is not supported\n");
    }

    l.Cutoff = ToDegree(light.mAngleOuterCone);

    printf("Cutoff angle %f\n", l.Cutoff);

    m_spotLights.push_back(l);
}


void DemolitionModel::LoadMeshBones(uint MeshIndex, const aiMesh* pMesh)
{
    if (pMesh->mNumBones > MAX_BONES) {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n", pMesh->mNumBones, MAX_BONES);
        printf("Make sure to increase the macro MAX_BONES in the C++ header as well as in the shader to the same value\n");
        assert(0);
    }

    // printf("Loading mesh bones %d\n", MeshIndex);
    for (uint i = 0 ; i < pMesh->mNumBones ; i++) {
        // printf("Bone %d %s\n", i, pMesh->mBones[i]->mName.C_Str());
        LoadSingleBone(MeshIndex, pMesh->mBones[i]);
    }
}


void DemolitionModel::LoadSingleBone(uint MeshIndex, const aiBone* pBone)
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_BoneInfo.size()) {
        BoneInfo bi(pBone->mOffsetMatrix);
        // bi.OffsetMatrix.Print();
        m_BoneInfo.push_back(bi);
    }

    for (uint i = 0 ; i < pBone->mNumWeights ; i++) {
        const aiVertexWeight& vw = pBone->mWeights[i];
        uint GlobalVertexID = m_Meshes[MeshIndex].BaseVertex + pBone->mWeights[i].mVertexId;
        // printf("%d: %d %f\n",i, pBone->mWeights[i].mVertexId, vw.mWeight);
        m_Bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
    }

    MarkRequiredNodesForBone(pBone);
}


void DemolitionModel::MarkRequiredNodesForBone(const aiBone* pBone)
{
    string NodeName(pBone->mName.C_Str());

    const aiNode* pParent = NULL;

    do {
        map<string,NodeInfo>::iterator it = m_requiredNodeMap.find(NodeName);

        if (it == m_requiredNodeMap.end()) {
            printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;

        pParent = it->second.pNode->mParent;

        if (pParent) {
            NodeName = string(pParent->mName.C_Str());
        }

    } while (pParent);
}


void DemolitionModel::InitializeRequiredNodeMap(const aiNode* pNode)
{
    string NodeName(pNode->mName.C_Str());

    NodeInfo info(pNode);

    m_requiredNodeMap[NodeName] = info;

    for (unsigned int i = 0 ; i < pNode->mNumChildren ; i++) {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
    }
}


int DemolitionModel::GetBoneId(const aiBone* pBone)
{
    int BoneIndex = 0;
    string BoneName(pBone->mName.C_Str());

    if (m_BoneNameToIndexMap.find(BoneName) == m_BoneNameToIndexMap.end()) {
        // Allocate an index for a new bone
        BoneIndex = (int)m_BoneNameToIndexMap.size();
        m_BoneNameToIndexMap[BoneName] = BoneIndex;
    }
    else {
        BoneIndex = m_BoneNameToIndexMap[BoneName];
    }

    return BoneIndex;
}


uint DemolitionModel::FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    for (uint i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void DemolitionModel::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    uint PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    uint NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    if (t1 > AnimationTimeTicks) {
        Out = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }
}


uint DemolitionModel::FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void DemolitionModel::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    uint RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    uint NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    if (t1 > AnimationTimeTicks) {
        Out = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    }

    Out.Normalize();
}


uint DemolitionModel::FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (uint i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (AnimationTimeTicks < t) {
            return i;
        }
    }

    return 0;
}


void DemolitionModel::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    uint ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    uint NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
    if (t1 > AnimationTimeTicks) {
        Out = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    } else {
        float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
        float DeltaTime = t2 - t1;
        float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
        assert(Factor >= 0.0f && Factor <= 1.0f);
        const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }
}


void DemolitionModel::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform, const aiAnimation& Animation)
{
    string NodeName(pNode->mName.data);

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = FindNodeAnim(Animation, NodeName);

    if (pNodeAnim) {
        LocalTransform Transform;
        CalcLocalTransform(Transform, AnimationTimeTicks, pNodeAnim);

        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(Transform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z);
        //        printf("Scaling %f %f %f\n", Transoform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z);

        Matrix4f RotationM = Matrix4f(Transform.Rotation.GetMatrix());

        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);
        //        printf("Translation %f %f %f\n", Transform.Translation.x, Transform.Translation.y, Transform.Translation.z);

        // Combine the above transformations
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
        uint BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        string ChildName(pNode->mChildren[i]->mName.data);

        map<string,NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation, Animation);
        }
    }
}


void DemolitionModel::ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks, const aiNode* pNode, const Matrix4f& ParentTransform,
                                           const aiAnimation& StartAnimation, const aiAnimation& EndAnimation, float BlendFactor)
{
    string NodeName(pNode->mName.data);

    Matrix4f NodeTransformation(pNode->mTransformation);

    const aiNodeAnim* pStartNodeAnim = FindNodeAnim(StartAnimation, NodeName);

    LocalTransform StartTransform;

    if (pStartNodeAnim) {
        CalcLocalTransform(StartTransform, StartAnimationTimeTicks, pStartNodeAnim);
    }

    LocalTransform EndTransform;

    const aiNodeAnim* pEndNodeAnim = FindNodeAnim(EndAnimation, NodeName);

    if ((pStartNodeAnim && !pEndNodeAnim) || (!pStartNodeAnim && pEndNodeAnim)) {
        printf("On the node %s there is an animation node for only one of the start/end animations.\n", NodeName.c_str());
        printf("This case is not supported\n");
        exit(0);
    }

    if (pEndNodeAnim) {
        CalcLocalTransform(EndTransform, EndAnimationTimeTicks, pEndNodeAnim);
    }

    if (pStartNodeAnim && pEndNodeAnim) {
        // Interpolate scaling
        const aiVector3D& Scale0 = StartTransform.Scaling;
        const aiVector3D& Scale1 = EndTransform.Scaling;
        aiVector3D BlendedScaling = (1.0f - BlendFactor) * Scale0 + Scale1 * BlendFactor;
        Matrix4f ScalingM;
        ScalingM.InitScaleTransform(BlendedScaling.x, BlendedScaling.y, BlendedScaling.z);

        // Interpolate rotation
        const aiQuaternion& Rot0 = StartTransform.Rotation;
        const aiQuaternion& Rot1 = EndTransform.Rotation;
        aiQuaternion BlendedRot;
        aiQuaternion::Interpolate(BlendedRot, Rot0, Rot1, BlendFactor);
        Matrix4f RotationM = Matrix4f(BlendedRot.GetMatrix());

        // Interpolate translation
        const aiVector3D& Pos0 = StartTransform.Translation;
        const aiVector3D& Pos1 = EndTransform.Translation;
        aiVector3D BlendedTranslation = (1.0f - BlendFactor) * Pos0 + Pos1 * BlendFactor;
        Matrix4f TranslationM;
        TranslationM.InitTranslationTransform(BlendedTranslation.x, BlendedTranslation.y, BlendedTranslation.z);

        // Combine it all
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    Matrix4f GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end()) {
        uint BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (uint i = 0 ; i < pNode->mNumChildren ; i++) {
        string ChildName(pNode->mChildren[i]->mName.data);

        map<string,NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end()) {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired) {
            ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks,
                                     pNode->mChildren[i], GlobalTransformation, StartAnimation, EndAnimation, BlendFactor);
        }
    }
}


void DemolitionModel::CalcLocalTransform(LocalTransform& Transform, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim)
{
    CalcInterpolatedScaling(Transform.Scaling, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedRotation(Transform.Rotation, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedPosition(Transform.Translation, AnimationTimeTicks, pNodeAnim);
}


void DemolitionModel::GetBoneTransforms(float TimeInSeconds, vector<Matrix4f>& Transforms, unsigned int AnimationIndex)
{
    if (AnimationIndex >= m_pScene->mNumAnimations) {
        printf("Invalid animation index %d, max is %d\n", AnimationIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    Matrix4f Identity;
    Identity.InitIdentity();

    float AnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, AnimationIndex);
    const aiAnimation& Animation = *m_pScene->mAnimations[AnimationIndex];

    ReadNodeHierarchy(AnimationTimeTicks, m_pScene->mRootNode, Identity, Animation);
    Transforms.resize(m_BoneInfo.size());

    for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}


void DemolitionModel::GetBoneTransformsBlended(float TimeInSeconds,
                                           vector<Matrix4f>& BlendedTransforms,
                                           unsigned int StartAnimIndex,
                                           unsigned int EndAnimIndex,
                                           float BlendFactor)
{
    if (StartAnimIndex >= m_pScene->mNumAnimations) {
        printf("Invalid start animation index %d, max is %d\n", StartAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if (EndAnimIndex >= m_pScene->mNumAnimations) {
        printf("Invalid end animation index %d, max is %d\n", EndAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if ((BlendFactor < 0.0f) || (BlendFactor > 1.0f)) {
        printf("Invalid blend factor %f\n", BlendFactor);
        assert(0);
    }

    float StartAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, StartAnimIndex);
    float EndAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, EndAnimIndex);

    const aiAnimation& StartAnimation = *m_pScene->mAnimations[StartAnimIndex];
    const aiAnimation& EndAnimation = *m_pScene->mAnimations[EndAnimIndex];

    Matrix4f Identity;
    Identity.InitIdentity();

    ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks, m_pScene->mRootNode, Identity, StartAnimation, EndAnimation, BlendFactor);

    BlendedTransforms.resize(m_BoneInfo.size());

    for (uint i = 0 ; i < m_BoneInfo.size() ; i++) {
        BlendedTransforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}


float DemolitionModel::CalcAnimationTimeTicks(float TimeInSeconds, unsigned int AnimationIndex)
{
    float TicksPerSecond = (float)(m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond != 0 ? m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    // we need to use the integral part of mDuration for the total length of the animation
    float Duration = 0.0f;
    float fraction = modf((float)m_pScene->mAnimations[AnimationIndex]->mDuration, &Duration);
    float AnimationTimeTicks = fmod(TimeInTicks, Duration);
    return AnimationTimeTicks;
}


const aiNodeAnim* DemolitionModel::FindNodeAnim(const aiAnimation&
                                            Animation, const string& NodeName)
{
    for (uint i = 0 ; i < Animation.mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = Animation.mChannels[i];

        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}

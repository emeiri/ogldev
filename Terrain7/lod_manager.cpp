#include <stdio.h>

#include "lod_manager.h"
#include "demo_config.h"


int LodManager::InitLodManager(int PatchSize, int NumPatchesX, int NumPatchesZ, float WorldScale)
{
    m_patchSize = PatchSize;
    m_numPatchesX = NumPatchesX;
    m_numPatchesZ = NumPatchesZ;
    m_worldScale = WorldScale;

    CalcMaxLOD();

    m_map.resize(NumPatchesX * NumPatchesZ);
    m_regions.resize(m_maxLOD + 1);

    CalcLodRegions();

    return m_maxLOD;
}


void LodManager::CalcMaxLOD()
{
    float patchSizeLog2 = log2f((float)m_patchSize);
    printf("log2 of patch size %d is %f\n", m_patchSize, patchSizeLog2);
    int TempMaxLOD = (int)floorf(patchSizeLog2) - 1;
    printf("TempMax LOD %d\n", TempMaxLOD);
    int MinPatchSizeForLOD = powi(2, TempMaxLOD + 1) + 1;
    printf("Min patch size for LOD %d\n", MinPatchSizeForLOD);
    if (m_patchSize > powi(2, TempMaxLOD + 1) + 1) {
        int SmallerPatch = powi(2, TempMaxLOD) + 1;
        if ((m_patchSize - 1) % (SmallerPatch - 1) != 0) {
            int RecommendedPatchSize = powi(2, TempMaxLOD + 2) + 1;
            printf("Invalid patch size %d. Try to use %d\n", m_patchSize, RecommendedPatchSize);
            exit(0);
        }
        m_maxLOD = TempMaxLOD - 1;
        printf("Warning! - max LOD reduced to %d (patch size %d > required %d)\n", m_maxLOD, m_patchSize, MinPatchSizeForLOD);
    }
    else {
        m_maxLOD = TempMaxLOD;
    }
}


void LodManager::Update(const Vector3f& CameraPos)
{
    UpdateLodMapPass1(CameraPos);
    UpdateLodMapPass2(CameraPos);
    //    PrintLodMap();
}


void LodManager::UpdateLodMapPass1(const Vector3f& CameraPos)
{
    int Step = m_patchSize / 2;

    for (int LodMapZ = 0 ; LodMapZ < m_numPatchesZ ; LodMapZ++) {
        //        printf("%d: ", LodMapZ);
        for (int LodMapX = 0 ; LodMapX < m_numPatchesX ; LodMapX++) {
            int x = LodMapX * (m_patchSize - 1) + Step;
            int z = LodMapZ * (m_patchSize - 1) + Step;

            Vector3f Pos = Vector3f(x * (float)m_worldScale, 0.0f, z * (float)m_worldScale);

            float DistanceToCamera = CameraPos.Distance(Pos);
            //            printf("%f ", DistanceToCamera);
            int Index = LodMapZ * m_numPatchesX + LodMapX;
            assert(Index < m_map.size());

            int CoreLod = DistanceToLod(DistanceToCamera);

            m_map[Index].Core = CoreLod;
        }
        // printf("\n");
    }
}


void LodManager::UpdateLodMapPass2(const Vector3f& CameraPos)
{
    int Step = m_patchSize / 2;

    for (int LodMapZ = 0 ; LodMapZ < m_numPatchesZ ; LodMapZ++) {
        //        printf("%d: ", LodMapZ);
        for (int LodMapX = 0 ; LodMapX < m_numPatchesX ; LodMapX++) {
            int Index = LodMapZ * m_numPatchesX + LodMapX;
            assert(Index < m_map.size());

            int CoreLod = m_map[Index].Core;

            int IndexLeft   = Index;
            int IndexRight  = Index;
            int IndexTop    = Index;
            int IndexBottom = Index;

            if (LodMapX > 0) {
                IndexLeft--;
                assert(IndexLeft < m_map.size());

                if (m_map[IndexLeft].Core > CoreLod) {
                    m_map[Index].Left = 1;
                } else {
                    m_map[Index].Left = 0;
                }
            }

            if (LodMapX < m_numPatchesX - 1) {
                IndexRight++;

                assert(IndexRight < m_map.size());

                if (m_map[IndexRight].Core > CoreLod) {
                    m_map[Index].Right = 1;
                } else {
                    m_map[Index].Right = 0;
                }
            }

            if (LodMapZ > 0) {
                IndexBottom -= m_numPatchesX;

                assert(IndexBottom < m_map.size());

                if (m_map[IndexBottom].Core > CoreLod) {
                    m_map[Index].Bottom = 1;
                } else {
                    m_map[Index].Bottom = 0;
                }
            }

            if (LodMapZ < m_numPatchesZ - 1) {
                IndexTop += m_numPatchesX;

                assert(IndexTop < m_map.size());

                if (m_map[IndexTop].Core > CoreLod) {
                    m_map[Index].Top = 1;
                } else {
                    m_map[Index].Top = 0;
                }
            }
        }
        // printf("\n");
    }
}


void LodManager::PrintLodMap()
{
    for (int LodMapZ = m_numPatchesZ - 1 ; LodMapZ >= 0 ; LodMapZ--) {
        printf("%d: ", LodMapZ);
        for (int LodMapX = 0 ; LodMapX < m_numPatchesX ; LodMapX++) {
            int Index = LodMapZ * m_numPatchesX + LodMapX;
            printf("%d ", m_map[Index].Core);
        }
        printf("\n");
    }
}


int LodManager::DistanceToLod(float Distance)
{
    int Lod = m_maxLOD;

    for (int i = 0 ; i <= m_maxLOD ; i++) {
        if (Distance < m_regions[i]) {
            Lod = i;
            break;
        }
    }

    return Lod;
}


const LodManager::PatchLod& LodManager::GetPatchLod(int PatchX, int PatchZ) const
{
    int LodMapIndex = PatchZ * m_numPatchesX + PatchX;
    assert(LodMapIndex < m_map.size());
    return m_map[LodMapIndex];
}


void LodManager::CalcLodRegions()
{
    int Sum = 0;

    for (int i = 0 ; i <= m_maxLOD ; i++) {
        Sum += (i + 1);
    }

    printf("Sum %d\n", Sum);

    float X = Z_FAR / (float)Sum;

    int Temp = 0;

    for (int i = 0 ; i <= m_maxLOD ; i++) {
        int CurRange = (int)(X * (i + 1));
        m_regions[i] = Temp + CurRange;
        Temp += CurRange;
        printf("%d %d\n", i, m_regions[i]);
    }
}

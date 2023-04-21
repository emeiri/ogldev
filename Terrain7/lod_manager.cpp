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

    PatchLod Zero;
    m_map.InitArray2D(NumPatchesX, NumPatchesZ, Zero);

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

            int CoreLod = DistanceToLod(DistanceToCamera);

            PatchLod* pPatchLOD = m_map.GetAddr(LodMapX, LodMapZ);
            pPatchLOD->Core = CoreLod;

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
            int CoreLod = m_map.Get(LodMapX, LodMapZ).Core;

            int IndexLeft   = LodMapX;
            int IndexRight  = LodMapX;
            int IndexTop    = LodMapZ;
            int IndexBottom = LodMapZ;

            if (LodMapX > 0) {
                IndexLeft--;

                if (m_map.Get(IndexLeft, LodMapZ).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Left = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Left = 0;
                }
            }

            if (LodMapX < m_numPatchesX - 1) {
                IndexRight++;

                if (m_map.Get(IndexRight, LodMapZ).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Right = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Right = 0;
                }
            }

            if (LodMapZ > 0) {
                IndexBottom--;

                if (m_map.Get(LodMapX, IndexBottom).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Bottom = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Bottom = 0;
                }
            }

            if (LodMapZ < m_numPatchesZ - 1) {
                IndexTop++;

                if (m_map.Get(LodMapX, IndexTop).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Top = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Top = 0;
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
            printf("%d ", m_map.Get(LodMapX, LodMapZ).Core);
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
    return m_map.Get(PatchX, PatchZ);
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

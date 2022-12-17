/*

        Copyright 2022 Etay Meiri

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


#ifndef OGLDEV_2D_ARRAY_H
#define OGLDEV_2D_ARRAY_H

#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif

template<typename Type>
class Array2D {
 public:
    Array2D() {}

    Array2D(int Cols, int Rows)
    {
        InitArray2D(Cols, Rows);
    }


    void InitArray2D(int Cols, int Rows)
    {
        m_cols = Cols;
        m_rows = Rows;

        if (m_p) {
            free(m_p);
        }

        m_p = (Type*)malloc(Cols * Rows * sizeof(Type));
    }


    void InitArray2D(int Cols, int Rows, Type InitVal)
    {
        InitArray2D(Cols, Rows);

        for (int i = 0 ; i < Cols * Rows ; i++) {
            m_p[i] = InitVal;
        }
    }


    void InitArray2D(int Cols, int Rows, void* pData)
    {
        m_cols = Cols;
        m_rows = Rows;

        if (m_p) {
            free(m_p);
        }

        m_p = (Type*)pData;
    }


    ~Array2D()
    {
        Destroy();
    }


    void Destroy()
    {
        if (m_p) {
            free(m_p);
            m_p = NULL;
        }
    }

    Type* GetAddr(int Col, int Row) const
    {
#ifndef NDEBUG
        if (Col < 0) {
            printf("%s:%d - negative col %d\n", __FILE__, __LINE__, Col);
        }

        if (Col >= m_cols) {
            printf("%s:%d - column overflow (%d vs %d)\n", __FILE__, __LINE__, Col, m_cols);
            exit(0);
        }

        if (Row < 0) {
            printf("%s:%d - negative row %d\n", __FILE__, __LINE__, Row);
        }

        if (Row >= m_rows) {
            printf("%s:%d - row overflow (%d vs %d)\n", __FILE__, __LINE__, Row, m_rows);
            exit(0);
        }
#endif
        size_t Index = Row * m_cols + Col;

        return &m_p[Index];
    }


    Type* GetBaseAddr() const
    {
        return m_p;
    }


    int GetSize() const
    {
        return m_rows * m_cols;
    }


    int GetSizeInBytes() const
    {
        return GetSize() * sizeof(Type);
    }


    Type Get(int Col, int Row) const
    {
        return *GetAddr(Col, Row);
    }


    Type Get(int Index) const
    {
#ifndef NDEBUG
        if (Index >= m_rows * m_cols) {
            printf("%s:%d - index %d is out of bounds (max size %d)\n", __FILE__, __LINE__, Index, m_rows * m_cols);
            exit(0);
        }
#endif

        return m_p[Index];
    }

    void Set(int Col, int Row, Type Val)
    {
        *GetAddr(Col, Row) = Val;
    }


    void Set(int Index, Type Val)
    {
#ifndef NDEBUG
        if (Index >= m_rows * m_cols) {
            printf("%s:%d - index %d is out of bounds (max size %d)\n", __FILE__, __LINE__, Index, m_rows * m_cols);
            exit(0);
        }
#endif

        m_p[Index] = Val;
    }


    void GetMinMax(Type& Min, Type& Max)
    {
        Max = Min = m_p[0];

        for (int i = 1 ; i < m_rows * m_cols ; i++) {
            if (m_p[i] < Min) {
                Min = m_p[i];
            }

            if (m_p[i] > Max) {
                Max = m_p[i];
            }
        }
    }


    void Normalize(Type MinRange, Type MaxRange)
    {
        Type Min, Max;

        GetMinMax(Min, Max);

        if (Max <= Min) {
            return;
        }

        Type MinMaxDelta = Max - Min;
        Type MinMaxRange = MaxRange - MinRange;

        for (int i = 0 ; i < m_rows * m_cols; i++) {
            m_p[i] = ((m_p[i] - Min)/MinMaxDelta) * MinMaxRange + MinRange;
        }
    }


    void PrintFloat()
    {
        for (int y = 0 ; y < m_rows ; y++) {
            printf("%d: ", y);
            for (int x = 0 ; x < m_cols ; x++) {
                float f = (float)m_p[y * m_cols + x];
                printf("%.6f ", f);
            }
            printf("\n");
        }
    }

 private:
    Type* m_p = NULL;
    int m_cols = 0;
    int m_rows = 0;
};


#endif

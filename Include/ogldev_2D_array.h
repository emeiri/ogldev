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
#include <unistd.h>

template<typename Type>
class OGLDEV2DArray {
 public:
    OGLDEV2DArray(int Cols, int Rows)
    {
        m_cols = Cols;
        m_rows = Rows;

        m_p = (Type*)malloc(Cols * Rows * sizeof(Type));
    }

    ~OGLDEV2DArray()
    {
        free(m_p);
    }

    Type* GetAddr(int Col, int Row) const
    {
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

        size_t Index = Row * m_cols + Col;

        return &m_p[Index];
    }


    Type* GetAddr() const
    {
        return m_p;
    }


    Type Get(int Col, int Row) const
    {
        return *GetAddr(Col, Row);
    }


    void Set(int Col, int Row, Type Val)
    {
        *GetAddr(Col, Row) = Val;
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


    void Normalize(Type MaxRange)
    {
        Type Min, Max;

        GetMinMax(Min, Max);

        if (Max <= Min) {
            return;
        }

        float Delta = Max - Min;

        for (int i = 0 ; i < m_rows * m_cols; i++) {
            m_p[i] = ((m_p[i] - Min)/Delta) * MaxRange;
        }
    }

 private:
    Type* m_p = NULL;
    int m_cols = 0;
    int m_rows = 0;
};


#endif

#ifndef TCL_MATRIX_H
#define TCL_MATRIX_H
#pragma once
#include<iostream>
#include<cmath>
#include<iomanip>
#include<vector>
#pragma warning(disable:4996)
#define PRECISION 1e-5

class Matrix
{
private:
    int rank;
public:
    int row;
    int col;
    double** matrix;

    /// <summary>
    /// 将已有矩阵深复制到对象矩阵中，初始时不知道秩，令秩为-1。
    /// </summary>
    /// <param name="matrix">使用时，可以将double**类型强转为double*类型输入</param>
    /// <param name="row"></param>
    /// <param name="col"></param>
    Matrix(double* newMatrix, int row, int col) :row(row), col(col), rank(-1)
    {
        matrix = new double* [row];
        for (int i = 0; i < row; i++)
            matrix[i] = new double[col];
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
            {
                matrix[i][j] = newMatrix[col * i + j];
            }
    }

    /// <summary>
    /// 创建新矩阵并初始化为0
    /// </summary>
    /// <param name="row"></param>
    /// <param name="col"></param>
    Matrix(int row = 1, int col = 1) :row(row), col(col), rank(-1)
    {
        if (row < 1)
            row = 1;
        if (col < 1)
            col = 1;
        matrix = new double* [row];
        for (int i = 0; i < row; i++)
            matrix[i] = new double[col];
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                matrix[i][j] = 0;
            }
        }
    }

    /// <summary>
    /// 复制构造函数，深复制
    /// </summary>
    /// <param name="A"></param>
    Matrix(const Matrix& A) : row(A.row), col(A.col), rank(A.rank)
    {
        matrix = new double* [row];
        for (int i = 0; i < row; i++)
            matrix[i] = new double[col];
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                matrix[i][j] = A.matrix[i][j];
            }
        }
    }

    ~Matrix()
    {
        for (int i = 0; i < row; i++)
            delete[] matrix[i];
        delete[] matrix;
    }

    /// <summary>
    /// 高斯消元，化为行简化阶梯型
    /// </summary>
    void Gauss_Jordan_Elimination()
    {
        int zeroRow = 0;
        for (int i = 0; i < col && i < row; i++)  //i看成列
        {
            int nonZero = row - 1;  //先设非零行为最后一行
            for (int j = i - zeroRow; j < row; j++) // 找到非0元素
                if (abs(matrix[j][i]) > PRECISION)
                {
                    nonZero = j;
                    break;
                }

            if (abs(matrix[nonZero][i]) > PRECISION)
            {
                if (nonZero != i - zeroRow) //如果非0元素不是第i-zeroRow行
                {
                    for (int k = i; k < col; k++) // 把非0元素所在行交换到当前行
                    {
                        double t = matrix[i - zeroRow][k];
                        matrix[i - zeroRow][k] = matrix[nonZero][k];
                        matrix[nonZero][k] = t;
                    }
                    nonZero = i - zeroRow;
                }

                if (abs(matrix[i - zeroRow][i] - 1.0) > PRECISION)  //归一化
                {
                    double temp = matrix[i - zeroRow][i];
                    for (int k = i; k < col; k++)
                    {
                        matrix[i - zeroRow][k] /= temp;
                    }
                }

                for (int j = i - zeroRow + 1; j < row; j++) //把下面部分消为0
                {
                    if (abs(matrix[j][i]) > PRECISION) //如果matrix[j][i]不是0
                    {
                        double temp = matrix[j][i];
                        for (int k = i; k < col; k++)
                        {
                            matrix[j][k] = matrix[j][k] - matrix[i - zeroRow][k] * temp;
                        }
                    }
                }

                for (int j = i - zeroRow - 1; j >= 0; j--) //把上面部分消为0
                {
                    if (abs(matrix[j][i]) > PRECISION)
                    {
                        double temp = matrix[j][i];
                        for (int k = i; k < col; k++)
                        {
                            matrix[j][k] = matrix[j][k] - matrix[i - zeroRow][k] * temp;
                        }
                    }
                }
            }
            else
            {
                zeroRow++;
            }
        }
        if (col > row)
            rank = row - zeroRow;
        else rank = col - zeroRow;
        if (col > row && rank < row) //若此时不满秩，且col>row，则最后一行可能不会被归一化，且最后一行的秩不会被计算，要单独处理
        {
            int k = row;
            while (abs(matrix[row - 1][k]) < PRECISION && k < col)
            {
                k++;
            }
            if (k < col)
            {
                double temp = matrix[row - 1][k];
                for (int i = k; i < col; i++)
                {
                    matrix[row - 1][i] /= temp;
                }
                rank++; //加上最后一行的秩
                for (int j = row - 2; j >= 0; j--) //把上面部分消为0
                {
                    if (abs(matrix[j][k]) > PRECISION)
                    {
                        double temp = matrix[j][k];
                        for (int t = k; t < col; t++)
                        {
                            matrix[j][t] = matrix[j][t] - matrix[row - 1][t] * temp;
                        }
                    }
                }
            }
        }
    }

    /// <summary>
    /// 若对象为增广矩阵，可用此函数求解
    /// </summary>
    /// <param name="solution">用于接收一个解向量</param>
    /// <returns>若返回值为false，则方程无解，未对返回参数操作</returns>
    bool GetAnswerForAugmentedMatrix(Matrix& ret)
    {
        Matrix original(*this);
        int zeroRow = 0;
        for (int i = 0; i < original.col - 1 && i < original.row; i++)  //i看成列，此时是增广矩阵，i最多到col - 1
        {
            int nonZero = original.row - 1;  //先设非零行为最后一行
            for (int j = i - zeroRow; j < original.row; j++) // 找到非0元素
                if (abs(original.matrix[j][i]) > PRECISION)
                {
                    nonZero = j;
                    break;
                }

            if (abs(original.matrix[nonZero][i]) > PRECISION)
            {
                if (nonZero != i - zeroRow) //如果非0元素不是第i-zeroRow行
                {
                    for (int k = i; k < original.col; k++) // 把非0元素所在行交换到当前行
                    {
                        double t = original.matrix[i - zeroRow][k];
                        original.matrix[i - zeroRow][k] = original.matrix[nonZero][k];
                        original.matrix[nonZero][k] = t;
                    }
                    nonZero = i - zeroRow;
                }

                if (abs(original.matrix[i - zeroRow][i] - 1.0) > PRECISION)  //归一化
                {
                    double temp = original.matrix[i - zeroRow][i];
                    for (int k = i; k < original.col; k++)
                    {
                        original.matrix[i - zeroRow][k] /= temp;
                    }
                }

                for (int j = i - zeroRow + 1; j < original.row; j++) //把下面部分消为0
                {
                    if (abs(original.matrix[j][i]) > PRECISION) //如果matrix[j][i]不是0
                    {
                        double temp = original.matrix[j][i];
                        for (int k = i; k < original.col; k++)
                        {
                            original.matrix[j][k] = original.matrix[j][k] - original.matrix[i - zeroRow][k] * temp;
                        }
                    }
                }

                for (int j = i - zeroRow - 1; j >= 0; j--) //把上面部分消为0
                {
                    if (abs(original.matrix[j][i]) > PRECISION)
                    {
                        double temp = original.matrix[j][i];
                        for (int k = i; k < original.col; k++)
                        {
                            original.matrix[j][k] = original.matrix[j][k] - original.matrix[i - zeroRow][k] * temp;
                        }
                    }
                }
            }
            else
            {
                zeroRow++;
            }
        }
        if (original.col - 1 > original.row)
            original.rank = original.row - zeroRow;
        else original.rank = original.col - 1 - zeroRow;
        if (original.col - 1 > original.row && original.rank < original.row) //若此时原矩阵不满秩，且col - 1>row，则最后一行可能不会被归一化，且最后一行的秩不会被计算，要单独处理
        {
            int k = original.row;
            while (abs(original.matrix[original.row - 1][k]) < PRECISION && k < original.col - 1)
            {
                k++;
            }
            if (k < original.col - 1)
            {
                double temp = original.matrix[original.row - 1][k];
                for (int i = k; i < original.col; i++)
                {
                    original.matrix[original.row - 1][i] /= temp;
                }
                original.rank++; //加上最后一行的秩
                for (int j = original.row - 2; j >= 0; j--) //把上面部分消为0
                {
                    if (abs(original.matrix[j][k]) > PRECISION)
                    {
                        double temp = original.matrix[j][k];
                        for (int t = k; t < original.col; t++)
                        {
                            original.matrix[j][t] = original.matrix[j][t] - original.matrix[original.row - 1][t] * temp;
                        }
                    }
                }
            }
        }

        //先看有没有解，即看行简化阶梯型的全零行对应增广矩阵最右行的元素是否为0
        for (int i = original.row - 1; i >= original.rank; i--)
        {
            if (abs(original.matrix[i][original.col - 1]) > PRECISION)
            {
                return false;
            }
        }

        Matrix solution(col - 1, 1);
        int numOfFreeColumn = 0;

        int temp;  //记录列
        for (temp = 0; temp < original.col - 1 && temp - numOfFreeColumn < original.row; temp++)  //主列位置对应分量为目标向量的对应分量，自由列对应分量直接取0
        {
            if (abs(original.matrix[temp - numOfFreeColumn][temp]) > PRECISION)
            {
                solution.matrix[temp][0] = original.matrix[temp - numOfFreeColumn][original.col - 1];
            }
            else
            {
                numOfFreeColumn++;
                solution.matrix[temp][0] = 0;//自由列位置处取0肯定满足方程
            }
        }

        while (temp < original.col - 1)
        {
            solution.matrix[temp++][0] = 0;
        }
        ret = solution;
        return true;
    }

    /// <summary>
    /// 若对象为增广矩阵，可用此函数找到线性方程组的所有解
    /// </summary>
    /// <param name="ret">前col-1列是零空间的一组基，最后1列是一个特解</param>
    /// <returns>若返回值为false，则方程无解，未对返回参数操作</returns>
    bool GetAllSolutionsForAugmentedMatrix(Matrix& ret)
    {
        Matrix original(*this);
        int zeroRow = 0;
        for (int i = 0; i < original.col - 1 && i < original.row; i++)  //i看成列，此时是增广矩阵，i最多到col - 1
        {
            int nonZero = original.row - 1;  //先设非零行为最后一行
            for (int j = i - zeroRow; j < original.row; j++) // 找到非0元素
                if (abs(original.matrix[j][i]) > PRECISION)
                {
                    nonZero = j;
                    break;
                }

            if (abs(original.matrix[nonZero][i]) > PRECISION)
            {
                if (nonZero != i - zeroRow) //如果非0元素不是第i-zeroRow行
                {
                    for (int k = i; k < original.col; k++) // 把非0元素所在行交换到当前行
                    {
                        double t = original.matrix[i - zeroRow][k];
                        original.matrix[i - zeroRow][k] = original.matrix[nonZero][k];
                        original.matrix[nonZero][k] = t;
                    }
                    nonZero = i - zeroRow;
                }

                if (abs(original.matrix[i - zeroRow][i] - 1.0) > PRECISION)  //归一化
                {
                    double temp = original.matrix[i - zeroRow][i];
                    for (int k = i; k < original.col; k++)
                    {
                        original.matrix[i - zeroRow][k] /= temp;
                    }
                }

                for (int j = i - zeroRow + 1; j < original.row; j++) //把下面部分消为0
                {
                    if (abs(original.matrix[j][i]) > PRECISION) //如果matrix[j][i]不是0
                    {
                        double temp = original.matrix[j][i];
                        for (int k = i; k < original.col; k++)
                        {
                            original.matrix[j][k] = original.matrix[j][k] - original.matrix[i - zeroRow][k] * temp;
                        }
                    }
                }

                for (int j = i - zeroRow - 1; j >= 0; j--) //把上面部分消为0
                {
                    if (abs(original.matrix[j][i]) > PRECISION)
                    {
                        double temp = original.matrix[j][i];
                        for (int k = i; k < original.col; k++)
                        {
                            original.matrix[j][k] = original.matrix[j][k] - original.matrix[i - zeroRow][k] * temp;
                        }
                    }
                }
            }
            else
            {
                zeroRow++;
            }
        }
        if (original.col - 1 > original.row)
            original.rank = original.row - zeroRow;  //这里是指原矩阵的秩，不是增广矩阵的秩。
        else original.rank = original.col - 1 - zeroRow;
        if (original.col - 1 > original.row && original.rank < original.row) //若此时原矩阵不满秩，且col - 1>row，则最后一行可能不会被归一化，且最后一行的秩不会被计算，要单独处理
        {
            int k = original.row;
            while (abs(original.matrix[original.row - 1][k]) < PRECISION && k < original.col - 1)
            {
                k++;
            }
            if (k < original.col - 1)
            {
                double temp = original.matrix[original.row - 1][k];
                for (int i = k; i < original.col; i++)
                {
                    original.matrix[original.row - 1][i] /= temp;
                }
                original.rank++; //加上最后一行的秩
                for (int j = original.row - 2; j >= 0; j--) //把上面部分消为0
                {
                    if (abs(original.matrix[j][k]) > PRECISION)
                    {
                        double temp = original.matrix[j][k];
                        for (int t = k; t < original.col; t++)
                        {
                            original.matrix[j][t] = original.matrix[j][t] - original.matrix[original.row - 1][t] * temp;
                        }
                    }
                }
            }
        }
        //先看有没有解，即看行简化阶梯型的全零行对应增广矩阵最右行的元素是否为0
        for (int i = original.row - 1; i >= original.rank; i--)
        {
            if (abs(original.matrix[i][original.col - 1]) > PRECISION)
            {
                return false;
            }
        }

        Matrix solution(original.col - 1, original.col - original.rank);
        int numOfFreeColumn = 0;
        int temp = 0;
        for (temp = 0; temp < original.col - 1 && temp - numOfFreeColumn < original.row; temp++) //先找零空间的基
        {
            if (abs(original.matrix[temp - numOfFreeColumn][temp]) > PRECISION)
                continue;
            else
            {
                solution.matrix[temp][numOfFreeColumn] = 1;
                for (int j = 0; j < temp - numOfFreeColumn; j++)
                {
                    solution.matrix[j][numOfFreeColumn] = -original.matrix[j][temp];
                }
                numOfFreeColumn++;
            }
        }
        while (temp < original.col - 1)
        {
            solution.matrix[temp][numOfFreeColumn] = 1;
            for (int j = 0; j < original.row; j++)
            {
                solution.matrix[j][numOfFreeColumn] = -original.matrix[j][temp];
            }
            numOfFreeColumn++;
            temp++;
        }

        numOfFreeColumn = 0;
        //找特解
        for (temp = 0; temp < original.col - 1 && temp - numOfFreeColumn < original.row; temp++)  //主列位置对应分量为目标向量的对应分量，自由列对应分量直接取0
        {
            if (abs(original.matrix[temp - numOfFreeColumn][temp]) > PRECISION)
            {
                solution.matrix[temp][original.col - original.rank - 1] = original.matrix[temp - numOfFreeColumn][original.col - 1];
            }
            else
            {
                numOfFreeColumn++;
                solution.matrix[temp][original.col - original.rank - 1] = 0;
            }
        }
        while (temp < original.col - 1)
        {
            solution.matrix[temp++][original.col - original.rank - 1] = 0;
        }
        ret = solution;
        return true;
    }

    /// <summary>
    /// 找到零空间的一组基
    /// </summary>
    /// <param name="bases">用于返回基</param>
    /// <returns>若矩阵可逆，则返回false；否则，返回一组基。注：无论矩阵是否可逆，理论上作为返回值的参数bases都会发生变化。</returns>
    bool GetBasesOfNullSpace(Matrix& bases)
    {
        Matrix ret(row, col + 1);
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                ret.matrix[i][j] = matrix[i][j];
            }
        }
        if (ret.GetAllSolutionsForAugmentedMatrix(ret))
        {
            if (ret.col > 1)
            {
                Matrix nullSpace(ret.row, ret.col - 1);
                for (int i = 0; i < nullSpace.row; i++)
                {
                    for (int j = 0; j < nullSpace.col; j++)
                    {
                        nullSpace.matrix[i][j] = ret.matrix[i][j];
                    }
                }
                bases = nullSpace;
                return true;
            }
            else
            {
                std::cout << "The matrix's null space only contains 0 so that it has no bases." << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "Something went wrong so that the matrix's null space is empty!" << std::endl;
            return false;
        }
    }

    /// <summary>
    /// 行列式，不为方阵时返回0
    /// </summary>
    double Determinant() const
    {
        if (row != col)
            return 0;
        int swapTimes = 0; //行交换次数
        double** mtemp = new double* [row];
        for (int i = 0; i < row; i++)
            mtemp[i] = new double[col];
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
            {
                mtemp[i][j] = matrix[i][j];
            }
        int zeroRow = 0;
        for (int i = 0; i < col && i < row; i++)  //i看成列
        {
            int nonZero = row - 1;  //先设非零行为最后一行
            for (int j = i - zeroRow; j < row; j++) // 找到非0元素
                if (abs(mtemp[j][i]) > PRECISION)
                {
                    nonZero = j;
                    break;
                }

            if (abs(mtemp[nonZero][i]) > PRECISION)
            {
                if (nonZero != i - zeroRow) //如果非0元素不是第i-zeroRow行
                {
                    for (int k = i; k < col; k++) // 把非0元素所在行交换到当前行
                    {
                        double t = mtemp[i - zeroRow][k];
                        mtemp[i - zeroRow][k] = mtemp[nonZero][k];
                        mtemp[nonZero][k] = t;
                    }
                    nonZero = i - zeroRow;
                    swapTimes++;
                }

                for (int j = i - zeroRow + 1; j < row; j++) //把下面部分消为0
                {
                    if (abs(mtemp[j][i]) > PRECISION) //如果mtemp[j][i]不是0
                    {
                        double temp = mtemp[j][i] / mtemp[i - zeroRow][i];
                        for (int k = i; k < col; k++)
                        {
                            mtemp[j][k] = mtemp[j][k] - mtemp[i - zeroRow][k] * temp;
                        }
                    }
                }
            }
        }
        double ret = 1;
        if (abs(mtemp[row - 1][col - 1]) < PRECISION)
            ret = 0;
        else
        {
            for (int i = 0; i < row; i++)
            {
                ret *= mtemp[i][i];
            }
            if (swapTimes % 2)
                ret = -ret;
        }

        for (int i = 0; i < row; i++)
            delete[] mtemp[i];
        delete[]mtemp;
        return ret;
    }

    /// <summary>
    /// 求矩阵的逆矩阵
    /// </summary>
    /// <param name="inverse">inverse应为row*row的矩阵对象，用于接收返回值。若矩阵可逆，则逆矩阵通过该参数返回；若矩阵不可逆，则inverse不会进行任何操作</param>
    /// <returns>返回矩阵是否可逆</returns>
    bool InverseMatrix(Matrix& inverse)
    {
        if (row != col)
            return false;
        if (inverse.row != inverse.col || inverse.row != row)
        {
            std::cout << "The matrix may be invertible, but the second parameter has wrong dimension so it failed to receive the returned value!" << std::endl;
            return false;
        }
        Matrix caculateInverse(row, 2 * row);
        for (int i = 0; i < row; i++)
            for (int j = 0; j < row; j++)
            {
                caculateInverse.matrix[i][j] = matrix[i][j];
                caculateInverse.matrix[i][j + row] = i == j ? 1 : 0;
            }
        caculateInverse.Gauss_Jordan_Elimination();
        if (abs(caculateInverse.matrix[row - 1][row - 1]) < PRECISION) //高斯消元后，右下角为0，则不可逆；否则可逆。
            return false;
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < row; j++)
            {
                inverse.matrix[i][j] = caculateInverse.matrix[i][j + row];
            }
        }
        return true;
    }

    /// <summary>
    /// 矩阵转置
    /// </summary>
    /// <param name="A">参数用于接收返回值</param>
    void Transpose(Matrix& A)
    {
        if (A.row != col || A.col != row)
        {
            double** tranm = new double* [col];
            for (int i = 0; i < col; i++)
            {
                tranm[i] = new double[row];
            }

            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    tranm[j][i] = matrix[i][j];
                }
            }

            for (int i = 0; i < A.row; i++)
                delete[] A.matrix[i];
            delete[] A.matrix;

            A.matrix = tranm;
            A.col = row;
            A.row = col;
        }
        else
        {
            for (int i = 0; i < A.row; i++)
            {
                for (int j = 0; j < A.col; j++)
                {
                    A.matrix[i][j] = matrix[j][i];
                }
            }
        }
    }

    /// <summary>
    /// 矩阵转置
    /// </summary>
    void Transpose()
    {
        if (row != col)
        {
            double** tranm = new double* [col];
            for (int i = 0; i < col; i++)
            {
                tranm[i] = new double[row];
            }

            for (int i = 0; i < row; i++)
            {
                for (int j = 0; j < col; j++)
                {
                    tranm[j][i] = matrix[i][j];
                }
            }

            for (int i = 0; i < row; i++)
                delete[] matrix[i];
            delete[] matrix;

            matrix = tranm;

            int t = col;
            col = row;
            row = t;
        }
        else
        {
            for (int i = 0; i < row; i++)
            {
                for (int j = i + 1; j < col; j++)
                {
                    double t = matrix[j][i];
                    matrix[j][i] = matrix[i][j];
                    matrix[i][j] = t;
                }
            }
        }
    }

    /// <summary>
    /// 矩阵乘法
    /// </summary>
    /// <param name="A"></param>
    /// <param name="B"></param>
    /// <returns>若维度失配，则返回第一个操作数</returns>
    friend Matrix operator *(const Matrix& A, const Matrix& B)
    {
        if (A.col != B.row)
        {
            std::cout << "The dimension does not match! Matrix multiplication failed. Return the first matrix." << std::endl;
            return A;
        }
        Matrix ret(A.row, B.col);
        for (int i = 0; i < ret.row; i++)
        {
            for (int j = 0; j < ret.col; j++)
            {
                for (int k = 0; k < A.col; k++)
                {
                    ret.matrix[i][j] += A.matrix[i][k] * B.matrix[k][j];
                }
            }
        }
        return ret;
    }

    /// <summary>
    /// 矩阵加法
    /// </summary>
    /// <param name="A"></param>
    /// <param name="B"></param>
    /// <returns>若维度失配，则返回第一个操作数</returns>
    friend Matrix operator +(const Matrix& A, const Matrix& B)
    {
        if (A.col != B.col || A.row != B.row)
        {
            std::cout << "The dimension does not match! Matrix addtion failed. Return the first matrix." << std::endl;
            return A;

        }
        Matrix ret(A.row, B.row);
        for (int i = 0; i < ret.row; i++)
        {
            for (int j = 0; j < ret.col; j++)
            {
                ret.matrix[i][j] += A.matrix[i][j] + B.matrix[i][j];
            }
        }
        return ret;
    }

    /// <summary>
    /// 矩阵减法
    /// </summary>
    /// <param name="A"></param>
    /// <param name="B"></param>
    /// <returns>若维度失配，则返回第一个操作数</returns>
    friend Matrix operator -(const Matrix& A, const Matrix& B)
    {
        if (A.col != B.col || A.row != B.row)
        {
            std::cout << "The dimension does not match! Matrix subtraction failed. Return the first matrix." << std::endl;
            return A;
        }
        Matrix ret(A.row, B.row);
        for (int i = 0; i < ret.row; i++)
        {
            for (int j = 0; j < ret.col; j++)
            {
                ret.matrix[i][j] += A.matrix[i][j] - B.matrix[i][j];
            }
        }
        return ret;
    }

    Matrix operator =(const Matrix& A)
    {
        rank = A.rank;
        if (A.col != col || A.row != row)
        {
            for (int i = 0; i < row; i++)
                delete[] matrix[i];
            delete[] matrix;
            row = A.row;
            col = A.col;
            matrix = new double* [row];
            for (int i = 0; i < row; i++)
                matrix[i] = new double[col];
        }
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                matrix[i][j] = A.matrix[i][j];
            }
        }
        return *this;
    }

    /// <summary>
    /// 矩阵的幂，矩阵必须为方阵
    /// </summary>
    /// <param name="n">必须是自然数</param>
    /// <returns>若矩阵不为方阵或参数小于0，则返回原矩阵</returns>
    Matrix Power(int n)
    {
        if (row != col)
        {
            std::cout << "The matrix is not a square!" << std::endl;
            return *this;
        }
        else if (n < 0)
        {
            std::cout << "The specified power is less than 0! No changes!" << std::endl;
            return *this;
        }
        else
        {
            Matrix ori(*this), ret(row, row);
            for (int i = 0; i < row; i++)
                ret.matrix[i][i] = 1;
            while (n > 0)
            {
                if (n % 2)
                {
                    ret = ret * ori;
                    n -= 1;
                    ori = ori * ori;
                    n /= 2;
                }
                else
                {
                    ori = ori * ori;
                    n /= 2;
                }
            }
            return ret;
        }
    }

    /// <summary>
    /// 计算矩阵的秩
    /// </summary>
    /// <returns></returns>
    int GetRank()
    {
        if (rank != -1)
            return rank;
        else
        {
            Matrix temp(*this);
            temp.Gauss_Jordan_Elimination();
            rank = temp.rank;
            return temp.rank;
        }
    }

    /// <summary>
    /// 求矩阵的最大奇异值
    /// </summary>
    /// <param name="precision">迭代精度</param>
    /// <returns></returns>
    double GetLargestSingularValue(double precision = PRECISION)
    {
        Matrix T;
        this->Transpose(T);
        int len;
        if (row < col)
        {
            len = row;
            T = *this * T;
        }
        else
        {
            len = col;
            T = T * (*this);
        }
        double now, last;
        //幂法求矩阵最大特征值
        Matrix vector(len, 1);
        for (int i = 0; i < len; i++)
        {
            vector.matrix[i][0] = 1;
        }
        vector = T * vector;
        int maxpos = 0; //最大位置
        for (int i = 0; i < len; i++)
        {
            if (abs(vector.matrix[i][0]) > abs(vector.matrix[maxpos][0]))
                maxpos = i;
        }
        last = vector.matrix[maxpos][0];
        now = last;
        do
        {
            last = now;
            for (int i = 0; i < len; i++)
            {
                vector.matrix[i][0] /= last;
            }
            vector = T * vector;
            for (int i = 0; i < len; i++)
            {
                if (abs(vector.matrix[i][0]) > abs(vector.matrix[maxpos][0]))
                    maxpos = i;
            }
            now = vector.matrix[maxpos][0];
        } while (abs(now - last) > precision);
        return sqrt(now);
    }

    /// <summary>
    /// 产生一个单位矩阵
    /// </summary>
    /// <param name="n">矩阵维度</param>
    /// <returns></returns>
    Matrix IdentityMatrix(int n)
    {
        Matrix I(n, n);
        for (int i = 0; i < n; i++)
        {
            I.matrix[i][i] = 1;
        }
        return I; //会调用复制构造函数，因此效率比直接构造要低；此函数只是为了方便
    }

    /// <summary>
    /// 打印矩阵
    /// </summary>
    /// <param name="width">指定输出宽度，默认为3</param>
    /// <param name="precision">指定输出精度，默认为3</param>
    void Display(int width = 3, int precision = 3)
    {
        for (int i = 0; i < row; i++)
            for (int j = 0; j < col; j++)
            {
                std::cout << std::setprecision(precision) << std::setw(width) << matrix[i][j] << " ";
                if (j == col - 1)
                    std::cout << std::endl;
            }
    }
};
#endif
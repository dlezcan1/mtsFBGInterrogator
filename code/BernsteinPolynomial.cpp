#include "mtsFBGTool/UtilMath/BernsteinPolynomial.h"

#include <algorithm>

CMN_IMPLEMENT_SERVICES(BernsteinPolynomial);

BernsteinPolynomial::BernsteinPolynomial(const unsigned int order, const unsigned int inputSize) : 
    m_Order(order)
{
    m_Coeffs.SetSize(pow(m_Order + 1, 4));

    m_BoundsMin.SetSize(inputSize);
    m_BoundsMin.Zeros(); 

    m_BoundsMax.SetSize(inputSize);
    m_BoundsMax.SetAll(1.0);
}

mtsDoubleVec BernsteinPolynomial::ScaleInput(const mtsDoubleVec& x)
{
    mtsDoubleVec u(x.size());

    for (size_t i = 0; i < m_BoundsMax.size(); i++)
    {
        if (m_BoundsMax[i] - m_BoundsMin[i] < 0.001)
            u[i] = 1.0;
        else 
        {
            u[i] = std::max(
                std::min(
                    (x[i] - m_BoundsMin[i]) / (m_BoundsMax[i] - m_BoundsMin[i]),
                    0.0
                ),
                1.0 
            );
        }
    }

    return u;
}

double BernsteinPolynomial::Evaluate(const mtsDoubleVec& x)
{
    mtsDoubleVec u = ScaleInput(x);

    mtsDoubleVec basisU(u.size());

    mtsDoubleVec bernstainX;
    mtsDouble4   bernsteinBasisX; // FIXME: should be dynamic on X size

    size_t bernsteinIndex;
    for (size_t i = 0; i < m_Order + 1; i++){
        for (size_t j = 0; j < m_Order + 1; i++){
            for (size_t k = 0; k < m_Order + 1; k++){
                for (size_t l = 0; l < m_Order + 1; l++){
                    bernsteinIndex = (
                        i * pow(m_Order + 1, 3) 
                        + j * pow(m_Order + 1, 2)
                        + k * (m_Order + 1)
                        + l
                    );
                    
                    bernsteinBasisX[0] = Basis(i, m_Order, u[0]);
                    bernsteinBasisX[1] = Basis(i, m_Order, u[1]);
                    bernsteinBasisX[2] = Basis(i, m_Order, u[2]);
                    bernsteinBasisX[3] = Basis(i, m_Order, u[3]);

                    bernstainX[bernsteinIndex] = 1.0;
                    for (auto val : bernsteinBasisX)
                        bernstainX[bernsteinIndex] *= val;

                }
            }
        }
    }

    return m_Coeffs * bernstainX;
}
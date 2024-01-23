#pragma once

#include <cisstCommon.h>
#include <cisstMultiTask.h>

class CISST_EXPORT BernsteinPolynomial : public cmnGenericObject
{
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_RUN_VERBOSE);
public:
    BernsteinPolynomial(const unsigned int order, const unsigned int inputSize);
    BernsteinPolynomial(
        const unsigned int order,
        const mtsDoubleVec coeffs,
        const mtsDoubleVec& boundsMin,
        const mtsDoubleVec& boundsMax
    ):
        m_Order(order),
        m_Coeffs(coeffs),
        m_BoundsMin(boundsMin),
        m_BoundsMax(boundsMax)
    {
        if (m_BoundsMin.size() != m_BoundsMax.size())
            throw std::invalid_argument("Bounds min and bounds max must have the same size!");
    }

    inline void SetCoeffs(const mtsDoubleVec& coeffs) { m_Coeffs = coeffs; }
    inline void SetBoundsMin(const mtsDoubleVec& bounds) { m_BoundsMin = bounds; }
    inline void SetBoundsMax(const mtsDoubleVec& bounds) { m_BoundsMax = bounds; }

    double Evaluate(const mtsDoubleVec& x);

    mtsDoubleVec ScaleInput(const mtsDoubleVec& x);
    
    static double Basis(const unsigned int k, const unsigned int n, const double x)
    {
        unsigned int u = twoChoosek(k);
        if (n == 2)
            return u * pow(1-x, n-k) * pow(x, k);

        return 0.0;
    }

    static unsigned int twoChoosek(unsigned int k)
    {
        if(k == 1)
            return 2;
        else if ((k == 0)||(k == 2))
            return 1;
        
        return 0;
    
    }

private:
    unsigned int m_Order = 2;
    mtsDoubleVec m_Coeffs;
    mtsDoubleVec m_BoundsMin;
    mtsDoubleVec m_BoundsMax;
};

CMN_DECLARE_SERVICES_INSTANTIATION(BernsteinPolynomial);
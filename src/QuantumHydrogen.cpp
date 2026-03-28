#include "QuantumHydrogen.hpp"

#include <algorithm>
#include <cmath>

HydrogenOrbitalSampler::HydrogenOrbitalSampler(int n, int l, int m) {
    setQuantumNumbers(n, l, m);
}

void HydrogenOrbitalSampler::setQuantumNumbers(int n, int l, int m) {
    if (n < 1) {
        n = 1;
    }
    if (l < 0) {
        l = 0;
    }
    if (l >= n) {
        l = n - 1;
    }
    if (m < -l) {
        m = -l;
    }
    if (m > l) {
        m = l;
    }

    if (m_n == n && m_l == l && m_m == m) {
        return;
    }

    m_m = m;
    m_mmag = std::abs(m);
    m_n = n;
    m_l = l;

    auto key = std::make_tuple(n, l, std::abs(m));
    auto it = m_cache.find(key);

    if (it != m_cache.end()) {
        m_rMid = it->second.rMid;
        m_rCdf = it->second.rCdf;
        m_thetaMid = it->second.thetaMid;
        m_thetaCdf = it->second.thetaCdf;
        m_rMax = it->second.rMax;
    } else {
        rebuildTables();
        CachedTables cached;
        cached.rMid = m_rMid;
        cached.rCdf = m_rCdf;
        cached.thetaMid = m_thetaMid;
        cached.thetaCdf = m_thetaCdf;
        cached.rMax = m_rMax;
        m_cache[key] = std::move(cached);
    }
}

float HydrogenOrbitalSampler::factorial(int k) {
    float r = 1.0f;
    for (int i = 2; i <= k; ++i) {
        r *= static_cast<float>(i);
    }
    return r;
}

float HydrogenOrbitalSampler::oddFactorialProduct(int m) {
    if (m <= 0) {
        return 1.0f;
    }
    float r = 1.0f;
    for (int k = 1; k <= m; ++k) {
        r *= static_cast<float>(2 * k - 1);
    }
    return r;
}

float HydrogenOrbitalSampler::assocLaguerre(int nPoly, float alpha, float x) {
    if (nPoly < 0) {
        return 0.0f;
    }
    if (nPoly == 0) {
        return 1.0f;
    }
    float L0 = 1.0f;
    float L1 = 1.0f + alpha - x;
    if (nPoly == 1) {
        return L1;
    }
    float Lnm2 = L0;
    float Lnm1 = L1;
    for (int k = 1; k < nPoly; ++k) {
        const float Lk = (((2.0f * k + alpha + 1.0f) - x) * Lnm1 - (k + alpha) * Lnm2) / (k + 1.0f);
        Lnm2 = Lnm1;
        Lnm1 = Lk;
    }
    return Lnm1;
}

float HydrogenOrbitalSampler::legendrePlm(int ll, int mm, float x) const {
    x = std::max(-1.0f, std::min(1.0f, x));
    if (mm < 0) {
        mm = -mm;
    }
    if (mm > ll) {
        return 0.0f;
    }
    const float sx = std::sqrt(std::max(0.0f, 1.0f - x * x));
    float Pmm = ((mm % 2) == 0 ? 1.0f : -1.0f) * oddFactorialProduct(mm) *
                std::pow(sx, static_cast<float>(mm));
    if (ll == mm) {
        return Pmm;
    }
    float Pm1m = x * (2 * mm + 1) * Pmm;
    if (ll == mm + 1) {
        return Pm1m;
    }
    float Pj2 = Pmm;
    float Pj1 = Pm1m;
    for (int j = mm + 2; j <= ll; ++j) {
        const float Pj = (x * (2 * j - 1) * Pj1 - (j + mm - 1) * Pj2) / (j - mm);
        Pj2 = Pj1;
        Pj1 = Pj;
    }
    return Pj1;
}

float HydrogenOrbitalSampler::radialRnl(float r) const {
    if (r <= 0.0f) {
        return 0.0f;
    }
    const int np = m_n - m_l - 1;
    const float alpha = 2.0f * m_l + 1.0f;
    const float Z = 1.0f;
    const float a = m_a0;
    const float rho = 2.0f * Z * r / (static_cast<float>(m_n) * a);
    const float norm =
        std::sqrt(std::pow(2.0f * Z / (static_cast<float>(m_n) * a), 3.0f) *
                  factorial(m_n - m_l - 1) / (2.0f * static_cast<float>(m_n) * factorial(m_n + m_l)));
    const float L = assocLaguerre(np, alpha, rho);
    return norm * std::exp(-rho * 0.5f) * std::pow(rho, static_cast<float>(m_l)) * L;
}

float HydrogenOrbitalSampler::ylmNorm(int ll, int mm) const {
    mm = std::abs(mm);
    return std::sqrt((2.0f * ll + 1.0f) / (4.0f * 3.14159265358979323846f) * factorial(ll - mm) /
                     factorial(ll + mm));
}

float HydrogenOrbitalSampler::angularDensity(int ll, int mm, float theta) const {
    const float ct = std::cos(theta);
    const float P = legendrePlm(ll, mm, ct);
    const float N = ylmNorm(ll, mm);
    const float y = N * P;
    return y * y;
}

float HydrogenOrbitalSampler::radialRadialDensity(float r) const {
    const float R = radialRnl(r);
    return r * r * R * R;
}

void HydrogenOrbitalSampler::rebuildTables() {
    m_rMax = 90.0f * static_cast<float>(m_n * m_n) * m_a0;
    constexpr int Nr = 4096;
    constexpr int Nt = 2048;
    m_rMid.clear();
    m_rCdf.clear();
    m_thetaMid.clear();
    m_thetaCdf.clear();
    m_rMid.resize(Nr);
    m_rCdf.resize(Nr);
    m_thetaMid.resize(Nt);
    m_thetaCdf.resize(Nt);
    float sumR = 0.0f;
    for (int i = 0; i < Nr; ++i) {
        const float r0 = static_cast<float>(i) / Nr * m_rMax;
        const float r1 = static_cast<float>(i + 1) / Nr * m_rMax;
        const float mid = 0.5f * (r0 + r1);
        m_rMid[static_cast<std::size_t>(i)] = mid;
        const float w = radialRadialDensity(mid) * (r1 - r0);
        sumR += w;
        m_rCdf[static_cast<std::size_t>(i)] = sumR;
    }
    if (sumR > 0.0f) {
        for (float& c : m_rCdf) {
            c /= sumR;
        }
    }
    float sumT = 0.0f;
    for (int j = 0; j < Nt; ++j) {
        const float t0 = 3.14159265358979323846f * j / Nt;
        const float t1 = 3.14159265358979323846f * (j + 1) / Nt;
        const float mid = 0.5f * (t0 + t1);
        m_thetaMid[static_cast<std::size_t>(j)] = mid;
        const float w = angularDensity(m_l, m_mmag, mid) * std::sin(mid) * (t1 - t0);
        sumT += w;
        m_thetaCdf[static_cast<std::size_t>(j)] = sumT;
    }
    if (sumT > 0.0f) {
        for (float& c : m_thetaCdf) {
            c /= sumT;
        }
    }
}


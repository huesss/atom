#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <random>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace std {
template <>
struct hash<std::tuple<int, int, int>> {
    std::size_t operator()(const std::tuple<int, int, int>& t) const noexcept {
        const auto h1 = std::hash<int>{}(std::get<0>(t));
        const auto h2 = std::hash<int>{}(std::get<1>(t));
        const auto h3 = std::hash<int>{}(std::get<2>(t));
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
}

class HydrogenOrbitalSampler {
public:
    HydrogenOrbitalSampler(int n, int l, int m);

    void setQuantumNumbers(int n, int l, int m);

    int n() const { return m_n; }
    int l() const { return m_l; }
    int m() const { return m_m; }
    int mmag() const { return m_mmag; }
    float a0() const { return m_a0; }
    float rMax() const { return m_rMax; }

    const std::vector<float>& rMid() const { return m_rMid; }
    const std::vector<float>& rCdf() const { return m_rCdf; }
    const std::vector<float>& thetaMid() const { return m_thetaMid; }
    const std::vector<float>& thetaCdf() const { return m_thetaCdf; }

private:
    int m_n = 1;
    int m_l = 0;
    int m_m = 0;
    int m_mmag = 0;
    float m_a0 = 1.0f;
    std::vector<float> m_rMid;
    std::vector<float> m_rCdf;
    std::vector<float> m_thetaMid;
    std::vector<float> m_thetaCdf;
    float m_rMax = 1.0f;

    struct CachedTables {
        std::vector<float> rMid;
        std::vector<float> rCdf;
        std::vector<float> thetaMid;
        std::vector<float> thetaCdf;
        float rMax;
    };

    mutable std::unordered_map<std::tuple<int, int, int>, CachedTables> m_cache;

    void rebuildTables();
    static float factorial(int k);
    static float assocLaguerre(int nPoly, float alpha, float x);
    static float oddFactorialProduct(int m);

    float legendrePlm(int ll, int mm, float x) const;
    float radialRnl(float r) const;
    float ylmNorm(int ll, int mm) const;
    float angularDensity(int ll, int mm, float theta) const;
    float radialRadialDensity(float r) const;
};

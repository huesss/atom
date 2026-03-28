#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <map>
#include <random>
#include <tuple>
#include <vector>

class HydrogenOrbitalSampler {
public:
    HydrogenOrbitalSampler(int n, int l, int m);

    void setQuantumNumbers(int n, int l, int m);
    void generate(std::vector<glm::vec4>& outPacked, std::uint32_t count, float timePhase,
                  std::mt19937& rng) const;
    
    void generateChunk(std::vector<glm::vec4>& outPacked, std::uint32_t startIdx, 
                      std::uint32_t chunkSize, std::uint32_t totalCount, 
                      float timePhase, std::mt19937& rng) const;

    static void stepCloudPositions(std::vector<glm::vec4>& packed, float dt, int mSigned,
                                   float simTime, int principalN);

    int n() const { return m_n; }
    int l() const { return m_l; }
    int m() const { return m_mmag; }

private:
    int m_n = 1;
    int m_l = 0;
    int m_m = 0;
    int m_mmag = 0;
    double m_a0 = 1.0;
    std::vector<double> m_rMid;
    std::vector<double> m_rCdf;
    std::vector<double> m_thetaMid;
    std::vector<double> m_thetaCdf;
    double m_rMax = 1.0;

    struct CachedTables {
        std::vector<double> rMid;
        std::vector<double> rCdf;
        std::vector<double> thetaMid;
        std::vector<double> thetaCdf;
        double rMax;
    };
    
    mutable std::map<std::tuple<int, int, int>, CachedTables> m_cache;

    void rebuildTables();
    static double factorial(int k);
    static double assocLaguerre(int nPoly, double alpha, double x);
    static double oddFactorialProduct(int m);

    double legendrePlm(int ll, int mm, double x) const;
    double radialRnl(double r) const;
    double ylmNorm(int ll, int mm) const;
    double angularDensity(int ll, int mm, double theta) const;
    double radialRadialDensity(double r) const;
    double psiReal(double r, double theta, double phi, float timePhase) const;
    double psiImag(double r, double theta, double phi, float timePhase) const;
    static double sampleFromCdf(const std::vector<double>& mid, const std::vector<double>& cdf,
                                double u);
    glm::vec3 infernoRgbLikeAtomsMain(double r, double theta) const;
};

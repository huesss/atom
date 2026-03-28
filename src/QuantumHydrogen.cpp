#include "QuantumHydrogen.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace {

glm::vec3 heatmapFire(float value) {
    value = std::max(0.0f, std::min(1.0f, value));
    constexpr int num_stops = 6;
    const glm::vec3 colors[num_stops] = {{0.0f, 0.0f, 0.0f},
                                         {0.5f, 0.0f, 0.99f},
                                         {0.8f, 0.0f, 0.0f},
                                         {1.0f, 0.5f, 0.0f},
                                         {1.0f, 1.0f, 0.0f},
                                         {1.0f, 1.0f, 1.0f}};
    const float scaled_v = value * static_cast<float>(num_stops - 1);
    const int i = static_cast<int>(scaled_v);
    const int next_i = std::min(i + 1, num_stops - 1);
    const float local_t = scaled_v - static_cast<float>(i);
    return colors[i] + local_t * (colors[next_i] - colors[i]);
}

} // namespace

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
    m_m = m;
    m_mmag = std::abs(m);
    m_n = n;
    m_l = l;
    rebuildTables();
}

double HydrogenOrbitalSampler::factorial(int k) {
    double r = 1.0;
    for (int i = 2; i <= k; ++i) {
        r *= static_cast<double>(i);
    }
    return r;
}

double HydrogenOrbitalSampler::oddFactorialProduct(int m) {
    if (m <= 0) {
        return 1.0;
    }
    double r = 1.0;
    for (int k = 1; k <= m; ++k) {
        r *= (2 * k - 1);
    }
    return r;
}

double HydrogenOrbitalSampler::assocLaguerre(int nPoly, double alpha, double x) {
    if (nPoly < 0) {
        return 0.0;
    }
    if (nPoly == 0) {
        return 1.0;
    }
    double L0 = 1.0;
    double L1 = 1.0 + alpha - x;
    if (nPoly == 1) {
        return L1;
    }
    double Lnm2 = L0;
    double Lnm1 = L1;
    for (int k = 1; k < nPoly; ++k) {
        const double Lk =
            (((2.0 * k + alpha + 1.0) - x) * Lnm1 - (k + alpha) * Lnm2) / (k + 1.0);
        Lnm2 = Lnm1;
        Lnm1 = Lk;
    }
    return Lnm1;
}

double HydrogenOrbitalSampler::legendrePlm(int ll, int mm, double x) const {
    x = std::max(-1.0, std::min(1.0, x));
    if (mm < 0) {
        mm = -mm;
    }
    if (mm > ll) {
        return 0.0;
    }
    const double sx = std::sqrt(std::max(0.0, 1.0 - x * x));
    double Pmm =
        ((mm % 2) == 0 ? 1.0 : -1.0) * oddFactorialProduct(mm) * std::pow(sx, static_cast<double>(mm));
    if (ll == mm) {
        return Pmm;
    }
    double Pm1m = x * (2 * mm + 1) * Pmm;
    if (ll == mm + 1) {
        return Pm1m;
    }
    double Pj2 = Pmm;
    double Pj1 = Pm1m;
    for (int j = mm + 2; j <= ll; ++j) {
        const double Pj = (x * (2 * j - 1) * Pj1 - (j + mm - 1) * Pj2) / (j - mm);
        Pj2 = Pj1;
        Pj1 = Pj;
    }
    return Pj1;
}

double HydrogenOrbitalSampler::radialRnl(double r) const {
    if (r <= 0.0) {
        return 0.0;
    }
    const int np = m_n - m_l - 1;
    const double alpha = 2.0 * m_l + 1.0;
    const double Z = 1.0;
    const double a = m_a0;
    const double rho = 2.0 * Z * r / (static_cast<double>(m_n) * a);
    const double norm = std::sqrt(
        std::pow(2.0 * Z / (static_cast<double>(m_n) * a), 3.0) * factorial(m_n - m_l - 1) /
        (2.0 * static_cast<double>(m_n) * factorial(m_n + m_l)));
    const double L = assocLaguerre(np, alpha, rho);
    return norm * std::exp(-rho * 0.5) * std::pow(rho, static_cast<double>(m_l)) * L;
}

double HydrogenOrbitalSampler::ylmNorm(int ll, int mm) const {
    mm = std::abs(mm);
    return std::sqrt((2.0 * ll + 1.0) / (4.0 * 3.14159265358979323846) * factorial(ll - mm) /
                     factorial(ll + mm));
}

double HydrogenOrbitalSampler::angularDensity(int ll, int mm, double theta) const {
    const double ct = std::cos(theta);
    const double P = legendrePlm(ll, mm, ct);
    const double N = ylmNorm(ll, mm);
    const double y = N * P;
    return y * y;
}

double HydrogenOrbitalSampler::radialRadialDensity(double r) const {
    const double R = radialRnl(r);
    return r * r * R * R;
}

void HydrogenOrbitalSampler::rebuildTables() {
    m_rMax = 90.0 * static_cast<double>(m_n * m_n) * m_a0;
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
    double sumR = 0.0;
    for (int i = 0; i < Nr; ++i) {
        const double r0 = static_cast<double>(i) / Nr * m_rMax;
        const double r1 = static_cast<double>(i + 1) / Nr * m_rMax;
        const double mid = 0.5 * (r0 + r1);
        m_rMid[static_cast<std::size_t>(i)] = mid;
        const double w = radialRadialDensity(mid) * (r1 - r0);
        sumR += w;
        m_rCdf[static_cast<std::size_t>(i)] = sumR;
    }
    if (sumR > 0.0) {
        for (double& c : m_rCdf) {
            c /= sumR;
        }
    }
    double sumT = 0.0;
    for (int j = 0; j < Nt; ++j) {
        const double t0 = 3.14159265358979323846 * j / Nt;
        const double t1 = 3.14159265358979323846 * (j + 1) / Nt;
        const double mid = 0.5 * (t0 + t1);
        m_thetaMid[static_cast<std::size_t>(j)] = mid;
        const double w = angularDensity(m_l, m_mmag, mid) * std::sin(mid) * (t1 - t0);
        sumT += w;
        m_thetaCdf[static_cast<std::size_t>(j)] = sumT;
    }
    if (sumT > 0.0) {
        for (double& c : m_thetaCdf) {
            c /= sumT;
        }
    }
}

double HydrogenOrbitalSampler::sampleFromCdf(const std::vector<double>& mid,
                                               const std::vector<double>& cdf, double u) {
    u = std::max(0.0, std::min(1.0 - 1e-15, u));
    const auto it = std::lower_bound(cdf.begin(), cdf.end(), u);
    const std::size_t idx = static_cast<std::size_t>(std::min(
        static_cast<int>(cdf.size() - 1), std::max(0, static_cast<int>(it - cdf.begin()))));
    return mid[idx];
}

double HydrogenOrbitalSampler::psiReal(double r, double theta, double phi,
                                       float timePhase) const {
    const double R = radialRnl(r);
    const double ct = std::cos(theta);
    const double P = legendrePlm(m_l, m_mmag, ct);
    const double N = ylmNorm(m_l, m_mmag);
    const double phiT = static_cast<double>(m_m) * phi + static_cast<double>(timePhase);
    const double signM = (m_m < 0 && (m_mmag % 2) == 1) ? -1.0 : 1.0;
    return signM * R * N * P * std::cos(phiT);
}

glm::vec3 HydrogenOrbitalSampler::infernoRgbLikeAtomsMain(double r, double theta) const {
    const double Rb = radialRnl(r);
    const double Plm = legendrePlm(m_l, m_mmag, std::cos(theta));
    const double intensity = Rb * Rb * Plm * Plm;
    const float mapped =
        static_cast<float>(intensity * 1.5 * std::pow(5.0, static_cast<double>(m_n)));
    const glm::vec3 c = heatmapFire(mapped);
    const glm::vec3 floorColor(0.22f, 0.32f, 0.68f);
    return glm::max(c, floorColor * 0.55f) * 0.92f + floorColor * 0.08f;
}

double HydrogenOrbitalSampler::psiImag(double r, double theta, double phi,
                                       float timePhase) const {
    const double R = radialRnl(r);
    const double ct = std::cos(theta);
    const double P = legendrePlm(m_l, m_mmag, ct);
    const double N = ylmNorm(m_l, m_mmag);
    const double phiT = static_cast<double>(m_m) * phi + static_cast<double>(timePhase);
    const double signM = (m_m < 0 && (m_mmag % 2) == 1) ? -1.0 : 1.0;
    return signM * R * N * P * std::sin(phiT);
}

void HydrogenOrbitalSampler::generate(std::vector<glm::vec4>& outPacked, std::uint32_t count,
                                      float timePhase, std::mt19937& rng) const {
    outPacked.resize(static_cast<std::size_t>(count) * 2);
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    double rhoMax = 0.0;
    for (double rmid : m_rMid) {
        for (double tmid : m_thetaMid) {
            const double R = radialRnl(rmid);
            const double ad = angularDensity(m_l, m_mmag, tmid);
            rhoMax = std::max(rhoMax, R * R * ad);
        }
    }
    if (rhoMax <= 0.0) {
        rhoMax = 1.0;
    }
    for (std::uint32_t i = 0; i < count; ++i) {
        const double r = sampleFromCdf(m_rMid, m_rCdf, u01(rng));
        const double theta = sampleFromCdf(m_thetaMid, m_thetaCdf, u01(rng));
        const double phi = u01(rng) * 2.0 * 3.14159265358979323846;
        const double x = r * std::sin(theta) * std::cos(phi);
        const double y = r * std::sin(theta) * std::sin(phi);
        const double z = r * std::cos(theta);
        const double pr = psiReal(r, theta, phi, timePhase);
        const double pi = psiImag(r, theta, phi, timePhase);
        const float phase = static_cast<float>(std::atan2(pi, pr));
        const double Rb = radialRnl(r);
        const double ad = angularDensity(m_l, m_mmag, theta);
        const double rho = Rb * Rb * ad;
        const float dnorm = static_cast<float>(std::min(1.0, rho / rhoMax));
        const glm::vec3 heat = infernoRgbLikeAtomsMain(r, theta);
        const std::size_t k = static_cast<std::size_t>(i) * 2;
        outPacked[k] = glm::vec4(static_cast<float>(x), static_cast<float>(y),
                                 static_cast<float>(z), dnorm);
        outPacked[k + 1] = glm::vec4(heat, phase);
    }
}

void HydrogenOrbitalSampler::stepCloudPositions(std::vector<glm::vec4>& packed, float dt,
                                                  int mSigned, float simTime, int principalN) {
    if (packed.empty()) {
        return;
    }
    const std::size_t n = packed.size() / 2;
    const float rFall =
        3.2f * static_cast<float>(std::max(1, principalN)) * static_cast<float>(std::max(1, principalN));
    for (std::size_t i = 0; i < n; ++i) {
        glm::vec3 p(packed[i * 2]);
        float r = glm::length(p);
        if (r < 1e-5f) {
            continue;
        }
        const float w = std::exp(-r / std::max(1.5f, rFall));
        float theta = std::acos(glm::clamp(p.z / r, -1.0f, 1.0f));
        float phi = std::atan2(p.y, p.x);
        if (mSigned != 0) {
            phi += static_cast<float>(mSigned) * dt * 0.055f;
        }
        const float sp = std::sin(theta);
        p.x = r * sp * std::cos(phi);
        p.y = r * sp * std::sin(phi);
        p.z = r * std::cos(theta);
        const float spin = 0.009f * dt;
        const glm::mat3 Ry =
            glm::mat3(glm::rotate(glm::mat4(1.0f), spin, glm::vec3(0.0f, 1.0f, 0.0f)));
        p = Ry * p;
        if (mSigned == 0) {
            const float pulse =
                1.0f + 0.0065f * w *
                           std::sin(simTime * 1.35f + static_cast<float>(i) * 2.718e-4f);
            p *= pulse;
        }
        packed[i * 2].x = p.x;
        packed[i * 2].y = p.y;
        packed[i * 2].z = p.z;
    }
}

#include "qasm_simulator.hpp"
#include <array>
#include <cmath>
#include <random>
#include <sstream>

namespace bloch {

    static std::mt19937 rng{std::random_device{}()};

    int QasmSimulator::allocateQubit() {
        int index = m_qubits++;
        std::vector<std::complex<double>> newState(m_state.size() * 2);
        for (size_t i = 0; i < m_state.size(); ++i) {
            newState[i] = m_state[i];
            newState[i + m_state.size()] = 0;
        }
        m_state.swap(newState);
        return index;
    }

    void QasmSimulator::applySingleQubitGate(int q, const std::array<std::complex<double>, 4>& m) {
        size_t step = size_t{1} << q;
        size_t size = m_state.size();
        for (size_t i = 0; i < size; i += 2 * step) {
            for (size_t j = 0; j < step; ++j) {
                size_t idx0 = i + j;
                size_t idx1 = idx0 + step;
                auto a0 = m_state[idx0];
                auto a1 = m_state[idx1];
                m_state[idx0] = m[0] * a0 + m[1] * a1;
                m_state[idx1] = m[2] * a0 + m[3] * a1;
            }
        }
    }

    void QasmSimulator::h(int q) {
        const std::array<std::complex<double>, 4> m{1 / std::sqrt(2.0), 1 / std::sqrt(2.0),
                                                    1 / std::sqrt(2.0), -1 / std::sqrt(2.0)};
        applySingleQubitGate(q, m);
        m_ops += "h q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::x(int q) {
        const std::array<std::complex<double>, 4> m{0, 1, 1, 0};
        applySingleQubitGate(q, m);
        m_ops += "x q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::y(int q) {
        const std::array<std::complex<double>, 4> m{0.0, std::complex<double>(0, -1),
                                                    std::complex<double>(0, 1), 0.0};
        applySingleQubitGate(q, m);
        m_ops += "y q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::z(int q) {
        const std::array<std::complex<double>, 4> m{1.0, 0.0, 0.0, -1.0};
        applySingleQubitGate(q, m);
        m_ops += "z q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::rx(int q, double t) {
        double ct = std::cos(t / 2);
        double st = std::sin(t / 2);
        const std::array<std::complex<double>, 4> m{ct, std::complex<double>(0, -st),
                                                    std::complex<double>(0, -st), ct};
        applySingleQubitGate(q, m);
        m_ops += "rx(" + std::to_string(t) + ") q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::ry(int q, double t) {
        double ct = std::cos(t / 2);
        double st = std::sin(t / 2);
        const std::array<std::complex<double>, 4> m{ct, -st, st, ct};
        applySingleQubitGate(q, m);
        m_ops += "ry(" + std::to_string(t) + ") q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::rz(int q, double t) {
        std::complex<double> epos = std::exp(std::complex<double>(0, -t / 2));
        std::complex<double> eneg = std::exp(std::complex<double>(0, t / 2));
        const std::array<std::complex<double>, 4> m{epos, 0.0, 0.0, eneg};
        applySingleQubitGate(q, m);
        m_ops += "rz(" + std::to_string(t) + ") q[" + std::to_string(q) + "];\n";
    }

    void QasmSimulator::cx(int control, int target) {
        size_t cbit = size_t{1} << control;
        size_t tbit = size_t{1} << target;
        for (size_t i = 0; i < m_state.size(); ++i) {
            if ((i & cbit) && !(i & tbit)) {
                size_t j = i | tbit;
                std::swap(m_state[i], m_state[j]);
            }
        }
        m_ops += "cx q[" + std::to_string(control) + "],q[" + std::to_string(target) + "];\n";
    }

    int QasmSimulator::measure(int q) {
        size_t bit = size_t{1} << q;
        double p1 = 0;
        for (size_t i = 0; i < m_state.size(); ++i)
            if (i & bit)
                p1 += std::norm(m_state[i]);
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double r = dist(rng);
        int res = r < p1 ? 1 : 0;
        double norm = std::sqrt(res ? p1 : 1 - p1);
        for (size_t i = 0; i < m_state.size(); ++i) {
            if (((i & bit) ? 1 : 0) != res)
                m_state[i] = 0;
            else
                m_state[i] /= norm;
        }
        m_ops += "measure q[" + std::to_string(q) + "] -> c[" + std::to_string(q) + "];\n";
        return res;
    }

    std::string QasmSimulator::getQasm() const {
        std::ostringstream out;
        out << "OPENQASM 2.0;\ninclude \"qelib1.inc\";\n";
        out << "qreg q[" << m_qubits << "];\n";
        out << "creg c[" << m_qubits << "];\n";
        out << m_ops;
        return out.str();
    }

}
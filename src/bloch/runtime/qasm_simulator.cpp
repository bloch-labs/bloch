// Copyright 2025 Akshay Pal (https://bloch-labs.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "qasm_simulator.hpp"
#include <array>
#include <cmath>
#include <random>
#include <sstream>
#include <stdexcept>

namespace bloch {

    static std::mt19937 rng{std::random_device{}()};

    int QasmSimulator::allocateQubit() {
        // Grow the state by a factor of two, keeping existing amplitudes
        // in the |...0> subspace and zeroing the |...1> subspace.
        int index = m_qubits++;
        if (index >= static_cast<int>(m_measured.size()))
            m_measured.resize(index + 1, false);
        else
            m_measured[index] = false;
        std::vector<std::complex<double>> newState(m_state.size() * 2);
        for (size_t i = 0; i < m_state.size(); ++i) {
            newState[i] = m_state[i];
            newState[i + m_state.size()] = 0;
        }
        m_state.swap(newState);
        return index;
    }

    void QasmSimulator::applySingleQubitGate(int q, const std::array<std::complex<double>, 4>& m) {
        ensureQubitActive(q);
        // Standard blocked application over basis pairs differing at bit q.
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
        if (m_logOps)
            m_ops.emplace_back("h q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::x(int q) {
        const std::array<std::complex<double>, 4> m{0, 1, 1, 0};
        applySingleQubitGate(q, m);
        if (m_logOps)
            m_ops.emplace_back("x q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::y(int q) {
        const std::array<std::complex<double>, 4> m{0.0, std::complex<double>(0, -1),
                                                    std::complex<double>(0, 1), 0.0};
        applySingleQubitGate(q, m);
        if (m_logOps)
            m_ops.emplace_back("y q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::z(int q) {
        const std::array<std::complex<double>, 4> m{1.0, 0.0, 0.0, -1.0};
        applySingleQubitGate(q, m);
        if (m_logOps)
            m_ops.emplace_back("z q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::rx(int q, double t) {
        double ct = std::cos(t / 2);
        double st = std::sin(t / 2);
        const std::array<std::complex<double>, 4> m{ct, std::complex<double>(0, -st),
                                                    std::complex<double>(0, -st), ct};
        applySingleQubitGate(q, m);
        if (m_logOps)
            m_ops.emplace_back("rx(" + std::to_string(t) + ") q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::ry(int q, double t) {
        double ct = std::cos(t / 2);
        double st = std::sin(t / 2);
        const std::array<std::complex<double>, 4> m{ct, -st, st, ct};
        applySingleQubitGate(q, m);
        if (m_logOps)
            m_ops.emplace_back("ry(" + std::to_string(t) + ") q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::rz(int q, double t) {
        std::complex<double> epos = std::exp(std::complex<double>(0, -t / 2));
        std::complex<double> eneg = std::exp(std::complex<double>(0, t / 2));
        const std::array<std::complex<double>, 4> m{epos, 0.0, 0.0, eneg};
        applySingleQubitGate(q, m);
        if (m_logOps)
            m_ops.emplace_back("rz(" + std::to_string(t) + ") q[" + std::to_string(q) + "];\n");
    }

    void QasmSimulator::cx(int control, int target) {
        ensureQubitActive(control);
        ensureQubitActive(target);
        // Swap amplitudes where control is 1 and target is 0 to flip target.
        size_t cbit = size_t{1} << control;
        size_t tbit = size_t{1} << target;
        for (size_t i = 0; i < m_state.size(); ++i) {
            if ((i & cbit) && !(i & tbit)) {
                size_t j = i | tbit;
                std::swap(m_state[i], m_state[j]);
            }
        }
        if (m_logOps)
            m_ops.emplace_back("cx q[" + std::to_string(control) + "],q[" + std::to_string(target) +
                               "];\n");
    }

    void QasmSimulator::reset(int q) {
        if (q < 0 || q >= m_qubits) {
            throw BlochError(ErrorCategory::Runtime, 0, 0,
                             "qubit index " + std::to_string(q) + " is out of range");
        }
        if (q >= 0 && q < static_cast<int>(m_measured.size()))
            m_measured[q] = false;
        // Put qubit q into |0>.
        // If the state already has amplitude in the |...0> subspace, zero the |...1> subspace
        // and renormalize. If all amplitude is in |...1>, deterministically move it into
        // the |...0> subspace (equivalent to an X on a measured |1>), avoiding NaNs.
        size_t bit = size_t{1} << q;
        double norm0 = 0.0;
        for (size_t i = 0; i < m_state.size(); ++i) {
            if (!(i & bit))
                norm0 += std::norm(m_state[i]);
        }

        if (norm0 == 0.0) {
            // All amplitude is in the |...1> subspace: swap it into |...0>.
            for (size_t i = 0; i < m_state.size(); ++i) {
                if (i & bit) {
                    size_t j = i ^ bit;  // flip target bit to 0
                    m_state[j] = m_state[i];
                    m_state[i] = 0.0;
                }
            }
        } else {
            // Zero |...1> and renormalize |...0>
            double inv = 1.0 / std::sqrt(norm0);
            for (size_t i = 0; i < m_state.size(); ++i) {
                if (i & bit) {
                    m_state[i] = 0.0;
                } else {
                    m_state[i] *= inv;
                }
            }
        }

        if (m_logOps)
            m_ops.emplace_back("reset q[" + std::to_string(q) + "];\n");
    }

    int QasmSimulator::measure(int q) {
        ensureQubitActive(q);
        // Compute probability of |1>, sample, and collapse the state accordingly.
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
        if (m_logOps)
            m_ops.emplace_back("measure q[" + std::to_string(q) + "] -> c[" + std::to_string(q) +
                               "];\n");
        if (q >= 0 && q < static_cast<int>(m_measured.size()))
            m_measured[q] = true;
        return res;
    }

    std::string QasmSimulator::getQasm() const {
        const std::string header = "OPENQASM 2.0;\ninclude \"qelib1.inc\";\n";
        const std::string qreg = "qreg q[" + std::to_string(m_qubits) + "];\n";
        const std::string creg = "creg c[" + std::to_string(m_qubits) + "];\n";
        size_t total = header.size() + qreg.size() + creg.size();
        for (const auto& op : m_ops) total += op.size();
        std::string out;
        out.reserve(total);
        out.append(header);
        out.append(qreg);
        out.append(creg);
        for (const auto& op : m_ops) out.append(op);
        return out;
    }

    void QasmSimulator::ensureQubitActive(int q) const {
        if (q < 0 || q >= m_qubits) {
            throw BlochError(ErrorCategory::Runtime, 0, 0,
                             "qubit index " + std::to_string(q) + " is out of range");
        }
        if (q < static_cast<int>(m_measured.size()) && m_measured[q]) {
            throw BlochError(ErrorCategory::Runtime, 0, 0,
                             "cannot operate on measured qubit q[" + std::to_string(q) + "]");
        }
    }
}

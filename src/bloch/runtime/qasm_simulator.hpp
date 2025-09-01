#pragma once

#include <array>
#include <complex>
#include <string>
#include <vector>

namespace bloch {

    // An ideal statevector simulator with a QASM log.
    // TODO: performance optimisation post 1.0.0
    class QasmSimulator {
       public:
        int allocateQubit();
        void h(int q);
        void x(int q);
        void y(int q);
        void z(int q);
        void rx(int q, double theta);
        void ry(int q, double theta);
        void rz(int q, double theta);
        void cx(int control, int target);
        void reset(int q);
        int measure(int q);
        std::string getQasm() const;

       private:
        int m_qubits = 0;
        std::vector<std::complex<double>> m_state{1, 0};
        std::string m_ops;

        // Apply a 2x2 unitary to qubit q.
        void applySingleQubitGate(int q, const std::array<std::complex<double>, 4>& m);
    };

}

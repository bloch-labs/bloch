#pragma once

#include <array>
#include <complex>
#include <string>
#include <vector>

namespace bloch {

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

        void applySingleQubitGate(int q, const std::array<std::complex<double>, 4>& m);
    };

}
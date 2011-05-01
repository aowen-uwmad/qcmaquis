
#ifndef MEASUREMENTS_HPP_
#define MEASUREMENTS_HPP_

#include "measurements.h"

#include "utils/DmrgParameters.h"

namespace app {
    
    template<class Matrix, class SymmGroup>
    struct pre_measurements {
        void operator() (const Lattice& lattice, ModelParameters& model,
                         std::vector<Measurement_Term<Matrix, SymmGroup> >& terms,
                         typename Measurement_Term<Matrix, SymmGroup>::op_t& ident)
        { }
    };
    
    // include specializations of pre_measurements
#include "meas_u1.hpp"
#include "meas_2u1.hpp"
    
    template<class Matrix, class SymmGroup>
    class CodedMeasurements : public Measurements<Matrix, SymmGroup>
    {
        typedef Measurements<Matrix, SymmGroup> super_t;
    public:
        CodedMeasurements (const Lattice& lattice, ModelParameters& model)
        {
            pre_measurements<Matrix, SymmGroup>()(lattice, model, super_t::terms, super_t::ident);
        }
    };
    
    
} // namespace
#endif /* MEASUREMENTS_HPP_ */

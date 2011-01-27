#ifndef IETL_JD_SOLVER_H
#define IETL_JD_SOLVER_H

#include "utils/zout.hpp"
#include "utils/DmrgParameters.h"

#include "ietl_lanczos_solver.h"

#include <ietl/jacobi.h>

template<class Matrix, class SymmGroup>
std::pair<double, MPSTensor<Matrix, SymmGroup> >
solve_ietl_jcd(SiteProblem<Matrix, SymmGroup> & sp,
        MPSTensor<Matrix, SymmGroup> const & initial,
        BaseParameters & params)
{
    typedef MPSTensor<Matrix, SymmGroup> Vector;
    
    SingleSiteVS<Matrix, SymmGroup> vs(initial);
    
    typedef ietl::vectorspace<Vector> Vecspace;
    typedef boost::lagged_fibonacci607 Gen;
    
    ietl::jcd_simple_solver<SiteProblem<Matrix, SymmGroup>, SingleSiteVS<Matrix, SymmGroup> > jcd_ss(sp, vs);
    ietl::jacobi_davidson<SiteProblem<Matrix, SymmGroup>, SingleSiteVS<Matrix, SymmGroup> > jd(sp, vs, ietl::Smallest);
    
    double tol = params.get<double>("ietl_jcd_tol");
    ietl::basic_iteration<double> iter(100, tol, tol);
    
    std::pair<double, Vector> r0 = jd.calculate_eigenvalue(initial, jcd_ss, iter);
    zout << "JCD used " << iter.iterations() << " iterations." << endl;
    
    return r0;
}

#endif

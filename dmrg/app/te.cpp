#include <cmath>
#include <iterator>
#include <iostream>
#include <sys/time.h>
#include <sys/stat.h>

using std::cerr;
using std::cout;
using std::endl;

#include "dense_matrix/dense_matrix.h"
#include "dense_matrix/matrix_interface.hpp"
#include "dense_matrix/resizable_matrix_interface.hpp"
#include "dense_matrix/dense_matrix_algorithms.h"
#include "dense_matrix/matrix_algorithms.hpp"
#include "dense_matrix/dense_matrix_blas.hpp"
#include "dense_matrix/aligned_allocator.h"

#ifdef USE_GPU
#include <cublas.h>
#endif

#ifdef USE_MTM
#define USE_MTM_MAIN
#include "dense_matrix/mt_matrix.h"
typedef mt_matrix<double> Matrix;
#else
typedef blas::dense_matrix<double> Matrix;
#endif

#include <alps/hdf5.hpp>

template<class T, class A>
alps::hdf5::oarchive & serialize(alps::hdf5::oarchive & ar,
                                 std::string const & p,
                                 std::vector<T, A> const & v)
{
    std::vector<T> foo(v.begin(), v.end());
    ar << alps::make_pvp(p, foo);
    return ar;
}

template<class T, class A>
alps::hdf5::iarchive & serialize(alps::hdf5::iarchive & ar,
                                 std::string const & p,
                                 std::vector<T, A> & v)
{
    std::vector<T> foo;
    ar >> alps::make_pvp(p, foo);
    v.resize(foo.size());
    std::copy(foo.begin(), foo.end(), v.begin());
    return ar;
}

#include "block_matrix/indexing.h"
#include "mp_tensors/mps.h"
#include "mp_tensors/mpo.h"
#include "mp_tensors/contractions.h"
#include "mp_tensors/mps_mpo_ops.h"
#include "mp_tensors/mpo_ops.h"
#include "mp_tensors/mps_initializers.h"

#include "utils/stream_storage.h"
#include "utils/logger.h"

#include "te_utils.hpp"
#include "mp_tensors/te.h"

#include "utils/DmrgParameters.h"

#include "models.h"

using namespace app;

#ifdef UseTwoU1
typedef TwoU1 grp;
#else
typedef U1 grp;
#endif

typedef std::vector<MPOTensor<Matrix, grp> > mpo_t;
typedef Boundary<Matrix, grp> boundary_t;

template<class Matrix>
mps_initializer<Matrix, grp> * initializer_factory(BaseParameters & params)
{
    if (params.get<std::string>("init_state") == "default")
        return new default_mps_init<Matrix, grp>();
    else if (params.get<std::string>("init_state") == "mott")
        return new mott_mps_init<Matrix, grp>();
    else if (params.get<std::string>("init_state") == "thin")
        return new thin_mps_init<Matrix, grp>();
    else {
        throw std::runtime_error("Don't know this initial state.");
        return NULL;
    }
}

int main(int argc, char ** argv)
{
    if (argc != 3)
    {
        cout << "Usage: <parms> <model_parms>" << endl;
        exit(1);
    }
    
    cout.precision(10);
    
#ifdef USE_GPU
	cublasInit();
#endif
    
    std::ifstream param_file(argv[1]);
    if (!param_file) {
        cerr << "Could not open parameter file." << endl;
        exit(1);
    }
    DmrgParameters parms(param_file);
    
    std::string model_file(argv[2]);
    
    std::string chkpfile = parms.get<std::string>("chkpfile");
    std::string rfile = parms.get<std::string>("resultfile");
    bool dns = (parms.get<int>("donotsave") != 0);
    
    bool restore = false;
    {
        struct stat tmp;
        if (stat(chkpfile.c_str(), &tmp) == 0 && S_ISREG(tmp.st_mode))
        {
            cout << "Restoring state." << endl;
            restore = true;
        }
    }
    
    srand48(parms.get<int>("seed"));
    
    Lattice * lat;
    Hamiltonian<Matrix, grp> H;
    grp::charge initc;
    Measurements<Matrix, grp> measurements;
    ModelParameters model = model_parser(parms.get<std::string>("model_library"), model_file, lat, H, initc, measurements);
    Index<grp> phys = H.get_phys();
    std::cout << "initc: " << initc << std::endl;
    
    std::cout << measurements << std::endl;
    
    std::vector<Hamiltonian<Matrix, grp> > splitted_H = separate_overlaps(H);
    std::vector<MPO<Matrix, grp> > expMPO(splitted_H.size(), MPO<Matrix, grp>(lat->size()));
    for (int i=0; i<splitted_H.size(); ++i)
        expMPO[i] = make_exp_mpo(lat->size(), splitted_H[i]);
    
    Measurements<Matrix, grp> meas_always;
    if (!parms.get<std::string>("always_measure").empty()) {
        meas_always.set_identity(measurements.get_identity());
        std::vector<std::string> meas_list = parms.get<std::vector<std::string> >("always_measure");
        for (int i=0; i<meas_list.size(); ++i)
            meas_always.add_term(measurements.get(meas_list[i]));
    }
    
    MPO<Matrix, grp> mpo = make_mpo(lat->size(), H);
    MPO<Matrix, grp> mpoc = mpo;
    if (parms.get<int>("use_compressed") > 0)
        mpoc.compress(1e-12);
    
    MPS<Matrix, grp> initial_mps(lat->size(),
                                 parms.get<std::size_t>("init_bond_dimension"),
                                 phys, initc,
                                 *initializer_factory<Matrix>(parms));
    
    int sweep = 0;
    if (restore) {
        alps::hdf5::iarchive h5ar_in(chkpfile);
        h5ar_in >> alps::make_pvp("/state", initial_mps);
        h5ar_in >> alps::make_pvp("/status/sweep", sweep);
        ++sweep;
    } else if (parms.get<std::string>("initfile").size() > 0) {
        alps::hdf5::iarchive h5ar_in(parms.get<std::string>("initfile"));
        h5ar_in >> alps::make_pvp("/state", initial_mps);
    }
    
    {
        alps::hdf5::oarchive h5ar(rfile);
        h5ar << alps::make_pvp("/parameters", parms);
        h5ar << alps::make_pvp("/parameters", model);
    }
    
    if (!dns) {
        alps::hdf5::oarchive h5ar(chkpfile);
        h5ar << alps::make_pvp("/parameters", parms);
        h5ar << alps::make_pvp("/parameters", model);
    }
    
//    StreamStorageMaster ssm(parms.get<std::string>("storagedir"));
    NoopStorageMaster ssm;
    
    timeval now, then, snow, sthen;
    gettimeofday(&now, NULL);
    
    std::vector<double> energies, entropies, renyi2;
    std::vector<std::size_t> truncations;
#ifndef MEASURE_ONLY
    
    bool early_exit = false;
    {   
        time_evolve<Matrix, grp, NoopStorageMaster> evolution(initial_mps,
                                                                expMPO,
                                                                parms, ssm);
        
        for ( ; sweep < parms.get<int>("nsweeps"); ++sweep) {
            gettimeofday(&snow, NULL);
            
            Logger iteration_log;
            
            evolution.sweep(sweep, iteration_log);
            
            MPS<Matrix, grp> cur_mps = evolution.get_current_mps();
            
            entropies = calculate_bond_entropies(cur_mps);
            
            double energy = expval(cur_mps, mpoc);

            gettimeofday(&sthen, NULL);
            double elapsed = sthen.tv_sec-snow.tv_sec + 1e-6 * (sthen.tv_usec-snow.tv_usec);
            
            {
                alps::hdf5::oarchive h5ar(rfile);
                
                std::ostringstream oss;
                
                oss.str("");
                oss << "/simulation/sweep" << sweep << "/results";
                h5ar << alps::make_pvp(oss.str().c_str(), iteration_log);

                oss.str("");
                oss << "/simulation/sweep" << sweep << "/results/Energy/mean/value";
                h5ar << alps::make_pvp(oss.str().c_str(), std::vector<double>(1, energy));

                oss.str("");
                oss << "/simulation/sweep" << sweep << "/results/Iteration Entropies/mean/value";
                h5ar << alps::make_pvp(oss.str().c_str(), entropies);
                
                cout << "Sweep done after " << elapsed << " seconds." << endl;
                oss.str("");
                oss << "/simulation/sweep" << sweep << "/results/Runtime/mean/value";
                h5ar << alps::make_pvp(oss.str().c_str(), std::vector<double>(1, elapsed));                
            }
            {
                std::ostringstream oss;
                oss << "/simulation/sweep" << sweep << "/results/";
                if (meas_always.n_terms() > 0)
                    measure(cur_mps, *lat, meas_always, rfile, oss.str());
            }
            
            if (parms.get<int>("donotsave") == 0)
            {
                alps::hdf5::oarchive h5ar(chkpfile);
                
                h5ar << alps::make_pvp("/state", cur_mps);
                h5ar << alps::make_pvp("/status/sweep", sweep);
            }
            
            gettimeofday(&then, NULL);
            elapsed = then.tv_sec-now.tv_sec + 1e-6 * (then.tv_usec-now.tv_usec);            
            int rs = parms.get<int>("run_seconds");
            if (rs > 0 && elapsed > rs) {
                early_exit = true;
                break;
            }
        }
    }
#endif
        
    gettimeofday(&then, NULL);
    double elapsed = then.tv_sec-now.tv_sec + 1e-6 * (then.tv_usec-now.tv_usec);
    
    cout << "Task took " << elapsed << " seconds." << endl;
    
#ifdef USE_GPU
	cublasShutdown();
#endif
}

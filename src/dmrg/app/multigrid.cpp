//
//  multigrid.cpp
//  
//
//  Created by Michele Dolfi on 01.07.11.
//  Copyright 2011 ETHZ. All rights reserved.
//

#include <cmath>
#include <iterator>
#include <iostream>
#include <sys/time.h>
#include <sys/stat.h>

#include <boost/shared_ptr.hpp>

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

#include "utils/DmrgParameters2.h"

#include "block_matrix/indexing.h"
#include "mp_tensors/mps.h"
#include "mp_tensors/mpo.h"
#include "mp_tensors/contractions.h"
#include "mp_tensors/mps_mpo_ops.h"
#include "mp_tensors/mpo_ops.h"
#include "mp_tensors/mps_initializers.h"
#include "mp_tensors/multigrid.h"

#include "utils/stream_storage.h"
#include "utils/logger.h"

#include "mp_tensors/ss_optimize.h"

#include "cont_lattice.h"
#include "cont_model.h"

using namespace app;

#ifdef UseTwoU1
typedef TwoU1 grp;
#else
#ifdef UseNULL
typedef NullGroup grp;
#else
typedef U1 grp;
#endif
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

template <class Matrix>
void cont_model_parser (BaseParameters & parms, boost::shared_ptr<Lattice> & lattice,
                        Hamiltonian<Matrix, grp>& H, Measurements<Matrix, grp>& meas)
{
    if (parms.get<std::string>("lattice") == std::string("continuous_chain"))
        lattice = boost::shared_ptr<Lattice>(new ContChain(parms, false));
    else if (parms.get<std::string>("lattice") == std::string("periodic_continuous_chain"))
        lattice = boost::shared_ptr<Lattice>(new ContChain(parms, true));
    else
        throw std::runtime_error("Don't know this lattice!");
    
#ifdef UseNULL
    if (parms.get<std::string>("model") == std::string("optical_lattice"))
    {
        OpticalLatticeNull<Matrix> model(*lattice, parms);
        H = model.H();
        meas = model.measurements();
    } else
        throw std::runtime_error("Don't know this model!");
#else
    if (parms.get<std::string>("model") == std::string("optical_lattice"))
    {
        OpticalLattice<Matrix> model(*lattice, parms);
        H = model.H();
        meas = model.measurements();
    } else
        throw std::runtime_error("Don't know this model!");
#endif
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
        cerr << "Could not open parameter file. (" << argv[1] << ")" << endl;
        exit(1);
    }
    DmrgParameters raw_parms(param_file);
    
    std::ifstream model_file(argv[2]);
    if (!model_file) {
        cerr << "Could not open model file. (" << argv[2] << ")" << endl;
        exit(1);
    }
    ModelParameters raw_model(model_file);
    
    
    srand48(raw_parms.get<int>("seed"));
    
    std::string chkpfile = raw_parms.get<std::string>("chkpfile");
    std::string rfile = raw_parms.get<std::string>("resultfile");
    bool dns = (raw_parms.get<int>("donotsave") != 0);
    
    int graining = 0;
    int sweep = 0;
    bool restore = false;
    {
        struct stat tmp;
        if (stat(chkpfile.c_str(), &tmp) == 0 && S_ISREG(tmp.st_mode))
        {
            cout << "Restoring state." << endl;
            restore = true;
            
            alps::hdf5::archive h5ar_in(chkpfile);
            h5ar_in >> alps::make_pvp("/status/sweep", sweep);
            ++sweep;
            h5ar_in >> alps::make_pvp("/status/graining", graining);
            
        }
    }
    
    
    
    {
        alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE);
        h5ar << alps::make_pvp("/parameters", raw_parms);
        h5ar << alps::make_pvp("/parameters", raw_model);
    }
    if (!dns) {
        alps::hdf5::archive h5ar(chkpfile, alps::hdf5::archive::WRITE);
        h5ar << alps::make_pvp("/parameters", raw_parms);
        h5ar << alps::make_pvp("/parameters", raw_model);
    }
    
    grp::charge initc;
#ifdef UseTwoU1
    initc[0] = raw_model.get<int>("u1_total_charge1");
    initc[1] = raw_model.get<int>("u1_total_charge2");
#else
#ifdef UseNULL
    initc = grp::SingletCharge;
#else
    initc = raw_model.get<int>("u1_total_charge");
#endif
#endif
    std::cout << "initc: " << initc << std::endl;
    
    
    boost::shared_ptr<Lattice> lat;
    Hamiltonian<Matrix, grp> H;
    Measurements<Matrix, grp> measurements;
    Index<grp> phys;
    
    MPO<Matrix, grp> mpo(0);
    MPO<Matrix, grp> mpoc(0);
    mps_initializer<Matrix, grp> * initializer = new empty_mps_init<Matrix, grp>();
    MPS<Matrix, grp> cur_mps(0, 0, phys, grp::SingletCharge, *initializer);
    
    if (restore) {
        alps::hdf5::archive h5ar_in(chkpfile);
        h5ar_in >> alps::make_pvp("/state", cur_mps);
    } else if (raw_parms.get<std::string>("initfile").size() > 0) {
        alps::hdf5::archive h5ar_in(raw_parms.get<std::string>("initfile"));
        h5ar_in >> alps::make_pvp("/state", cur_mps);
    }
    
    timeval now, then, snow, sthen;
    gettimeofday(&now, NULL);
    for (; graining < raw_parms.get<int>("ngrainings")+1; ++graining)
    {
        
        BaseParameters parms = raw_parms.get_at_index("graining", graining);
        BaseParameters model = raw_model.get_at_index("graining", graining);
        
        std::cout << model << std::endl;

        cont_model_parser(model, lat, H, measurements);
        phys = H.get_phys();
        
        std::cout << measurements << std::endl;
        
        std::cout << H << std::endl;
        
        std::cout << "LATTICE:" << std::endl;
        for (int i=0; i<lat->size(); ++i)
            std::cout << i << ": " << lat->get_prop<double>("x", i) << std::endl;
        
        Measurements<Matrix, grp> meas_always;
        if (!parms.get<std::string>("always_measure").empty()) {
            meas_always.set_identity(measurements.get_identity());
            std::vector<std::string> meas_list = parms.get<std::vector<std::string> >("always_measure");
            for (int i=0; i<meas_list.size(); ++i)
                meas_always.add_term(measurements.get(meas_list[i]));
        }
        
        mpo = make_mpo(lat->size(), H);
        mpoc = mpo;
        if (parms.get<int>("use_compressed") > 0)
            mpoc.compress(1e-12);
        
        MPS<Matrix, grp> initial_mps(lat->size(),
                                     parms.get<std::size_t>("init_bond_dimension"),
                                     phys, initc,
                                     *initializer_factory<Matrix>(parms));
        
        
        if (cur_mps.length() > 0)
        {
            std::cout << "*** Starting grainings ***" << std::endl;
            
            std::cout << "Old MPS:" << std::endl << initial_mps.description() << std::endl;
            if (cur_mps.length() < initial_mps.length())
                multigrid::extension(cur_mps, initial_mps);
            else if (cur_mps.length() > initial_mps.length())
                multigrid::restriction(cur_mps, initial_mps);
            std::cout << "New MPS:" << std::endl << initial_mps.description();
        }
        
        
                
#ifndef MEASURE_ONLY
        
        //    BaseStorageMaster * bsm = bsm_factory(parms);
        StreamStorageMaster ssm(parms.get<std::string>("storagedir"));
        
        
        bool early_exit = false;
        cur_mps = initial_mps;
        {   
            std::cout << "*** Starting optimization ***" << std::endl;
            ss_optimize<Matrix, grp, StreamStorageMaster> optimizer(initial_mps,
                                                                    parms.get<int>("use_compressed") == 0 ? mpo : mpoc,
                                                                    parms, ssm);
            
            for ( ; sweep < parms.get<int>("nsweeps"); ++sweep) {
                gettimeofday(&snow, NULL);
                
                Logger iteration_log;
                
                optimizer.sweep(sweep, iteration_log);
                ssm.sync();
                
                cur_mps = optimizer.get_current_mps();
                
                std::vector<double> energies, entropies, renyi2;
                std::vector<std::size_t> truncations;

                entropies = calculate_bond_entropies(cur_mps);
                
                gettimeofday(&sthen, NULL);
                double elapsed = sthen.tv_sec-snow.tv_sec + 1e-6 * (sthen.tv_usec-snow.tv_usec);
                cout << "Sweep done after " << elapsed << " seconds." << endl;
                
                {
                    alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE);
                    
                    std::ostringstream oss;
                    oss.str("");
                    oss << "/simulation/iteration/graining/" << graining << "/sweep" << sweep << "/results";
                    
                    h5ar << alps::make_pvp(oss.str(), iteration_log);                    
                    h5ar << alps::make_pvp(oss.str()+"/Iteration Entropies/mean/value", entropies);
                    h5ar << alps::make_pvp(oss.str()+"/Runtime/mean/value", std::vector<double>(1, elapsed));                
                }
                {
                    std::ostringstream oss;
                    oss << "/simulation/iteration/graining/" << graining << "/sweep" << sweep << "/results/";
                    if (meas_always.n_terms() > 0)
                        measure(cur_mps, *lat, meas_always, rfile, oss.str());
                    
                    alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE);
                    double dens;
                    h5ar >> alps::make_pvp(oss.str()+"Density/mean/value", dens);
                    std::cout << "Density: " << dens << std::endl;
                    
                }
                
                if (!dns)
                {
                    alps::hdf5::archive h5ar(chkpfile, alps::hdf5::archive::WRITE);
                    
                    h5ar << alps::make_pvp("/state", cur_mps);
                    h5ar << alps::make_pvp("/status/sweep", sweep);
                    h5ar << alps::make_pvp("/status/graining", graining);
                }
                
                gettimeofday(&then, NULL);
                elapsed = then.tv_sec-now.tv_sec + 1e-6 * (then.tv_usec-now.tv_usec);            
                int rs = parms.get<int>("run_seconds");
                if (rs > 0 && elapsed > rs) {
                    early_exit = true;
                    break;
                }
            }
            ssm.sync();
        }
        sweep = 0;
#endif
    }
    
#ifdef MEASURE_ONLY
    {
        MPS<Matrix, grp> mps = cur_mps;
        
        std::vector<double> energies, entropies, renyi2;
        std::vector<std::size_t> truncations;
        
        cout << "Measurements." << endl;
        measure(mps, *lat, measurements, rfile);
        
        Timer tvn("vN entropy"), tr2("Renyi n=2");
        cout << "Calculating vN entropy." << endl;
        tvn.begin(); entropies = calculate_bond_entropies(mps); tvn.end();
        cout << "Calculating n=2 Renyi entropy." << endl;
        tr2.begin(); renyi2 = calculate_bond_renyi_entropies(mps, 2); tr2.end();
        
        {
            alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE);
            if (entropies.size() > 0)
                h5ar << alps::make_pvp("/spectrum/results/Entropy/mean/value", entropies);
            if (renyi2.size() > 0)
                h5ar << alps::make_pvp("/spectrum/results/Renyi2/mean/value", renyi2);
        }
        
        double energy = expval(mps, mpoc);
        cout << "Energy before: " << expval(mps, mpo) << endl;
        cout << "Energy after: " << expval(mps, mpoc) << endl;
        {
            alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE);
            h5ar << alps::make_pvp("/spectrum/results/Energy/mean/value", std::vector<double>(1, energy));
        }
        
        if (parms.get<int>("calc_h2") > 0) {
            Timer tt1("square"), tt2("compress");
            tt1.begin(); MPO<Matrix, grp> mpo2 = square_mpo(mpo); tt1.end();
            tt2.begin(); mpo2.compress(1e-12); tt2.end();
            
            Timer t3("expval mpo2"), t4("expval mpo2c");
            
            t4.begin();
            double energy2 = expval(mps, mpo2, true);
            t4.end();
            
            cout << "Energy^2: " << energy2 << endl;
            cout << "Variance: " << energy2 - energy*energy << endl;
            
            {
                alps::hdf5::archive h5ar(rfile, alps::hdf5::archive::WRITE);
                h5ar << alps::make_pvp("/spectrum/results/Energy^2/mean/value", std::vector<double>(1, energy2));
                h5ar << alps::make_pvp("/spectrum/results/EnergyVariance/mean/value",
                                       std::vector<double>(1, energy2 - energy*energy));
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

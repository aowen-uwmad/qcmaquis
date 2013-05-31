/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef DMRGPARAMETERS2_H
#define DMRGPARAMETERS2_H

#include "BaseParameters.h"

class DmrgParameters : public BaseParameters
{
public:
    DmrgParameters() : BaseParameters() { init_options(); }
    DmrgParameters(std::ifstream& param_file)
    : BaseParameters(param_file)
    {
        init_options();
    }
    DmrgParameters(BaseParameters const& p)
    : BaseParameters(p)
    {
        init_options();
    }
    
private:
    
    void init_options()
    {
        using parameters::value;
        
        add_option("truncation_initial", "Initial value for the truncation error", value(1e-16));
        add_option("truncation_final", "Final value for the truncation", value(1e-16));
        
        add_option("init_bond_dimension", "", value(5));
        add_option("max_bond_dimension", "");
        add_option("sweep_bond_dimensions", "");

        add_option("optimization", "singlesite or twosite", value("singlesite"));
        
        add_option("alpha_initial","", value(1e-2));
        add_option("alpha_main", "", value(1e-4));
        add_option("alpha_final", "", value(1e-8));
        
        add_option("eigensolver", "", value("IETL_JCD"));
        add_option("ietl_jcd_tol", "", value(1e-8));
        add_option("ietl_jcd_gmres", "", value(0));
        add_option("ietl_jcd_maxiter", "", value(12));
        
        add_option("nsweeps", "");
        add_option("nmainsweeps", "");
        add_option("ngrowsweeps", "");
        
        add_option("resultfile", "");
        add_option("chkpfile", "");
        add_option("initfile", "", value(""));
        
        add_option("donotsave", "", value(0));
        add_option("run_seconds", "", value(0));
        add_option("storagedir", "", value(""));
        add_option("use_compressed", "", value(0));
        add_option("calc_h2", "", value(0));
        add_option("seed", "", value(42));
        add_option("always_measure", "comma separated list of measurements", value(""));
        add_option("measure_each", "", value(1)); 
        add_option("chkp_each", "", value(1)); 
        add_option("update_each", "", value(-1));
        add_option("entanglement_spectra", "", value(0));
        
        
        add_option("expm_method", "algorithm used for exp(-i H dt): heev (default), geev", value("heev"));
        add_option("te_type", "time evolution algorithm: nn (default), mpo", value("nn"));
        add_option("te_optim", "optimized nn time evolution", value(true));
		add_option("te_order", "trotter decomposition: second, fourth (default)", value("fourth"));
        add_option("dt", "time step in time eovlution", value(1e-3));
        add_option("nsweeps_img", "number of imaginary time steps", value(0));
        add_option("exp_rescaling", "rescale H before exp(-I*H*dt)", value(1.));
        
        add_option("ngrainings", "", value(0));
        add_option("finegrain_optim", "", value(false));
        
        add_option("init_state", "", value("default"));
        add_option("init_coeff", "coefficients for coherent init", value(""));
        add_option("init_basis_state", "local indexes for basis state init", value(""));
        
        add_option("symmetry", "null, u1 or 2u1", value("u1"));
        add_option("lattice_library", "", value("alps"));
        add_option("model_library", "", value("alps"));
        
        add_option("beta_mode", "", value(0));

        add_option("n_ortho_states", "", value(0));
        add_option("ortho_states", "", value(""));
    }
    
};

class ModelParameters : public BaseParameters
{
public:
    ModelParameters() : BaseParameters() { init_options(); }
	ModelParameters(std::ifstream& param_file)
    : BaseParameters(param_file)
    {
        init_options();
    }
	ModelParameters(BaseParameters const& p)
    : BaseParameters(p)
    {
        init_options();
    }
    
    
private:
    
    void init_options()
    {
        using parameters::value;
        
        add_option("MODEL", "");
        add_option("LATTICE", "");
        add_option("alps_lattice", "");
        
        add_option("L", "");
        add_option("W", "");
        
        add_option("Jxy", "");
        add_option("Jx", "");
        add_option("Jy", "");
        add_option("Jz", "");
        add_option("Jxy1", "");
        add_option("Jz1", "");
        add_option("J1", "");
        add_option("J2", "");
        
        add_option("U", "");
        add_option("t", "");
        add_option("t1", "");
        add_option("t2", "");
        
        add_option("theta", "");
        add_option("h0", "");
        add_option("pin", "");
        add_option("y", "", value(1));
        add_option("x", "", value(1));
        add_option("z", "", value(1));
        add_option("delta", "");
        
        add_option("K0", "");
        add_option("K1", "");
        
        add_option("penalty", "");
        add_option("twist", "");
        add_option("move", "");
        
        add_option("Nmax", "");
        add_option("mu", "", value(0));
        add_option("mu0", "", value(0));
        add_option("h", "", value(1));
        add_option("c", "", value(0));
        add_option("V0", "", value(0));
        add_option("k", "", value(0));
        add_option("a", "", value(1));
        add_option("Ndiscr", "");
        add_option("omega", "", value(0.));
        add_option("shift", "", value(0.));

        add_option("V"      , "", value(0.));
        add_option("Lambda"  , "", value(0.));
        add_option("Delta"  , "", value(0.));
        add_option("Gamma1a", "", value(0.));
        add_option("Gamma1b", "", value(0.));
        add_option("Gamma2" , "", value(0.));
        add_option("nbar"   , "", value(0.));
        
        add_option("u1_total_charge", "");
        add_option("u1_total_charge1", "");
        add_option("u1_total_charge2", "");
        
        add_option("MEASURE_CONTINUUM[Psi energy]", "", value(false));
        add_option("MEASURE_CONTINUUM[Density]", "", value(true));
        add_option("MEASURE_CONTINUUM[Local density]", "", value(true));
        add_option("MEASURE_CONTINUUM[Onebody density matrix]", "", value(false));

        add_option("ENABLE_MEASURE[Density]", "", value(false));
        add_option("ENABLE_MEASURE[Local density]", "", value(false));
        add_option("ENABLE_MEASURE[Local density^2]", "", value(false));
        add_option("ENABLE_MEASURE[Onebody density matrix]", "", value(false));
        add_option("ENABLE_MEASURE[Density correlation]", "", value(false));

        add_option("RUN_FINITE_T", "", value(false));
   }

};

#endif

#!/bin/bash
STATE="void"
STATE_AMBIENT="void"
STATE_TYPES="build"
STATE_DMRG="void"
# end of target states #
SITE=eiger # the dashboard name (uname -n)
ROOT_DIR=~/maquis2012/src
COMPILER=iccxe
COMPILER_VERSION=2011
MPI_WRAPPER=impi
MPI_WRAPPER_VERSION=4.0
BOOST=boost
BOOST_VERSION=1.46.1
CMAKE=cmake

export CXX=icpc
export CC=icc
export BOOST_ROOT=/apps/eiger/boost_1_46_1

module load ${COMPILER}/${COMPILER_VERSION}
module load ${MPI_WRAPPER}/${MPI_WRAPPER_VERSION}
module load ${BOOST}/${BOOST_VERSION}
module load ${CMAKE}

## common ##
MAQUIS_COMMON_CMAKE_C_COMPILER=icc
MAQUIS_COMMON_CMAKE_CXX_COMPILER=icpc
MAQUIS_COMMON_MPI_CXX_FOUND=TRUE
MAQUIS_COMMON_MPI_CXX_INCLUDE_PATH=/apps/eiger/Intel-MPI-4.0/intel64/include
MAQUIS_COMMON_MPI_CXX_LINK_FLAGS=""
MAQUIS_COMMON_MPI_CXX_LIBRARIES="-L/apps/eiger/Intel-MPI-4.0/lib64 -lmpi_mt"
MAQUIS_COMMON_MPIEXEC=/apps/eiger/Intel-MPI-4.0/intel64/bin/mpirun
MAQUIS_COMMON_MPIEXEC_NUMPROC_FLAG="-np"
MAQUIS_COMMON_MPIEXEC_MAX_NUMPROCS="2"
MAQUIS_COMMON_MPIEXEC_POSTFLAGS=""
## types ##
MAQUIS_TYPES_BUILD_AMBIENT=ON 
MAQUIS_TYPES_ENABLE_PARALLEL=ON
MAQUIS_TYPES_ENABLE_REGRESSION_FUNCTIONAL=ON
MAQUIS_TYPES_BOOST_BINDINGS_INCLUDE=/project/h07/ALPS_INTEL_EIGER/include
## dmrg ##
MAQUIS_DMRG_DEFAULT_BLAS_LAPACK=manual
MAQUIS_DMRG_BLAS_LAPACK_MANUAL_LIBS="-lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread"
MAQUIS_DMRG_BLAS_LAPACK_MANUAL_LIBS_DIR=/apps/eiger/Intel-Composer-XE-2011/mkl/lib/intel64
MAQUIS_DMRG_BLAS_LAPACK_MANUAL_INCLUDES=/apps/eiger/Intel-Composer-XE-2011/mkl/include
MAQUIS_DMRG_ALPS_ROOT_DIR=/project/h07/ALPS_INTEL_EIGER
MAQUIS_DMRG_BUILD_REGRESSION=ON 
MAQUIS_DMRG_BUILD_AMBIENT=ON 
MAQUIS_DMRG_USE_AMBIENT=ON

source common.sh 
execute $*

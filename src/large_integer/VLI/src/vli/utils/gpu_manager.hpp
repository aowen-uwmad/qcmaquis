/*
 *  GpuManager.cpp
 *  Untitled
 *
 *  Created by Tim Ewart on 24/03/11.
 *  Copyright 2011 Univerisité de Geneve . All rights reserved.
 *
 */

#include "vli/utils/gpu_manager.h"

namespace gpu 
{
	gpu_manager::gpu_manager(int device = 0):device_(device)
	{
#ifndef CUDA_NO_SM_11_ATOMIC_INTRINSICS
        printf("WARNING! Not using atomics!\n");
#endif

		cuInit(device_);
		cudaGetDeviceProperties(&deviceProp_, device_);
	}

        gpu_manager::~gpu_manager()
	{
	}
    
	gpu_manager& gpu_manager::instance()
	{
		static gpu_manager* singleton = NULL;
		if (!singleton)
		{
			singleton = new gpu_manager();
		}
		return *singleton;
	}
    
    void gpu_manager::constructor()
    {
        this->instance();        
    }
    
	void gpu_manager::destructor()
	{
	}
	
    std::size_t gpu_manager::GetmaxThreadsPerBlock()
    {
        return this->instance().GetDeviceProperties().maxThreadsPerBlock; 
    }    
    	
	cudaDeviceProp gpu_manager::GetDeviceProperties() const
	{
		return deviceProp_;
	}
	
	
}


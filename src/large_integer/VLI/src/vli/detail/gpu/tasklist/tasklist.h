/*
*Very Large Integer Library, License - Version 1.0 - May 3rd, 2012
*
*Timothee Ewart - University of Geneva, 
*Andreas Hehn - Swiss Federal Institute of technology Zurich.
*Maxim Milakov – NVIDIA 
*
*Permission is hereby granted, free of charge, to any person or organization
*obtaining a copy of the software and accompanying documentation covered by
*this license (the "Software") to use, reproduce, display, distribute,
*execute, and transmit the Software, and to prepare derivative works of the
*Software, and to permit third-parties to whom the Software is furnished to
*do so, all subject to the following:
*
*The copyright notices in the Software and this entire statement, including
*the above license grant, this restriction and the following disclaimer,
*must be included in all copies of the Software, in whole or in part, and
*all derivative works of the Software, unless such copies or derivative
*works are solely in the form of machine-executable object code generated by
*a source language processor.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
*SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
*FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
*ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*DEALINGS IN THE SOFTWARE.
*/
#ifndef GPU_HARDWARE_CARRYOVER_IMPLEMENTATION_H
#define GPU_HARDWARE_CARRYOVER_IMPLEMENTATION_H

#include "vli/detail/gpu/kernels/numeric.h"

namespace vli {
    namespace detail {

    template <std::size_t NumBits, class MaxOrder, int NumVars>
    class tasklist_keep_order;

    template <std::size_t NumBits, int Order, int NumVars>
    class tasklist_keep_order<NumBits, max_order_each<Order>, NumVars >  {
        public:
        tasklist_keep_order();
        void plan();
        single_coefficient_task* execution_plan_;// we do not care the type
        unsigned int* workblock_count_by_warp_; // we do not care the type
        unsigned int  element_count_prepared;
    };

    template <std::size_t NumBits, int Order, int NumVars>
    class tasklist_keep_order<NumBits, max_order_combined<Order>, NumVars> {
        public:
        tasklist_keep_order();
        void plan();
        single_coefficient_task* execution_plan_;// we do not care the type
        unsigned int* workblock_count_by_warp_; // we do not care the type
        unsigned int  element_count_prepared;
    };

    } // end namespace detail
 }//end namespace vli

#include "vli/detail/gpu/tasklist/tasklist_max_order_each.hpp"
#include "vli/detail/gpu/tasklist/tasklist_max_order_combined.hpp"

#endif 

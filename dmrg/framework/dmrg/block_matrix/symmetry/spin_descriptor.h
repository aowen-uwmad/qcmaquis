/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *               2014-2014 by Sebastian Keller <sebkelle@phys.ethz.ch>
 * 
 * This software is part of the ALPS Applications, published under the ALPS
 * Application License; you can use, redistribute it and/or modify it under
 * the terms of the license, either version 1 or (at your option) any later
 * version.
 * 
 * You should have received a copy of the ALPS Application License along with
 * the ALPS Applications; see the file LICENSE.txt. If not, the license is also
 * available from http://alps.comp-phys.org/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef SPIN_DESCRIPTOR_H
#define SPIN_DESCRIPTOR_H

#include "dmrg/block_matrix/symmetry/symmetry_traits.h"

template <class SymmType>
class SpinDescriptor
{
public:
    void clear() { }
    int get() const { return 0; }

    bool operator!=(SpinDescriptor rhs) const { return false; }
};

template <>
class SpinDescriptor<SymmTraits::SU2Tag>
{
    typedef int spin_t;
public:
    SpinDescriptor() : twoS(0), twoSaction(0) {}
    SpinDescriptor(spin_t twoS_) : twoS(twoS_), twoSaction(0) {}
    SpinDescriptor(spin_t twoS_, spin_t twoSaction_) : twoS(twoS_), twoSaction(twoSaction_) {}

    SpinDescriptor & operator += (SpinDescriptor rhs)
    {
        // apply action of operator rhs 
        twoS += rhs.twoSaction;
        return *this;
    }

    SpinDescriptor & operator-()
    {
        twoSaction = -twoSaction;
    }

    spin_t get() const { return twoS; }

    void clear()
    {
        twoS = 0; twoSaction = 0;
    }

    bool operator!=(SpinDescriptor rhs) const { twoS != rhs.twoS || twoSaction != rhs.twoSaction; }

private:
    spin_t twoS;
    spin_t twoSaction; // only used for operators in the MPO
};

template <class SymmType>
inline
SpinDescriptor<SymmType> couple(SpinDescriptor<SymmType> a, SpinDescriptor<SymmType> b)
{
    return a;
}

template <>
inline
SpinDescriptor<SymmTraits::SU2Tag> couple(SpinDescriptor<SymmTraits::SU2Tag> a, SpinDescriptor<SymmTraits::SU2Tag> b)
{
    return a += b;
}

#endif

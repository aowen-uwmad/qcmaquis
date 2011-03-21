/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef SYMMETRY_H
#define SYMMETRY_H

#include <iostream>
#include <vector>
#include <list>

#ifdef HAVE_ALPS_HDF5
#include <alps/hdf5.hpp>
#endif

class NullGroup
{
public:
	typedef enum { Plus } charge;
	static const charge SingletCharge = Plus;
    
	static inline charge fuse(charge a, charge b) { return Plus; }
	template<int R> static charge fuse(boost::array<charge, R>) { return Plus; }
};

#ifdef HAVE_ALPS_HDF5
inline alps::hdf5::oarchive & serialize(alps::hdf5::oarchive & ar,
                                        std::string const & p,
                                        NullGroup::charge const & v)
{
    int k = 1;
    ar << alps::make_pvp(p, k);
    return ar;
}
        
inline alps::hdf5:: iarchive & serialize(alps::hdf5::iarchive & ar,
                                         std::string const & p,
                                         NullGroup::charge & v) {
    v = NullGroup::Plus;
    return ar;
}
#endif

inline NullGroup::charge operator-(NullGroup::charge a) { return a; }

class Ztwo
	{
	public:
		typedef enum { Plus, Minus } charge;
		
		static const charge SingletCharge = Plus;
		
		static inline charge fuse(charge a, charge b)
		{
			if (a == b)
				return Plus;
			else
				return Minus;
		}
		
		template<int R>
		static charge fuse(boost::array<charge, R> v)
		{
			// this operation actually could be rearranged into a tree
			for (int i = 1; i < R; i++)
				v[0] = fuse(v[0], v[i]);
			return v[0];
		}
	};	

#ifdef HAVE_ALPS_HDF5
alps::hdf5::oarchive & serialize(alps::hdf5::oarchive & ar,
                                 std::string const & p,
                                 Ztwo::charge const & v);
alps::hdf5::iarchive & serialize(alps::hdf5::iarchive & ar,
                                 std::string const & p,
                                 Ztwo::charge & v);
#endif

inline Ztwo::charge operator-(Ztwo::charge a) { return a; }

inline std::ostream& operator<<(std::ostream& ost, Ztwo::charge c)
{
	if (c == Ztwo::Plus)
		ost << "Plus";
	else if (c == Ztwo::Minus)
		ost << "Minus";
	else
		ost << "???";
	return ost;
}
inline std::ostream& operator<<(std::ostream& ost, const std::vector<Ztwo::charge> &c)
{
	ost << "[ ";
	for (std::vector<Ztwo::charge>::const_iterator it = c.begin();
		 it != c.end();
		 it++)
		ost << ", " << *it;
	ost << " ]";
	return ost;
}

class U1
{
public:
	typedef int charge;

	static const charge SingletCharge = 0;
	
	static charge fuse(charge a, charge b) { return a + b; }
	
	template<int R> static charge fuse(const boost::array<charge, R> &v)
	{
		charge ret = 0;
		for (int i = 0; i < R; i++)
			ret += v[i];
		return ret;
	}
};

template<int Q>
class ZqCharge
{
    friend ZqCharge<Q> operator+(ZqCharge<Q> a, ZqCharge<Q> b)
    {
        return ZqCharge<Q>((a.c_+b.c_)%Q);
    }

    friend std::ostream& operator<<(std::ostream& os, ZqCharge<Q> c)
    {
        os << c.get();
        return os;
    }

    friend bool operator<(ZqCharge<Q> a, ZqCharge<Q> b)
    {
        return a.c_ < b.c_;
    }
    
public:
    ZqCharge(unsigned int c = 0) : c_(c) { }
    
    bool operator==(ZqCharge<Q> b) const { return c_ == b.c_; }
    bool operator!=(ZqCharge<Q> b) const { return c_ != b.c_; }
    ZqCharge operator-() const
    {
        if (c_ == 0)
            return 0;
        else
            return Q-c_;
    }
    
    unsigned int get() { return c_; }
    
#ifdef HAVE_ALPS_HDF5
    void serialize(alps::hdf5::oarchive & ar) const { ar << alps::make_pvp("c", c_); }
    void serialize(alps::hdf5::iarchive & ar) { ar >> alps::make_pvp("c", c_); }
#endif
    
protected:
    unsigned int c_;
};

template<int Q>
class Zq
{
public:
    typedef ZqCharge<Q> charge;
    
    static const charge SingletCharge;
    static const int q = Q;
    
    static charge fuse(charge a, charge b)
    {
        return a+b;
    }
    
    template<int R> static charge fuse(const boost::array<charge, R> &v)
    {
        charge ret = v[0];
        for (int i = 1; i < R; i++)
            ret = fuse(ret, v[i]);
        return ret;
    }
};

template<int Q> const typename Zq<Q>::charge Zq<Q>::SingletCharge = ZqCharge<Q>(0);

template<int N>
class NU1Charge : public boost::array<int, N>
{   
public:
    NU1Charge()
    {
        for (int i = 0; i < N; ++i) (*this)[i] = 0;
    }
    
    NU1Charge operator-() const
    {
        NU1Charge ret;
        for (int i = 0; i < N; ++i)
            ret[i] = -(*this)[i];
        return ret;
    }
    
#ifdef HAVE_ALPS_HDF5
    void serialize(alps::hdf5::oarchive & ar) const
    {
        for (int i = 0; i < N; ++i)
            ar << alps::make_pvp(boost::lexical_cast<std::string>(i), (*this)[i]);
    }
    
    void serialize(alps::hdf5::iarchive & ar)
    {
        for (int i = 0; i < N; ++i)
            ar >> alps::make_pvp(boost::lexical_cast<std::string>(i), (*this)[i]);
    }
#endif
};

template<int N>
NU1Charge<N> operator+(NU1Charge<N> const & a,
                       NU1Charge<N> const & b)
{
    NU1Charge<N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = a[i] + b[i];
    return ret;
}

template<int N>
std::ostream& operator<<(std::ostream& os, NU1Charge<N> const & c)
{
    os << "<";
    for (int i = 0; i < N; ++i) {
        os << c[i];
        if (i+1 < N)
            os << ",";
    }
    os << ">";
    return os;
}

template<int N>
class NU1
{
public:
    typedef NU1Charge<N> charge;
    typedef std::vector<charge> charge_v;
    
    static const charge SingletCharge;

    static charge fuse(charge a, charge b)
    {
        return a+b;
    }
    
    template<int R> static charge fuse(boost::array<charge, R> const & v)
    {
        charge ret = v[0];
        for (int i = 1; i < R; ++i)
            ret = fuse(ret, v[i]);
        return ret;
    }
};

template<int N> const typename NU1<N>::charge NU1<N>::SingletCharge = typename NU1<N>::charge();

typedef NU1<2> TwoU1;
typedef NU1<3> ThreeU1;

#endif

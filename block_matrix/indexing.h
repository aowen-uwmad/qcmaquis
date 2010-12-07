#ifndef TENSOR_INDEXING_H
#define TENSOR_INDEXING_H

#include <vector>
#include <algorithm>
#include <utility>

#include <boost/array.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#ifdef PYTHON_EXPORTS
#include <cpp_dmrg/wrappers.h>
#endif

enum IndexName { alpha, sigma, beta, empty };

namespace index_detail
{
    template<class SymmGroup>
    bool lt(std::pair<typename SymmGroup::charge, std::size_t> const & a,
            std::pair<typename SymmGroup::charge, std::size_t> const & b)
    {
        return a.first < b.first;
    }
    
    template<class SymmGroup>
    bool gt(std::pair<typename SymmGroup::charge, std::size_t> const & a,
            std::pair<typename SymmGroup::charge, std::size_t> const & b)
    {
        return a.first > b.first;
    }
    
    template<class SymmGroup>
    typename SymmGroup::charge get_first(std::pair<typename SymmGroup::charge, std::size_t> const & x)
    {
        return x.first;
    }
}

template<class SymmGroup>
class basis_iterator_;

template<class SymmGroup>
class Index : public std::vector<std::pair<typename SymmGroup::charge, std::size_t> >
{
public:
    typedef typename SymmGroup::charge charge;
    typedef typename std::vector<std::pair<typename SymmGroup::charge, std::size_t> >::iterator iterator;
    typedef typename std::vector<std::pair<typename SymmGroup::charge, std::size_t> >::const_iterator const_iterator;
    
    typedef basis_iterator_<SymmGroup> basis_iterator;
    
    IndexName name;
    
    std::size_t get_size(charge c) const
    {
        return std::find_if(this->begin(), this->end(),
                            c == boost::lambda::bind(index_detail::get_first<SymmGroup>,
                                                     boost::lambda::_1))->second;
    }
    
    std::size_t position(charge c) const
    {
        return std::find_if(this->begin(), this->end(),
                            c == boost::lambda::bind(index_detail::get_first<SymmGroup>,
                                                     boost::lambda::_1)) - this->begin();
    }
    
    std::size_t destination(charge c) const
    {
        return std::find_if(this->begin(), this->end(),
                            boost::lambda::bind(index_detail::lt<SymmGroup>,
                                                boost::lambda::_1,
                                                std::make_pair(c, 0))) - this->begin();
    }
    
    bool has(charge c) const
    {
        return std::count_if(this->begin(), this->end(),
                             boost::lambda::bind(index_detail::lt<SymmGroup>,
                                                 boost::lambda::_1,
                                                 std::make_pair(c, 0))) > 0;
    }
    
    void sort()
    {
        std::sort(this->begin(), this->end(), index_detail::gt<SymmGroup>);
    }
    
    std::size_t insert(std::pair<charge, std::size_t> const & x)
    {
        std::size_t d = destination(x.first);
        std::vector<std::pair<charge, std::size_t> >::insert(this->begin() + d, x);
        return d;
    }
    
    Index(IndexName n = empty)
    : std::vector<std::pair<charge, std::size_t> >()
    , name(n)
    { }
    
    Index(std::size_t M, IndexName n = empty)
    : std::vector<std::pair<charge, std::size_t> >(1, std::make_pair(SymmGroup::SingletCharge, M))
    , name(n)
    { }
    
    Index(std::vector<charge> const & charges, std::vector<std::size_t> const & sizes, IndexName n = empty)
    : name(n)
    {
        assert(sizes.size() == charges.size());
        std::transform(charges.begin(), charges.end(), sizes.begin(), std::back_inserter(*this),
                       std::make_pair<charge, std::size_t>);
    }
    
    template<int L>
    Index(boost::array<charge, L> const & charges, boost::array<std::size_t, L> const & sizes, IndexName n = empty)
    : name(n)
    {
        std::transform(charges.begin(), charges.end(), sizes.begin(), std::back_inserter(*this),
                       std::make_pair<charge, std::size_t>);
    }
    
    std::vector<charge> charges() const
    {
        std::vector<charge> ret(this->size());
        for (std::size_t k = 0; k < this->size(); ++k) ret[k] = (*this)[k].first;
        return ret;
    }
    
    std::vector<std::size_t> sizes() const
    {
        std::vector<std::size_t> ret(this->size());
        for (std::size_t k = 0; k < this->size(); ++k) ret[k] = (*this)[k].second;
        return ret;
    }
    
    bool operator==(const Index<SymmGroup> &o)
    {
        return name == o.name && this->size() == o.size() && std::equal(this->begin(), this->end(), o.begin());
    }
    
    basis_iterator basis_begin() const
    {
        return basis_iterator(*this);
    }
    
#ifdef PYTHON_EXPORTS
    std::size_t py_insert(wrapped_pair<SymmGroup> p)
    {
        return this->insert(p.data_);
    }
    
    list py_sizes() const
    {
        list l;
        for (const_iterator it = this->begin(); it != this->end(); ++it)
            l.append(it->second);
        return l;
    }
    
    list py_charges() const
    {
        list l;
        for (const_iterator it = this->begin(); it != this->end(); ++it)
            l.append(typename charge_wrapped_as<SymmGroup>::type(it->second));
        return l;
    }
#endif /* PYTHON_EXPORTS */
};

template<class SymmGroup>
class basis_iterator_
{
public:
    typedef typename SymmGroup::charge charge;
    
    basis_iterator_(Index<SymmGroup> const & idx, bool at_end = false)
    : idx_(idx)
    , cur_block(idx.begin())
    , cur_i(0)
    , max_i(cur_block->second)
    { }
    
    std::pair<charge, std::size_t> operator*() const
    {
        return std::make_pair(cur_block->first, cur_i);
    }
    
    basis_iterator_ & operator++()
    {
        ++cur_i;
        if (cur_i != max_i)
            return *this;
        else {
            ++cur_block;
            if (cur_block != idx_.end()) {
                cur_i = 0;
                max_i = cur_block->second;
            }
            return *this;
        }
    }
    
    bool end() const
    {
        return cur_block == idx_.end();
    }
    
private:
    Index<SymmGroup> const & idx_;
    typename Index<SymmGroup>::const_iterator cur_block;
    std::size_t cur_i, max_i;
};

template<class SymmGroup>
Index<SymmGroup> adjoin(Index<SymmGroup> const & inp)
{
    typedef typename SymmGroup::charge charge;
    
    std::vector<charge> oc = inp.charges(), nc = inp.charges();
    std::transform(nc.begin(), nc.end(), nc.begin(), std::negate<charge>());
    std::sort(nc.begin(), nc.end());
    
    std::vector<std::size_t> nd(inp.size()), od = inp.sizes();
    for (unsigned int i = 0; i < nd.size(); ++i)
        nd[i] = od[std::find(oc.begin(), oc.end(),
                             -nc[i])-oc.begin()];
    
    return Index<SymmGroup>(nc, nd, inp.name);
}

template<class SymmGroup>
std::ostream& operator<<(std::ostream& os, Index<SymmGroup> const & idx)
{
    if (idx.name != empty)
        os << idx.name << std::endl;
    
    os << "|";
    for (typename Index<SymmGroup>::const_iterator it = idx.begin();
         it != idx.end();
         ++it)
    {
        os << "( " << it->first << ": " << it->second << " )";
    }
    os << "|";
    
    return os;
}

template<class SymmGroup>
Index<SymmGroup> operator*(Index<SymmGroup> const & i1,
                           Index<SymmGroup> const & i2)
{
    typedef typename SymmGroup::charge charge;
    
    Index<SymmGroup> ret;
    for (typename Index<SymmGroup>::const_iterator it1 = i1.begin(); it1 != i1.end(); ++it1)
        for (typename Index<SymmGroup>::const_iterator it2 = i2.begin(); it2 != i2.end(); ++it2)
        {
            charge pdc = SymmGroup::fuse(it1->first, it2->first);
            std::size_t ps = it1->second * it2->second;
            if (ret.has(pdc))
                ret[ret.position(pdc)].second += ps;
            else
                ret.insert(std::make_pair(pdc, ps));
        }
    ret.sort();
    return ret;
}

template<class T> boost::array<T, 1> _(T const & a)
{ 
    boost::array<T, 1> r;
    r[0] = a;
    return r;
}

#define IMPL_COMMA(tpl, type) \
tpl boost::array<type, 2> operator^(type const & a, type const & b) { \
    boost::array<type, 2> ret; \
    ret[0] = a; \
    ret[1] = b; \
    return ret; \
}
#define IMPL_COMMA_2(tpl, type) \
tpl boost::array<type, L+1> operator^(boost::array<type, L> const & a, type const & b) { \
    boost::array<type, L+1> ret; \
    std::copy(a.begin(), a.end(), ret.begin()); \
    ret[L] = b; \
    return ret; \
}

#define CO ,

IMPL_COMMA(inline, IndexName)
IMPL_COMMA(template<class SymmGroup>, Index<SymmGroup>)
IMPL_COMMA(template<class charge>, std::pair<charge CO std::size_t>)

#undef CO
#undef IMPL_COMMA
#undef IMPL_COMMA_2

template<class T, unsigned long L>
boost::array<T, L+1> operator^(boost::array<T, L> const & a, T const & b)
{
	boost::array<T, L+1> ret;
    std::copy(a.begin(), a.end(), ret.begin());
	ret[L] = b;
	return ret;
}

template<class T, unsigned long L>
boost::array<T, L+1> operator^(T const & a, boost::array<T, L> const & b)
{
	boost::array<T, L+1> ret;
	ret[0] = a;
	for (int i = 0; i < L; i++)
		ret[i+1] = b[i];
	return ret;
}

#endif

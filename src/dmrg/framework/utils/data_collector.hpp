
#ifndef MAQUIS_DATA_COLLECTOR_HPP_
#define MAQUIS_DATA_COLLECTOR_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

class DataCollector;

#ifdef ENABLE_DATACOLLECTORS

#define DCOLLECTOR_CREATE(var, name, length) DataCollector var(name, length);
#define DCOLLECTOR_GROUP(var, keyname) var.set_key(keyname);
#define DCOLLECTOR_SET_SIZE(var, num) var.set_size(num);
#define DCOLLECTOR_ADD(var, value) var.add_data(value);
#define DCOLLECTOR_ADD_VERB(var, value) var.add_data(value, true);
#define DCOLLECTOR_ADD_AT(var, keyname, value) var.add_data(keyname, value);
#define DCOLLECTOR_SAVE(var, ar, path) ar[path] << var;
#define DCOLLECTOR_SAVE_TO_FILE(var, fname, path)                           \
{                                                                           \
    storage::archive ar_dcollector(fname, "w");                             \
    ar_dcollector[path] << var;                                             \
}

extern DataCollector gemm_collector;
extern DataCollector svd_collector;
extern DataCollector num_blocks_gemm_collector;
extern DataCollector num_blocks_svd_collector;

#else

#define DCOLLECTOR_CREATE(var, name, length)
#define DCOLLECTOR_GROUP(var, keyname)
#define DCOLLECTOR_SET_SIZE(var, num)
#define DCOLLECTOR_ADD(var, value)
#define DCOLLECTOR_ADD_VERB(var, value)
#define DCOLLECTOR_ADD_AT(var, keyname, value)
#define DCOLLECTOR_SAVE(var, ar, path)
#define DCOLLECTOR_SAVE_TO_FILE(var, fname, path) 

#endif

class DataCollector
{
public:
    typedef std::size_t size_t;
    
	DataCollector(std::string const & name, std::size_t maxsize_=10) : name_(name), maxsize(maxsize_), active_key("none")
    {
        data[active_key] = std::vector<size_t>(maxsize, 0);
    }

	std::string name() const {return name_;}

	void set_key (std::string const & key)
	{
		if (data.count(key) == 0)
            data[key] = std::vector<size_t>(maxsize, 0);
        active_key = key;
	}

    void set_size (size_t size)
    {
        maxsize = size;
		if (maxsize >= data[active_key].size()) {
            data[active_key].resize(maxsize, 0);
        }
    }
    
	void add_data (const size_t& val, bool verbose=false)
	{
		if (val >= data[active_key].size()) {
            if (maxsize <= val)
                maxsize = val + 1;
            data[active_key].resize(maxsize, 0);
        }
           data[active_key][val]++;
	}
	void add_data (std::string const & key, const size_t& val)
	{
		if (val >= data[key].size()) {
            if (maxsize <= val)
                maxsize = val + 1;
            data[key].resize(maxsize, 0);
        }
        data[key][val]++;
	}

    template<class Archive>
	void save(Archive & ar) const
	{
        if (data.size() == 1) {
			ar[name_ + "/mean/value"] << data.begin()->second;
		} else if (data.size() > 1) {
			std::vector<std::string> keys;
            std::vector<std::vector<size_t> > values;
			for (std::map<std::string, std::vector<size_t> >::const_iterator it = data.begin();
				it != data.end();
				it++)
			{
                keys.push_back(it->first);
                values.push_back(it->second);
			}
			ar[name_ + "/mean/value"] << values;
			ar[name_ + "/labels"] << keys;
		}
	}

private:
    std::size_t maxsize;
	std::string active_key;
	std::string name_;
	std::map<std::string, std::vector<size_t> > data;
};


#endif /* DATA_COLLECTOR_HPP_ */

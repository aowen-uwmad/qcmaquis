#ifndef AMBIENT_INTERFACE
#define AMBIENT_INTERFACE
#ifndef AMBIENT
#define AMBIENT
#endif
// {{{ system includes
#include <mpi.h>
#include <complex>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <limits>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <memory.h>
#include <stdarg.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <algorithm>
#include <pthread.h>
#define BOOST_SP_NO_SP_CONVERTIBLE
#include <boost/shared_ptr.hpp>
// }}}
#include "ambient/utils/memory.hpp"
#include "ambient/channels/mpi/channel.h"
#include "ambient/models/velvet/model.h"
#include "ambient/controllers/velvet/controller.h"
#include "ambient/utils/auxiliary.hpp"
#include "ambient/utils/io.hpp"
#include "ambient/interface/typed.hpp"
#include "ambient/interface/kernel.hpp"
#include "ambient/interface/future.hpp"
#endif

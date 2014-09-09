/*
 * Copyright Institute for Theoretical Physics, ETH Zurich 2014.
 * Distributed under the Boost Software License, Version 1.0.
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMBIENT_UTILS_FENCE
#define AMBIENT_UTILS_FENCE

namespace ambient {

    class fence {
    public:
        fence() : f(NULL) { }
        bool once(){ if(!f){ f = &nptr; return true; } return false; }
    protected:
        static void* nptr;
        void** f;
    };

    #ifdef AMBIENT_BUILD_LIBRARY
    void* fence::nptr = NULL;
    #endif

    class ordered_fence : public fence {
    public:
        ordered_fence(){ }
        ordered_fence(void**& o){ this->adhere(o); }
        void adhere(void**& o){ this->f = o; o = (void**)&this->f; }
        bool ordered_once()   { if(!*this->f){ this->f = NULL; return true; } return false; }
        bool empty()          { return !this->f; }
        static void reset()   { order = &fence::nptr; }
        static void** order;  // = &fence::nptr;
    };

}

#endif

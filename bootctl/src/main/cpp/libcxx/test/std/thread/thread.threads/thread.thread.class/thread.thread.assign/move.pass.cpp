//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads, c++98, c++03

// <thread>

// class thread

// thread& operator=(thread&& t);

#include <thread>
#include <cassert>

class G
{
    int alive_;
public:
    static int n_alive;
    static bool op_run;

    G() : alive_(1) {++n_alive;}
    G(const G& g) : alive_(g.alive_) {++n_alive;}
    ~G() {alive_ = 0; --n_alive;}

    void operator()(int i, double j)
    {
        assert(alive_ == 1);
        assert(n_alive >= 1);
        assert(i == 5);
        assert(j == 5.5);
        op_run = true;
    }
};

int G::n_alive = 0;
bool G::op_run = false;

int main()
{
    {
        assert(G::n_alive == 0);
        assert(!G::op_run);
        {
        G g;
        std::thread t0(g, 5, 5.5);
        std::thread::id id = t0.get_id();
        std::thread t1;
        t1 = std::move(t0);
        assert(t1.get_id() == id);
        assert(t0.get_id() == std::thread::id());
        t1.join();
        }
        assert(G::n_alive == 0);
        assert(G::op_run);
    }
}

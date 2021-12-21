//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_map>

// class unordered_map

// template <class M>
//  pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);            // C++17
// template <class M>
//  pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);                 // C++17
// template <class M>
//  iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);   // C++17
// template <class M>
//  iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);        // C++17

#include <unordered_map>
#include <cassert>
#include <tuple>


class Moveable
{
    Moveable(const Moveable&);
    Moveable& operator=(const Moveable&);

    int int_;
    double double_;
public:
    Moveable() : int_(0), double_(0) {}
    Moveable(int i, double d) : int_(i), double_(d) {}
    Moveable(Moveable&& x)
        : int_(x.int_), double_(x.double_)
            {x.int_ = -1; x.double_ = -1;}
    Moveable& operator=(Moveable&& x)
        {int_ = x.int_; x.int_ = -1;
         double_ = x.double_; x.double_ = -1;
         return *this;
        }

    bool operator==(const Moveable& x) const
        {return int_ == x.int_ && double_ == x.double_;}
    bool operator<(const Moveable& x) const
        {return int_ < x.int_ || (int_ == x.int_ && double_ < x.double_);}
    size_t hash () const { return std::hash<int>()(int_) + std::hash<double>()(double_); }

    int get() const {return int_;}
    bool moved() const {return int_ == -1;}
};

namespace std {
    template <> struct hash<Moveable> {
        size_t operator () (const Moveable &m) const { return m.hash(); }
    };
}

int main()
{

    { // pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);
        typedef std::unordered_map<int, Moveable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r;
        for (int i = 0; i < 20; i += 2)
            m.emplace ( i, Moveable(i, (double) i));
        assert(m.size() == 10);

        for (int i=0; i < 20; i += 2)
        {
            Moveable mv(i+1, i+1);
            r = m.insert_or_assign(i, std::move(mv));
            assert(m.size() == 10);
            assert(!r.second);                    // was not inserted
            assert(mv.moved());                   // was moved from
            assert(r.first->first == i);          // key
            assert(r.first->second.get() == i+1); // value
        }

        Moveable mv1(5, 5.0);
        r = m.insert_or_assign(-1, std::move(mv1));
        assert(m.size() == 11);
        assert(r.second);                    // was inserted
        assert(mv1.moved());                 // was moved from
        assert(r.first->first        == -1); // key
        assert(r.first->second.get() == 5);  // value

        Moveable mv2(9, 9.0);
        r = m.insert_or_assign(3, std::move(mv2));
        assert(m.size() == 12);
        assert(r.second);                   // was inserted
        assert(mv2.moved());                // was moved from
        assert(r.first->first        == 3); // key
        assert(r.first->second.get() == 9); // value

        Moveable mv3(-1, 5.0);
        r = m.insert_or_assign(117, std::move(mv3));
        assert(m.size() == 13);
        assert(r.second);                     // was inserted
        assert(mv3.moved());                  // was moved from
        assert(r.first->first        == 117); // key
        assert(r.first->second.get() == -1);  // value
    }
    { // pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);
        typedef std::unordered_map<Moveable, Moveable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r;
        for (int i = 0; i < 20; i += 2)
            m.emplace ( Moveable(i, (double) i), Moveable(i+1, (double) i+1));
        assert(m.size() == 10);

        Moveable mvkey1(2, 2.0);
        Moveable mv1(4, 4.0);
        r = m.insert_or_assign(std::move(mvkey1), std::move(mv1));
        assert(m.size() == 10);
        assert(!r.second);                  // was not inserted
        assert(!mvkey1.moved());            // was not moved from
        assert(mv1.moved());                // was moved from
        assert(r.first->first == mvkey1);   // key
        assert(r.first->second.get() == 4); // value

        Moveable mvkey2(3, 3.0);
        Moveable mv2(5, 5.0);
        r = m.try_emplace(std::move(mvkey2), std::move(mv2));
        assert(m.size() == 11);
        assert(r.second);                   // was inserted
        assert(mv2.moved());                // was moved from
        assert(mvkey2.moved());             // was moved from
        assert(r.first->first.get()  == 3); // key
        assert(r.first->second.get() == 5); // value
    }
    { // iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);
        typedef std::unordered_map<int, Moveable> M;
        M m;
        M::iterator r;
        for (int i = 0; i < 20; i += 2)
            m.emplace ( i, Moveable(i, (double) i));
        assert(m.size() == 10);
        M::const_iterator it = m.find(2);

        Moveable mv1(3, 3.0);
        r = m.insert_or_assign(it, 2, std::move(mv1));
        assert(m.size() == 10);
        assert(mv1.moved());           // was moved from
        assert(r->first        == 2);  // key
        assert(r->second.get() == 3);  // value

        Moveable mv2(5, 5.0);
        r = m.insert_or_assign(it, 3, std::move(mv2));
        assert(m.size() == 11);
        assert(mv2.moved());           // was moved from
        assert(r->first        == 3);  // key
        assert(r->second.get() == 5);  // value
    }
    { // iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);
        typedef std::unordered_map<Moveable, Moveable> M;
        M m;
        M::iterator r;
        for (int i = 0; i < 20; i += 2)
            m.emplace ( Moveable(i, (double) i), Moveable(i+1, (double) i+1));
        assert(m.size() == 10);
        M::const_iterator it = std::next(m.cbegin());

        Moveable mvkey1(2, 2.0);
        Moveable mv1(4, 4.0);
        r = m.insert_or_assign(it, std::move(mvkey1), std::move(mv1));
        assert(m.size() == 10);
        assert(mv1.moved());          // was moved from
        assert(!mvkey1.moved());      // was not moved from
        assert(r->first == mvkey1);   // key
        assert(r->second.get() == 4); // value

        Moveable mvkey2(3, 3.0);
        Moveable mv2(5, 5.0);
        r = m.insert_or_assign(it, std::move(mvkey2), std::move(mv2));
        assert(m.size() == 11);
        assert(mv2.moved());           // was moved from
        assert(mvkey2.moved());        // was moved from
        assert(r->first.get()  == 3);  // key
        assert(r->second.get() == 5);  // value
    }

}

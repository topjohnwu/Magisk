//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Not a portable test

// Precondition:  __root->__is_black_ == true
// template <class _NodePtr>
// void
// __tree_balance_after_insert(_NodePtr __root, _NodePtr __x)

#include <__tree>
#include <cassert>

struct Node
{
    Node* __left_;
    Node* __right_;
    Node* __parent_;
    bool __is_black_;

    Node* __parent_unsafe() const { return __parent_; }
    void __set_parent(Node* x) { __parent_ = x;}

    Node() : __left_(), __right_(), __parent_(), __is_black_() {}
};

void
test1()
{
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &a;
        b.__right_ = 0;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = 0;
        d.__right_ = 0;
        d.__is_black_ = false;

        a.__parent_ = &b;
        a.__left_ = 0;
        a.__right_ = 0;
        a.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == &a);
        assert(b.__right_ == 0);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == 0);
        assert(d.__right_ == 0);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &b);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = 0;
        b.__right_ = &a;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = 0;
        d.__right_ = 0;
        d.__is_black_ = false;

        a.__parent_ = &b;
        a.__left_ = 0;
        a.__right_ = 0;
        a.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == 0);
        assert(b.__right_ == &a);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == 0);
        assert(d.__right_ == 0);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &b);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = 0;
        b.__right_ = 0;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = &a;
        d.__right_ = 0;
        d.__is_black_ = false;

        a.__parent_ = &d;
        a.__left_ = 0;
        a.__right_ = 0;
        a.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == 0);
        assert(b.__right_ == 0);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == &a);
        assert(d.__right_ == 0);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &d);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = 0;
        b.__right_ = 0;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = 0;
        d.__right_ = &a;
        d.__is_black_ = false;

        a.__parent_ = &d;
        a.__left_ = 0;
        a.__right_ = 0;
        a.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == 0);
        assert(b.__right_ == 0);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == 0);
        assert(d.__right_ == &a);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &d);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;
        Node h;
        Node i;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &a;
        b.__right_ = &g;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = &h;
        d.__right_ = &i;
        d.__is_black_ = false;

        a.__parent_ = &b;
        a.__left_ = &e;
        a.__right_ = &f;
        a.__is_black_ = false;

        e.__parent_ = &a;
        e.__is_black_ = true;

        f.__parent_ = &a;
        f.__is_black_ = true;

        g.__parent_ = &b;
        g.__is_black_ = true;

        h.__parent_ = &d;
        h.__is_black_ = true;

        i.__parent_ = &d;
        i.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == &a);
        assert(b.__right_ == &g);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == &h);
        assert(d.__right_ == &i);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &b);
        assert(a.__left_ == &e);
        assert(a.__right_ == &f);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;
        Node h;
        Node i;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &g;
        b.__right_ = &a;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = &h;
        d.__right_ = &i;
        d.__is_black_ = false;

        a.__parent_ = &b;
        a.__left_ = &e;
        a.__right_ = &f;
        a.__is_black_ = false;

        e.__parent_ = &a;
        e.__is_black_ = true;

        f.__parent_ = &a;
        f.__is_black_ = true;

        g.__parent_ = &b;
        g.__is_black_ = true;

        h.__parent_ = &d;
        h.__is_black_ = true;

        i.__parent_ = &d;
        i.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == &g);
        assert(b.__right_ == &a);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == &h);
        assert(d.__right_ == &i);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &b);
        assert(a.__left_ == &e);
        assert(a.__right_ == &f);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;
        Node h;
        Node i;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &g;
        b.__right_ = &h;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = &a;
        d.__right_ = &i;
        d.__is_black_ = false;

        a.__parent_ = &d;
        a.__left_ = &e;
        a.__right_ = &f;
        a.__is_black_ = false;

        e.__parent_ = &a;
        e.__is_black_ = true;

        f.__parent_ = &a;
        f.__is_black_ = true;

        g.__parent_ = &b;
        g.__is_black_ = true;

        h.__parent_ = &b;
        h.__is_black_ = true;

        i.__parent_ = &d;
        i.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == &g);
        assert(b.__right_ == &h);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == &a);
        assert(d.__right_ == &i);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &d);
        assert(a.__left_ == &e);
        assert(a.__right_ == &f);
        assert(a.__is_black_ == false);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;
        Node h;
        Node i;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &d;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &g;
        b.__right_ = &h;
        b.__is_black_ = false;

        d.__parent_ = &c;
        d.__left_ = &i;
        d.__right_ = &a;
        d.__is_black_ = false;

        a.__parent_ = &d;
        a.__left_ = &e;
        a.__right_ = &f;
        a.__is_black_ = false;

        e.__parent_ = &a;
        e.__is_black_ = true;

        f.__parent_ = &a;
        f.__is_black_ = true;

        g.__parent_ = &b;
        g.__is_black_ = true;

        h.__parent_ = &b;
        h.__is_black_ = true;

        i.__parent_ = &d;
        i.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &c);

        assert(c.__parent_ == &root);
        assert(c.__left_ == &b);
        assert(c.__right_ == &d);
        assert(c.__is_black_ == true);

        assert(b.__parent_ == &c);
        assert(b.__left_ == &g);
        assert(b.__right_ == &h);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &c);
        assert(d.__left_ == &i);
        assert(d.__right_ == &a);
        assert(d.__is_black_ == true);

        assert(a.__parent_ == &d);
        assert(a.__left_ == &e);
        assert(a.__right_ == &f);
        assert(a.__is_black_ == false);
    }
}

void
test2()
{
    {
        Node root;
        Node a;
        Node b;
        Node c;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &a;
        c.__right_ = 0;
        c.__is_black_ = true;

        a.__parent_ = &c;
        a.__left_ = 0;
        a.__right_ = &b;
        a.__is_black_ = false;

        b.__parent_ = &a;
        b.__left_ = 0;
        b.__right_ = 0;
        b.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &b);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(c.__parent_ == &b);
        assert(c.__left_ == 0);
        assert(c.__right_ == 0);
        assert(c.__is_black_ == false);

        assert(a.__parent_ == &b);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;

        root.__left_ = &a;

        a.__parent_ = &root;
        a.__left_ = 0;
        a.__right_ = &c;
        a.__is_black_ = true;

        c.__parent_ = &a;
        c.__left_ = &b;
        c.__right_ = 0;
        c.__is_black_ = false;

        b.__parent_ = &c;
        b.__left_ = 0;
        b.__right_ = 0;
        b.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &b);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(a.__parent_ == &b);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);

        assert(c.__parent_ == &b);
        assert(c.__left_ == 0);
        assert(c.__right_ == 0);
        assert(c.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &a;
        c.__right_ = &g;
        c.__is_black_ = true;

        a.__parent_ = &c;
        a.__left_ = &d;
        a.__right_ = &b;
        a.__is_black_ = false;

        b.__parent_ = &a;
        b.__left_ = &e;
        b.__right_ = &f;
        b.__is_black_ = false;

        d.__parent_ = &a;
        d.__is_black_ = true;

        e.__parent_ = &b;
        e.__is_black_ = true;

        f.__parent_ = &b;
        f.__is_black_ = true;

        g.__parent_ = &c;
        g.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &b);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(c.__parent_ == &b);
        assert(c.__left_ == &f);
        assert(c.__right_ == &g);
        assert(c.__is_black_ == false);

        assert(a.__parent_ == &b);
        assert(a.__left_ == &d);
        assert(a.__right_ == &e);
        assert(a.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &a);
        assert(d.__is_black_ == true);

        assert(e.__parent_ == &a);
        assert(e.__is_black_ == true);

        assert(f.__parent_ == &c);
        assert(f.__is_black_ == true);

        assert(g.__parent_ == &c);
        assert(g.__is_black_ == true);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;

        root.__left_ = &a;

        a.__parent_ = &root;
        a.__left_ = &d;
        a.__right_ = &c;
        a.__is_black_ = true;

        c.__parent_ = &a;
        c.__left_ = &b;
        c.__right_ = &g;
        c.__is_black_ = false;

        b.__parent_ = &c;
        b.__left_ = &e;
        b.__right_ = &f;
        b.__is_black_ = false;

        d.__parent_ = &a;
        d.__is_black_ = true;

        e.__parent_ = &b;
        e.__is_black_ = true;

        f.__parent_ = &b;
        f.__is_black_ = true;

        g.__parent_ = &c;
        g.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &b);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(c.__parent_ == &b);
        assert(c.__left_ == &f);
        assert(c.__right_ == &g);
        assert(c.__is_black_ == false);

        assert(a.__parent_ == &b);
        assert(a.__left_ == &d);
        assert(a.__right_ == &e);
        assert(a.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &a);
        assert(d.__is_black_ == true);

        assert(e.__parent_ == &a);
        assert(e.__is_black_ == true);

        assert(f.__parent_ == &c);
        assert(f.__is_black_ == true);

        assert(g.__parent_ == &c);
        assert(g.__is_black_ == true);
    }
}

void
test3()
{
    {
        Node root;
        Node a;
        Node b;
        Node c;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = 0;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &a;
        b.__right_ = 0;
        b.__is_black_ = false;

        a.__parent_ = &b;
        a.__left_ = 0;
        a.__right_ = 0;
        a.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(c.__parent_ == &b);
        assert(c.__left_ == 0);
        assert(c.__right_ == 0);
        assert(c.__is_black_ == false);

        assert(a.__parent_ == &b);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;

        root.__left_ = &a;

        a.__parent_ = &root;
        a.__left_ = 0;
        a.__right_ = &b;
        a.__is_black_ = true;

        b.__parent_ = &a;
        b.__left_ = 0;
        b.__right_ = &c;
        b.__is_black_ = false;

        c.__parent_ = &b;
        c.__left_ = 0;
        c.__right_ = 0;
        c.__is_black_ = false;

        std::__tree_balance_after_insert(root.__left_, &c);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(a.__parent_ == &b);
        assert(a.__left_ == 0);
        assert(a.__right_ == 0);
        assert(a.__is_black_ == false);

        assert(c.__parent_ == &b);
        assert(c.__left_ == 0);
        assert(c.__right_ == 0);
        assert(c.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;

        root.__left_ = &c;

        c.__parent_ = &root;
        c.__left_ = &b;
        c.__right_ = &g;
        c.__is_black_ = true;

        b.__parent_ = &c;
        b.__left_ = &a;
        b.__right_ = &f;
        b.__is_black_ = false;

        a.__parent_ = &b;
        a.__left_ = &d;
        a.__right_ = &e;
        a.__is_black_ = false;

        d.__parent_ = &a;
        d.__is_black_ = true;

        e.__parent_ = &a;
        e.__is_black_ = true;

        f.__parent_ = &b;
        f.__is_black_ = true;

        g.__parent_ = &c;
        g.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &a);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(c.__parent_ == &b);
        assert(c.__left_ == &f);
        assert(c.__right_ == &g);
        assert(c.__is_black_ == false);

        assert(a.__parent_ == &b);
        assert(a.__left_ == &d);
        assert(a.__right_ == &e);
        assert(a.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &a);
        assert(d.__is_black_ == true);

        assert(e.__parent_ == &a);
        assert(e.__is_black_ == true);

        assert(f.__parent_ == &c);
        assert(f.__is_black_ == true);

        assert(g.__parent_ == &c);
        assert(g.__is_black_ == true);
    }
    {
        Node root;
        Node a;
        Node b;
        Node c;
        Node d;
        Node e;
        Node f;
        Node g;

        root.__left_ = &a;

        a.__parent_ = &root;
        a.__left_ = &d;
        a.__right_ = &b;
        a.__is_black_ = true;

        b.__parent_ = &a;
        b.__left_ = &e;
        b.__right_ = &c;
        b.__is_black_ = false;

        c.__parent_ = &b;
        c.__left_ = &f;
        c.__right_ = &g;
        c.__is_black_ = false;

        d.__parent_ = &a;
        d.__is_black_ = true;

        e.__parent_ = &b;
        e.__is_black_ = true;

        f.__parent_ = &c;
        f.__is_black_ = true;

        g.__parent_ = &c;
        g.__is_black_ = true;

        std::__tree_balance_after_insert(root.__left_, &c);

        assert(std::__tree_invariant(root.__left_));

        assert(root.__left_ == &b);

        assert(c.__parent_ == &b);
        assert(c.__left_ == &f);
        assert(c.__right_ == &g);
        assert(c.__is_black_ == false);

        assert(a.__parent_ == &b);
        assert(a.__left_ == &d);
        assert(a.__right_ == &e);
        assert(a.__is_black_ == false);

        assert(b.__parent_ == &root);
        assert(b.__left_ == &a);
        assert(b.__right_ == &c);
        assert(b.__is_black_ == true);

        assert(d.__parent_ == &a);
        assert(d.__is_black_ == true);

        assert(e.__parent_ == &a);
        assert(e.__is_black_ == true);

        assert(f.__parent_ == &c);
        assert(f.__is_black_ == true);

        assert(g.__parent_ == &c);
        assert(g.__is_black_ == true);
    }
}

void
test4()
{
    Node root;
    Node a;
    Node b;
    Node c;
    Node d;
    Node e;
    Node f;
    Node g;
    Node h;

    root.__left_ = &a;
    a.__parent_ = &root;

    std::__tree_balance_after_insert(root.__left_, &a);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &a);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(a.__parent_ == &root);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == true);

    a.__right_ = &b;
    b.__parent_ = &a;

    std::__tree_balance_after_insert(root.__left_, &b);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &a);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(a.__parent_ == &root);
    assert(a.__left_ == 0);
    assert(a.__right_ == &b);
    assert(a.__is_black_ == true);

    assert(b.__parent_ == &a);
    assert(b.__left_ == 0);
    assert(b.__right_ == 0);
    assert(b.__is_black_ == false);

    b.__right_ = &c;
    c.__parent_ = &b;

    std::__tree_balance_after_insert(root.__left_, &c);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &b);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == false);

    assert(b.__parent_ == &root);
    assert(b.__left_ == &a);
    assert(b.__right_ == &c);
    assert(b.__is_black_ == true);

    assert(c.__parent_ == &b);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
    assert(c.__is_black_ == false);

    c.__right_ = &d;
    d.__parent_ = &c;

    std::__tree_balance_after_insert(root.__left_, &d);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &b);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == true);

    assert(b.__parent_ == &root);
    assert(b.__left_ == &a);
    assert(b.__right_ == &c);
    assert(b.__is_black_ == true);

    assert(c.__parent_ == &b);
    assert(c.__left_ == 0);
    assert(c.__right_ == &d);
    assert(c.__is_black_ == true);

    assert(d.__parent_ == &c);
    assert(d.__left_ == 0);
    assert(d.__right_ == 0);
    assert(d.__is_black_ == false);

    d.__right_ = &e;
    e.__parent_ = &d;

    std::__tree_balance_after_insert(root.__left_, &e);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &b);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(b.__parent_ == &root);
    assert(b.__left_ == &a);
    assert(b.__right_ == &d);
    assert(b.__is_black_ == true);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == true);

    assert(d.__parent_ == &b);
    assert(d.__left_ == &c);
    assert(d.__right_ == &e);
    assert(d.__is_black_ == true);

    assert(c.__parent_ == &d);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
    assert(c.__is_black_ == false);

    assert(e.__parent_ == &d);
    assert(e.__left_ == 0);
    assert(e.__right_ == 0);
    assert(e.__is_black_ == false);

    e.__right_ = &f;
    f.__parent_ = &e;

    std::__tree_balance_after_insert(root.__left_, &f);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &b);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(b.__parent_ == &root);
    assert(b.__left_ == &a);
    assert(b.__right_ == &d);
    assert(b.__is_black_ == true);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == true);

    assert(d.__parent_ == &b);
    assert(d.__left_ == &c);
    assert(d.__right_ == &e);
    assert(d.__is_black_ == false);

    assert(c.__parent_ == &d);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
    assert(c.__is_black_ == true);

    assert(e.__parent_ == &d);
    assert(e.__left_ == 0);
    assert(e.__right_ == &f);
    assert(e.__is_black_ == true);

    assert(f.__parent_ == &e);
    assert(f.__left_ == 0);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == false);

    f.__right_ = &g;
    g.__parent_ = &f;

    std::__tree_balance_after_insert(root.__left_, &g);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &b);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(b.__parent_ == &root);
    assert(b.__left_ == &a);
    assert(b.__right_ == &d);
    assert(b.__is_black_ == true);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == true);

    assert(d.__parent_ == &b);
    assert(d.__left_ == &c);
    assert(d.__right_ == &f);
    assert(d.__is_black_ == false);

    assert(c.__parent_ == &d);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
    assert(c.__is_black_ == true);

    assert(f.__parent_ == &d);
    assert(f.__left_ == &e);
    assert(f.__right_ == &g);
    assert(f.__is_black_ == true);

    assert(e.__parent_ == &f);
    assert(e.__left_ == 0);
    assert(e.__right_ == 0);
    assert(e.__is_black_ == false);

    assert(g.__parent_ == &f);
    assert(g.__left_ == 0);
    assert(g.__right_ == 0);
    assert(g.__is_black_ == false);

    g.__right_ = &h;
    h.__parent_ = &g;

    std::__tree_balance_after_insert(root.__left_, &h);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &d);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(d.__parent_ == &root);
    assert(d.__left_ == &b);
    assert(d.__right_ == &f);
    assert(d.__is_black_ == true);

    assert(b.__parent_ == &d);
    assert(b.__left_ == &a);
    assert(b.__right_ == &c);
    assert(b.__is_black_ == false);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == true);

    assert(c.__parent_ == &b);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
    assert(c.__is_black_ == true);

    assert(f.__parent_ == &d);
    assert(f.__left_ == &e);
    assert(f.__right_ == &g);
    assert(f.__is_black_ == false);

    assert(e.__parent_ == &f);
    assert(e.__left_ == 0);
    assert(e.__right_ == 0);
    assert(e.__is_black_ == true);

    assert(g.__parent_ == &f);
    assert(g.__left_ == 0);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == true);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == false);
}

void
test5()
{
    Node root;
    Node a;
    Node b;
    Node c;
    Node d;
    Node e;
    Node f;
    Node g;
    Node h;

    root.__left_ = &h;
    h.__parent_ = &root;

    std::__tree_balance_after_insert(root.__left_, &h);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &h);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(h.__parent_ == &root);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);

    h.__left_ = &g;
    g.__parent_ = &h;

    std::__tree_balance_after_insert(root.__left_, &g);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &h);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(h.__parent_ == &root);
    assert(h.__left_ == &g);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);

    assert(g.__parent_ == &h);
    assert(g.__left_ == 0);
    assert(g.__right_ == 0);
    assert(g.__is_black_ == false);

    g.__left_ = &f;
    f.__parent_ = &g;

    std::__tree_balance_after_insert(root.__left_, &f);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &g);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(g.__parent_ == &root);
    assert(g.__left_ == &f);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == true);

    assert(f.__parent_ == &g);
    assert(f.__left_ == 0);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == false);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == false);

    f.__left_ = &e;
    e.__parent_ = &f;

    std::__tree_balance_after_insert(root.__left_, &e);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &g);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(g.__parent_ == &root);
    assert(g.__left_ == &f);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == true);

    assert(f.__parent_ == &g);
    assert(f.__left_ == &e);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == true);

    assert(e.__parent_ == &f);
    assert(e.__left_ == 0);
    assert(e.__right_ == 0);
    assert(e.__is_black_ == false);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);

    e.__left_ = &d;
    d.__parent_ = &e;

    std::__tree_balance_after_insert(root.__left_, &d);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &g);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(g.__parent_ == &root);
    assert(g.__left_ == &e);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == true);

    assert(e.__parent_ == &g);
    assert(e.__left_ == &d);
    assert(e.__right_ == &f);
    assert(e.__is_black_ == true);

    assert(d.__parent_ == &e);
    assert(d.__left_ == 0);
    assert(d.__right_ == 0);
    assert(d.__is_black_ == false);

    assert(f.__parent_ == &e);
    assert(f.__left_ == 0);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == false);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);

    d.__left_ = &c;
    c.__parent_ = &d;

    std::__tree_balance_after_insert(root.__left_, &c);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &g);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(g.__parent_ == &root);
    assert(g.__left_ == &e);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == true);

    assert(e.__parent_ == &g);
    assert(e.__left_ == &d);
    assert(e.__right_ == &f);
    assert(e.__is_black_ == false);

    assert(d.__parent_ == &e);
    assert(d.__left_ == &c);
    assert(d.__right_ == 0);
    assert(d.__is_black_ == true);

    assert(c.__parent_ == &d);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
    assert(c.__is_black_ == false);

    assert(f.__parent_ == &e);
    assert(f.__left_ == 0);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == true);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);

    c.__left_ = &b;
    b.__parent_ = &c;

    std::__tree_balance_after_insert(root.__left_, &b);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &g);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(g.__parent_ == &root);
    assert(g.__left_ == &e);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == true);

    assert(e.__parent_ == &g);
    assert(e.__left_ == &c);
    assert(e.__right_ == &f);
    assert(e.__is_black_ == false);

    assert(c.__parent_ == &e);
    assert(c.__left_ == &b);
    assert(c.__right_ == &d);
    assert(c.__is_black_ == true);

    assert(b.__parent_ == &c);
    assert(b.__left_ == 0);
    assert(b.__right_ == 0);
    assert(b.__is_black_ == false);

    assert(d.__parent_ == &c);
    assert(d.__left_ == 0);
    assert(d.__right_ == 0);
    assert(d.__is_black_ == false);

    assert(f.__parent_ == &e);
    assert(f.__left_ == 0);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == true);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);

    b.__left_ = &a;
    a.__parent_ = &b;

    std::__tree_balance_after_insert(root.__left_, &a);

    assert(std::__tree_invariant(root.__left_));

    assert(root.__parent_ == 0);
    assert(root.__left_ == &e);
    assert(root.__right_ == 0);
    assert(root.__is_black_ == false);

    assert(e.__parent_ == &root);
    assert(e.__left_ == &c);
    assert(e.__right_ == &g);
    assert(e.__is_black_ == true);

    assert(c.__parent_ == &e);
    assert(c.__left_ == &b);
    assert(c.__right_ == &d);
    assert(c.__is_black_ == false);

    assert(b.__parent_ == &c);
    assert(b.__left_ == &a);
    assert(b.__right_ == 0);
    assert(b.__is_black_ == true);

    assert(a.__parent_ == &b);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(a.__is_black_ == false);

    assert(d.__parent_ == &c);
    assert(d.__left_ == 0);
    assert(d.__right_ == 0);
    assert(d.__is_black_ == true);

    assert(g.__parent_ == &e);
    assert(g.__left_ == &f);
    assert(g.__right_ == &h);
    assert(g.__is_black_ == false);

    assert(f.__parent_ == &g);
    assert(f.__left_ == 0);
    assert(f.__right_ == 0);
    assert(f.__is_black_ == true);

    assert(h.__parent_ == &g);
    assert(h.__left_ == 0);
    assert(h.__right_ == 0);
    assert(h.__is_black_ == true);
}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
}

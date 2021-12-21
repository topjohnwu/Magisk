//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Not a portable test

// Precondition:  __x->__right_ != nullptr
// template <class _NodePtr>
// void
// __tree_left_rotate(_NodePtr __x);

#include <__tree>
#include <cassert>

struct Node
{
    Node* __left_;
    Node* __right_;
    Node* __parent_;

    Node* __parent_unsafe() const { return __parent_; }
    void __set_parent(Node* x) { __parent_ = x;}

    Node() : __left_(), __right_(), __parent_() {}
};

void
test1()
{
    Node root;
    Node x;
    Node y;
    root.__left_ = &x;
    x.__left_ = 0;
    x.__right_ = &y;
    x.__parent_ = &root;
    y.__left_ = 0;
    y.__right_ = 0;
    y.__parent_ = &x;
    std::__tree_left_rotate(&x);
    assert(root.__parent_ == 0);
    assert(root.__left_ == &y);
    assert(root.__right_ == 0);
    assert(y.__parent_ == &root);
    assert(y.__left_ == &x);
    assert(y.__right_ == 0);
    assert(x.__parent_ == &y);
    assert(x.__left_ == 0);
    assert(x.__right_ == 0);
}

void
test2()
{
    Node root;
    Node x;
    Node y;
    Node a;
    Node b;
    Node c;
    root.__left_ = &x;
    x.__left_ = &a;
    x.__right_ = &y;
    x.__parent_ = &root;
    y.__left_ = &b;
    y.__right_ = &c;
    y.__parent_ = &x;
    a.__parent_ = &x;
    b.__parent_ = &y;
    c.__parent_ = &y;
    std::__tree_left_rotate(&x);
    assert(root.__parent_ == 0);
    assert(root.__left_ == &y);
    assert(root.__right_ == 0);
    assert(y.__parent_ == &root);
    assert(y.__left_ == &x);
    assert(y.__right_ == &c);
    assert(x.__parent_ == &y);
    assert(x.__left_ == &a);
    assert(x.__right_ == &b);
    assert(a.__parent_ == &x);
    assert(a.__left_ == 0);
    assert(a.__right_ == 0);
    assert(b.__parent_ == &x);
    assert(b.__left_ == 0);
    assert(b.__right_ == 0);
    assert(c.__parent_ == &y);
    assert(c.__left_ == 0);
    assert(c.__right_ == 0);
}

int main()
{
    test1();
    test2();
}

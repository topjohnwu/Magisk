//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <memory>

// unique_ptr

// Test unique_ptr converting move ctor

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "unique_ptr_test_helper.h"
#include "type_id.h"

template <int ID = 0>
struct GenericDeleter {
  void operator()(void*) const {}
};

template <int ID = 0>
struct GenericConvertingDeleter {

  template <int OID>
  GenericConvertingDeleter(GenericConvertingDeleter<OID>) {}

  template <int OID>
  GenericConvertingDeleter& operator=(GenericConvertingDeleter<OID> const&) {
    return *this;
  }

  void operator()(void*) const {}
};

template <class T, class U>
using EnableIfNotSame = typename std::enable_if<
    !std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value
>::type;

template <class Templ, class Other>
struct is_specialization;

template <template <int> class Templ, int ID1, class Other>
struct is_specialization<Templ<ID1>, Other> : std::false_type {};

template <template <int> class Templ, int ID1, int ID2>
struct is_specialization<Templ<ID1>, Templ<ID2> > : std::true_type {};

template <class Templ, class Other>
using EnableIfSpecialization = typename std::enable_if<
    is_specialization<Templ, typename std::decay<Other>::type >::value
  >::type;

template <int ID> struct TrackingDeleter;
template <int ID> struct ConstTrackingDeleter;

template <int ID>
struct TrackingDeleter {
  TrackingDeleter() : arg_type(&makeArgumentID<>()) {}

  TrackingDeleter(TrackingDeleter const&)
      : arg_type(&makeArgumentID<TrackingDeleter const&>()) {}

  TrackingDeleter(TrackingDeleter&&)
      : arg_type(&makeArgumentID<TrackingDeleter &&>()) {}

  template <class T, class = EnableIfSpecialization<TrackingDeleter, T> >
  TrackingDeleter(T&&) : arg_type(&makeArgumentID<T&&>()) {}

  TrackingDeleter& operator=(TrackingDeleter const&) {
    arg_type = &makeArgumentID<TrackingDeleter const&>();
    return *this;
  }

  TrackingDeleter& operator=(TrackingDeleter &&) {
    arg_type = &makeArgumentID<TrackingDeleter &&>();
    return *this;
  }

  template <class T, class = EnableIfSpecialization<TrackingDeleter, T> >
  TrackingDeleter& operator=(T&&) {
    arg_type = &makeArgumentID<T&&>();
    return *this;
  }

  void operator()(void*) const {}

public:
  TypeID const* reset() const {
    TypeID const* tmp = arg_type;
    arg_type = nullptr;
    return tmp;
  }

  mutable TypeID const* arg_type;
};

template <int ID>
struct ConstTrackingDeleter {
  ConstTrackingDeleter() : arg_type(&makeArgumentID<>()) {}

  ConstTrackingDeleter(ConstTrackingDeleter const&)
      : arg_type(&makeArgumentID<ConstTrackingDeleter const&>()) {}

  ConstTrackingDeleter(ConstTrackingDeleter&&)
      : arg_type(&makeArgumentID<ConstTrackingDeleter &&>()) {}

  template <class T, class = EnableIfSpecialization<ConstTrackingDeleter, T> >
  ConstTrackingDeleter(T&&) : arg_type(&makeArgumentID<T&&>()) {}

  const ConstTrackingDeleter& operator=(ConstTrackingDeleter const&) const {
    arg_type = &makeArgumentID<ConstTrackingDeleter const&>();
    return *this;
  }

  const ConstTrackingDeleter& operator=(ConstTrackingDeleter &&) const {
    arg_type = &makeArgumentID<ConstTrackingDeleter &&>();
    return *this;
  }

  template <class T, class = EnableIfSpecialization<ConstTrackingDeleter, T> >
  const ConstTrackingDeleter& operator=(T&&) const {
    arg_type = &makeArgumentID<T&&>();
    return *this;
  }

  void operator()(void*) const {}

public:
  TypeID const* reset() const {
    TypeID const* tmp = arg_type;
    arg_type = nullptr;
    return tmp;
  }

  mutable TypeID const* arg_type;
};

template <class ExpectT, int ID>
bool checkArg(TrackingDeleter<ID> const& d) {
  return d.arg_type && *d.arg_type == makeArgumentID<ExpectT>();
}

template <class ExpectT, int ID>
bool checkArg(ConstTrackingDeleter<ID> const& d) {
  return d.arg_type && *d.arg_type == makeArgumentID<ExpectT>();
}

template <class From, bool AssignIsConst = false>
struct AssignDeleter {
  AssignDeleter() = default;
  AssignDeleter(AssignDeleter const&) = default;
  AssignDeleter(AssignDeleter&&) = default;

  AssignDeleter& operator=(AssignDeleter const&) = delete;
  AssignDeleter& operator=(AssignDeleter &&) = delete;

  template <class T> AssignDeleter& operator=(T&&) && = delete;
  template <class T> AssignDeleter& operator=(T&&) const && = delete;

  template <class T, class = typename std::enable_if<
      std::is_same<T&&, From>::value && !AssignIsConst
    >::type>
  AssignDeleter& operator=(T&&) & { return *this; }

  template <class T, class = typename std::enable_if<
      std::is_same<T&&, From>::value && AssignIsConst
    >::type>
  const AssignDeleter& operator=(T&&) const & { return *this; }

  template <class T>
  void operator()(T) const {}
};

template <class VT, class DDest, class DSource>
  void doDeleterTest() {
    using U1 = std::unique_ptr<VT, DDest>;
    using U2 = std::unique_ptr<VT, DSource>;
    static_assert(std::is_nothrow_assignable<U1, U2&&>::value, "");
    typename std::decay<DDest>::type ddest;
    typename std::decay<DSource>::type dsource;
    U1 u1(nullptr, ddest);
    U2 u2(nullptr, dsource);
    u1 = std::move(u2);
}

template <bool IsArray>
void test_sfinae() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;

  { // Test that different non-reference deleter types are allowed so long
    // as they convert to each other.
    using U1 = std::unique_ptr<VT, GenericConvertingDeleter<0> >;
    using U2 = std::unique_ptr<VT, GenericConvertingDeleter<1> >;
    static_assert(std::is_assignable<U1, U2&&>::value, "");
  }
  { // Test that different non-reference deleter types are disallowed when
    // they cannot convert.
    using U1 = std::unique_ptr<VT, GenericDeleter<0> >;
    using U2 = std::unique_ptr<VT, GenericDeleter<1> >;
    static_assert(!std::is_assignable<U1, U2&&>::value, "");
  }
  { // Test that if the deleter assignment is not valid the assignment operator
    // SFINAEs.
    using U1 = std::unique_ptr<VT, GenericConvertingDeleter<0> const& >;
    using U2 = std::unique_ptr<VT, GenericConvertingDeleter<0> >;
    using U3 = std::unique_ptr<VT, GenericConvertingDeleter<0> &>;
    using U4 = std::unique_ptr<VT, GenericConvertingDeleter<1> >;
    using U5 = std::unique_ptr<VT, GenericConvertingDeleter<1> const&>;
    static_assert(!std::is_assignable<U1, U2&&>::value, "");
    static_assert(!std::is_assignable<U1, U3&&>::value, "");
    static_assert(!std::is_assignable<U1, U4&&>::value, "");
    static_assert(!std::is_assignable<U1, U5&&>::value, "");

    using U1C = std::unique_ptr<const VT, GenericConvertingDeleter<0> const&>;
    static_assert(std::is_nothrow_assignable<U1C, U1&&>::value, "");
  }
  { // Test that if the deleter assignment is not valid the assignment operator
    // SFINAEs.
    using U1 = std::unique_ptr<VT, GenericConvertingDeleter<0> & >;
    using U2 = std::unique_ptr<VT, GenericConvertingDeleter<0> >;
    using U3 = std::unique_ptr<VT, GenericConvertingDeleter<0> &>;
    using U4 = std::unique_ptr<VT, GenericConvertingDeleter<1> >;
    using U5 = std::unique_ptr<VT, GenericConvertingDeleter<1> const&>;

    static_assert(std::is_nothrow_assignable<U1, U2&&>::value, "");
    static_assert(std::is_nothrow_assignable<U1, U3&&>::value, "");
    static_assert(std::is_nothrow_assignable<U1, U4&&>::value, "");
    static_assert(std::is_nothrow_assignable<U1, U5&&>::value, "");

    using U1C = std::unique_ptr<const VT, GenericConvertingDeleter<0> &>;
    static_assert(std::is_nothrow_assignable<U1C, U1&&>::value, "");
  }
  { // Test that non-reference destination deleters can be assigned
    // from any source deleter type with a sutible conversion. Including
    // reference types.
    using U1 = std::unique_ptr<VT, GenericConvertingDeleter<0> >;
    using U2 = std::unique_ptr<VT, GenericConvertingDeleter<0> &>;
    using U3 = std::unique_ptr<VT, GenericConvertingDeleter<0> const &>;
    using U4 = std::unique_ptr<VT, GenericConvertingDeleter<1> >;
    using U5 = std::unique_ptr<VT, GenericConvertingDeleter<1> &>;
    using U6 = std::unique_ptr<VT, GenericConvertingDeleter<1> const&>;
    static_assert(std::is_assignable<U1, U2&&>::value, "");
    static_assert(std::is_assignable<U1, U3&&>::value, "");
    static_assert(std::is_assignable<U1, U4&&>::value, "");
    static_assert(std::is_assignable<U1, U5&&>::value, "");
    static_assert(std::is_assignable<U1, U6&&>::value, "");
  }
  /////////////////////////////////////////////////////////////////////////////
  {
    using Del = GenericDeleter<0>;
    using AD = AssignDeleter<Del&&>;
    using ADC = AssignDeleter<Del&&, /*AllowConstAssign*/true>;
    doDeleterTest<VT, AD, Del>();
    doDeleterTest<VT, AD&, Del>();
    doDeleterTest<VT, ADC const&, Del>();
  }
  {
    using Del = GenericDeleter<0>;
    using AD = AssignDeleter<Del&>;
    using ADC = AssignDeleter<Del&, /*AllowConstAssign*/true>;
    doDeleterTest<VT, AD, Del&>();
    doDeleterTest<VT, AD&, Del&>();
    doDeleterTest<VT, ADC const&, Del&>();
  }
  {
    using Del = GenericDeleter<0>;
    using AD = AssignDeleter<Del const&>;
    using ADC = AssignDeleter<Del const&, /*AllowConstAssign*/true>;
    doDeleterTest<VT, AD, Del const&>();
    doDeleterTest<VT, AD&, Del const&>();
    doDeleterTest<VT, ADC const&, Del const&>();
  }
}


template <bool IsArray>
void test_noexcept() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;
  {
    typedef std::unique_ptr<const VT> APtr;
    typedef std::unique_ptr<VT> BPtr;
    static_assert(std::is_nothrow_assignable<APtr, BPtr>::value, "");
  }
  {
    typedef std::unique_ptr<const VT, CDeleter<const VT> > APtr;
    typedef std::unique_ptr<VT, CDeleter<VT> > BPtr;
    static_assert(std::is_nothrow_assignable<APtr, BPtr>::value, "");
  }
  {
    typedef std::unique_ptr<const VT, NCDeleter<const VT>&> APtr;
    typedef std::unique_ptr<VT, NCDeleter<const VT>&> BPtr;
    static_assert(std::is_nothrow_assignable<APtr, BPtr>::value, "");
  }
  {
    typedef std::unique_ptr<const VT, const NCConstDeleter<const VT>&> APtr;
    typedef std::unique_ptr<VT, const NCConstDeleter<const VT>&> BPtr;
    static_assert(std::is_nothrow_assignable<APtr, BPtr>::value, "");
  }
}

template <bool IsArray>
void test_deleter_value_category() {
  typedef typename std::conditional<IsArray, A[], A>::type VT;
  using TD1 = TrackingDeleter<1>;
  using TD2 = TrackingDeleter<2>;
  TD1 d1;
  TD2 d2;
  using CD1 = ConstTrackingDeleter<1>;
  using CD2 = ConstTrackingDeleter<2>;
  CD1 cd1;
  CD2 cd2;

  { // Test non-reference deleter conversions
    using U1 = std::unique_ptr<VT, TD1 >;
    using U2 = std::unique_ptr<VT, TD2 >;
    U1 u1;
    U2 u2;
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<TD2&&>(u1.get_deleter()));
  }
  { // Test assignment to non-const ref
    using U1 = std::unique_ptr<VT, TD1& >;
    using U2 = std::unique_ptr<VT, TD2 >;
    U1 u1(nullptr, d1);
    U2 u2;
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<TD2&&>(u1.get_deleter()));
  }
  { // Test assignment to const&.
    using U1 = std::unique_ptr<VT, CD1 const& >;
    using U2 = std::unique_ptr<VT, CD2 >;
    U1 u1(nullptr, cd1);
    U2 u2;
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<CD2&&>(u1.get_deleter()));
  }

  { // Test assignment from non-const ref
    using U1 = std::unique_ptr<VT, TD1 >;
    using U2 = std::unique_ptr<VT, TD2& >;
    U1 u1;
    U2 u2(nullptr, d2);
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<TD2&>(u1.get_deleter()));
  }
  { // Test assignment from const ref
    using U1 = std::unique_ptr<VT, TD1 >;
    using U2 = std::unique_ptr<VT, TD2 const& >;
    U1 u1;
    U2 u2(nullptr, d2);
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<TD2 const&>(u1.get_deleter()));
  }

  { // Test assignment from non-const ref
    using U1 = std::unique_ptr<VT, TD1& >;
    using U2 = std::unique_ptr<VT, TD2& >;
    U1 u1(nullptr, d1);
    U2 u2(nullptr, d2);
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<TD2&>(u1.get_deleter()));
  }
  { // Test assignment from const ref
    using U1 = std::unique_ptr<VT, TD1& >;
    using U2 = std::unique_ptr<VT, TD2 const& >;
    U1 u1(nullptr, d1);
    U2 u2(nullptr, d2);
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<TD2 const&>(u1.get_deleter()));
  }

  { // Test assignment from non-const ref
    using U1 = std::unique_ptr<VT, CD1 const& >;
    using U2 = std::unique_ptr<VT, CD2 & >;
    U1 u1(nullptr, cd1);
    U2 u2(nullptr, cd2);
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<CD2 &>(u1.get_deleter()));
  }
  { // Test assignment from const ref
    using U1 = std::unique_ptr<VT, CD1 const& >;
    using U2 = std::unique_ptr<VT, CD2 const& >;
    U1 u1(nullptr, cd1);
    U2 u2(nullptr, cd2);
    u1.get_deleter().reset();
    u1 = std::move(u2);
    assert(checkArg<CD2 const&>(u1.get_deleter()));
  }
}

int main() {
  {
    test_sfinae</*IsArray*/false>();
    test_noexcept<false>();
    test_deleter_value_category<false>();
  }
  {
    test_sfinae</*IsArray*/true>();
    test_noexcept<true>();
    test_deleter_value_category<true>();
  }
}

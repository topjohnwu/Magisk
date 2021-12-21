// UNSUPPORTED: c++98, c++03

#include <memory>

template <int> struct Tag {};

template <int ID>
using SPtr = std::shared_ptr<void(Tag<ID>)>;

template <int ID>
using FnType = void(Tag<ID>);

template <int ID>
void TestFn(Tag<ID>) {}

template <int ID>
FnType<ID>* getFn() {
  return &TestFn<ID>;
}

struct Deleter {
  template <class Tp>
  void operator()(Tp) const {
    using RawT = typename std::remove_pointer<Tp>::type;
    static_assert(std::is_function<RawT>::value ||
                  std::is_same<typename std::remove_cv<RawT>::type,
                               std::nullptr_t>::value,
                  "");
  }
};

int main() {
  {
    SPtr<0> s; // OK
    SPtr<1> s1(nullptr); // OK
    SPtr<2> s2(getFn<2>(), Deleter{}); // OK
    SPtr<3> s3(nullptr, Deleter{}); // OK
  }
  // expected-error-re@memory:* 2 {{static_assert failed{{.*}} "default_delete cannot be instantiated for function types"}}
  {
    SPtr<4> s4(getFn<4>()); // expected-note {{requested here}}
    SPtr<5> s5(getFn<5>(), std::default_delete<FnType<5>>{}); // expected-note {{requested here}}
  }
}

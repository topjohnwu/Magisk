#pragma once

namespace utils {
	template< class T > struct remove_reference      {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	template< class T > struct remove_reference<T&&> {typedef T type;};

	template< class T >
	constexpr typename remove_reference<T>::type&& move( T&& t ) noexcept {
		return static_cast<typename remove_reference<T>::type&&>(t);
	}
}
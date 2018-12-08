#include "CharArray.h"
#include "utils.h"

CharArray::CharArray() : _buf(nullptr), _size(0){}

CharArray::CharArray(const char *s) : CharArray() {
	this->operator=(s);
}

CharArray::CharArray(const CharArray &s) : CharArray() {
	this->operator=(s);
}

CharArray::CharArray(size_t i) {
	_size = i;
	_buf = new char[i](); /* Zero initialize */
}

CharArray::~CharArray() {
	delete[] _buf;
}

CharArray::operator char *() {
	return _buf;
}

CharArray::operator const char *() const {
	return _buf;
}

const char *CharArray::c_str() const {
	return _buf;
}

size_t CharArray::length() const {
	return strlen(_buf);
}

size_t CharArray::size() const {
	return _size;
}

CharArray &CharArray::operator=(const CharArray &s) {
	delete[] _buf;
	_size = s._size;
	_buf = new char[_size];
	memcpy(_buf, s._buf, _size);
	return *this;
}

CharArray &CharArray::operator=(const char *s) {
	delete[] _buf;
	_buf = strdup2(s, &_size);
	return *this;
}

CharArray &CharArray::operator=(CharArray &&s) {
	delete[] _buf;
	_size = s._size;
	_buf = s._buf;
	s._buf = nullptr;
	s._size = 0;
	return *this;
}

bool CharArray::operator==(const char *s) const {
	if (_buf == nullptr || s == nullptr)
		return false;
	return strcmp(_buf, s) == 0;
}

bool CharArray::operator==(char *s) const {
	return *this == (const char *) s;
}

bool CharArray::operator!=(const char *s) const {
	return !(*this == s);
}

int CharArray::compare(const char *s) const {
	return strcmp(_buf, s);
}

int CharArray::compare(const char *s, size_t len) const {
	return strncmp(_buf, s, len);
}

bool CharArray::contains(const char *s) const {
	return s == nullptr ? false : strstr(_buf, s) != nullptr;
}

bool CharArray::starts_with(const char *s) const {
	return s == nullptr ? false : compare(s, strlen(s)) == 0;
}

bool CharArray::empty() const {
	return _buf == nullptr || _buf[0] == '\0';
}


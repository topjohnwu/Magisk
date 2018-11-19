#pragma once

#include <string.h>

/* A wrapper around char array */
class CharArray {
public:
	CharArray();
	CharArray(const char *s);
	CharArray(const CharArray &s);
	CharArray(size_t i);
	~CharArray();

	CharArray &operator=(const CharArray &s);
	CharArray &operator=(CharArray &&s);
	CharArray &operator=(const char *s);

	operator char *();
	operator const char *() const;
	bool operator==(char *s) const;
	bool operator==(const char *s) const;
	bool operator!=(const char *s) const;
	int compare(const char *s) const;
	int compare(const char *s, size_t len) const;
	bool starts_with(const char *s) const;
	bool contains(const char *s) const;
	bool empty() const;
	const char *c_str() const;
	size_t length() const;
	size_t size() const;

	/* These 2 ops are incompatible with implicit char* conversion */
//	char &operator[](size_t i);
//	const char &operator[](size_t i) const;

private:
	char *_buf;
	size_t _size;
};

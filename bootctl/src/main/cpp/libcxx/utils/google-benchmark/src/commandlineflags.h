#ifndef BENCHMARK_COMMANDLINEFLAGS_H_
#define BENCHMARK_COMMANDLINEFLAGS_H_

#include <cstdint>
#include <string>

// Macro for referencing flags.
#define FLAG(name) FLAGS_##name

// Macros for declaring flags.
#define DECLARE_bool(name) extern bool FLAG(name)
#define DECLARE_int32(name) extern int32_t FLAG(name)
#define DECLARE_int64(name) extern int64_t FLAG(name)
#define DECLARE_double(name) extern double FLAG(name)
#define DECLARE_string(name) extern std::string FLAG(name)

// Macros for defining flags.
#define DEFINE_bool(name, default_val, doc) bool FLAG(name) = (default_val)
#define DEFINE_int32(name, default_val, doc) int32_t FLAG(name) = (default_val)
#define DEFINE_int64(name, default_val, doc) int64_t FLAG(name) = (default_val)
#define DEFINE_double(name, default_val, doc) double FLAG(name) = (default_val)
#define DEFINE_string(name, default_val, doc) \
  std::string FLAG(name) = (default_val)

namespace benchmark {
// Parses 'str' for a 32-bit signed integer.  If successful, writes the result
// to *value and returns true; otherwise leaves *value unchanged and returns
// false.
bool ParseInt32(const std::string& src_text, const char* str, int32_t* value);

// Parses a bool/Int32/string from the environment variable
// corresponding to the given Google Test flag.
bool BoolFromEnv(const char* flag, bool default_val);
int32_t Int32FromEnv(const char* flag, int32_t default_val);
double DoubleFromEnv(const char* flag, double default_val);
const char* StringFromEnv(const char* flag, const char* default_val);

// Parses a string for a bool flag, in the form of either
// "--flag=value" or "--flag".
//
// In the former case, the value is taken as true if it passes IsTruthyValue().
//
// In the latter case, the value is taken as true.
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseBoolFlag(const char* str, const char* flag, bool* value);

// Parses a string for an Int32 flag, in the form of
// "--flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseInt32Flag(const char* str, const char* flag, int32_t* value);

// Parses a string for a Double flag, in the form of
// "--flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseDoubleFlag(const char* str, const char* flag, double* value);

// Parses a string for a string flag, in the form of
// "--flag=value".
//
// On success, stores the value of the flag in *value, and returns
// true.  On failure, returns false without changing *value.
bool ParseStringFlag(const char* str, const char* flag, std::string* value);

// Returns true if the string matches the flag.
bool IsFlag(const char* str, const char* flag);

// Returns true unless value starts with one of: '0', 'f', 'F', 'n' or 'N', or
// some non-alphanumeric character. As a special case, also returns true if
// value is the empty string.
bool IsTruthyFlagValue(const std::string& value);
}  // end namespace benchmark

#endif  // BENCHMARK_COMMANDLINEFLAGS_H_

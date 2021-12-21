/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_STRING16_H
#define ANDROID_STRING16_H

#include <iostream>
#include <string>

#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/TypeHelpers.h>

// ---------------------------------------------------------------------------

namespace android {

// ---------------------------------------------------------------------------

template <size_t N>
class StaticString16;

// DO NOT USE: please use std::u16string

//! This is a string holding UTF-16 characters.
class String16
{
public:
                                String16();
                                String16(const String16& o);
                                String16(const String16& o,
                                         size_t len,
                                         size_t begin=0);
    explicit                    String16(const char16_t* o);
    explicit                    String16(const char16_t* o, size_t len);
    explicit                    String16(const String8& o);
    explicit                    String16(const char* o);
    explicit                    String16(const char* o, size_t len);

                                ~String16();

    inline  const char16_t*     string() const;

private:
    static inline std::string   std_string(const String16& str);
public:
            size_t              size() const;
            void                setTo(const String16& other);
            status_t            setTo(const char16_t* other);
            status_t            setTo(const char16_t* other, size_t len);
            status_t            setTo(const String16& other,
                                      size_t len,
                                      size_t begin=0);

            status_t            append(const String16& other);
            status_t            append(const char16_t* other, size_t len);

    inline  String16&           operator=(const String16& other);

    inline  String16&           operator+=(const String16& other);
    inline  String16            operator+(const String16& other) const;

            status_t            insert(size_t pos, const char16_t* chrs);
            status_t            insert(size_t pos,
                                       const char16_t* chrs, size_t len);

            ssize_t             findFirst(char16_t c) const;
            ssize_t             findLast(char16_t c) const;

            bool                startsWith(const String16& prefix) const;
            bool                startsWith(const char16_t* prefix) const;

            bool                contains(const char16_t* chrs) const;

            status_t            replaceAll(char16_t replaceThis,
                                           char16_t withThis);

    inline  int                 compare(const String16& other) const;

    inline  bool                operator<(const String16& other) const;
    inline  bool                operator<=(const String16& other) const;
    inline  bool                operator==(const String16& other) const;
    inline  bool                operator!=(const String16& other) const;
    inline  bool                operator>=(const String16& other) const;
    inline  bool                operator>(const String16& other) const;

    inline  bool                operator<(const char16_t* other) const;
    inline  bool                operator<=(const char16_t* other) const;
    inline  bool                operator==(const char16_t* other) const;
    inline  bool                operator!=(const char16_t* other) const;
    inline  bool                operator>=(const char16_t* other) const;
    inline  bool                operator>(const char16_t* other) const;

    inline                      operator const char16_t*() const;

    // Static and non-static String16 behave the same for the users, so
    // this method isn't of much use for the users. It is public for testing.
            bool                isStaticString() const;

  private:
    /*
     * A flag indicating the type of underlying buffer.
     */
    static constexpr uint32_t kIsSharedBufferAllocated = 0x80000000;

    /*
     * alloc() returns void* so that SharedBuffer class is not exposed.
     */
    static void* alloc(size_t size);
    static char16_t* allocFromUTF8(const char* u8str, size_t u8len);
    static char16_t* allocFromUTF16(const char16_t* u16str, size_t u16len);

    /*
     * edit() and editResize() return void* so that SharedBuffer class
     * is not exposed.
     */
    void* edit();
    void* editResize(size_t new_size);

    void acquire();
    void release();

    size_t staticStringSize() const;

    const char16_t* mString;

protected:
    /*
     * Data structure used to allocate static storage for static String16.
     *
     * Note that this data structure and SharedBuffer are used interchangably
     * as the underlying data structure for a String16.  Therefore, the layout
     * of this data structure must match the part in SharedBuffer that is
     * visible to String16.
     */
    template <size_t N>
    struct StaticData {
        // The high bit of 'size' is used as a flag.
        static_assert(N - 1 < kIsSharedBufferAllocated, "StaticString16 too long!");
        constexpr StaticData() : size(N - 1), data{0} {}
        const uint32_t size;
        char16_t data[N];

        constexpr StaticData(const StaticData<N>&) = default;
    };

    /*
     * Helper function for constructing a StaticData object.
     */
    template <size_t N>
    static constexpr const StaticData<N> makeStaticData(const char16_t (&s)[N]) {
        StaticData<N> r;
        // The 'size' field is at the same location where mClientMetadata would
        // be for a SharedBuffer.  We do NOT set kIsSharedBufferAllocated flag
        // here.
        for (size_t i = 0; i < N - 1; ++i) r.data[i] = s[i];
        return r;
    }

    template <size_t N>
    explicit constexpr String16(const StaticData<N>& s) : mString(s.data) {}

public:
    template <size_t N>
    explicit constexpr String16(const StaticString16<N>& s) : mString(s.mString) {}
};

// String16 can be trivially moved using memcpy() because moving does not
// require any change to the underlying SharedBuffer contents or reference count.
ANDROID_TRIVIAL_MOVE_TRAIT(String16)

static inline std::ostream& operator<<(std::ostream& os, const String16& str) {
    os << String8(str);
    return os;
}

// ---------------------------------------------------------------------------

/*
 * A StaticString16 object is a specialized String16 object.  Instead of holding
 * the string data in a ref counted SharedBuffer object, it holds data in a
 * buffer within StaticString16 itself.  Note that this buffer is NOT ref
 * counted and is assumed to be available for as long as there is at least a
 * String16 object using it.  Therefore, one must be extra careful to NEVER
 * assign a StaticString16 to a String16 that outlives the StaticString16
 * object.
 *
 * THE SAFEST APPROACH IS TO USE StaticString16 ONLY AS GLOBAL VARIABLES.
 *
 * A StaticString16 SHOULD NEVER APPEAR IN APIs.  USE String16 INSTEAD.
 */
template <size_t N>
class StaticString16 : public String16 {
public:
    constexpr StaticString16(const char16_t (&s)[N]) : String16(mData), mData(makeStaticData(s)) {}

    constexpr StaticString16(const StaticString16<N>& other)
        : String16(mData), mData(other.mData) {}

    constexpr StaticString16(const StaticString16<N>&&) = delete;

    // There is no reason why one would want to 'new' a StaticString16.  Delete
    // it to discourage misuse.
    static void* operator new(std::size_t) = delete;

private:
    const StaticData<N> mData;
};

template <typename F>
StaticString16(const F&)->StaticString16<sizeof(F) / sizeof(char16_t)>;

// ---------------------------------------------------------------------------
// No user servicable parts below.

inline int compare_type(const String16& lhs, const String16& rhs)
{
    return lhs.compare(rhs);
}

inline int strictly_order_type(const String16& lhs, const String16& rhs)
{
    return compare_type(lhs, rhs) < 0;
}

inline const char16_t* String16::string() const
{
    return mString;
}

inline std::string String16::std_string(const String16& str)
{
    return std::string(String8(str).string());
}

inline String16& String16::operator=(const String16& other)
{
    setTo(other);
    return *this;
}

inline String16& String16::operator+=(const String16& other)
{
    append(other);
    return *this;
}

inline String16 String16::operator+(const String16& other) const
{
    String16 tmp(*this);
    tmp += other;
    return tmp;
}

inline int String16::compare(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size());
}

inline bool String16::operator<(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size()) < 0;
}

inline bool String16::operator<=(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size()) <= 0;
}

inline bool String16::operator==(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size()) == 0;
}

inline bool String16::operator!=(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size()) != 0;
}

inline bool String16::operator>=(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size()) >= 0;
}

inline bool String16::operator>(const String16& other) const
{
    return strzcmp16(mString, size(), other.mString, other.size()) > 0;
}

inline bool String16::operator<(const char16_t* other) const
{
    return strcmp16(mString, other) < 0;
}

inline bool String16::operator<=(const char16_t* other) const
{
    return strcmp16(mString, other) <= 0;
}

inline bool String16::operator==(const char16_t* other) const
{
    return strcmp16(mString, other) == 0;
}

inline bool String16::operator!=(const char16_t* other) const
{
    return strcmp16(mString, other) != 0;
}

inline bool String16::operator>=(const char16_t* other) const
{
    return strcmp16(mString, other) >= 0;
}

inline bool String16::operator>(const char16_t* other) const
{
    return strcmp16(mString, other) > 0;
}

inline String16::operator const char16_t*() const
{
    return mString;
}

}  // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_STRING16_H

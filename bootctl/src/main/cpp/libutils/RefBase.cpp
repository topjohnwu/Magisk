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

#define LOG_TAG "RefBase"
// #define LOG_NDEBUG 0

#include <memory>

#include <android-base/macros.h>

#include <log/log.h>

#include <utils/RefBase.h>

#include <utils/Mutex.h>

#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

// Compile with refcounting debugging enabled.
#define DEBUG_REFS 0

// The following three are ignored unless DEBUG_REFS is set.

// whether ref-tracking is enabled by default, if not, trackMe(true, false)
// needs to be called explicitly
#define DEBUG_REFS_ENABLED_BY_DEFAULT 0

// whether callstack are collected (significantly slows things down)
#define DEBUG_REFS_CALLSTACK_ENABLED 1

// folder where stack traces are saved when DEBUG_REFS is enabled
// this folder needs to exist and be writable
#define DEBUG_REFS_CALLSTACK_PATH "/data/debug"

// log all reference counting operations
#define PRINT_REFS 0

// Continue after logging a stack trace if ~RefBase discovers that reference
// count has never been incremented. Normally we conspicuously crash in that
// case.
#define DEBUG_REFBASE_DESTRUCTION 1

#if !defined(_WIN32) && !defined(__APPLE__)
// CallStack is only supported on linux type platforms.
#define CALLSTACK_ENABLED 1
#else
#define CALLSTACK_ENABLED 0
#endif

#if CALLSTACK_ENABLED
#include <utils/CallStack.h>
#endif

// ---------------------------------------------------------------------------

namespace android {

// Observations, invariants, etc:

// By default, obects are destroyed when the last strong reference disappears
// or, if the object never had a strong reference, when the last weak reference
// disappears.
//
// OBJECT_LIFETIME_WEAK changes this behavior to retain the object
// unconditionally until the last reference of either kind disappears.  The
// client ensures that the extendObjectLifetime call happens before the dec
// call that would otherwise have deallocated the object, or before an
// attemptIncStrong call that might rely on it.  We do not worry about
// concurrent changes to the object lifetime.
//
// AttemptIncStrong will succeed if the object has a strong reference, or if it
// has a weak reference and has never had a strong reference.
// AttemptIncWeak really does succeed only if there is already a WEAK
// reference, and thus may fail when attemptIncStrong would succeed.
//
// mStrong is the strong reference count.  mWeak is the weak reference count.
// Between calls, and ignoring memory ordering effects, mWeak includes strong
// references, and is thus >= mStrong.
//
// A weakref_impl holds all the information, including both reference counts,
// required to perform wp<> operations.  Thus these can continue to be performed
// after the RefBase object has been destroyed.
//
// A weakref_impl is allocated as the value of mRefs in a RefBase object on
// construction.
// In the OBJECT_LIFETIME_STRONG case, it is normally deallocated in decWeak,
// and hence lives as long as the last weak reference. (It can also be
// deallocated in the RefBase destructor iff the strong reference count was
// never incremented and the weak count is zero, e.g.  if the RefBase object is
// explicitly destroyed without decrementing the strong count.  This should be
// avoided.) In this case, the RefBase destructor should be invoked from
// decStrong.
// In the OBJECT_LIFETIME_WEAK case, the weakref_impl is always deallocated in
// the RefBase destructor, which is always invoked by decWeak. DecStrong
// explicitly avoids the deletion in this case.
//
// Memory ordering:
// The client must ensure that every inc() call, together with all other
// accesses to the object, happens before the corresponding dec() call.
//
// We try to keep memory ordering constraints on atomics as weak as possible,
// since memory fences or ordered memory accesses are likely to be a major
// performance cost for this code. All accesses to mStrong, mWeak, and mFlags
// explicitly relax memory ordering in some way.
//
// The only operations that are not memory_order_relaxed are reference count
// decrements. All reference count decrements are release operations.  In
// addition, the final decrement leading the deallocation is followed by an
// acquire fence, which we can view informally as also turning it into an
// acquire operation.  (See 29.8p4 [atomics.fences] for details. We could
// alternatively use acq_rel operations for all decrements. This is probably
// slower on most current (2016) hardware, especially on ARMv7, but that may
// not be true indefinitely.)
//
// This convention ensures that the second-to-last decrement synchronizes with
// (in the language of 1.10 in the C++ standard) the final decrement of a
// reference count. Since reference counts are only updated using atomic
// read-modify-write operations, this also extends to any earlier decrements.
// (See "release sequence" in 1.10.)
//
// Since all operations on an object happen before the corresponding reference
// count decrement, and all reference count decrements happen before the final
// one, we are guaranteed that all other object accesses happen before the
// object is destroyed.


#define INITIAL_STRONG_VALUE (1<<28)

#define MAX_COUNT 0xfffff

// Test whether the argument is a clearly invalid strong reference count.
// Used only for error checking on the value before an atomic decrement.
// Intended to be very cheap.
// Note that we cannot just check for excess decrements by comparing to zero
// since the object would be deallocated before that.
#define BAD_STRONG(c) \
        ((c) == 0 || ((c) & (~(MAX_COUNT | INITIAL_STRONG_VALUE))) != 0)

// Same for weak counts.
#define BAD_WEAK(c) ((c) == 0 || ((c) & (~MAX_COUNT)) != 0)

// ---------------------------------------------------------------------------

class RefBase::weakref_impl : public RefBase::weakref_type
{
public:
    std::atomic<int32_t>    mStrong;
    std::atomic<int32_t>    mWeak;
    RefBase* const          mBase;
    std::atomic<int32_t>    mFlags;

#if !DEBUG_REFS

    explicit weakref_impl(RefBase* base)
        : mStrong(INITIAL_STRONG_VALUE)
        , mWeak(0)
        , mBase(base)
        , mFlags(0)
    {
    }

    void addStrongRef(const void* /*id*/) { }
    void removeStrongRef(const void* /*id*/) { }
    void renameStrongRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void addWeakRef(const void* /*id*/) { }
    void removeWeakRef(const void* /*id*/) { }
    void renameWeakRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void printRefs() const { }
    void trackMe(bool, bool) { }

#else

    weakref_impl(RefBase* base)
        : mStrong(INITIAL_STRONG_VALUE)
        , mWeak(0)
        , mBase(base)
        , mFlags(0)
        , mStrongRefs(NULL)
        , mWeakRefs(NULL)
        , mTrackEnabled(!!DEBUG_REFS_ENABLED_BY_DEFAULT)
        , mRetain(false)
    {
    }

    ~weakref_impl()
    {
        bool dumpStack = false;
        if (!mRetain && mStrongRefs != NULL) {
            dumpStack = true;
            ALOGE("Strong references remain:");
            ref_entry* refs = mStrongRefs;
            while (refs) {
                char inc = refs->ref >= 0 ? '+' : '-';
                ALOGD("\t%c ID %p (ref %d):", inc, refs->id, refs->ref);
#if DEBUG_REFS_CALLSTACK_ENABLED && CALLSTACK_ENABLED
                CallStack::logStack(LOG_TAG, refs->stack.get());
#endif
                refs = refs->next;
            }
        }

        if (!mRetain && mWeakRefs != NULL) {
            dumpStack = true;
            ALOGE("Weak references remain!");
            ref_entry* refs = mWeakRefs;
            while (refs) {
                char inc = refs->ref >= 0 ? '+' : '-';
                ALOGD("\t%c ID %p (ref %d):", inc, refs->id, refs->ref);
#if DEBUG_REFS_CALLSTACK_ENABLED && CALLSTACK_ENABLED
                CallStack::logStack(LOG_TAG, refs->stack.get());
#endif
                refs = refs->next;
            }
        }
        if (dumpStack) {
            ALOGE("above errors at:");
#if CALLSTACK_ENABLED
            CallStack::logStack(LOG_TAG);
#endif
        }
    }

    void addStrongRef(const void* id) {
        //ALOGD_IF(mTrackEnabled,
        //        "addStrongRef: RefBase=%p, id=%p", mBase, id);
        addRef(&mStrongRefs, id, mStrong.load(std::memory_order_relaxed));
    }

    void removeStrongRef(const void* id) {
        //ALOGD_IF(mTrackEnabled,
        //        "removeStrongRef: RefBase=%p, id=%p", mBase, id);
        if (!mRetain) {
            removeRef(&mStrongRefs, id);
        } else {
            addRef(&mStrongRefs, id, -mStrong.load(std::memory_order_relaxed));
        }
    }

    void renameStrongRefId(const void* old_id, const void* new_id) {
        //ALOGD_IF(mTrackEnabled,
        //        "renameStrongRefId: RefBase=%p, oid=%p, nid=%p",
        //        mBase, old_id, new_id);
        renameRefsId(mStrongRefs, old_id, new_id);
    }

    void addWeakRef(const void* id) {
        addRef(&mWeakRefs, id, mWeak.load(std::memory_order_relaxed));
    }

    void removeWeakRef(const void* id) {
        if (!mRetain) {
            removeRef(&mWeakRefs, id);
        } else {
            addRef(&mWeakRefs, id, -mWeak.load(std::memory_order_relaxed));
        }
    }

    void renameWeakRefId(const void* old_id, const void* new_id) {
        renameRefsId(mWeakRefs, old_id, new_id);
    }

    void trackMe(bool track, bool retain) {
        mTrackEnabled = track;
        mRetain = retain;
    }

    void printRefs() const
    {
        String8 text;

        {
            Mutex::Autolock _l(mMutex);
            char buf[128];
            snprintf(buf, sizeof(buf),
                     "Strong references on RefBase %p (weakref_type %p):\n",
                     mBase, this);
            text.append(buf);
            printRefsLocked(&text, mStrongRefs);
            snprintf(buf, sizeof(buf),
                     "Weak references on RefBase %p (weakref_type %p):\n",
                     mBase, this);
            text.append(buf);
            printRefsLocked(&text, mWeakRefs);
        }

        {
            char name[100];
            snprintf(name, sizeof(name), DEBUG_REFS_CALLSTACK_PATH "/%p.stack",
                     this);
            int rc = open(name, O_RDWR | O_CREAT | O_APPEND, 644);
            if (rc >= 0) {
                (void)write(rc, text.string(), text.length());
                close(rc);
                ALOGD("STACK TRACE for %p saved in %s", this, name);
            }
            else ALOGE("FAILED TO PRINT STACK TRACE for %p in %s: %s", this,
                      name, strerror(errno));
        }
    }

private:
    struct ref_entry
    {
        ref_entry* next;
        const void* id;
#if DEBUG_REFS_CALLSTACK_ENABLED && CALLSTACK_ENABLED
        CallStack::CallStackUPtr stack;
#endif
        int32_t ref;
    };

    void addRef(ref_entry** refs, const void* id, int32_t mRef)
    {
        if (mTrackEnabled) {
            AutoMutex _l(mMutex);

            ref_entry* ref = new ref_entry;
            // Reference count at the time of the snapshot, but before the
            // update.  Positive value means we increment, negative--we
            // decrement the reference count.
            ref->ref = mRef;
            ref->id = id;
#if DEBUG_REFS_CALLSTACK_ENABLED && CALLSTACK_ENABLED
            ref->stack = CallStack::getCurrent(2);
#endif
            ref->next = *refs;
            *refs = ref;
        }
    }

    void removeRef(ref_entry** refs, const void* id)
    {
        if (mTrackEnabled) {
            AutoMutex _l(mMutex);

            ref_entry* const head = *refs;
            ref_entry* ref = head;
            while (ref != NULL) {
                if (ref->id == id) {
                    *refs = ref->next;
                    delete ref;
                    return;
                }
                refs = &ref->next;
                ref = *refs;
            }

            ALOGE("RefBase: removing id %p on RefBase %p"
                    "(weakref_type %p) that doesn't exist!",
                    id, mBase, this);

            ref = head;
            while (ref) {
                char inc = ref->ref >= 0 ? '+' : '-';
                ALOGD("\t%c ID %p (ref %d):", inc, ref->id, ref->ref);
                ref = ref->next;
            }

#if CALLSTACK_ENABLED
            CallStack::logStack(LOG_TAG);
#endif
        }
    }

    void renameRefsId(ref_entry* r, const void* old_id, const void* new_id)
    {
        if (mTrackEnabled) {
            AutoMutex _l(mMutex);
            ref_entry* ref = r;
            while (ref != NULL) {
                if (ref->id == old_id) {
                    ref->id = new_id;
                }
                ref = ref->next;
            }
        }
    }

    void printRefsLocked(String8* out, const ref_entry* refs) const
    {
        char buf[128];
        while (refs) {
            char inc = refs->ref >= 0 ? '+' : '-';
            snprintf(buf, sizeof(buf), "\t%c ID %p (ref %d):\n",
                     inc, refs->id, refs->ref);
            out->append(buf);
#if DEBUG_REFS_CALLSTACK_ENABLED && CALLSTACK_ENABLED
            out->append(CallStack::stackToString("\t\t", refs->stack.get()));
#else
            out->append("\t\t(call stacks disabled)");
#endif
            refs = refs->next;
        }
    }

    mutable Mutex mMutex;
    ref_entry* mStrongRefs;
    ref_entry* mWeakRefs;

    bool mTrackEnabled;
    // Collect stack traces on addref and removeref, instead of deleting the stack references
    // on removeref that match the address ones.
    bool mRetain;

#endif
};

// ---------------------------------------------------------------------------

void RefBase::incStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);

    refs->addStrongRef(id);
    const int32_t c = refs->mStrong.fetch_add(1, std::memory_order_relaxed);
    ALOG_ASSERT(c > 0, "incStrong() called on %p after last strong ref", refs);
#if PRINT_REFS
    ALOGD("incStrong of %p from %p: cnt=%d\n", this, id, c);
#endif
    if (c != INITIAL_STRONG_VALUE)  {
        return;
    }

    int32_t old __unused = refs->mStrong.fetch_sub(INITIAL_STRONG_VALUE, std::memory_order_relaxed);
    // A decStrong() must still happen after us.
    ALOG_ASSERT(old > INITIAL_STRONG_VALUE, "0x%x too small", old);
    refs->mBase->onFirstRef();
}

void RefBase::incStrongRequireStrong(const void* id) const {
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);

    refs->addStrongRef(id);
    const int32_t c = refs->mStrong.fetch_add(1, std::memory_order_relaxed);

    LOG_ALWAYS_FATAL_IF(c <= 0 || c == INITIAL_STRONG_VALUE,
                        "incStrongRequireStrong() called on %p which isn't already owned", refs);
#if PRINT_REFS
    ALOGD("incStrong (requiring strong) of %p from %p: cnt=%d\n", this, id, c);
#endif
}

void RefBase::decStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->removeStrongRef(id);
    const int32_t c = refs->mStrong.fetch_sub(1, std::memory_order_release);
#if PRINT_REFS
    ALOGD("decStrong of %p from %p: cnt=%d\n", this, id, c);
#endif
    LOG_ALWAYS_FATAL_IF(BAD_STRONG(c), "decStrong() called on %p too many times",
            refs);
    if (c == 1) {
        std::atomic_thread_fence(std::memory_order_acquire);
        refs->mBase->onLastStrongRef(id);
        int32_t flags = refs->mFlags.load(std::memory_order_relaxed);
        if ((flags&OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            delete this;
            // The destructor does not delete refs in this case.
        }
    }
    // Note that even with only strong reference operations, the thread
    // deallocating this may not be the same as the thread deallocating refs.
    // That's OK: all accesses to this happen before its deletion here,
    // and all accesses to refs happen before its deletion in the final decWeak.
    // The destructor can safely access mRefs because either it's deleting
    // mRefs itself, or it's running entirely before the final mWeak decrement.
    //
    // Since we're doing atomic loads of `flags`, the static analyzer assumes
    // they can change between `delete this;` and `refs->decWeak(id);`. This is
    // not the case. The analyzer may become more okay with this patten when
    // https://bugs.llvm.org/show_bug.cgi?id=34365 gets resolved. NOLINTNEXTLINE
    refs->decWeak(id);
}

void RefBase::forceIncStrong(const void* id) const
{
    // Allows initial mStrong of 0 in addition to INITIAL_STRONG_VALUE.
    // TODO: Better document assumptions.
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);

    refs->addStrongRef(id);
    const int32_t c = refs->mStrong.fetch_add(1, std::memory_order_relaxed);
    ALOG_ASSERT(c >= 0, "forceIncStrong called on %p after ref count underflow",
               refs);
#if PRINT_REFS
    ALOGD("forceIncStrong of %p from %p: cnt=%d\n", this, id, c);
#endif

    switch (c) {
    case INITIAL_STRONG_VALUE:
        refs->mStrong.fetch_sub(INITIAL_STRONG_VALUE,
                std::memory_order_relaxed);
        FALLTHROUGH_INTENDED;
    case 0:
        refs->mBase->onFirstRef();
    }
}

int32_t RefBase::getStrongCount() const
{
    // Debugging only; No memory ordering guarantees.
    return mRefs->mStrong.load(std::memory_order_relaxed);
}

RefBase* RefBase::weakref_type::refBase() const
{
    return static_cast<const weakref_impl*>(this)->mBase;
}

void RefBase::weakref_type::incWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->addWeakRef(id);
    const int32_t c __unused = impl->mWeak.fetch_add(1,
            std::memory_order_relaxed);
    ALOG_ASSERT(c >= 0, "incWeak called on %p after last weak ref", this);
}

void RefBase::weakref_type::incWeakRequireWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->addWeakRef(id);
    const int32_t c __unused = impl->mWeak.fetch_add(1,
            std::memory_order_relaxed);
    LOG_ALWAYS_FATAL_IF(c <= 0, "incWeakRequireWeak called on %p which has no weak refs", this);
}

void RefBase::weakref_type::decWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->removeWeakRef(id);
    const int32_t c = impl->mWeak.fetch_sub(1, std::memory_order_release);
    LOG_ALWAYS_FATAL_IF(BAD_WEAK(c), "decWeak called on %p too many times",
            this);
    if (c != 1) return;
    atomic_thread_fence(std::memory_order_acquire);

    int32_t flags = impl->mFlags.load(std::memory_order_relaxed);
    if ((flags&OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
        // This is the regular lifetime case. The object is destroyed
        // when the last strong reference goes away. Since weakref_impl
        // outlives the object, it is not destroyed in the dtor, and
        // we'll have to do it here.
        if (impl->mStrong.load(std::memory_order_relaxed)
                == INITIAL_STRONG_VALUE) {
            // Decrementing a weak count to zero when object never had a strong
            // reference.  We assume it acquired a weak reference early, e.g.
            // in the constructor, and will eventually be properly destroyed,
            // usually via incrementing and decrementing the strong count.
            // Thus we no longer do anything here.  We log this case, since it
            // seems to be extremely rare, and should not normally occur. We
            // used to deallocate mBase here, so this may now indicate a leak.
            ALOGW("RefBase: Object at %p lost last weak reference "
                    "before it had a strong reference", impl->mBase);
        } else {
            // ALOGV("Freeing refs %p of old RefBase %p\n", this, impl->mBase);
            delete impl;
        }
    } else {
        // This is the OBJECT_LIFETIME_WEAK case. The last weak-reference
        // is gone, we can destroy the object.
        impl->mBase->onLastWeakRef(id);
        delete impl->mBase;
    }
}

bool RefBase::weakref_type::attemptIncStrong(const void* id)
{
    incWeak(id);

    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    int32_t curCount = impl->mStrong.load(std::memory_order_relaxed);

    ALOG_ASSERT(curCount >= 0,
            "attemptIncStrong called on %p after underflow", this);

    while (curCount > 0 && curCount != INITIAL_STRONG_VALUE) {
        // we're in the easy/common case of promoting a weak-reference
        // from an existing strong reference.
        if (impl->mStrong.compare_exchange_weak(curCount, curCount+1,
                std::memory_order_relaxed)) {
            break;
        }
        // the strong count has changed on us, we need to re-assert our
        // situation. curCount was updated by compare_exchange_weak.
    }

    if (curCount <= 0 || curCount == INITIAL_STRONG_VALUE) {
        // we're now in the harder case of either:
        // - there never was a strong reference on us
        // - or, all strong references have been released
        int32_t flags = impl->mFlags.load(std::memory_order_relaxed);
        if ((flags&OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            // this object has a "normal" life-time, i.e.: it gets destroyed
            // when the last strong reference goes away
            if (curCount <= 0) {
                // the last strong-reference got released, the object cannot
                // be revived.
                decWeak(id);
                return false;
            }

            // here, curCount == INITIAL_STRONG_VALUE, which means
            // there never was a strong-reference, so we can try to
            // promote this object; we need to do that atomically.
            while (curCount > 0) {
                if (impl->mStrong.compare_exchange_weak(curCount, curCount+1,
                        std::memory_order_relaxed)) {
                    break;
                }
                // the strong count has changed on us, we need to re-assert our
                // situation (e.g.: another thread has inc/decStrong'ed us)
                // curCount has been updated.
            }

            if (curCount <= 0) {
                // promote() failed, some other thread destroyed us in the
                // meantime (i.e.: strong count reached zero).
                decWeak(id);
                return false;
            }
        } else {
            // this object has an "extended" life-time, i.e.: it can be
            // revived from a weak-reference only.
            // Ask the object's implementation if it agrees to be revived
            if (!impl->mBase->onIncStrongAttempted(FIRST_INC_STRONG, id)) {
                // it didn't so give-up.
                decWeak(id);
                return false;
            }
            // grab a strong-reference, which is always safe due to the
            // extended life-time.
            curCount = impl->mStrong.fetch_add(1, std::memory_order_relaxed);
            // If the strong reference count has already been incremented by
            // someone else, the implementor of onIncStrongAttempted() is holding
            // an unneeded reference.  So call onLastStrongRef() here to remove it.
            // (No, this is not pretty.)  Note that we MUST NOT do this if we
            // are in fact acquiring the first reference.
            if (curCount != 0 && curCount != INITIAL_STRONG_VALUE) {
                impl->mBase->onLastStrongRef(id);
            }
        }
    }

    impl->addStrongRef(id);

#if PRINT_REFS
    ALOGD("attemptIncStrong of %p from %p: cnt=%d\n", this, id, curCount);
#endif

    // curCount is the value of mStrong before we incremented it.
    // Now we need to fix-up the count if it was INITIAL_STRONG_VALUE.
    // This must be done safely, i.e.: handle the case where several threads
    // were here in attemptIncStrong().
    // curCount > INITIAL_STRONG_VALUE is OK, and can happen if we're doing
    // this in the middle of another incStrong.  The subtraction is handled
    // by the thread that started with INITIAL_STRONG_VALUE.
    if (curCount == INITIAL_STRONG_VALUE) {
        impl->mStrong.fetch_sub(INITIAL_STRONG_VALUE,
                std::memory_order_relaxed);
    }

    return true;
}

bool RefBase::weakref_type::attemptIncWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);

    int32_t curCount = impl->mWeak.load(std::memory_order_relaxed);
    ALOG_ASSERT(curCount >= 0, "attemptIncWeak called on %p after underflow",
               this);
    while (curCount > 0) {
        if (impl->mWeak.compare_exchange_weak(curCount, curCount+1,
                std::memory_order_relaxed)) {
            break;
        }
        // curCount has been updated.
    }

    if (curCount > 0) {
        impl->addWeakRef(id);
    }

    return curCount > 0;
}

int32_t RefBase::weakref_type::getWeakCount() const
{
    // Debug only!
    return static_cast<const weakref_impl*>(this)->mWeak
            .load(std::memory_order_relaxed);
}

void RefBase::weakref_type::printRefs() const
{
    static_cast<const weakref_impl*>(this)->printRefs();
}

void RefBase::weakref_type::trackMe(bool enable, bool retain)
{
    static_cast<weakref_impl*>(this)->trackMe(enable, retain);
}

RefBase::weakref_type* RefBase::createWeak(const void* id) const
{
    mRefs->incWeak(id);
    return mRefs;
}

RefBase::weakref_type* RefBase::getWeakRefs() const
{
    return mRefs;
}

RefBase::RefBase()
    : mRefs(new weakref_impl(this))
{
}

RefBase::~RefBase()
{
    int32_t flags = mRefs->mFlags.load(std::memory_order_relaxed);
    // Life-time of this object is extended to WEAK, in
    // which case weakref_impl doesn't out-live the object and we
    // can free it now.
    if ((flags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK) {
        // It's possible that the weak count is not 0 if the object
        // re-acquired a weak reference in its destructor
        if (mRefs->mWeak.load(std::memory_order_relaxed) == 0) {
            delete mRefs;
        }
    } else if (mRefs->mStrong.load(std::memory_order_relaxed) == INITIAL_STRONG_VALUE) {
        // We never acquired a strong reference on this object.
#if DEBUG_REFBASE_DESTRUCTION
        // Treating this as fatal is prone to causing boot loops. For debugging, it's
        // better to treat as non-fatal.
        ALOGD("RefBase: Explicit destruction, weak count = %d (in %p)", mRefs->mWeak.load(), this);

#if CALLSTACK_ENABLED
        CallStack::logStack(LOG_TAG);
#endif
#else
        LOG_ALWAYS_FATAL("RefBase: Explicit destruction, weak count = %d", mRefs->mWeak.load());
#endif
    }
    // For debugging purposes, clear mRefs.  Ineffective against outstanding wp's.
    const_cast<weakref_impl*&>(mRefs) = nullptr;
}

void RefBase::extendObjectLifetime(int32_t mode)
{
    // Must be happens-before ordered with respect to construction or any
    // operation that could destroy the object.
    mRefs->mFlags.fetch_or(mode, std::memory_order_relaxed);
}

void RefBase::onFirstRef()
{
}

void RefBase::onLastStrongRef(const void* /*id*/)
{
}

bool RefBase::onIncStrongAttempted(uint32_t flags, const void* /*id*/)
{
    return (flags&FIRST_INC_STRONG) ? true : false;
}

void RefBase::onLastWeakRef(const void* /*id*/)
{
}

// ---------------------------------------------------------------------------

#if DEBUG_REFS
void RefBase::renameRefs(size_t n, const ReferenceRenamer& renamer) {
    for (size_t i=0 ; i<n ; i++) {
        renamer(i);
    }
}
#else
void RefBase::renameRefs(size_t /*n*/, const ReferenceRenamer& /*renamer*/) { }
#endif

void RefBase::renameRefId(weakref_type* ref,
        const void* old_id, const void* new_id) {
    weakref_impl* const impl = static_cast<weakref_impl*>(ref);
    impl->renameStrongRefId(old_id, new_id);
    impl->renameWeakRefId(old_id, new_id);
}

void RefBase::renameRefId(RefBase* ref,
        const void* old_id, const void* new_id) {
    ref->mRefs->renameStrongRefId(old_id, new_id);
    ref->mRefs->renameWeakRefId(old_id, new_id);
}

}; // namespace android

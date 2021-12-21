/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_HYBRIDINTERFACE_H
#define ANDROID_HYBRIDINTERFACE_H

#include <vector>
#include <mutex>

#include <binder/Parcel.h>
#include <hidl/HidlSupport.h>

#include <cinttypes>
#include <variant>

/**
 * Hybrid Interfaces
 * =================
 *
 * A hybrid interface is a binder interface that
 * 1. is implemented both traditionally and as a wrapper around a hidl
 *    interface, and allows querying whether the underlying instance comes from
 *    a hidl interface or not; and
 * 2. allows efficient calls to a hidl interface (if the underlying instance
 *    comes from a hidl interface) by automatically creating the wrapper in the
 *    process that calls it.
 *
 * Terminology:
 * - `HalToken`: The type for a "token" of a hidl interface. This is defined to
 *   be compatible with `ITokenManager.hal`.
 * - `HInterface`: The base type for a hidl interface. Currently, it is defined
 *   as `::android::hidl::base::V1_0::IBase`.
 * - `HALINTERFACE`: The hidl interface that will be sent through binders.
 * - `INTERFACE`: The binder interface that will be the wrapper of
 *   `HALINTERFACE`. `INTERFACE` is supposed to be somewhat similar to
 *   `HALINTERFACE`.
 *
 * To demonstrate how this is done, here is an example. Suppose `INTERFACE` is
 * `IFoo` and `HALINTERFACE` is `HFoo`. The required steps are:
 * 1. Use `DECLARE_HYBRID_META_INTERFACE` instead of `DECLARE_META_INTERFACE` in
 *    the declaration of `IFoo`. `DECLARE_HYBRID_META_INTERFACE` takes an
 *    additional argument that is the hidl interface to be converted into a
 *    binder interface. Example:
 *        Change from `DECLARE_META_INTERFACE(Foo)`
 *                 to `DECLARE_HYBRID_META_INTERFACE(Foo, HFoo)`
 * 2. Create a converter class that derives from
 *    `H2BConverter<HFoo, BnFoo>`. Let us call this `H2BFoo`.
 * 3. Add the following constructor in `H2BFoo` that call the corresponding
 *    constructors in `H2BConverter`:
 *        `H2BFoo(const sp<HalInterface>& base) : CBase(base) {}`
 *    Note: `CBase = H2BConverter<HFoo, BnFoo>` and `HalInterface = HFoo` are
 *    member typedefs of `H2BConverter<HFoo, BnFoo>`, so the above line can be
 *    copied verbatim into `H2BFoo`.
 * 4. Implement `IFoo` in `H2BFoo` on top of `HFoo`. `H2BConverter` provides a
 *    protected `mBase` of type `sp<HFoo>` that can be used to access the `HFoo`
 *    instance. (There is also a public function named `getBase()` that returns
 *    `mBase`.)
 * 5. Create a hardware proxy class that derives from
 *    `HpInterface<BpFoo, H2BFoo>`. Name this class `HpFoo`. (This name cannot
 *    deviate. See step 8 below.)
 * 6. Add the following constructor to `HpFoo`:
 *        `HpFoo(const sp<IBinder>& base): PBase(base) {}`
 *    Note: `PBase` a member typedef of `HpInterface<BpFoo, H2BFoo>` that is
 *    equal to `HpInterface<BpFoo, H2BFoo>` itself, so the above line can be
 *    copied verbatim into `HpFoo`.
 * 7. Delegate all functions in `HpFoo` that come from `IFoo` (except those that
 *    are defined by the macro `DECLARE_HYBRID_META_INTERFACE`) to the protected
 *    member `mBase`. `mBase` is defined in `HpInterface<BpFoo, H2BFoo>` (hence
 *    in `HpFoo`) with type `IFoo`. There is also a public function named
 *    `getBase()` that returns `mBase`.
 * 8. Replace the existing `IMPLEMENT_META_INTERFACE` for `IFoo` by
 *    `IMPLEMENT_HYBRID_META_INTERFACE`. This macro assumes that the subclass of
 *    `HpInterface` for `IFoo` is named `HpFoo`.
 *
 * After the hybrid interface has been put in place properly, it can be used to
 * do the following tasks:
 * 1. Create an `IFoo` instance from an `HFoo` by passing `sp<HFoo>` to
 *    the constructor of `H2BFoo`.
 * 2. Retrieve an `HFoo` from an `HpFoo` instance by calling
 *    `HpFoo::getHalInterface<HFoo>()`. This function may return `nullptr` if
 *    the `HpFoo` object is not backed by `HFoo`. The template parameter is
 *    required because `HpFoo` in fact may be backed by multiple H2B converter
 *    classes.
 *
 * Multiple H2B Converters
 * =======================
 *
 * Because the system may support multiple versions of hidl interfaces for the
 * same object, one binder interface may correspond to multiple H2B converters.
 * The hybrid interface is designed to handle this as
 * well---`DECLARE_HYBRID_META_INTERFACE` and `HpInterface` can take a variable
 * number of arguments.
 *
 * As a concrete example, suppose `IFoo` is a binder interface that corresponds
 * to two hidl interfaces `HFoo1` and `HFoo2`. That means `HpFoo`, the hybrid
 * interface presenting `IFoo`, may be backed by `HFoo1` or `HFoo2`. This is
 * achievable by
 *
 *   - Replacing `DECLARE_META_INTERFACE(Foo)` by
 *     `DECLARE_HYBRID_META_INTERFACE(Foo, HFoo1, HFoo2)` in the declaration of
 *     `IFoo`.
 *   - Creating `H2BFoo1` as a subclass of `H2BConverter<HFoo1, BnFoo>`;
 *   - Creating `H2BFoo2` as a subclass of `H2BConverter<HFoo2, BnFoo>`; and
 *   - Creating `HpFoo` as a subclass of `HpInterface<BpFoo, H2BFoo1, H2BFoo2>`.
 *
 * It is important that `HFoo1` and `HFoo2` are different hidl interfaces. [The
 * actual requirement is that for each pair `<HFoo, IFoo>`, there can be only
 * one subclass of `H2BConverter<HFoo, BnFoo>`.]
 *
 * As mentioned in the previous section, `HpFoo::getHalInterface` requires a
 * template argument because it must be able to return different hidl
 * interface types based on which hidl interface is being used. The user of
 * `HpFoo` can query the type of the underlying hidl interface by calling
 * `HpFoo::getHalIndex()`. The return value is a 1-based index into the list of
 * all the supported hidl interfaces. In the example with 2 hidl interfaces
 * `HFoo1` and `HFoo2`, index 1 corresponds to `HFoo1` and index 2 corresponds
 * to `HFoo2`. A typical code block that accesses the underlying hidl interface
 * of would look like this:
 *
 * void someFunction(const sp<IFoo> &foo) {
 *
 *     switch (foo->getHalIndex()) {
 *     case 1: {
 *             sp<HFoo1> hFoo1 = foo->getHalInterface<HFoo1>();
 *             ...
 *             break;
 *         }
 *     case 2: {
 *             sp<HFoo2> hFoo2 = foo->getHalInterface<HFoo2>();
 *             ...
 *             break;
 *         }
 *     default: // Not backed by a hidl interface.
 *              // Alternatively, "case 0:" can be used.
 *     }
 *
 * }
 *
 * Error State
 * ===========
 *
 * A corrupted transaction may cause an `HpInterface` to be in an error state.
 * This could cause `getHalInterface<ExpectedHalInterface>()` to return
 * `nullptr` even though `getHalIndex()` returns a non-zero index and
 * `ExpectedHalInterface` is the corresponding hidl interface. It is therefore
 * recommended that a null check be performed on the return value of
 * `getHalInterface` before using it.
 *
 * DECLARE_HYBRID_META_INTERFACE_WITH_CODE
 * =======================================
 *
 * `H2BConverter` and `HpInterface` use `transact()` to send over tokens with
 * the transaction code (the first argument of `transact()`) equal to `_GHT`,
 * which is defined as a global constant named
 * `DEFAULT_GET_HAL_TOKEN_TRANSACTION_CODE`.
 *
 * In the rare occasion that this value clashes with other values already used
 * by the `Bp` class and modifying the `Bp` class is difficult, the
 * "GET_HAL_TOKEN" transaction code can be changed to a different value simply
 * by replacing `DECLARE_HYBRID_META_INTERFACE` with
 * `DECLARE_HYBRID_META_INTERFACE_WITH_CODE` in the declaration of the base
 * interface and supplying the new transaction code in the first argument of
 * this macro.
 *
 */

namespace android {

typedef ::android::hardware::hidl_vec<uint8_t> HalToken;
typedef ::android::hidl::base::V1_0::IBase HInterface;

constexpr uint32_t DEFAULT_GET_HAL_TOKEN_TRANSACTION_CODE =
        B_PACK_CHARS('_', 'G', 'H', 'T');

sp<HInterface> retrieveHalInterface(const HalToken& token);
bool createHalToken(const sp<HInterface>& interface, HalToken* token);
bool deleteHalToken(const HalToken& token);

template <typename HINTERFACE,
          typename BNINTERFACE>
class H2BConverter : public BNINTERFACE {
public:
    typedef H2BConverter<HINTERFACE, BNINTERFACE> CBase; // Converter Base
    typedef typename BNINTERFACE::BaseInterface BaseInterface;
    typedef HINTERFACE HalInterface;
    typedef typename BaseInterface::HalVariant HalVariant;
    using BaseInterface::sGetHalTokenTransactionCode;

    H2BConverter(const sp<HalInterface>& base) : mBase{base} {}
    virtual status_t onTransact(uint32_t code,
            const Parcel& data, Parcel* reply, uint32_t flags = 0);
    virtual status_t linkToDeath(
            const sp<IBinder::DeathRecipient>& recipient,
            void* cookie = nullptr,
            uint32_t flags = 0);
    virtual status_t unlinkToDeath(
            const wp<IBinder::DeathRecipient>& recipient,
            void* cookie = nullptr,
            uint32_t flags = 0,
            wp<IBinder::DeathRecipient>* outRecipient = nullptr);
    virtual HalVariant getHalVariant() const override { return { mBase }; }
    HalInterface* getBase() { return mBase.get(); }

protected:
    sp<HalInterface> mBase;

private:
    struct Obituary : public hardware::hidl_death_recipient {
        wp<IBinder::DeathRecipient> recipient;
        void* cookie;
        uint32_t flags;
        wp<IBinder> who;
        Obituary(
                const wp<IBinder::DeathRecipient>& r,
                void* c, uint32_t f,
                const wp<IBinder>& w) :
            recipient(r), cookie(c), flags(f), who(w) {
        }
        Obituary(const Obituary& o) :
            recipient(o.recipient),
            cookie(o.cookie),
            flags(o.flags),
            who(o.who) {
        }
        Obituary& operator=(const Obituary& o) {
            recipient = o.recipient;
            cookie = o.cookie;
            flags = o.flags;
            who = o.who;
            return *this;
        }
        void serviceDied(uint64_t, const wp<HInterface>&) override {
            sp<IBinder::DeathRecipient> dr = recipient.promote();
            if (dr != nullptr) {
                dr->binderDied(who);
            }
        }
    };
    std::mutex mObituariesLock;
    std::vector<sp<Obituary> > mObituaries;

    template <size_t Index = std::variant_size_v<HalVariant> - 1>
    static constexpr size_t _findIndex() {
        if constexpr (Index == 0) {
            return Index;
        } else if constexpr (
                std::is_same_v<
                    std::variant_alternative_t<Index, HalVariant>,
                    sp<HalInterface>>) {
            return Index;
        } else {
            return _findIndex<Index - 1>();
        }
    }

    static constexpr size_t sHalIndex = _findIndex<>();
    static_assert(sHalIndex != 0,
                  "H2BConverter from an unrecognized HAL interface.");
};

template <typename BPINTERFACE, typename CONVERTER, typename... CONVERTERS>
class HpInterface : public CONVERTER::BaseInterface {
public:
    typedef HpInterface<BPINTERFACE, CONVERTER, CONVERTERS...> PBase; // Proxy Base
    typedef typename CONVERTER::BaseInterface BaseInterface;
    typedef typename BaseInterface::HalVariant HalVariant;
    using BaseInterface::sGetHalTokenTransactionCode;

    explicit HpInterface(const sp<IBinder>& impl);
    BaseInterface* getBase() { return mBase.get(); }
    virtual HalVariant getHalVariant() const override { return mHalVariant; }

protected:
    IBinder* mBpBinder;
    sp<BPINTERFACE> mBp;
    sp<BaseInterface> mBase;
    HalVariant mHalVariant;
    bool mHasConverter{false};
    IBinder* onAsBinder() override { return mBpBinder; }

private:
    typedef std::variant<std::monostate,
            CONVERTER, CONVERTERS...> _ConverterVar;
    typedef std::variant<std::monostate,
            typename CONVERTER::HalInterface,
            typename CONVERTERS::HalInterface...> _ConverterHalVar;
    typedef std::variant<std::monostate,
            sp<typename CONVERTER::HalInterface>,
            sp<typename CONVERTERS::HalInterface>...> _ConverterHalPointerVar;

    static_assert(std::is_same_v<_ConverterHalPointerVar, HalVariant>,
                  "Converter classes do not match HAL interfaces.");

    template <size_t Index = std::variant_size_v<HalVariant> - 1>
    bool _castFromHalBaseAndConvert(size_t halIndex,
                                    const sp<HInterface>& halBase) {
        if constexpr (Index == 0) {
            return false;
        } else {
            if (halIndex != Index) {
                return _castFromHalBaseAndConvert<Index - 1>(halIndex, halBase);
            }
            typedef std::variant_alternative_t<Index, _ConverterHalVar>
                    HalInterface;
            sp<HalInterface> halInterface = HalInterface::castFrom(halBase);
            mHalVariant.template emplace<Index>(halInterface);
            if (!halInterface) {
                return false;
            }
            if (mHasConverter) {
                typedef std::variant_alternative_t<Index, _ConverterVar>
                        Converter;
                sp<Converter> converter = new Converter(halInterface);
                if (converter) {
                    mBase = converter;
                } else {
                    ALOGW("HpInterface: Failed to create an H2B converter -- "
                          "index = %zu.", Index);
                }
            }
            return true;
        }
    }

    bool castFromHalBaseAndConvert(size_t halIndex,
                                   const sp<HInterface>& halBase) {
        if (!_castFromHalBaseAndConvert<>(halIndex, halBase)) {
            return false;
        }
        return true;
    }

};

// ----------------------------------------------------------------------

#define DECLARE_HYBRID_META_INTERFACE(INTERFACE, ...)                     \
        DECLARE_HYBRID_META_INTERFACE_WITH_CODE(                          \
            ::android::DEFAULT_GET_HAL_TOKEN_TRANSACTION_CODE,            \
            INTERFACE, __VA_ARGS__)                                       \


#define DECLARE_HYBRID_META_INTERFACE_WITH_CODE(GTKCODE, INTERFACE, ...)  \
private:                                                                  \
    typedef ::std::variant<::std::monostate, __VA_ARGS__> _HalVariant;    \
    template <typename... Types>                                          \
    using _SpVariant =                                                    \
            ::std::variant<::std::monostate, ::android::sp<Types>...>;    \
public:                                                                   \
    typedef _SpVariant<__VA_ARGS__> HalVariant;                           \
    virtual HalVariant getHalVariant() const;                             \
    size_t getHalIndex() const;                                           \
    template <size_t Index>                                               \
    using HalInterface = ::std::variant_alternative_t<Index, _HalVariant>;\
    template <typename HAL>                                               \
    sp<HAL> getHalInterface() const {                                     \
        HalVariant halVariant = getHalVariant();                          \
        const sp<HAL>* hal = std::get_if<sp<HAL>>(&halVariant);           \
        return hal ? *hal : nullptr;                                      \
    }                                                                     \
                                                                          \
    static const ::android::String16 descriptor;                          \
    static ::android::sp<I##INTERFACE> asInterface(                       \
            const ::android::sp<::android::IBinder>& obj);                \
    virtual const ::android::String16& getInterfaceDescriptor() const;    \
    I##INTERFACE();                                                       \
    virtual ~I##INTERFACE();                                              \
    static constexpr uint32_t sGetHalTokenTransactionCode = GTKCODE;      \


#define IMPLEMENT_HYBRID_META_INTERFACE(INTERFACE, NAME)                  \
    I##INTERFACE::HalVariant I##INTERFACE::getHalVariant() const {        \
        return HalVariant{std::in_place_index<0>};                        \
    }                                                                     \
    size_t I##INTERFACE::getHalIndex() const {                            \
        return getHalVariant().index();                                   \
    }                                                                     \
    constexpr uint32_t I##INTERFACE::sGetHalTokenTransactionCode;         \
    static const ::android::StaticString16 I##INTERFACE##_desc_str16(     \
        u##NAME);                                                         \
    const ::android::String16 I##INTERFACE::descriptor(                   \
        I##INTERFACE##_desc_str16);                                       \
    const ::android::String16&                                            \
            I##INTERFACE::getInterfaceDescriptor() const {                \
        return I##INTERFACE::descriptor;                                  \
    }                                                                     \
    ::android::sp<I##INTERFACE> I##INTERFACE::asInterface(                \
            const ::android::sp<::android::IBinder>& obj)                 \
    {                                                                     \
        ::android::sp<I##INTERFACE> intr;                                 \
        if (obj != nullptr) {                                             \
            intr = static_cast<I##INTERFACE*>(                            \
                obj->queryLocalInterface(                                 \
                        I##INTERFACE::descriptor).get());                 \
            if (intr == nullptr) {                                        \
                intr = new Hp##INTERFACE(obj);                            \
            }                                                             \
        }                                                                 \
        return intr;                                                      \
    }                                                                     \
    I##INTERFACE::I##INTERFACE() { }                                      \
    I##INTERFACE::~I##INTERFACE() { }                                     \

// ----------------------------------------------------------------------

template <typename HINTERFACE,
          typename BNINTERFACE>
status_t H2BConverter<HINTERFACE, BNINTERFACE>::
        onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    if (code == sGetHalTokenTransactionCode) {
        if (!data.enforceInterface(BaseInterface::getInterfaceDescriptor())) {
            return BAD_TYPE;
        }

        HalToken token;
        bool result;
        result = createHalToken(mBase, &token);
        // Write whether a HAL token is present.
        reply->writeBool(result);
        if (!result) {
            ALOGE("H2BConverter: Failed to create HAL token.");
            return NO_ERROR;
        }

        // Write the HAL token.
        reply->writeByteArray(token.size(), token.data());

        // Write the HAL index.
        reply->writeUint32(static_cast<uint32_t>(sHalIndex));

        // Write a flag indicating that a converter needs to be created.
        reply->writeBool(true);

        return NO_ERROR;
    }
    return BNINTERFACE::onTransact(code, data, reply, flags);
}

template <typename HINTERFACE,
          typename BNINTERFACE>
status_t H2BConverter<HINTERFACE, BNINTERFACE>::linkToDeath(
        const sp<IBinder::DeathRecipient>& recipient,
        void* cookie, uint32_t flags) {
    LOG_ALWAYS_FATAL_IF(
            recipient == nullptr,
            "linkToDeath(): recipient must not be null.");
    {
        std::lock_guard<std::mutex> lock(mObituariesLock);
        mObituaries.push_back(new Obituary(recipient, cookie, flags, this));
        if (!mBase->linkToDeath(mObituaries.back(), 0)) {
           return DEAD_OBJECT;
        }
    }
    return NO_ERROR;
}

template <typename HINTERFACE,
          typename BNINTERFACE>
status_t H2BConverter<HINTERFACE, BNINTERFACE>::unlinkToDeath(
        const wp<IBinder::DeathRecipient>& recipient,
        void* cookie, uint32_t flags,
        wp<IBinder::DeathRecipient>* outRecipient) {
    std::lock_guard<std::mutex> lock(mObituariesLock);
    for (auto i = mObituaries.begin(); i != mObituaries.end(); ++i) {
        if ((flags = (*i)->flags) && (
                (recipient == (*i)->recipient) ||
                ((recipient == nullptr) && (cookie == (*i)->cookie)))) {
            if (outRecipient != nullptr) {
                *outRecipient = (*i)->recipient;
            }
            bool success = mBase->unlinkToDeath(*i);
            mObituaries.erase(i);
            return success ? NO_ERROR : DEAD_OBJECT;
        }
    }
    return NAME_NOT_FOUND;
}

template <typename BPINTERFACE, typename CONVERTER, typename... CONVERTERS>
HpInterface<BPINTERFACE, CONVERTER, CONVERTERS...>::HpInterface(
        const sp<IBinder>& impl)
      : mBpBinder{impl.get()},
        mBp{new BPINTERFACE(impl)} {
    mBase = mBp;
    if (!mBpBinder->remoteBinder()) {
        return;
    }
    Parcel data, reply;
    data.writeInterfaceToken(BaseInterface::getInterfaceDescriptor());
    if (mBpBinder->transact(sGetHalTokenTransactionCode,
                            data, &reply) == NO_ERROR) {
        // Read whether a HAL token is present.
        bool tokenCreated;
        if (reply.readBool(&tokenCreated) != OK) {
            ALOGW("HpInterface: Corrupted parcel from GET_HAL_TOKEN "
                  "(tokenCreated).");
        }

        if (!tokenCreated) {
            ALOGW("HpInterface: No HAL token was created.");
            return;
        }

        // Read the HAL token.
        std::vector<uint8_t> tokenVector;
        if (reply.readByteVector(&tokenVector) != OK) {
            ALOGW("HpInterface: Corrupted parcel from GET_HAL_TOKEN "
                  "(halToken).");
            return;
        }

        // Retrieve the HAL interface from the token.
        HalToken token{tokenVector};
        sp<HInterface> halBase = retrieveHalInterface(token);
        deleteHalToken(token);

        if (!halBase) {
            ALOGW("HpInterface: Failed to retrieve HAL interface.");
            return;
        }

        uint32_t halIndex;
        // Read the hal index.
        if (reply.readUint32(&halIndex) != OK) {
            ALOGW("HpInterface: Corrupted parcel from GET_HAL_TOKEN "
                  "(halIndex).");
            return;
        }

        // Read the converter flag.
        if (reply.readBool(&mHasConverter) != OK) {
            ALOGW("HpInterface: Corrupted parcel from GET_HAL_TOKEN "
                  "(hasConverter).");
            return;
        }

        // Call castFrom from the right HAL interface and create a converter if
        // needed.
        if (!castFromHalBaseAndConvert(static_cast<size_t>(halIndex),
                                       halBase)) {
            ALOGW("HpInterface: Failed to cast to the correct HAL interface -- "
                  "HAL index = %" PRIu32 ".", halIndex);
        }
    }
}

// ----------------------------------------------------------------------

}  // namespace android

#endif // ANDROID_HYBRIDINTERFACE_H

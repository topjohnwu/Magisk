/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "HidlStatus"
#include <android-base/logging.h>

#include <hidl/Status.h>
#include <utils/CallStack.h>

#include <unordered_map>

namespace android {
namespace hardware {


static std::string exceptionToString(int32_t ex) {
#define EXCEPTION_CASE(EXCEPTION)      \
    case Status::Exception::EXCEPTION: \
        return #EXCEPTION

    switch (ex) {
        EXCEPTION_CASE(EX_NONE);
        EXCEPTION_CASE(EX_SECURITY);
        EXCEPTION_CASE(EX_BAD_PARCELABLE);
        EXCEPTION_CASE(EX_ILLEGAL_ARGUMENT);
        EXCEPTION_CASE(EX_NULL_POINTER);
        EXCEPTION_CASE(EX_ILLEGAL_STATE);
        EXCEPTION_CASE(EX_NETWORK_MAIN_THREAD);
        EXCEPTION_CASE(EX_UNSUPPORTED_OPERATION);
        EXCEPTION_CASE(EX_HAS_REPLY_HEADER);
        EXCEPTION_CASE(EX_TRANSACTION_FAILED);
#undef EXCEPTION_CASE
    }
    return std::to_string(ex);
}

Status Status::ok() {
    return Status();
}

Status Status::fromExceptionCode(int32_t exceptionCode) {
    if (exceptionCode == EX_TRANSACTION_FAILED) {
        return Status(exceptionCode, FAILED_TRANSACTION);
    }
    return Status(exceptionCode, OK);
}

Status Status::fromExceptionCode(int32_t exceptionCode,
                                 const char *message) {
    if (exceptionCode == EX_TRANSACTION_FAILED) {
        return Status(exceptionCode, FAILED_TRANSACTION, message);
    }
    return Status(exceptionCode, OK, message);
}

Status Status::fromStatusT(status_t status) {
    Status ret;
    ret.setFromStatusT(status);
    return ret;
}

Status::Status(int32_t exceptionCode, int32_t errorCode)
    : mException(exceptionCode),
      mErrorCode(errorCode) {}

Status::Status(int32_t exceptionCode, int32_t errorCode, const char *message)
    : mException(exceptionCode),
      mErrorCode(errorCode),
      mMessage(message) {}

void Status::setException(int32_t ex, const char *message) {
    mException = ex;
    mErrorCode = ex == EX_TRANSACTION_FAILED ? FAILED_TRANSACTION : NO_ERROR;
    mMessage = message;
}

void Status::setFromStatusT(status_t status) {
    mException = (status == NO_ERROR) ? EX_NONE : EX_TRANSACTION_FAILED;
    mErrorCode = status;
    mMessage.clear();
}

std::string Status::description() const {
    std::ostringstream oss;
    oss << (*this);
    return oss.str();
}

std::ostream& operator<< (std::ostream& stream, const Status& s) {
    if (s.exceptionCode() == Status::EX_NONE) {
        stream << "No error";
    } else {
        stream << "Status(" << exceptionToString(s.exceptionCode()) << "): '";
        if (s.exceptionCode() == Status::EX_TRANSACTION_FAILED) {
            stream << statusToString(s.transactionError()) << ": ";
        }
        stream << s.exceptionMessage() << "'";
    }
    return stream;
}

static HidlReturnRestriction gReturnRestriction = HidlReturnRestriction::NONE;
void setProcessHidlReturnRestriction(HidlReturnRestriction restriction) {
    gReturnRestriction = restriction;
}

namespace details {
    void return_status::onValueRetrieval() const {
        if (!isOk()) {
            LOG(FATAL) << "Attempted to retrieve value from failed HIDL call: " << description();
        }
    }

    void return_status::onIgnored() const {
        if (gReturnRestriction == HidlReturnRestriction::NONE) {
            return;
        }

        if (gReturnRestriction == HidlReturnRestriction::ERROR_IF_UNCHECKED) {
            LOG(ERROR) << "Failed to check status of HIDL Return.";
            CallStack::logStack("unchecked HIDL return", CallStack::getCurrent(10).get(),
                                ANDROID_LOG_ERROR);
        } else {
            LOG(FATAL) << "Failed to check status of HIDL Return.";
        }
    }

    void return_status::assertOk() const {
        if (!isOk()) {
            LOG(FATAL) << "Failed HIDL return status not checked. Usually this happens because of "
                          "a transport error (error parceling, binder driver, or from unparceling)"
                          ". If you see this in code calling into \"Bn\" classes in for a HAL "
                          "server process, then it is likely that the code there is returning "
                          "transport errors there (as opposed to errors defined within its "
                          "protocol). Error is: " << description();
        }
    }

    return_status::~return_status() {
        // mCheckedStatus must be checked before isOk since isOk modifies mCheckedStatus
        if (mCheckedStatus) return;

        assertOk();
        onIgnored();
    }

    return_status& return_status::operator=(return_status&& other) noexcept {
        if (!mCheckedStatus) {
            assertOk();
            onIgnored();
        }

        std::swap(mStatus, other.mStatus);
        std::swap(mCheckedStatus, other.mCheckedStatus);
        return *this;
    }

}  // namespace details

}  // namespace hardware
}  // namespace android

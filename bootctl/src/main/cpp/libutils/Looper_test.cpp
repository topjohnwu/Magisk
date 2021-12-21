//
// Copyright 2010 The Android Open Source Project
//

#include <gtest/gtest.h>
#include <time.h>
#include <unistd.h>
#include <utils/Looper.h>
#include <utils/StopWatch.h>
#include <utils/Timers.h>
#include "Looper_test_pipe.h"

#include <utils/threads.h>

// b/141212746 - increased for virtual platforms with higher volatility
// # of milliseconds to fudge stopwatch measurements
#define TIMING_TOLERANCE_MS 100

namespace android {

enum {
    MSG_TEST1 = 1,
    MSG_TEST2 = 2,
    MSG_TEST3 = 3,
    MSG_TEST4 = 4,
};

class DelayedTask : public Thread {
    int mDelayMillis;

public:
    explicit DelayedTask(int delayMillis) : mDelayMillis(delayMillis) { }

protected:
    virtual ~DelayedTask() { }

    virtual void doTask() = 0;

    virtual bool threadLoop() {
        usleep(mDelayMillis * 1000);
        doTask();
        return false;
    }
};

class DelayedWake : public DelayedTask {
    sp<Looper> mLooper;

public:
    DelayedWake(int delayMillis, const sp<Looper> looper) :
        DelayedTask(delayMillis), mLooper(looper) {
    }

protected:
    virtual void doTask() {
        mLooper->wake();
    }
};

class DelayedWriteSignal : public DelayedTask {
    Pipe* mPipe;

public:
    DelayedWriteSignal(int delayMillis, Pipe* pipe) :
        DelayedTask(delayMillis), mPipe(pipe) {
    }

protected:
    virtual void doTask() {
        mPipe->writeSignal();
    }
};

class CallbackHandler {
public:
    void setCallback(const sp<Looper>& looper, int fd, int events) {
        looper->addFd(fd, 0, events, staticHandler, this);
    }

protected:
    virtual ~CallbackHandler() { }

    virtual int handler(int fd, int events) = 0;

private:
    static int staticHandler(int fd, int events, void* data) {
        return static_cast<CallbackHandler*>(data)->handler(fd, events);
    }
};

class StubCallbackHandler : public CallbackHandler {
public:
    int nextResult;
    int callbackCount;

    int fd;
    int events;

    explicit StubCallbackHandler(int nextResult) : nextResult(nextResult),
            callbackCount(0), fd(-1), events(-1) {
    }

protected:
    virtual int handler(int fd, int events) {
        callbackCount += 1;
        this->fd = fd;
        this->events = events;
        return nextResult;
    }
};

class StubMessageHandler : public MessageHandler {
public:
    Vector<Message> messages;

    virtual void handleMessage(const Message& message) {
        messages.push(message);
    }
};

class LooperTest : public testing::Test {
protected:
    sp<Looper> mLooper;

    virtual void SetUp() {
        mLooper = new Looper(true);
    }

    virtual void TearDown() {
        mLooper.clear();
    }
};


TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndNotAwoken_WaitsForTimeout) {
    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be LOOPER_POLL_TIMEOUT";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndAwokenBeforeWaiting_ImmediatelyReturns) {
    mLooper->wake();

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because wake() was called before waiting";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because loop was awoken";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndAwokenWhileWaiting_PromptlyReturns) {
    sp<DelayedWake> delayedWake = new DelayedWake(100, mLooper);
    delayedWake->run("LooperTest");

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal wake delay";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because loop was awoken";
}

TEST_F(LooperTest, PollOnce_WhenZeroTimeoutAndNoRegisteredFDs_ImmediatelyReturns) {
    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT";
}

TEST_F(LooperTest, PollOnce_WhenZeroTimeoutAndNoSignalledFDs_ImmediatelyReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT";
    EXPECT_EQ(0, handler.callbackCount)
            << "callback should not have been invoked because FD was not signalled";
}

TEST_F(LooperTest, PollOnce_WhenZeroTimeoutAndSignalledFD_ImmediatelyInvokesCallbackAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    ASSERT_EQ(OK, pipe.writeSignal());
    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked exactly once";
    EXPECT_EQ(pipe.receiveFd, handler.fd)
            << "callback should have received pipe fd as parameter";
    EXPECT_EQ(Looper::EVENT_INPUT, handler.events)
            << "callback should have received Looper::EVENT_INPUT as events";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndNoSignalledFDs_WaitsForTimeoutAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT";
    EXPECT_EQ(0, handler.callbackCount)
            << "callback should not have been invoked because FD was not signalled";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndSignalledFDBeforeWaiting_ImmediatelyInvokesCallbackAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    pipe.writeSignal();
    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked exactly once";
    EXPECT_EQ(pipe.receiveFd, handler.fd)
            << "callback should have received pipe fd as parameter";
    EXPECT_EQ(Looper::EVENT_INPUT, handler.events)
            << "callback should have received Looper::EVENT_INPUT as events";
}

TEST_F(LooperTest, PollOnce_WhenNonZeroTimeoutAndSignalledFDWhileWaiting_PromptlyInvokesCallbackAndReturns) {
    Pipe pipe;
    StubCallbackHandler handler(true);
    sp<DelayedWriteSignal> delayedWriteSignal = new DelayedWriteSignal(100, & pipe);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);
    delayedWriteSignal->run("LooperTest");

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal signal delay";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked exactly once";
    EXPECT_EQ(pipe.receiveFd, handler.fd)
            << "callback should have received pipe fd as parameter";
    EXPECT_EQ(Looper::EVENT_INPUT, handler.events)
            << "callback should have received Looper::EVENT_INPUT as events";
}

TEST_F(LooperTest, PollOnce_WhenCallbackAddedThenRemoved_CallbackShouldNotBeInvoked) {
    Pipe pipe;
    StubCallbackHandler handler(true);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);
    pipe.writeSignal(); // would cause FD to be considered signalled
    mLooper->removeFd(pipe.receiveFd);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal timeout because FD was no longer registered";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT";
    EXPECT_EQ(0, handler.callbackCount)
            << "callback should not be invoked";
}

TEST_F(LooperTest, PollOnce_WhenCallbackReturnsFalse_CallbackShouldNotBeInvokedAgainLater) {
    Pipe pipe;
    StubCallbackHandler handler(false);

    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    // First loop: Callback is registered and FD is signalled.
    pipe.writeSignal();

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal zero because FD was already signalled";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should be invoked";

    // Second loop: Callback is no longer registered and FD is signalled.
    pipe.writeSignal();

    stopWatch.reset();
    result = mLooper->pollOnce(0);
    elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. equal zero because timeout was zero";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT";
    EXPECT_EQ(1, handler.callbackCount)
            << "callback should not be invoked this time";
}

TEST_F(LooperTest, PollOnce_WhenNonCallbackFdIsSignalled_ReturnsIdent) {
    const int expectedIdent = 5;
    void* expectedData = this;

    Pipe pipe;

    pipe.writeSignal();
    mLooper->addFd(pipe.receiveFd, expectedIdent, Looper::EVENT_INPUT, nullptr, expectedData);

    StopWatch stopWatch("pollOnce");
    int fd;
    int events;
    void* data;
    int result = mLooper->pollOnce(100, &fd, &events, &data);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should be approx. zero";
    EXPECT_EQ(expectedIdent, result)
            << "pollOnce result should be the ident of the FD that was signalled";
    EXPECT_EQ(pipe.receiveFd, fd)
            << "pollOnce should have returned the received pipe fd";
    EXPECT_EQ(Looper::EVENT_INPUT, events)
            << "pollOnce should have returned Looper::EVENT_INPUT as events";
    EXPECT_EQ(expectedData, data)
            << "pollOnce should have returned the data";
}

TEST_F(LooperTest, AddFd_WhenCallbackAdded_ReturnsOne) {
    Pipe pipe;
    int result = mLooper->addFd(pipe.receiveFd, 0, Looper::EVENT_INPUT, nullptr, nullptr);

    EXPECT_EQ(1, result)
            << "addFd should return 1 because FD was added";
}

TEST_F(LooperTest, AddFd_WhenIdentIsNegativeAndCallbackIsNull_ReturnsError) {
    Pipe pipe;
    int result = mLooper->addFd(pipe.receiveFd, -1, Looper::EVENT_INPUT, nullptr, nullptr);

    EXPECT_EQ(-1, result)
            << "addFd should return -1 because arguments were invalid";
}

TEST_F(LooperTest, AddFd_WhenNoCallbackAndAllowNonCallbacksIsFalse_ReturnsError) {
    Pipe pipe;
    sp<Looper> looper = new Looper(false /*allowNonCallbacks*/);
    int result = looper->addFd(pipe.receiveFd, 0, 0, nullptr, nullptr);

    EXPECT_EQ(-1, result)
            << "addFd should return -1 because arguments were invalid";
}

TEST_F(LooperTest, RemoveFd_WhenCallbackNotAdded_ReturnsZero) {
    int result = mLooper->removeFd(1);

    EXPECT_EQ(0, result)
            << "removeFd should return 0 because FD not registered";
}

TEST_F(LooperTest, RemoveFd_WhenCallbackAddedThenRemovedTwice_ReturnsOnceFirstTimeAndReturnsZeroSecondTime) {
    Pipe pipe;
    StubCallbackHandler handler(false);
    handler.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);

    // First time.
    int result = mLooper->removeFd(pipe.receiveFd);

    EXPECT_EQ(1, result)
            << "removeFd should return 1 first time because FD was registered";

    // Second time.
    result = mLooper->removeFd(pipe.receiveFd);

    EXPECT_EQ(0, result)
            << "removeFd should return 0 second time because FD was no longer registered";
}

TEST_F(LooperTest, PollOnce_WhenCallbackAddedTwice_OnlySecondCallbackShouldBeInvoked) {
    Pipe pipe;
    StubCallbackHandler handler1(true);
    StubCallbackHandler handler2(true);

    handler1.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT);
    handler2.setCallback(mLooper, pipe.receiveFd, Looper::EVENT_INPUT); // replace it
    pipe.writeSignal(); // would cause FD to be considered signalled

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    ASSERT_EQ(OK, pipe.readSignal())
            << "signal should actually have been written";
    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because FD was already signalled";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because FD was signalled";
    EXPECT_EQ(0, handler1.callbackCount)
            << "original handler callback should not be invoked because it was replaced";
    EXPECT_EQ(1, handler2.callbackCount)
            << "replacement handler callback should be invoked";
}

TEST_F(LooperTest, SendMessage_WhenOneMessageIsEnqueue_ShouldInvokeHandlerDuringNextPoll) {
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessage(handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
}

TEST_F(LooperTest, SendMessage_WhenMultipleMessagesAreEnqueued_ShouldInvokeHandlersInOrderDuringNextPoll) {
    sp<StubMessageHandler> handler1 = new StubMessageHandler();
    sp<StubMessageHandler> handler2 = new StubMessageHandler();
    mLooper->sendMessage(handler1, Message(MSG_TEST1));
    mLooper->sendMessage(handler2, Message(MSG_TEST2));
    mLooper->sendMessage(handler1, Message(MSG_TEST3));
    mLooper->sendMessage(handler1, Message(MSG_TEST4));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(3), handler1->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler1->messages[0].what)
            << "handled message";
    EXPECT_EQ(MSG_TEST3, handler1->messages[1].what)
            << "handled message";
    EXPECT_EQ(MSG_TEST4, handler1->messages[2].what)
            << "handled message";
    EXPECT_EQ(size_t(1), handler2->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST2, handler2->messages[0].what)
            << "handled message";
}

TEST_F(LooperTest, SendMessageDelayed_WhenSentToTheFuture_ShouldInvokeHandlerAfterDelayTime) {
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessageDelayed(ms2ns(100), handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "first poll should end quickly because next message timeout was computed";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be Looper::POLL_WAKE due to wakeup";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no message handled yet";

    result = mLooper->pollOnce(1000);
    elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "second poll should end around the time of the delayed message dispatch";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";

    result = mLooper->pollOnce(100);
    elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(100 + 100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "third poll should timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT because there were no messages left";
}

TEST_F(LooperTest, SendMessageDelayed_WhenSentToThePast_ShouldInvokeHandlerDuringNextPoll) {
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessageDelayed(ms2ns(-1000), handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
}

TEST_F(LooperTest, SendMessageDelayed_WhenSentToThePresent_ShouldInvokeHandlerDuringNextPoll) {
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessageDelayed(0, handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
}

TEST_F(LooperTest, SendMessageAtTime_WhenSentToTheFuture_ShouldInvokeHandlerAfterDelayTime) {
    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessageAtTime(now + ms2ns(100), handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(1000);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "first poll should end quickly because next message timeout was computed";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be Looper::POLL_WAKE due to wakeup";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no message handled yet";

    result = mLooper->pollOnce(1000);
    elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
    EXPECT_NEAR(100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "second poll should end around the time of the delayed message dispatch";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";

    result = mLooper->pollOnce(100);
    elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(100 + 100, elapsedMillis, TIMING_TOLERANCE_MS)
            << "third poll should timeout";
    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT because there were no messages left";
}

TEST_F(LooperTest, SendMessageAtTime_WhenSentToThePast_ShouldInvokeHandlerDuringNextPoll) {
    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessageAtTime(now - ms2ns(1000), handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
}

TEST_F(LooperTest, SendMessageAtTime_WhenSentToThePresent_ShouldInvokeHandlerDuringNextPoll) {
    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessageAtTime(now, handler, Message(MSG_TEST1));

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(100);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was already sent";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because message was sent";
    EXPECT_EQ(size_t(1), handler->messages.size())
            << "handled message";
    EXPECT_EQ(MSG_TEST1, handler->messages[0].what)
            << "handled message";
}

TEST_F(LooperTest, RemoveMessage_WhenRemovingAllMessagesForHandler_ShouldRemoveThoseMessage) {
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessage(handler, Message(MSG_TEST1));
    mLooper->sendMessage(handler, Message(MSG_TEST2));
    mLooper->sendMessage(handler, Message(MSG_TEST3));
    mLooper->removeMessages(handler);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was sent so looper was awoken";
    EXPECT_EQ(Looper::POLL_WAKE, result)
            << "pollOnce result should be Looper::POLL_WAKE because looper was awoken";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no messages to handle";

    result = mLooper->pollOnce(0);

    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT because there was nothing to do";
    EXPECT_EQ(size_t(0), handler->messages.size())
            << "no messages to handle";
}

TEST_F(LooperTest, RemoveMessage_WhenRemovingSomeMessagesForHandler_ShouldRemoveThoseMessage) {
    sp<StubMessageHandler> handler = new StubMessageHandler();
    mLooper->sendMessage(handler, Message(MSG_TEST1));
    mLooper->sendMessage(handler, Message(MSG_TEST2));
    mLooper->sendMessage(handler, Message(MSG_TEST3));
    mLooper->sendMessage(handler, Message(MSG_TEST4));
    mLooper->removeMessages(handler, MSG_TEST3);
    mLooper->removeMessages(handler, MSG_TEST1);

    StopWatch stopWatch("pollOnce");
    int result = mLooper->pollOnce(0);
    int32_t elapsedMillis = ns2ms(stopWatch.elapsedTime());

    EXPECT_NEAR(0, elapsedMillis, TIMING_TOLERANCE_MS)
            << "elapsed time should approx. zero because message was sent so looper was awoken";
    EXPECT_EQ(Looper::POLL_CALLBACK, result)
            << "pollOnce result should be Looper::POLL_CALLBACK because two messages were sent";
    EXPECT_EQ(size_t(2), handler->messages.size())
            << "no messages to handle";
    EXPECT_EQ(MSG_TEST2, handler->messages[0].what)
            << "handled message";
    EXPECT_EQ(MSG_TEST4, handler->messages[1].what)
            << "handled message";

    result = mLooper->pollOnce(0);

    EXPECT_EQ(Looper::POLL_TIMEOUT, result)
            << "pollOnce result should be Looper::POLL_TIMEOUT because there was nothing to do";
    EXPECT_EQ(size_t(2), handler->messages.size())
            << "no more messages to handle";
}

} // namespace android

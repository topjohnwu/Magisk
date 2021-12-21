// FIXME: license file if you have one

package android.fmq.test;

@Backing(type="int")
enum EventFlagBits {
    FMQ_NOT_EMPTY = 1 << 0,
    FMQ_NOT_FULL = 1 << 1,
}

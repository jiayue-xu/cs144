#include "wrapping_integers.hh"

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return isn+n;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
//! 每2^32个绝对索引号划分为一个区域，n对应的绝对索引号一定是以下3个索引号中的一个，找出离checkpoint最近的那个
//!     checkpoint所在区域里n对应的索引号
//!     checkpoint下一个区域里n对应的索引号
//!     checkpoint前一个区域里n对应的索引号
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t ret = (checkpoint & 0xffffffff00000000) + (n.raw_value() - isn.raw_value());
    if (abs(int64_t(ret + (1ul << 32) - checkpoint)) < abs(int64_t(ret - checkpoint)))
        ret += (1ul << 32);
    if (ret >= (1ul << 32) && abs(int64_t(ret - (1ul << 32) - checkpoint)) < abs(int64_t(ret - checkpoint)))
        ret -= (1ul << 32);
    return ret;
}

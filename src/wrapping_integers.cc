#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  const uint64_t target = this->raw_value_ - zero_point.raw_value_ + ( checkpoint & 0xffffffff00000000 );
  const uint64_t diff = target > checkpoint ? target - checkpoint : checkpoint - target;
  if ( diff < 0x0000000080000000 ) {
    return target;
  }
  if ( target > checkpoint && target & 0xffffffff00000000 ) {
    return target - 0x0000000100000000;
  }
  return target < checkpoint ? target + 0x0000000100000000 : target;
}

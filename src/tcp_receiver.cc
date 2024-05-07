#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if ( message.RST ) {
    reader().set_error();
    return;
  }
  if ( message.SYN ) {
    zero_point_.emplace( message.seqno );
  }
  if ( !zero_point_.has_value() ) {
    return;
  }
  uint64_t first_index = message.seqno.unwrap( zero_point_.value(), writer().bytes_pushed() );
  first_index -= message.payload.size() > 0 && !message.SYN;
  first_index -= !message.SYN && message.payload.size() == 0 && message.FIN && writer().bytes_pushed() == 0;

  reassembler_.insert( first_index, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  std::optional<Wrap32> ack
    = zero_point_.has_value()
        ? std::optional<Wrap32> { zero_point_.value() + ( writer().bytes_pushed() + 1 + writer().is_closed() ) }
        : std::optional<Wrap32> {};

  return { ack, window_size(), reader().has_error() };
}

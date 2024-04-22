#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return this->closed_;
}

void Writer::push( string data )
{
  // Your code here.
  uint64_t len = min( data.size(), this->available_capacity() );
  if ( len == 0 || this->closed_ ) {
    return;
  }
  data.resize( len );
  this->total_pushed_ += len;
  this->total_buffered_ += len;
  this->buffer_.emplace( std::move( data ) );
}

void Writer::close()
{
  // Your code here.
  this->closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return this->capacity_ - this->total_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return this->total_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return this->bytes_buffered() == 0 && this->closed_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return this->total_poped_;
}

string_view Reader::peek() const
{
  // Your code here.
  if ( this->bytes_buffered() == 0 ) {
    return string_view( "" );
  }
  return string_view( this->buffer_.front() ).substr( this->removed_ );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  len = min( len, this->bytes_buffered() );
  this->total_poped_ += len;
  this->total_buffered_ -= len;
  while ( len > 0 ) {
    auto& s = this->buffer_.front();
    uint64_t sz = min( len, s.size() - this->removed_ );
    len -= sz;
    this->removed_ += sz;
    if ( this->removed_ == s.size() ) {
      this->buffer_.pop();
      this->removed_ = 0;
    }
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return this->total_buffered_;
}

#include "reassembler.hh"
#include <format>
#include <vector>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{

  auto try_close = [&]() {
    if ( !this->last_byte_.empty() && this->byte_pushed() == this->last_byte() ) {
      this->output_.writer().close();
    }
  };
  // Your code here.
  if ( is_last_substring && last_byte_.empty() ) {
    last_byte_.push_back( first_index + data.size() );
    std::cout << std::format( "last byte {} ", first_index + data.size() ) << std::endl;
  }
  std::cout << std::format( "next: {}, last: {}, cap: {}, end: {}, in_start: {}, data: {}",
                            next_byte(),
                            last_byte(),
                            capacity_,
                            next_byte() + capacity_,
                            first_index,
                            data )
            << std::endl;
  if ( first_index >= this->next_byte() + this->capacity_ ) {
    return try_close();
  } else if ( first_index + data.size() > this->next_byte() + this->capacity_ ) {
    std::cout << std::format( "{} outbound", data ) << std::endl;

    data.resize( data.size() - ( first_index + data.size() - ( this->next_byte() + this->capacity_ ) ) );
  }

  this->data_.emplace_back( std::move( data ) );
  string_view data_view( this->data_.back() );

  if ( first_index < this->byte_pushed() ) {
    uint64_t l = min( data_view.size(), this->byte_pushed() - first_index );
    data_view = data_view.substr( l );
    first_index += l;
  }

  std::vector<slice_view> tmp_slice;
  bool inserted = false;
  for ( auto it = this->buffer_.begin(); data_view.size() > 0 && it != this->buffer_.end(); it++ ) {
    std::cout << std::format( "first: {}, idx: {}", first_index, it->index ) << std::endl;
  loop:
    if ( first_index < it->index ) {
      if ( first_index + data_view.size() <= it->index ) {
        this->pending_ += data_view.size();
        std::cout << std::format( "1 emplace {} @ {} pending: {}", data_view.size(), first_index, pending_ )
                  << std::endl;
        this->buffer_.emplace( it, slice_view { first_index, data_view } );
        data_view.remove_prefix( data_view.size() );
        inserted = true;
      } else {
        string_view sub_view = data_view.substr( 0, it->index - first_index );
        data_view.remove_prefix( sub_view.size() );
        std::cout << std::format( "2 emplace {} @ {} pending: {}", sub_view.size(), first_index, pending_ )
                  << std::endl;
        this->pending_ += sub_view.size();
        auto sub_size = sub_view.size();
        this->buffer_.emplace( it, slice_view { first_index, sub_view } );
        first_index += sub_size;
        inserted = true;
        goto loop;
      }
    } else if ( first_index < it->index + it->view.size() ) {
      if ( first_index + data_view.size() > it->index + it->view.size() ) {
        string_view sub_view = data_view.substr( 0, it->index + it->view.size() - first_index );
        first_index += sub_view.size();
        data_view.remove_prefix( sub_view.size() );
      } else {
        data_view.remove_prefix( data_view.size() );
      }
    }
  }
  if ( data_view.size() > 0 ) {
    this->pending_ += data_view.size();
    std::cout << std::format( "emplace_back {} @ {} pending: {}", data_view.size(), first_index, pending_ )
              << std::endl;
    this->buffer_.emplace_back( slice_view { first_index, data_view } );
    inserted = true;
  }
  if ( !inserted ) {
    return try_close();
  }

  std::cout << std::format( "{} {}", this->buffer_.empty(), this->output_.writer().available_capacity() )
            << std::endl;
  while ( !this->buffer_.empty() && this->output_.writer().available_capacity() > 0 ) {

    auto& front = this->buffer_.front();
    std::cout << std::format( "idx {}, len {}, un: {}", front.index, front.view.size(), this->first_unassemble_ )
              << std::endl;
    if ( front.index == this->byte_pushed() ) {
      uint64_t push_byte = min( front.view.size(), this->writer().available_capacity() );
      string_view sub_view = front.view.substr( 0, push_byte );
      std::cout << std::format( "before pending:{} push {} cap {} push_byte {}, bytes_push {}",
                                pending_,
                                sub_view.size(),
                                this->writer().available_capacity(),
                                push_byte,
                                byte_pushed() )
                << std::endl;
      // this->next_byte() += sub_view.size();
      this->pending_ -= sub_view.size();
      this->first_unassemble_ += sub_view.size();
      front.index += sub_view.size();
      front.view.remove_prefix( sub_view.size() );
      this->output_.writer().push( std::string( sub_view ) );
      std::cout << std::format( "after pending: {}, next: {}, last: {} cap: {}, bytes_push: {}",
                                this->pending_,
                                this->next_byte(),
                                this->last_byte(),
                                this->writer().available_capacity(),
                                this->byte_pushed() )
                << std::endl;
      if ( front.view.empty() ) {
        this->buffer_.pop_front();
      }

    } else {
      break;
    }
  }

  return try_close();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return this->pending_;
}

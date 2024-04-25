#include "reassembler.hh"
#include <format>
#include <vector>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  auto try_close = [&]() {
    if ( !this->last_byte_.empty() && this->first_unassemble_ == this->last_byte() ) {
      this->output_.writer().close();
    }
  };
  if ( is_last_substring && last_byte_.empty() ) {
    last_byte_.push_back( first_index + data.size() );
  }
  if ( first_index >= this->next_byte() + this->capacity_ ) {
    return try_close();
  } else if ( first_index + data.size() > this->next_byte() + this->capacity_ ) {
    data.resize( data.size() - ( first_index + data.size() - ( this->next_byte() + this->capacity_ ) ) );
  }

  string_view data_view( data );

  if ( first_index < this->first_unassemble_ ) {
    uint64_t l = min( data_view.size(), this->first_unassemble_ - first_index );
    data_view = data_view.substr( l );
    first_index += l;
  }

  bool inserted = false;
  for ( auto it = this->buffer_.begin(); data_view.size() > 0 && it != this->buffer_.end(); it++ ) {
  loop:
    if ( first_index < it->index ) {
      if ( first_index + data_view.size() <= it->index ) {
        this->pending_ += data_view.size();
        this->buffer_.emplace( it, slice_view { first_index, {}, std::string( data_view ) } );
        data_view.remove_prefix( data_view.size() );
        inserted = true;
      } else {
        string_view sub_view = data_view.substr( 0, it->index - first_index );
        data_view.remove_prefix( sub_view.size() );
        this->pending_ += sub_view.size();
        auto sub_size = sub_view.size();
        this->buffer_.emplace( it, slice_view { first_index, {}, std::string( sub_view ) } );
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
    this->buffer_.emplace_back( slice_view { first_index, {}, std::string( data_view ) } );
    inserted = true;
  }
  if ( !inserted ) {
    return try_close();
  }

  for ( auto& item : this->buffer_ ) {
    item.view = string_view( item.data );
  }

  while ( !this->buffer_.empty() && this->output_.writer().available_capacity() > 0 ) {
    auto& front = this->buffer_.front();
    if ( front.index == this->first_unassemble_ ) {
      uint64_t push_byte = min( front.view.size(), this->writer().available_capacity() );
      string_view sub_view = front.view.substr( 0, push_byte );
      this->pending_ -= sub_view.size();
      this->first_unassemble_ += sub_view.size();
      front.index += sub_view.size();
      front.view.remove_prefix( sub_view.size() );
      this->output_.writer().push( std::string( sub_view ) );
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

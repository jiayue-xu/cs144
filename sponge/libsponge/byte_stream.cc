#include "byte_stream.hh"
using namespace std;

ByteStream::ByteStream(const size_t capacity):_capacity(capacity) { }

size_t ByteStream::write(const string &data) {
    size_t written_size = 0;
  
    for(size_t i = 0; i < data.size(); i++) {
        if(remaining_capacity()) {
            _buffer.push_back(data[i]);
            written_size++;
        } else {
            break;
        }
    } 
    _written_bytes += written_size;
    return written_size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string str = "";
    size_t read_size = len>buffer_size()? buffer_size() : len;
    for(size_t i = 0; i < read_size; i++){
        str += _buffer[i];
    }
    return str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    for(size_t i = 0; i < len && buffer_size(); i++){
        _buffer.pop_front();
        _read_bytes += 1;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    if(eof()) {
        return "";
    } else {
        string read_str = peek_output(len);
        pop_output(len);
        return read_str;
    }
}

void ByteStream::end_input() { _endinput= true; }

bool ByteStream::input_ended() const { return _endinput; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.empty(); }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _written_bytes; }

size_t ByteStream::bytes_read() const { return _read_bytes; }

size_t ByteStream::remaining_capacity() const { return _capacity-buffer_size(); }

#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if(seg.header().syn){
        if(!_get_syn){
            _get_syn = true;
            _isn = seg.header().seqno;
            _reassembler.push_substring(seg.payload().copy(), 0, seg.header().fin);
            return;
        }
    } else if(!_get_syn){
        return;
    }
    if(_get_syn && stream_out().bytes_written()==0 && unwrap(seg.header().seqno, _isn, 0)==1)
        _reassembler.push_substring(seg.payload().copy(), 0, seg.header().fin);
    else 
        _reassembler.push_substring(seg.payload().copy(), unwrap(seg.header().seqno, _isn, stream_out().bytes_written()-1)-1, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(_get_syn){
        if(stream_out().input_ended()) 
            return wrap(stream_out().bytes_written()+2, WrappingInt32(_isn));
        else return wrap(stream_out().bytes_written()+1, WrappingInt32(_isn));
    } else return nullopt;
}

size_t TCPReceiver::window_size() const { 
    return stream_out().remaining_capacity();
 }

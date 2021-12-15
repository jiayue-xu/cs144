#include "stream_reassembler.hh"

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
    for(size_t i = 0; i < capacity; i++){
        _unassemble_buffer.push_back('0');
        _unassemble_used.push_back(false);
    }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t c = _capacity, w = _output.bytes_written(), r = _output.bytes_read();
    size_t ss = index, se = dindex2sindex(data.length()-1, index);
    size_t ds = 0, de = data.length()-1;

    // _output可能被读取了一些数据，更新暂存区
    for(size_t i = _last_read_index; i < r; i++){
        _unassemble_buffer.pop_front();
        _unassemble_buffer.push_back('0');
        _unassemble_used.pop_front();
        _unassemble_used.push_back(false);
    }
    _last_read_index=r;

    // 已接收所有数据，单独收到eof信号
    if(eof && !data.length() && !_unassemble_bytes){
        _geteof = true;
        _output.end_input();
        return;
    }
    
    // 收到空串，忽略数据
    if(!eof && !data.length()) return;

    // 已结束输入，或子串不在接收区域内
    if(_output.input_ended() || ss>c+r-1 || se<w) return;

    // 截取子串在可接收区域内的范围
    ds = ss<w?sindex2dindex(data.length(), w, index):ds;
    de = se>c+r-1?sindex2dindex(data.length(), c+r-1, index):de;
    ss = ss<w?w:ss;
    se = se>c+r-1?c+r-1:se;

    // 合并子串和暂存区
    for(size_t i = ds; i <= de; i++){
        size_t d2s = dindex2sindex(i, index);
        size_t s2u = sindex2uindex(d2s);
        if(!_unassemble_used[s2u]){
            _unassemble_buffer[s2u] = data[i];
            _unassemble_bytes++;
            _unassemble_used[s2u] = true;
        }
    }

    // 可以直接写入_output
    if(ss==w){
        string str = "";
        for(size_t i = sindex2uindex(w); i < _capacity; i++){
            if(!_unassemble_used[i]) break;
            _unassemble_used[i] = false;
            _unassemble_bytes--;
            str += _unassemble_buffer[i];
            _unassemble_buffer[i]='0';
        }
        _output.write(str);
        if((eof && de==data.length()-1) || (_geteof && !_unassemble_bytes))
            _output.end_input();
    } 
    else if(eof && de==data.length()-1){ // 收到eof
            _geteof = true;
    }
}

// 返回暂存区的字节数
size_t StreamReassembler::unassembled_bytes() const { return _unassemble_bytes; }

// 暂存区为空返回true
bool StreamReassembler::empty() const { return unassembled_bytes()==0; }

// 字符串索引号对应的字节流索引号
size_t StreamReassembler::dindex2sindex(size_t data_index, size_t first_index)const{
    return first_index + data_index;
}

// 字节流索引号对应的字符串索引号
size_t StreamReassembler::sindex2dindex(size_t data_size, size_t stream_index, size_t first_index)const{
    return stream_index>=first_index && stream_index-first_index < data_size ? stream_index-first_index : -1; 
}

// 字节流索引号对应的暂存区索引号
size_t StreamReassembler::sindex2uindex(size_t stream_index)const{
    return stream_index - _output.bytes_read();
}

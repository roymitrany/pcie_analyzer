//
// Created by irinag on 2/3/2020.
//

#ifndef TLP_PARSER_LOGICCHUNK_H
#define TLP_PARSER_LOGICCHUNK_H

/*****************************************************************************/
/**                              Includes                                   **/
/*****************************************************************************/
#include "VovitChunk.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>


/*****************************************************************************/
/**                               Class                                     **/
/*****************************************************************************/
namespace pcie {

    /**
     * Represents Collection of port_width number of physical Vovit Chunks.
     * Has a build in iterator that iterates through data in its logic order.
     */
    class LogicChunk {

    private:
        uint8_t port_speed;
        uint8_t port_width;
        uint8_t* data_buffer;
        uint8_t* logic_data_buffer;
        uint32_t base_partition_idx;
        uint32_t sync_count;
        size_t data_block_size;   // size in Bytes

        void toLogicBuffer();


    public:

        LogicChunk(uint32_t base_byte_idx, std::string file);

        ~LogicChunk();

        uint32_t getSyncCount();

        uint8_t getPortSpead(){return port_speed;}

        size_t getDataBlockSize();

        /**
         * For debug only.
         */
        void printAll();


        class iterator {
        public:
            typedef iterator self_type;
            typedef uint8_t value_type;
            typedef value_type& reference;
            typedef value_type* pointer;
            iterator(pointer ptr) : ptr_(ptr), idx(0) { }

            iterator() : ptr_(nullptr), idx(0){}

            self_type operator++() { self_type i = *this; idx++; ptr_++; return i; }
            self_type operator++(int junk) { idx++; ptr_++; return *this; }
            pointer operator*() { return ptr_; }
            pointer operator->() { return ptr_; }
            bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
            bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
            uint32_t getIdx(){ return idx;}
        private:
            pointer ptr_;
            uint32_t idx;
        };

        class const_iterator {
        public:
            typedef const_iterator self_type;
            typedef uint8_t value_type;
            typedef value_type& reference;
            typedef value_type* pointer;
            const_iterator(pointer ptr) : ptr_(ptr), idx(0) { }
            self_type operator++() { self_type i = *this; idx++; ptr_++; return i; }
            self_type operator++(int junk) { idx++; ptr_++; return *this; }
            const pointer operator*() { return ptr_; }
            const pointer operator->() { return ptr_; }
            bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
            bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
            uint32_t getIdx(){ return idx;}
        private:
            pointer ptr_;
            uint32_t idx;
        };

        iterator it;

        size_t size() const;
        uint8_t& operator[](size_t index) ;
        const uint8_t& operator[](size_t index) const;
        iterator begin() ;
        iterator end();
        const_iterator begin() const ;
        const_iterator end() const ;
        uint32_t xBytesAheadIt(){ return static_cast<uint32_t >(data_block_size) - it.getIdx();}

    };


}  // namespace pcie


#endif //TLP_PARSER_LOGICCHUNK_H

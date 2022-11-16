#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
//#include <iostream>

namespace sjtu {
    template<typename T>
    class vector;

    template<typename T>
    class vector {

    private:
        int max_size;
        int current_size;
        T *data;

        //线性表中要为插入操作留一定的余量，可以扩充
        void DoubleSpace() {
            T *tmp = data;
            max_size *= 2;
            data = (T *) malloc(sizeof(T) * max_size);
            for (int i = 0; i < max_size / 2; ++i)new(data + i)T(tmp[i]);
            ////std::cout<< "free the ptr" << (unsigned long) tmp << std::endl;
            for (int i = 0; i < max_size / 2; ++i)(tmp[i]).~T();
            free(tmp);
            tmp = nullptr;
        }

    public:
        class iterator {
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            T *ptr;
            const vector<T> *enclose_this;
        public:
            iterator(T *p = nullptr, const vector<T> *enclose = nullptr) : ptr(p), enclose_this(enclose) {};

            ~iterator() {};

            iterator operator+(const int &n) const {
                if (ptr + n > enclose_this->data + enclose_this->current_size || ptr + n < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    iterator tmp = *this;
                    tmp.ptr += n;
                    return tmp;
                }
            }

            iterator operator-(const int &n) const {
                if (ptr - n < enclose_this->data || ptr - n > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    iterator tmp = *this;
                    tmp.ptr -= n;
                    return tmp;
                }
            }

            int operator-(const iterator &rhs) const {
                if (rhs > enclose_this->data + enclose_this->current_size || rhs < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else return (int) (rhs - ptr);
            }

            iterator &operator+=(const int &n) {
                if (ptr + n > enclose_this->data + enclose_this->current_size || ptr + n < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr += n;
                    return *this;
                }
            }

            iterator &operator-=(const int &n) {
                if (ptr - n < enclose_this->data || ptr - n > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr -= n;
                    return *this;
                }
            }

            iterator &operator++() {
                if (ptr + 1 > enclose_this->data + enclose_this->current_size || ptr + 1 < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr += 1;
                    return *this;
                }
            }

            iterator operator++(int) {
                if (ptr + 1 > enclose_this->data + enclose_this->current_size || ptr + 1 < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    iterator tmp = *this;
                    ptr += 1;
                    return tmp;
                }
            }

            iterator &operator--() {
                if (ptr - 1 < enclose_this->data || ptr - 1 > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr -= 1;
                    return *this;
                }
            }

            iterator operator--(int) {
                if (ptr - 1 < enclose_this->data || ptr - 1 > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    iterator tmp = *this;
                    ptr -= 1;
                    return tmp;
                }
            }

            //用引用是因为可能解引用之后要给他赋个新的值，const是因为不会改变本身的迭代器
            T &operator*() const {
                return *ptr;
            }

            bool operator==(const iterator &rhs) {
                if (rhs > enclose_this->data + enclose_this->current_size || rhs < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                }
                return (ptr == rhs.ptr);
            }

            bool operator!=(const iterator &rhs) {
                if (rhs.ptr > enclose_this->data + enclose_this->current_size || rhs.ptr < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                }
                return (ptr != rhs.ptr);
            }

            friend class vector<T>;
        };

        class const_iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            T *ptr;
            const vector<T> *enclose_this;
        public:
            const_iterator(T *p = nullptr, const vector<T> *enclose = nullptr) : ptr(p), enclose_this(enclose) {};

            ~const_iterator() {};

            const_iterator operator+(const int &n) const {
                if (ptr + n > enclose_this->data + enclose_this->current_size || ptr + n < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    const_iterator tmp = *this;
                    tmp.ptr += n;
                    return tmp;
                }
            }

            const_iterator operator-(const int &n) const {
                if (ptr - n < enclose_this->data || ptr - n > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    const_iterator tmp = *this;
                    tmp.ptr += n;
                    return tmp;
                }
            }

            const_iterator &operator+=(const int &n) {
                if (ptr + n > enclose_this->data + enclose_this->current_size || ptr + n < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr += n;
                    return *this;
                }
            }

            const_iterator &operator-=(const int &n) {
                if (ptr - n < enclose_this->data || ptr - n > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr -= n;
                    return *this;
                }
            }

            const_iterator &operator++() {
                if (ptr + 1 > enclose_this->data + enclose_this->current_size || ptr + 1 < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr += 1;
                    return *this;
                }
            }

            const_iterator operator++(int) {
                if (ptr + 1 > enclose_this->data + enclose_this->current_size || ptr + 1 < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else {
                    const_iterator tmp = *this;
                    ptr += 1;
                    return tmp;
                }
            }

            const_iterator &operator--() {
                if (ptr - 1 < enclose_this->data || ptr - 1 > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    ptr -= 1;
                    return *this;
                }
            }

            const_iterator operator--(int) {
                if (ptr - 1 < enclose_this->data || ptr - 1 > enclose_this->data + enclose_this->current_size) {
                    invalid_iterator e;
                    throw e;
                } else {
                    const_iterator tmp = *this;
                    ptr -= 1;
                    return tmp;
                }
            }


            int operator-(const const_iterator &rhs) const {
                if (rhs.ptr > enclose_this->data + enclose_this->current_size || rhs.ptr < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                } else return (int) (rhs.ptr - ptr);
            }

            T &operator*() const { return *ptr; }

            //用引用是因为可能解引用之后要给他赋个新的值，const是因为不会改变本身的迭代器
            bool operator==(const const_iterator &rhs) {
                if (rhs.ptr > enclose_this->data + enclose_this->current_size || rhs.ptr < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                }
                return (ptr == rhs.ptr);
            }

            bool operator!=(const const_iterator &rhs) {
                if (rhs.ptr > enclose_this->data + enclose_this->current_size || rhs.ptr < enclose_this->data) {
                    invalid_iterator e;
                    throw e;
                }
                return (ptr != rhs.ptr);
            }

            friend class vector<T>;
        };

        //设置max_size的默认实际值
        vector(int m = 5) {
            max_size = m;
            current_size = 0;
            //std::cout<<"construct malloc"<<std::endl;
            data = (T *) malloc(sizeof(T) * max_size);
            //std::cout<<"constructed"<<std::endl;
//            for (int i = 0; i < current_size; ++i)new(data + i) T();
//           ////std::cout<< "construction success" << current_size << std::endl;
//           ////std::cout<< "constructor data: " << (unsigned long) data << std::endl;//
        }

        vector(const vector &other) {
            max_size = other.max_size;
            current_size = other.current_size;
            data = (T *) malloc(sizeof(T) * max_size);
            for (int i = 0; i < current_size; ++i)new(data + i) T(other[i]);
        }

        ~vector() {
            ////std::cout<< "Performed by the Vector" << std::endl;//
            if (data) {
                //std::cout<<"destruct ~T()"<<std::endl;
                for (int i = 0; i < current_size; ++i) data[i].~T();
                //std::cout<< "destruct free the ptr" << std::endl;
                free(data);
                data = nullptr;
            }
        }

        //记得判this==&other
        vector &operator=(const vector &other) {
            if (this == &other)return *this;
            if (data) for (int i = 0; i < max_size; ++i)data[i].~T();
            max_size = other.max_size;
            current_size = other.current_size;
            if (data) {
                ////std::cout<< "free the ptr" << (unsigned long) data << std::endl;
                free(data);
                data = nullptr;
            }
            data = (T *) malloc(sizeof(T) * max_size);
            for (int i = 0; i < current_size; ++i)new(data + i) T(other[i]);
            return *this;
        }

        T &at(const size_t &pos) {
            if (pos >= current_size || pos < 0) {
                index_out_of_bound e;
                throw e;
            }
            return data[pos];
        }

        const T &at(const size_t &pos) const {
            if (pos >= current_size || pos < 0) {
                index_out_of_bound e;
                throw e;
            }
            return data[pos];
        }

        T &operator[](const size_t &pos) {
            if (pos >= current_size || pos < 0) {
                index_out_of_bound e;
                throw e;
            }
            return data[pos];
        }

        const T &operator[](const size_t &pos) const {
            if (pos >= current_size || pos < 0) {
                index_out_of_bound e;
                throw e;
            }
            return data[pos];
        }

        const T &front() const {
            if (current_size == 0) {
                container_is_empty e;
                throw e;
            }
            return data[0];
        }

        const T &back() const {
            if (current_size == 0) {
                container_is_empty e;
                throw e;
            }
            return data[current_size - 1];
        }

        iterator begin() const {
            iterator beg(data, this);
            return beg;
        }

        const_iterator cbegin() const {
            const_iterator beg(data, this);
            return beg;
        }

        iterator end() const {
            iterator end(data + current_size, this);
            return end;
        }

        const_iterator cend() const {
            const_iterator end(data + current_size, this);
            return end;
        }

        bool empty() const { return (current_size == 0); }

        size_t size() const { return current_size; }

        void clear() { current_size = 0; }

        iterator insert(iterator pos, const T &value) {
            if (pos.ptr > vector<T>::data + vector<T>::current_size || pos.ptr < vector<T>::data) {
                invalid_iterator e;
                throw e;
            }
            if (current_size == max_size)DoubleSpace();
            int index = pos.ptr - data;
            for (int i = current_size - 1; i >= index; --i)data[i + 1] = data[i];
            new(data + index) T(value);
            ++current_size;
            iterator tmp(data + index, this);
            return tmp;
        }

        iterator insert(const size_t &ind, const T &value) {
            if (ind >= current_size || ind < 0) {
                index_out_of_bound e;
                throw e;
            }
            if (current_size == max_size)DoubleSpace();
            int index = ind;
            for (int i = current_size - 1; i >= index; --i)data[i + 1] = data[i];
            new(data + index) T(value);
            ++current_size;
            iterator tmp(data + index, this);
            return tmp;
        }

        iterator erase(iterator pos) {
            if (pos.ptr > vector<T>::data + vector<T>::current_size || pos.ptr < vector<T>::data) {
                invalid_iterator e;
                throw e;
            }
            int index = pos.ptr - data;
            data[index].~T();
            for (int i = index; i < current_size - 1; ++i)data[i] = data[i + 1];
            current_size--;
            iterator tmp(data + index, this);
            return tmp;
        }

        iterator erase(const size_t &ind) {
            if (ind >= current_size || ind < 0) {
                index_out_of_bound e;
                throw e;
            }
            data[ind].~T();
            int index = ind;
            for (int i = index; i < current_size - 1; ++i)data[i] = data[i + 1];
            current_size--;
            iterator tmp(data + index, this);
            return tmp;
        }

        void push_back(const T &value) {
//           //std::cout<< "ready to push" << std::endl;
//            //std::cout.flush();
            if (current_size == max_size) {
//               ////std::cout<< "to double space" << std::endl;
//                //std::cout.flush();
                DoubleSpace();
            }
            //std::cout<< "push new T" << std::endl;
//            //std::cout.flush();
//            data[current_size]=new T;
            new(data + current_size)  T(value);
//           ////std::cout<< "assign value finished" << std::endl;
//            //std::cout.flush();
//           ////std::cout<< "after push data: " << (unsigned long) data << std::endl;//
            current_size++;
//           ////std::cout<< "push success" << current_size << std::endl;
//            //std::cout.flush();
        }

        void pop_back() {
            if (current_size == 0) {
                container_is_empty e;
                throw e;
            }
            data[current_size - 1].~T();
            current_size--;
        }

    };
}
#endif //STLITE_VECTOR_HPP

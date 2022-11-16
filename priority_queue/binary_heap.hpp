//二项堆
#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

/**
 * a container like std::priority_queue which is a heap internal.
 */
    template<typename T, class Compare = std::less<T>>
    class priority_queue {
    private:
        T *data;
        int current_size;
        int max_size;
        Compare cmp;

        void DoubleSpace() {
            max_size *= 2;
            T *tmp = data;
            data = (T *) malloc(sizeof(T) * max_size);
            for (int i = 1; i <= current_size; ++i)new(data + i)T(tmp[i]);
            for (int i = 1; i <= current_size; ++i)tmp[i].~T();
            free((void *) tmp);
        }

        void percolate(int hole) {//find downwards for a suitable place for the element in the hole
            while (hole <= current_size) {
                int child = hole * 2;//the hole's left child,
                // then we need to check if its the right child exists and find the smaller one to exchange
//                T test = data[child + 1], test_ = data[child];//to do
                if (child > current_size)break;
                if (child + 1 <= current_size && cmp.operator()(data[child], data[child + 1]))
                    child = child + 1;//find the bigger child
                if (cmp.operator()(data[hole], data[child])) {
                    T tmp = data[hole];
                    data[hole] = data[child];
                    data[child] = tmp;
                    hole = child;
                } else break;
            }
        }

    public:
        void traverse() {
            std::cout << "Traverse: " << std::endl;
            for (int i = 1; i <= current_size; ++i)std::cout << data[i] << " ";
            std::cout << std::endl;
            std::cout << "Traverse ends" << std::endl;
            std::cout.flush();
        }

        priority_queue(int max = 10, int curr = 0) {
            current_size = curr;
            max_size = max;
            data = (T *) malloc(sizeof(T) * max_size);
        }

        priority_queue(const priority_queue &other) {
            current_size = other.current_size;
            max_size = other.max_size;
            data = (T *) malloc(sizeof(T) * max_size);
            for (int i = 1; i <= current_size; ++i)new(data + i)T(other.data[i]);
        }

        ~priority_queue() {
            for (int i = 1; i <= current_size; ++i)data[i].~T();
            free((void *) data);
        }

        priority_queue &operator=(const priority_queue &other) {
            if (this == &other)return *this;
            for (int i = 0; i < current_size; ++i)data[i].~T();
            free((void *) data);
            max_size = other.max_size;
            current_size = other.current_size;
            data = (T *) malloc(sizeof(T) * max_size);
            for (int i = 1; i <= current_size; ++i)new(data + i)T(other.data[i]);
            return *this;
        }

        /**
         * get the top of the queue.
         * @return a reference of the top element.
         * throw container_is_empty if empty() returns true;
         */
        const T &top() const {
            if (current_size == 0) {
                container_is_empty e;
                throw e;
            }
            return data[1];
        }

        /**
         * push new element to the priority queue.
         */
        void push(const T &e) {
            if (current_size == 0) {
                ++current_size;
                new(data + current_size)T(e);
                return;
            }
            if (current_size + 1 == max_size)DoubleSpace();
            ++current_size;
            new(data + current_size)T(e);//add a node for the pushing,
            // the new element will be put into this one, now up and find the correct place
            for (int i = current_size; i > 0;)//find its parent and judge if the place is correct
            {
                int parent = i / 2;
                if (parent > 0 && cmp.operator()(data[parent], e)) {
                    data[i] = data[parent];
                    i = parent;
                } else {
                    data[i] = e;
                    break;
                }
            }
        }

        /*
         * delete the top element.
         * throw container_is_empty if empty() returns true;
         */
        void pop() {
            if (current_size == 0) {
                container_is_empty e;
                throw e;
            }
            data[1].~T();
            new(data + 1)T(data[current_size]);//put the last one into the root node
            data[current_size].~T();
            --current_size;
            percolate(1);
        }

        /**
         * return the number of the elements.
         */
        size_t size() const {
            return current_size;
        }

        /**
         * check if the container has at least an element.
         * @return true if it is empty, false if it has at least an element.
         */
        bool empty() const {
            return (current_size == 0);
        }

        /**
         * return a merged priority_queue with at least O(logn) complexity.
         */
        void merge(priority_queue &other) {
            if (this == &other) {
                DoubleSpace();
                current_size *= 2;
                for (int i = current_size / 2 + 1; i <= current_size; ++i)data[i] = data[i - current_size / 2];
                for (int i = current_size / 2; i > 0; --i)percolate(i);
            } else {
                T *tmp = data;
                data = (T *) malloc(sizeof(T) * (max_size + other.max_size));
                int i;
                for (i = 1; i <= current_size; ++i)new(data + i)T(tmp[i]);
                for (int j = 1; j <= other.current_size; ++j, ++i)new(data + i)T(other.data[j]);
                for (int i = 1; i <= current_size; ++i)tmp[i].~T();
                free((void *) tmp);
                current_size += other.current_size;
                max_size += other.max_size;
                for (int i = 1; i <= current_size; ++i)other.data[i].~T();
                other.current_size = 0;
                for (int i = current_size / 2; i > 0; --i)percolate(i);
            }
        }
    };

}

#endif

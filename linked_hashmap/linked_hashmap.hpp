/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

    template<
            class Key,
            class T,
            class Hash = std::hash<Key>,
            class Equal = std::equal_to<Key>
    >
    class linked_hashmap {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::linked_hashmap as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;

        struct node {
            value_type *value;
            node *before = nullptr;
            node *after = nullptr;
            node *next = nullptr;

            node(value_type v) {
                value = (value_type *) malloc(sizeof(value_type));
                new(value)value_type(v);
            }

            node(const node &other) {
                value = (value_type *) malloc(sizeof(value));
                new(value)value_type(*(other.value));
                next = other.next;
                after = other.after;
                before = other.before;
            }

            node() {
                value = nullptr;
            }

            ~node() {
                if (value != nullptr) {
                    value->second.~T();
                    value->first.~Key();
                    free(value);
                }
            }
        };


        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = linked_hashmap.begin(); --it;
         *       or it = linked_hashmap.end(); ++end();
         */
        class const_iterator;

        class iterator {
        public:
            linked_hashmap<Key, T, Hash, Equal> *me;
            node *ptr;
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the following
            // typedef) to work properly.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
            using difference_type = std::ptrdiff_t;
            using value_type = typename linked_hashmap::value_type;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::output_iterator_tag;


            iterator(node *p = nullptr, linked_hashmap<Key, T, Hash, Equal> *m = nullptr) {
                ptr = p;
                me = m;
            }

            iterator(const iterator &other) {
                ptr = other.ptr;
                me = other.me;
            }

            iterator operator++(int) {
                if (ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                iterator tmp = *this;
                ptr = ptr->after;
                return tmp;
            }

            iterator &operator++() {
                if (ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                ptr = ptr->after;
                return *this;
            }

            iterator operator--(int) {
                if (ptr == me->head->after) {
                    runtime_error e;
                    throw e;
                }
                iterator tmp = *this;
                if (ptr)ptr = ptr->before;
                else ptr = me->rear;
                return tmp;
            }

            iterator &operator--() {
                if (ptr == me->head->after) {
                    runtime_error e;
                    throw e;
                }
                if (ptr)ptr = ptr->before;
                else ptr = me->rear;
                return *this;
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type &operator*() const {
                if (ptr == me->head || ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                return *(ptr->value);
            }

            bool operator==(const iterator &rhs) const {
                if (me != rhs.me)return false;
                if (ptr != rhs.ptr)return false;
                else return true;
            }

            bool operator==(const const_iterator &rhs) const {
                if (me != rhs.me)return false;
                if (ptr != rhs.ptr)return false;
                else return true;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                if (me != rhs.me)return true;
                if (ptr != rhs.ptr)return true;
                else return false;
            }

            bool operator!=(const const_iterator &rhs) const {
                if (me != rhs.me)return true;
                if (ptr != rhs.ptr)return true;
                else return false;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const noexcept {
                if (ptr == me->head || ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                return ptr->value;
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        public:
            const linked_hashmap<Key, T, Hash, Equal> *me;
            node *ptr = nullptr;
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the following
            // typedef) to work properly.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
            using difference_type = std::ptrdiff_t;
            using value_type = typename linked_hashmap::value_type;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::output_iterator_tag;


            const_iterator(node *p = nullptr, const linked_hashmap<Key, T, Hash, Equal> *m = nullptr) {
                ptr = p;
                me = m;
            }

            const_iterator(const const_iterator &other) {
                ptr = other.ptr;
                me = other.me;
            }

            const_iterator(const iterator &other) {
                ptr = other.ptr;
                me = other.me;
            }


            const_iterator operator++(int) {
                if (ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                const_iterator tmp = *this;
                ptr = ptr->after;
                return tmp;
            }

            const_iterator &operator++() {
                if (ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                ptr = ptr->after;
                return *this;
            }

            const_iterator operator--(int) {
                if (ptr == me->head->after) {
                    runtime_error e;
                    throw e;
                }
                const_iterator tmp = *this;
                if (ptr)ptr = ptr->before;
                else ptr = me->rear;
                return tmp;
            }

            const_iterator &operator--() {
                if (ptr == me->head->after) {
                    runtime_error e;
                    throw e;
                }
                if (ptr)ptr = ptr->before;
                else ptr = me->rear;
                return *this;
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type &operator*() const {
                if (ptr == me->head || ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                return *(ptr->value);
            }

            bool operator==(const iterator &rhs) const {
                if (me != rhs.me)return false;
                if (ptr != rhs.ptr)return false;
                else return true;
            }

            bool operator==(const const_iterator &rhs) const {
                if (me != rhs.me)return false;
                if (ptr != rhs.ptr)return false;
                else return true;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                if (me != rhs.me)return true;
                if (ptr != rhs.ptr)return true;
                else return false;
            }

            bool operator!=(const const_iterator &rhs) const {
                if (me != rhs.me)return true;
                if (ptr != rhs.ptr)return true;
                else return false;
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() {
                if (ptr == me->head || ptr == nullptr) {
                    runtime_error e;
                    throw e;
                }
                return ptr->value;
            }
        };

    private:
        Hash hash;
        Equal equal;
        unsigned int scale = 0;
        node *head;
        node *rear;
    public:
        node **data;//new 出来的东西就是头节点
        unsigned int array_size = 41519;

    private:
        pair<node *, bool> insert_(value_type v) {
            unsigned int index = hash.operator()(v.first) % array_size;
            node *ptr = data[index];
            while (ptr->next != nullptr) {
                if (equal.operator()(ptr->next->value->first, v.first))break;
                ptr = ptr->next;
            }
            //现在ptr->next是相等的,或者说是ptr->next=nullptr
            if (ptr->next)return {ptr->next, false};
            else {
                ptr->next = new node(v);
                ptr = ptr->next;
            }
            //现在ptr就是刚刚新加入的元素
            rear->after = ptr;
            ptr->before = rear;
            rear = ptr;
            return {ptr, true};
        }

        void remove_(const Key &key) {
            unsigned int index = hash.operator()(key) % array_size;
            node *ptr = data[index];
            while (ptr->next != nullptr) {
                if (equal.operator()(ptr->next->value->first, key))break;//找到了要删除的元素
                else ptr = ptr->next;
            }
            if (ptr->next) {
                //ptr->next就是要删除的那个元素。
                node *del = ptr->next;
                ptr->next = ptr->next->next;
                if (del == rear)rear = del->before;
                if (del->after) del->after->before = del->before;
                if (del->before)del->before->after = del->after;
                delete del;
            } else {
                index_out_of_bound e;
                throw e;
            }
        }

        node *find_(const Key &key) const {
            unsigned int index = hash.operator()(key) % array_size;
            node *ptr = data[index]->next;
            while (ptr != nullptr) {
                if (equal.operator()(ptr->value->first, key))return ptr;
                else ptr = ptr->next;
            }
            return nullptr;
        }

        void insert_data(node *now) const noexcept {
            unsigned int index = hash.operator()(now->value->first) % array_size;
            node *p = data[index];
            while (p->next != nullptr) {
                if (equal.operator()(p->next->value->first, now->value->first))break;
                p = p->next;
            }
            p->next = now;
        }

    public:

        linked_hashmap() {
            data = new node *[array_size];
            for (int i = 0; i < array_size; ++i)data[i] = new node;
            head = new node;
            rear = head;
            //现在他们作为头节点，所有的东西都是空的
        }

        linked_hashmap(const linked_hashmap &other) {
            scale = other.scale;
            data = new node *[array_size];
            for (int i = 0; i < array_size; ++i)data[i] = new node;
            node *ptr = other.head->after;
            head = new node;
            rear = head;
            node *now;
            rear = head;
            while (ptr != nullptr) {
                now = new node(*(ptr->value));
                rear->after = now;
                now->before = rear;
                insert_data(now);
                rear = now;
                ptr = ptr->after;
            }
        }

        linked_hashmap &operator=(const linked_hashmap &other) {
            if (&other == this)return *this;
            clear();
            scale = other.scale;
            node *ptr = other.head->after;
            node *now;
            rear = head;
            while (ptr != nullptr) {
                now = new node(*(ptr->value));
                rear->after = now;
                now->before = rear;
                insert_data(now);
                rear = now;
                ptr = ptr->after;
            }
            return *this;
        }

        ~linked_hashmap() {
            clear();//clear之后数组还在，只是数组全部都清空了
            for (int i = 0; i < array_size; ++i)delete data[i];
            delete[]data;
            delete head;
        }

        T &at(const Key &key) {
            unsigned int index = hash.operator()(key) % array_size;
            node *ptr = data[index]->next;
            while (ptr != nullptr) {
                if (equal.operator()(ptr->value->first, key))break;
                else ptr = ptr->next;
            }
            if (ptr != nullptr)return ptr->value->second;
            else {
                index_out_of_bound e;
                throw e;
            }
        }

        const T &at(const Key &key) const {
            unsigned int index = hash.operator()(key) % array_size;
            node *ptr = data[index]->next;
            while (ptr != nullptr) {
                if (equal.operator()(ptr->value->first, key))break;
                else ptr = ptr->next;
            }
            if (ptr != nullptr)return ptr->value->second;
            else {
                index_out_of_bound e;
                throw e;
            }
        }

        T &operator[](const Key &key) {
            unsigned int index = hash.operator()(key) % array_size;
            node *ptr = data[index]->next;
            while (ptr != nullptr) {
                if (equal.operator()(ptr->value->first, key))break;
                else ptr = ptr->next;
            }
            if (ptr != nullptr)return ptr->value->second;
            else {
                T t;
                pair<node *, bool> tmp = insert_({key, t});
                scale++;
                return tmp.first->value->second;
            }
        }

        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T &operator[](const Key &key) const {
            unsigned int index = hash.operator()(key) % array_size;
            node *ptr = data[index]->next;
            while (ptr != nullptr) {
                if (equal.operator()(ptr->value->first, key))break;
                else ptr = ptr->next;
            }
            if (ptr != nullptr)return ptr->value->second;
            else {
                index_out_of_bound e;
                throw e;
            }
        }

        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            iterator tmp(head->after, this);
            return tmp;
        }

        const_iterator cbegin() const {
            const_iterator tmp(head->after, this);
            return tmp;
        }

        iterator end() {
            iterator tmp(nullptr, this);
            return tmp;
        }

        const_iterator cend() const {
            const_iterator tmp(nullptr, this);
            return tmp;
        }

        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return scale == 0;
        }

        /**
         * returns the number of elements.
         */
        size_t size() const {
            return scale;
        }

        /**
         * clears the contents
         */
        void clear() {
            //把每个数组后面拖着的链清空
            for (int i = 0; i < array_size; ++i) {
                node *ptr = data[i]->next;
                data[i]->next = nullptr;
                while (ptr != nullptr) {
                    node *del = ptr;
                    ptr = ptr->next;
                    delete del;
                }
            }
            scale = 0;
            head->after = nullptr;
            rear = head;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &v) {
            pair<node *, bool> tmp = insert_(v);
            iterator tmpi(tmp.first, this);
            if (tmp.second)scale++;
            return {tmpi, tmp.second};
        }

        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (this != pos.me || pos.ptr == nullptr || pos.ptr == head) {
                runtime_error e;
                throw e;
            }
            scale--;
            remove_(pos.ptr->value->first);
        }

        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         */
        size_t count(const Key &key) const {
            node *tmp = find_(key);
            if (tmp == nullptr)return 0;
            else return 1;
        }

        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key) {
            node *tmp = find_(key);
            if (tmp == nullptr)return end();
            else {
                iterator tmpi(tmp, this);
                return tmpi;
            }
        }

        const_iterator find(const Key &key) const {
            node *tmp = find_(key);
            if (tmp == nullptr)return cend();
            else {
                const_iterator tmpi(tmp, this);
                return tmpi;
            }
        }
    };

}

//void Double_Space() {
//    node **tmp = data;//先把数组找个地方放好
//    data = new node *[array_size * 2];
//    for (int i = 0; i < array_size; ++i) {
//        data[i] = new node;
//        //现在作为头节点都是空的
//        data[i]->next = tmp->next;
//        tmp->next = nullptr;
//        //把原来的链放到这里
//        delete tmp[i];
//    }
//    for (int i = array_size; i < array_size * 2; ++i) {
//        data[i] = new node;
//    }
//    array_size *= 2;
//}
#endif

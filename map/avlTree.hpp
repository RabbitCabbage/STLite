/**
 * implement a container like //std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

//#include <iostream>
// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    int max(int a, int b) {
        return (a > b ? a : b);
    }

    enum direction {
        left, right
    };
    enum position {
        end, inside, cend
    };
    //这里的 true false是指能否赋值
    //iterator的能否复制的性质是true，const iterator是false
    //然后就用my_type_traits 得到他所想代表的迭代器的性质，就可以有相应的行为
    //这样即使map的种类不同，只是通过using value_type = typename it::value_type;就可以实现普适
    struct my_true_type {
    };
    struct my_false_type {
    };
    template<class it>
    struct my_type_traits {
        using value_type = typename it::value_type;
        using difference_type = typename it::difference_type;
        using pointer = typename it::pointer;
        using reference = typename it::reference;
        using iterator_category = typename it::iterator_category;
        using iterator_assignable = typename it::iterator_assignable;
    };

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >
    class map {
    public:
        typedef pair<const Key, T> value_type;

        struct node {
            value_type *data;//maybe it doesn't have the default constructor;
            int height;
            node *left;
            node *right;
            node *parent;

            node(const Key &k, const T &t, node *p = nullptr, int h = 1, node *l = nullptr, node *r = nullptr)
                    : height(h), left(l), right(r), parent(p) {
                data = (value_type *) malloc(sizeof(value_type));
                new(data) value_type(k, t);
            }

            node(const node &other) {
                height = other.height;
                data = (value_type *) malloc(sizeof(value_type));
                new(data)value_type(other.data->first, other.data->second);
            }

            ~node() {
                if (data) {
                    data->first.~Key();
                    data->second.~T();
                    free(data);
                }
            }
        };

        class avl {
        private:
            Compare cmp;

            void LL(node *&t) {
                node *future_t = t->left;
                future_t->parent = t->parent;
                t->parent = future_t;

                t->left = future_t->right;
                if (future_t->right)future_t->right->parent = t;

                future_t->right = t;
                t->parent = future_t;

                t = future_t;
                future_t->right->height = height(future_t->right);//this is on the lower place;
                t->height = height(t);
            }

            void RR(node *&t) {
                node *future_t = t->right;
                future_t->parent = t->parent;
                t->parent = future_t;

                t->right = future_t->left;
                if (future_t->left)future_t->left->parent = t;

                future_t->left = t;
                t->parent = future_t;

                t = future_t;
                future_t->left->height = height(future_t->left);
                t->height = height(t);
            }

            void RL(node *&t) {
                LL(t->right);
                RR(t);
            }

            void LR(node *&t) {
                RR(t->left);
                LL(t);
            }

            node *find(const Key &k, node *t) const {
                if (t == nullptr || (!cmp.operator()(t->data->first, k) && !cmp.operator()(k, t->data->first)))return t;
                else if (cmp.operator()(t->data->first, k))return find(k, t->right);
                else return find(k, t->left);
                //data->first is less than what we want so go to the right tree;
            }

            pair<node *, bool> insert(const Key &k, const T &d, node *&t, node *t_parent) {
                if (t == nullptr) {
                    t = new node(k, d, t_parent);
                    return {t, true};
                }
                if (!cmp.operator()(t->data->first, k) && !cmp.operator()(k, t->data->first))return {t, false};
                else if (cmp.operator()(t->data->first, k)) {
                    pair<node *, bool> tmp_pair = insert(k, d, t->right, t);
                    node *tmp = tmp_pair.first;
                    //then check the height of the right tree
                    if (height(t->right) - height(t->left) == 2) {
                        //the right child is now out of baljudgeance
                        //so we need to check the insertion to  RR or RL
                        if (cmp.operator()(t->right->data->first, k))RR(t);
                            //the insertion happens in the right tree of the right child
                        else RL(t);
                    }
                    t->height = max(height(t->left), height(t->right)) + 1;
                    return tmp_pair;
                } else {
                    //the insertion happens in the left part of the t;
                    pair<node *, bool> tmp_pair = insert(k, d, t->left, t);
                    node *tmp = tmp_pair.first;
                    if (height(t->left) - height(t->right) == 2) {
                        if (cmp.operator()(t->left->data->first, k))LR(t);
                            //the insertion happens in the right tree of the left child;
                        else LL(t);
                    }
                    t->height = max(height(t->left), height(t->right)) + 1;
                    return tmp_pair;
                }
            }

            /* the height of one of t's children is absolutely changed
            *and the augment of d manifests the part of the removal;
            *and you should check the balance of t
            *and restore the balance of t if the node is out of balance
            *then update the height of t
            *and then return whether t's height has changed
            *this returned value will determine whether the parent of t needs adjust;
             */
            bool adjust(node *&t, direction d) {
                if (d == sjtu::right) {
                    //the removal happens in the right part of the tree
                    //the height of the right tree has been changed, -1;
                    //and we think when it comes to t the things of t's children have been dealt with;
                    int right_h = height(t->right), left_h = height(t->left);
                    if (right_h == left_h) {
                        //t is now in well balance but its height -1
                        t->height--;
                        return false;
                    } else if (left_h - right_h == 1) {
                        // t is now in well balance and the height is all the same;
                        return true;
                    } else {
                        // t is out of balance
                        //and we should now discuss the children of the left children;
                        //int lchild_h = height(t->left->left), rchild_h = height(t->left->right);
                        int lchild_h = height(t->left->left), rchild_h = height(t->left->right);
                        if (lchild_h < rchild_h) {
                            LR(t);
                            return false;
                        } else if (lchild_h > rchild_h) {
                            LL(t);
                            return false;
                        } else if (lchild_h == rchild_h) {
                            LL(t);
                            return true;
                        }
                    }
                } else if (d == sjtu::left) {
                    int right_h = height(t->right), left_h = height(t->left);
                    if (right_h == left_h) {
                        --t->height;
                        return false;
                    } else if (right_h - left_h == 1) {
                        return true;
                    } else {
                        int lchild_h = height(t->right->left), rchild_h = height(t->right->right);
                        if (lchild_h < rchild_h) {
                            RR(t);
                            return false;
                        } else if (lchild_h > rchild_h) {
                            RL(t);
                            return false;
                        } else if (lchild_h == rchild_h) {
                            RR(t);
                            return true;
                        }
                    }
                }
                return true;
            }

            bool remove(const Key &k, node *&t) {
                if (t == nullptr)return true;//the removal has been done, and the height doesn't change;
                else if (!cmp.operator()(t->data->first, k) &&
                         !cmp.operator()(k, t->data->first)) {//we have found the node to delete;
                    if (t->left == nullptr || t->right == nullptr) {
                        //we the node has no children or just one child;
                        //put the child to the parent and then delete the node;
                        node *tmp = t, *former_parent = t->parent;
                        t = (t->left == nullptr) ? t->right : t->left;//update the child
                        if (t != nullptr)t->parent = former_parent;
                        delete tmp;
                        return false;//the height of t has changed;
                    } else {
                        //the node has two children and we need to find a substitutional node;
                        //find the future t in the left tree;
                        node *future_t = t->left, *parent = t;
                        bool flag = false;
                        while (future_t->right != nullptr) {
                            flag = true;
                            parent = future_t;
                            future_t = future_t->right;
                        }
                        //now the future t is the substitution of t;
                        //now we need to deal with the pointers of t and future_t;
                        //you should change the pointer instead of the value;
                        // or the information in the iterator may be invalid;
                        if (flag) {
                            future_t->parent = t->parent;
                            t->parent = parent;
                            parent->right = t;

                            node *tmp_r = t->right;
                            t->right = future_t->right;
                            if (future_t->right)future_t->right->parent = t;

                            future_t->right = tmp_r;
                            if (tmp_r)tmp_r->parent = future_t;

                            node *tmp_l = t->left;
                            t->left = future_t->left;
                            if (future_t->left)future_t->left->parent = t;

                            future_t->left = tmp_l;
                            if (tmp_l)tmp_l->parent = future_t;

                            t->data->first.~Key();
                            t->data->second.~T();
                            new(t->data) value_type(future_t->data->first, future_t->data->second);
                            int tmp_h = t->height;
                            t->height = future_t->height;
                            future_t->height = tmp_h;
                            t = future_t;
                        } else {
                            future_t->parent = t->parent;
                            t->parent = future_t;

                            node *tmp_r = t->right;
                            t->right = future_t->right;
                            if (future_t->right)future_t->right->parent = t;

                            t->left = future_t->left;
                            if (future_t->left)future_t->left->parent = t;

                            future_t->left = t;

                            future_t->right = tmp_r;
                            if (tmp_r)tmp_r->parent = future_t;

                            t->data->first.~Key();
                            t->data->second.~T();
                            new(t->data) value_type(future_t->data->first, future_t->data->second);
                            int tmp_h = t->height;
                            t->height = future_t->height;
                            future_t->height = tmp_h;
                            t = future_t;
                        }
                        if (remove(future_t->data->first, t->left))return true;
                        else return adjust(t, left);
                        //we should adjust t and return the result of the adjustment;
                        //and the insertion happens in the left part of the tree;
                    }
                } else {
                    //the node is not the node to remove
                    if (cmp.operator()(t->data->first, k)) {
                        //the node to remove is on the right tree;
                        if (remove(k, t->right))return true;
                        else return adjust(t, right);
                    } else {
                        //tne node to remove is on the left part of the tree;
                        if (remove(k, t->left))return true;
                        else return adjust(t, left);
                    }
                }
            }

            void deconstruct(node *&t) {
                if (t == nullptr)return;
                else {
                    deconstruct(t->left);
                    deconstruct(t->right);
                    delete t;
                }
            }

        public:
            node *root = nullptr;

            ~avl() {
                deconstruct(root);
                root = nullptr;
            }

            avl operator=(const avl &other) {
                if (this == &other)return *this;
                create(root, other.root);
                return *this;
            }

            void create(node *&me, const node *other) {
                if (other == nullptr) {
                    me = nullptr;
                    return;
                } else {
                    me = new node(*other);
                    if (other->parent == nullptr)me->parent = nullptr;
                    create(me->left, other->left);
                    create(me->right, other->right);
                    if (me->left != nullptr)me->left->parent = me;
                    if (me->right != nullptr)me->right->parent = me;
                }
            }

            //return a node* if the ptr is null it means the search is failed;
            node *find(const Key &k) const {
                return find(k, root);
            }

            //return a node* if the ptr is null it means the insertion is failed;
            pair<node *, bool> insert(const value_type &d) {
                //std::cout << "Traverse" << std::endl;
                //traverse(root);//todo
                //std::cout << std::endl;
                return insert(d.first, d.second, root, nullptr);
            }

            void remove(const Key &k) {
                remove(k, root);
                //std::cout << "Traverse" << std::endl;
                //traverse(root);//todo
                //std::cout << std::endl;
            }

            int height(const node *t) const {
                if (t == nullptr)return 0;
                else {
                    int lh, rh;
                    if (t->left == nullptr)lh = 0;
                    else lh = t->left->height;
                    if (t->right == nullptr)rh = 0;
                    else rh = t->right->height;
                    return max(lh, rh) + 1;
                }
            }

            node *smallest() const {
                if (root == nullptr)return root;
                node *tmp = root;
                while (tmp->left != nullptr)tmp = tmp->left;
                return tmp;
            }

            node *biggest() const {
                if (root == nullptr)return root;
                node *tmp = root;
                while (tmp->right != nullptr)tmp = tmp->right;
                return tmp;
            }

//            void traverse(node *tree) {
//                if (tree == nullptr)return;
//                else {
////                    std::cout << "{ " << tree << " } and height "
////                              << tree->height << " ";
////                    if (tree->left != nullptr)std::cout << "left child " << tree->left << ' ';
////                    else std::cout << "left child nullptr ";
////                    if (tree->right != nullptr)std::cout << "right child " << tree->right << '\n';
////                    else std::cout << "right child nullptr\n";
//                    if (tree == tree->parent)std::cout << "WRONG ANSWER" << std::endl;
//                    traverse(tree->left);
//                    traverse(tree->right);
//                }
//            }
        };

        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */

        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
        class const_iterator;


        class iterator {
        private:
            Compare cmp;
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = map::value_type;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::output_iterator_tag;
            using iterator_assignable = my_true_type;

            map *me;
            position iterator_pos;
            node *ptr;

            iterator() {
                ptr = nullptr;
                me = nullptr;
                iterator_pos = sjtu::end;
            }

            iterator(node *p, map *m, const position &poss) {
                ptr = p;
                me = m;
                iterator_pos = poss;
            }

            iterator(const iterator &other) {
                ptr = other.ptr;
                me = other.me;
                iterator_pos = other.iterator_pos;
            }

            //移动构造函数，因为下面函数传参的时候有用到
//            iterator(iterator &&other) {
//                ptr = other.ptr;
//                me = other.me;
//                iterator_pos = other.pos;
//            }

            iterator operator++(int) {
                if (ptr == nullptr && iterator_pos == sjtu::end) {
                    runtime_error e;
                    throw e;
                }
                if (ptr->right != nullptr) {
                    node *leftest = ptr->right;
                    while (leftest->left != nullptr)leftest = leftest->left;
                    iterator tmp(ptr, me, inside);
                    ptr = leftest;
                    return tmp;
                } else {
                    //find ptr's parent and the parent should have ptr as the left child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->left == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        if (cmp.operator()(ptr->data->first, me->tree.root->right->data->first)) {
                            iterator tmp(*this);
                            ptr = me->tree.root->right;
                            while (ptr->left != nullptr)ptr = ptr->left;
                            return tmp;
                        }
                        iterator tmp(*this);
                        ptr = the_latest_proper_parent;
                        iterator_pos = sjtu::end;
                        return tmp;
                    } else {
                        iterator tmp(*this);
                        ptr = the_latest_proper_parent;
                        return tmp;
                    }
                }
            }

            iterator &operator++() {
                if (ptr == nullptr && iterator_pos == sjtu::end) {
                    runtime_error e;
                    throw e;
                }
                if (ptr->right != nullptr) {
                    node *leftest = ptr->right;
                    while (leftest->left != nullptr)leftest = leftest->left;
                    ptr = leftest;
                    return *this;
                } else {
                    //find ptr's parent and the parent should have ptr as the left child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->left == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        //check the right child of the root;
                        if (cmp.operator()(ptr->data->first, me->tree.root->right->data->first)) {
                            ptr = me->tree.root->right;
                            while (ptr->left != nullptr)ptr = ptr->left;
                            return *this;
                        }
                        ptr = the_latest_proper_parent;
                        iterator_pos = sjtu::end;
                        return *this;
                    } else {
                        ptr = the_latest_proper_parent;
                        return *this;
                    }
                }
            }

            iterator operator--(int) {
                if (me->empty()) {
                    runtime_error e;
                    throw e;
                }
                if (ptr == nullptr && iterator_pos == sjtu::end) {
                    iterator tmp(ptr, me, sjtu::end);
                    ptr = me->tree.biggest();
                    iterator_pos = sjtu::inside;
                    return tmp;
                }
                if (ptr->left != nullptr) {
                    node *rightest = ptr->left;
                    while (rightest->right != nullptr)rightest = rightest->right;
                    iterator tmp(ptr, me, sjtu::inside);
                    ptr = rightest;
                    return tmp;
                } else {
                    //find ptr's parent and the parent should have ptr as the right child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->right == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        //this manifest ptr is biggest element in the avl
                        if (cmp.operator()(me->tree.root->left->data->first, ptr->data->first)) {
                            iterator tmp(*this);
                            ptr = me->tree.root->left;
                            while (ptr->right != nullptr)ptr = ptr->right;
                            return tmp;
                        }
                        runtime_error e;
                        throw e;
                    } else {
                        iterator tmp(*this);
                        ptr = the_latest_proper_parent;
                        return tmp;
                    }
                }
            }

            iterator &operator--() {
                if (me->empty()) {
                    runtime_error e;
                    throw e;
                }
                if (ptr == nullptr && iterator_pos == sjtu::end) {
                    ptr = me->tree.biggest();
                    iterator_pos = sjtu::inside;
                    return *this;
                }
                if (ptr->left != nullptr) {
                    node *rightest = ptr->left;
                    while (rightest->right != nullptr)rightest = rightest->right;
                    ptr = rightest;
                    return *this;
                } else {
                    //find ptr's parent and the parent should have ptr as the right child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->right == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        if (cmp.operator()(me->tree.root->left->data->first, ptr->data->first)) {
                            ptr = me->tree.root->left;
                            while (ptr->right != nullptr)ptr = ptr->right;
                            return *this;
                        }
                        //this manifest ptr is biggest element in the avl
                        runtime_error e;
                        throw e;
                    } else {
                        ptr = the_latest_proper_parent;
                        return *this;
                    }
                }
            }

            /**
             * an operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type &operator*() const {
                if (iterator_pos != sjtu::end && iterator_pos != sjtu::cend)return *(ptr->data);
                else {
                    runtime_error e;
                    throw e;
                }
            }

            bool operator==(const iterator &other) const {
                if (me == other.me)
                    return (ptr == other.ptr && iterator_pos == other.iterator_pos);
                else return false;
            }

            bool operator==(const const_iterator &other) const {
                if (me == other.me)
                    if (iterator_pos == sjtu::end)
                        return (ptr == other.ptr && (iterator_pos == sjtu::end && other.iterator_pos == sjtu::cend));
                    else return ptr == other.ptr && (iterator_pos == other.iterator_pos);
                else return false;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return (ptr != rhs.ptr || me != rhs.me);
            }

            bool operator!=(const const_iterator &rhs) const {
                return (ptr != rhs.ptr || me != rhs.me);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const noexcept {
                if (iterator_pos != sjtu::end && iterator_pos != sjtu::cend)return ptr->data;
                else {
                    runtime_error e;
                    throw e;
                }
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            Compare cmp;
        public:
            const map *me;
            position iterator_pos;
            node *ptr;
            using difference_type = std::ptrdiff_t;
            using value_type = map::value_type;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::output_iterator_tag;
            using iterator_assignable = my_false_type;

            const_iterator() {
                ptr = nullptr;
                me = nullptr;
                iterator_pos = sjtu::end;
            }

            const_iterator(node *p, map *m, const position &poss) {
                ptr = p;
                me = m;
                iterator_pos = poss;
            }

            const_iterator(node *p, const map *m, const position &poss) {
                ptr = p;
                me = m;
                iterator_pos = poss;
            }

            const_iterator(const const_iterator &other) {
                ptr = other.ptr;
                me = other.me;
                iterator_pos = other.iterator_pos;
            }

            const_iterator(const iterator &other) {
                ptr = other.ptr;
                me = other.me;
                iterator_pos = other.iterator_pos;
            }

//            const_iterator(const iterator &&other) {
//                ptr = other.ptr;
//                me = other.me;
//                iterator_pos = other.iterator_pos;
//            }

            const_iterator operator++(int) {
                if (ptr == nullptr && iterator_pos == sjtu::cend) {
                    runtime_error e;
                    throw e;
                }
                if (ptr->right != nullptr) {
                    node *leftest = ptr->right;
                    while (leftest->left != nullptr)leftest = leftest->left;
                    const_iterator tmp(ptr, me, inside);
                    ptr = leftest;
                    return tmp;
                } else {
                    //find ptr's parent and the parent should have ptr as the left child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->left == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        if (cmp.operator()(ptr->data->first, me->tree.root->right->data->first)) {
                            const_iterator tmp(*this);
                            ptr = me->tree.root->right;
                            while (ptr->left != nullptr)ptr = ptr->left;
                            return tmp;
                        }
                        const_iterator tmp(*this);
                        ptr = the_latest_proper_parent;
                        iterator_pos = sjtu::end;
                        return tmp;
                    } else {
                        const_iterator tmp(*this);
                        ptr = the_latest_proper_parent;
                        return tmp;
                    }
                }
            }

            const_iterator &operator++() {
                if (ptr == nullptr && iterator_pos == sjtu::cend) {
                    runtime_error e;
                    throw e;
                }
                if (ptr->right != nullptr) {
                    node *leftest = ptr->right;
                    while (leftest->left != nullptr)leftest = leftest->left;
                    ptr = leftest;
                    return *this;
                } else {
                    //find ptr's parent and the parent should have ptr as the left child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->left == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        if (cmp.operator()(ptr->data->first, me->tree.root->right->data->first)) {
                            ptr = me->tree.root->right;
                            while (ptr->left != nullptr)ptr = ptr->left;
                            return *this;
                        }
                        ptr = the_latest_proper_parent;
                        iterator_pos = sjtu::cend;
                        return *this;
                    } else {
                        ptr = the_latest_proper_parent;
                        return *this;
                    }
                }
            }

            const_iterator operator--(int) {
                if (me->empty()) {
                    runtime_error e;
                    throw e;
                }
                if (ptr == nullptr && iterator_pos == sjtu::cend) {
                    const_iterator tmp(ptr, me, sjtu::cend);
                    ptr = me->tree.biggest();
                    iterator_pos = sjtu::inside;
                    return tmp;
                }
                if (ptr->left != nullptr) {
                    node *rightest = ptr->left;
                    while (rightest->right != nullptr)rightest = rightest->right;
                    const_iterator tmp(ptr, me, sjtu::inside);
                    ptr = rightest;
                    return tmp;
                } else {
                    //find ptr's parent and the parent should have ptr as the right child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->right == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        if (cmp.operator()(me->tree.root->left->data->first, ptr->data->first)) {
                            const_iterator tmp(*this);
                            ptr = me->tree.root->left;
                            while (ptr->right != nullptr)ptr = ptr->right;
                            return tmp;
                        }
                        //this manifest ptr is biggest element in the avl
                        runtime_error e;
                        throw e;
                    } else {
                        const_iterator tmp(*this);
                        ptr = the_latest_proper_parent;
                        return tmp;
                    }
                }
            }

            const_iterator &operator--() {
                if (me->empty()) {
                    runtime_error e;
                    throw e;
                }
                if (ptr == nullptr && iterator_pos == sjtu::cend) {
                    ptr = me->tree.biggest();
                    iterator_pos = sjtu::inside;
                    return *this;
                }
                if (ptr->left != nullptr) {
                    node *rightest = ptr->left;
                    while (rightest->right != nullptr)rightest = rightest->right;
                    ptr = rightest;
                    return *this;
                } else {
                    //find ptr's parent and the parent should have ptr as the right child;
                    node *the_latest_proper_parent = ptr;
                    while (1) {
                        node *child = the_latest_proper_parent;
                        the_latest_proper_parent = the_latest_proper_parent->parent;
                        if (the_latest_proper_parent == nullptr)break;
                        if (the_latest_proper_parent->right == child)break;
                        else continue;
                    }
                    if (the_latest_proper_parent == nullptr) {
                        if (cmp.operator()(me->tree.root->left->data->first, ptr->data->first)) {
                            ptr = me->tree.root->left;
                            while (ptr->right != nullptr)ptr = ptr->right;
                            return *this;
                        }
                        //this manifest ptr is biggest element in the avl
                        runtime_error e;
                        throw e;
                    } else {
                        ptr = the_latest_proper_parent;
                        return *this;
                    }
                }
            }

            /**
             * an operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type &operator*() const {
                if (iterator_pos != sjtu::end && iterator_pos != sjtu::cend)return *(ptr->data);
                else {
                    runtime_error e;
                    throw e;
                }
            }

            bool operator==(const iterator &other) const {
                if (me == other.me)
                    if (iterator_pos == sjtu::cend)
                        return (ptr == other.ptr && other.iterator_pos == sjtu::end);
                    else return ptr == other.ptr && (iterator_pos == other.iterator_pos);
                else return false;
            }

            bool operator==(const const_iterator &other) const {
                if (me == other.me)
                    return (ptr == other.ptr && iterator_pos == other.iterator_pos);
                else return false;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return (ptr != rhs.ptr || me != rhs.me);
            }

            bool operator!=(const const_iterator &rhs) const {
                return (ptr != rhs.ptr || me != rhs.me);
            }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type *operator->() const noexcept {
                if (iterator_pos != sjtu::end && iterator_pos != sjtu::cend)return ptr->data;
                else {
                    runtime_error e;
                    throw e;
                }
            }
        };

    private:
        avl tree;
        int size_;

    public:

        map() {
            size_ = 0;
        }

        map(const map &other) {
            size_ = other.size_;
            tree.create(tree.root, other.tree.root);
        }

        map &operator=(const map &other) {
            if (this == &other)return *this;
            size_ = other.size_;
            tree.create(tree.root, other.tree.root);
            return *this;
        }

        ~map() = default;

        /**
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T &at(const Key &key) {
            node *tmp = tree.find(key);
            if (tmp == nullptr) {
                index_out_of_bound e;
                throw e;
//                auto *v = (value_type *) malloc(sizeof(value_type));
//                auto *k = (Key *) malloc(sizeof(Key));
//                new(k)Key(key);
//                T tmpt;
//                new(v)value_type(*k, tmpt);
//                node *tmpn = tree.insert(*v);
//                return (tmpn->data->second);
            } else return tmp->data->second;
        }

        const T &at(const Key &key) const {
            node *tmp = tree.find(key);
            if (tmp == nullptr) {
                index_out_of_bound e;
                throw e;
            } else return tmp->data->second;
        }

        /**
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        T &operator[](const Key &key) {
            T tmp_t;
            pair<node *, bool> tmp_pair = tree.insert({key, tmp_t});
            if (tmp_pair.second)size_++;
            return tmp_pair.first->data->second;
        }

        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T &operator[](const Key &key) const {
            node *tmp = tree.find(key);
            if (tmp == nullptr) {
                index_out_of_bound e;
                throw e;
            } else return tmp->data->second;
        }

        /**
         * return a iterator to the beginning
         */
        iterator begin() {
            if (this->empty()) {
                iterator tmp(nullptr, this, sjtu::end);
                return tmp;
            }
            iterator tmp(tree.smallest(), this, sjtu::inside);
            return tmp;
        }

        const_iterator cbegin() const {
            if (this->empty()) {
                const_iterator tmp(nullptr, this, sjtu::cend);
                return tmp;
            }
            const_iterator tmp(tree.smallest(), this, sjtu::inside);
            return tmp;
        }

        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end() {
            iterator tmp(nullptr, this, sjtu::end);
            return tmp;
        }

        const_iterator cend() const {
            const_iterator tmp(nullptr, this, sjtu::cend);
            return tmp;
        }

        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {
            return size_ == 0;
        }

        /**
         * returns the number of elements.
         */
        size_t size() const {
            return size_;
        }

        /**
         * clears the contents
         */
        void clear() {
            tree.~avl();
            tree.root = nullptr;
            size_ = 0;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),//todo
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            pair<node *, bool> tmp_pair = tree.insert(value);
            node *tmp = tmp_pair.first;
            if (tmp_pair.second == false) {//already exists
                iterator tmpn(tmp, this, sjtu::inside);
                pair<iterator, bool> tmpp(tmpn, false);
                return tmpp;
            } else {
                ++size_;
                iterator tmpn(tmp, this, sjtu::inside);
                pair<iterator, bool> tmpp(tmpn, true);
                return tmpp;
            }
        }

        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase(iterator pos) {
            if (pos.me != this || pos == end() || pos == cend()) {
                runtime_error e;
                throw e;
            }
            --size_;
            tree.remove((*pos).first);
        }

        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const {
            node *tmp = tree.find(key);
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
            node *tmpn = tree.find(key);
            if (tmpn == nullptr) {
                iterator tmp(tmpn, this, sjtu::end);
                return tmp;
            } else {
                iterator tmp(tmpn, this, sjtu::inside);
                return tmp;
            }
        }

        const_iterator find(const Key &key) const {
            node *tmpn = tree.find(key);
            if (tmpn == nullptr) {
                const_iterator tmp(tmpn, this, sjtu::cend);
                return tmp;
            } else {
                const_iterator tmp(tmpn, this, sjtu::inside);
                return tmp;
            }
        }

//        void traverse() {
//            std::cout << "TRAVERSE" << std::endl;
//            tree.traverse(tree.root);
//            std::cout << "TRAVERSE" << std::endl;
//        }
    };

}

#endif

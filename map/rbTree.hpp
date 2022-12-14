/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
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
    enum colortype {
        red, black
    };
    enum direction {
        left_child, right_child
    };
    enum position {
        end, cend, inside
    };

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >
    class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;

        struct node {
            node *left = nullptr, *right = nullptr, *parent = nullptr;
            value_type *data;
            colortype color;

            node(node *p, Key k, T t, colortype c, node *l = nullptr, node *r = nullptr) {
                parent = p;
                left = l;
                right = r;
                color = c;
                data = (value_type *) malloc(sizeof(value_type));
                new(data) value_type({k, t});
            }

            node(const node &other) {
                parent = other.parent;
                left = other.left;
                right = other.right;
                color = other.color;
                data->first.~Key();
                data->second.~T();
                free(data);
                data = (value_type) malloc(sizeof(value_type));
                new(data) value_type({other.data->first, other.data->second});
            }

            ~node() {
                parent = left = right = nullptr;
                data->first.~Key();
                data->second.~T();
                free(data);
            }
        };

        class rbTree {
            typedef pair<const Key, T> value_type;

        public:
            node *root;
            Compare cmp;

            rbTree() { root = nullptr; }

            rbTree(const rbTree &other) {
                create(root, other.root, nullptr);
            }

            ~rbTree() {
                destroy(root);
            }

            //?????????????????????????????????????????????????????????me????????????????????????????????????????????????????????????
            void create(node *&me, const node *other, node *myparent) {
                if (other == nullptr)return;
                else {
                    //??????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                    me = new node(myparent, other->data->first, other->data->second, other->color);
                    create(me->left, other->left, me);
                    create(me->right, other->right, me);
                }
            }

            void destroy(node *tree) {
                if (tree == nullptr)return;
                else {
                    destroy(tree->left);
                    destroy(tree->right);
                    delete tree;
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

            pair<node *, bool> insert(const Key &k, const T &t) {
                if (root == nullptr) {
                    root = new node(nullptr, k, t, black);
                    return {root, true};
                }
                node *current = root;//????????????????????????
                node *p = nullptr;//????????????????????????????????????
                node *gp = nullptr;//???????????????????????????????????????
                direction dir = left_child;
                while (1) {
                    //while???????????????????????????????????????????????????????????????????????????????????????
                    if (current) {
                        if (!cmp.operator()(k, current->data->first) && !cmp.operator()(current->data->first, k))
                            return {current, false};
                        if (current->left && current->right && current->left->color == red &&
                            current->right->color == red) {
                            //??????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????adjust?????????
                            current->color = red;
                            current->left->color = black;
                            current->right->color = black;
                            insert_adjust(gp, p, current);
                        }
                        //???????????????????????????????????????????????????????????????????????????
                        //????????????????????????????????????????????????gp???????????????????????????????????????
                        gp = p;
                        p = current;
                        current = ((cmp.operator()(k, current->data->first)) ? current->left : current->right);
                        dir = ((cmp.operator()(k, p->data->first)) ? left_child : right_child);
                    } else break;
                }
                //?????????????????????????????????
                current = new node(p, k, t, red);//???????????????
                if (dir == left_child)p->left = current;
                else p->right = current;
                insert_adjust(gp, p, current);
                root->color = black;//????????????????????????????????????????????????????????????
                return {current, true};
            }

            void remove(const Key &k_rm) {
                Key *k = new Key(k_rm);
                if (root == nullptr) {
                    delete k;
                    return;
                } else if ((!cmp.operator()(*k, root->data->first) && !cmp.operator()(root->data->first, *k)) &&
                           root->left == nullptr && root->right == nullptr) {
                    delete root;
                    root = nullptr;
                    delete k;
                    return;//??????root????????????????????????????????????????????????????????????????????????root???????????????????????????
                }
                node *current = root;
                node *bro = nullptr, *p = nullptr;
                while (1) {
                    if (current == nullptr) {
//                        std::cout << "?????????????????????????????????????????????return???" << std::endl;
                        delete k;
                        return;
                    }
                    remove_adjust(p, current, bro, *k);
                    //??????c??????????????????????????????????????????????????????????????????
                    if ((!cmp.operator()(*k, current->data->first) && !cmp.operator()(current->data->first, *k)) &&
                        current->left != nullptr && current->right != nullptr) {
                        //?????????????????????????????????????????????????????????
                        node *tmp = current->right;
                        while (tmp->left != nullptr) tmp = tmp->left;
                        //??????tmp??????????????????????????????????????????????????????????????????????????????????????????????????????
                        //?????????????????????????????????????????????????????????????????????????????????tmp??????current??????
                        //k = tmp->data->first;//???????????????????????????????????????k
                        if (current == root)root = tmp;
                        delete k;
                        k = new Key(tmp->data->first);
                        current->data->first.~Key();
                        current->data->second.~T();//???current????????????????????????????????????
                        new(current->data) value_type({tmp->data->first, tmp->data->second});
                        colortype tmp_color = current->color;
                        current->color = tmp->color;
                        tmp->color = tmp_color;
                        if (current->right == tmp) {
                            //??????????????????????????????????????????
                            if (p) {
                                if (p->left == current)p->left = tmp;
                                else p->right = tmp;
                            }
                            current->parent = tmp;
                            tmp->parent = p;
                            current->right = tmp->right;
                            if (tmp->right)tmp->right->parent = current;
                            tmp->right = current;
                            tmp->left = current->left;
                            if (current->left)current->left->parent = tmp;
                            current->left = nullptr;
                            //???????????????????????????????????????????????????current?????????????????????????????????p bro
                            //?????????????????????????????????
                            p = current->parent;
                            bro = p->left;
                            continue;//????????????c?????????????????????????????????????????????????????????
                        } else {
                            if (p) {
                                if (p->left == current)p->left = tmp;
                                else p->right = tmp;
                            }
                            node *tmp_parent = tmp->parent;
                            if (tmp->parent->left == tmp)tmp_parent->left = current;
                            else tmp_parent->right = current;
                            current->parent = tmp_parent;
                            tmp->parent = p;
                            //tmp???????????????????????????????????????
                            tmp->left = current->left;
                            if (current->left)current->left->parent = tmp;
                            current->left = nullptr;
                            node *tmp_r = tmp->right;
                            tmp->right = current->right;
                            if (current->right)current->right->parent = tmp;
                            current->right = tmp_r;
                            if (tmp_r)tmp_r->parent = current;
                            //???????????????????????????
                            current = tmp->right;
                            bro = tmp->left;
                            p = tmp;
                            continue;
                        }
                    } else if (!cmp.operator()(*k, current->data->first) && !cmp.operator()(current->data->first, *k))
                        break;//??????????????????????????????????????????????????????????????????????????????????????????
                    else {
                        //???????????????????????????
                        p = current;
                        current = ((cmp.operator()(*k, current->data->first)) ? current->left : current->right);
                        bro = (current == p->left ? p->right : p->left);
                    }
                }
                //???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                //???????????????????????????????????????????????? ????????????????????????
                //????????????p???????????????????????????
                if (current == p->left)p->left = nullptr;
                else p->right = nullptr;
                delete current;
                delete k;
                root->color = black;
            }

            node *find(const Key &k) const {
                return find(k, root);
            }

        private:
            void ll(node *tree_root) {
                node *future_root = tree_root->left;
                node *parent = tree_root->parent;
                if (parent) {
                    if (parent->left == tree_root)parent->left = future_root;
                    else parent->right = future_root;
                }
                future_root->parent = parent;
                tree_root->left = future_root->right;
                if (future_root->right)future_root->right->parent = tree_root;
                future_root->right = tree_root;
                tree_root->parent = future_root;
                if (tree_root == root)root = future_root;
            }

            void rr(node *tree_root) {
                node *future_root = tree_root->right;
                node *parent = tree_root->parent;
                if (parent) {
                    if (parent->left == tree_root)parent->left = future_root;
                    else parent->right = future_root;
                }
                future_root->parent = parent;
                tree_root->right = future_root->left;
                if (future_root->left)future_root->left->parent = tree_root;
                future_root->left = tree_root;
                tree_root->parent = future_root;
                if (tree_root == root)root = future_root;
            }

            void rl(node *tree_root) {
                node *future_root = tree_root->right->left;
                node *middle = tree_root->right;
                node *parent = tree_root->parent;
                if (parent) {
                    if (parent->left == tree_root)parent->left = future_root;
                    else parent->right = future_root;
                }//?????????parent????????????????????????
                future_root->parent = parent;
                tree_root->right = future_root->left;
                if (future_root->left)future_root->left->parent = tree_root;
                middle->left = future_root->right;
                if (future_root->right)future_root->right->parent = middle;
                future_root->right = middle;
                middle->parent = future_root;
                future_root->left = tree_root;
                tree_root->parent = future_root;
                if (tree_root == root)root = future_root;
            }

            void lr(node *tree_root) {
                node *future_root = tree_root->left->right;
                node *middle = tree_root->left;
                node *parent = tree_root->parent;
                if (parent) {
                    if (parent->left == tree_root)parent->left = future_root;
                    else parent->right = future_root;
                }
                future_root->parent = parent;
                middle->right = future_root->left;
                if (future_root->left)future_root->left->parent = middle;
                tree_root->left = future_root->right;
                if (future_root->right)future_root->right->parent = tree_root;
                future_root->right = tree_root;
                tree_root->parent = future_root;
                future_root->left = middle;
                middle->parent = future_root;
                if (tree_root == root)root = future_root;
            }

            void insert_adjust(node *gp, node *p, node *c) {
                //??????????????????c,?????????????????????????????????
                if (p == nullptr || p->color == black)return;
                else if (p == root) {
                    p->color = black;
                    return;
                }
                if (gp->left == p) {
                    if (p->left == c) {
                        p->color = black;
                        gp->color = red;
                        ll(gp);
                    } else {
                        c->color = black;
                        gp->color = red;
                        lr(gp);
                    }
                } else {
                    if (p->right == c) {
                        p->color = black;
                        gp->color = red;
                        rr(gp);
                    } else {
                        c->color = black;
                        gp->color = red;
                        rl(gp);
                    }
                }
            }

            void remove_adjust(node *&p, node *&c, node *&bro, const Key &k) {
                //??????????????????c?????????????????????????????????????????????????????????????????????
                if (c->color == red)return;
                if (c == root) {
                    //??????c????????????root????????????????????????????????????????????????,??????????????????????????????????????????root??????????????????remove?????????????????????
                    if (c->left && c->right && c->left->color == c->right->color) {
                        c->color = red;
                        c->left->color = c->right->color = black;
                        return;
                    }
                    //?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                    // ?????????????????????????????????????????????????????????????????????????????????????????????????????????
                }
                if ((c->left == nullptr || c->left->color == black) &&
                    (c->right == nullptr || c->right->color == black)) {
                    //??????c???????????????p????????????????????????????????????????????????????????????c?????????????????????????????????
                    //???????????????????????????????????????????????????
                    if ((bro->left == nullptr || bro->left->color == black) &&
                        (bro->right == nullptr || bro->right->color == black)) {
                        //????????????c???????????????????????????????????????????????????????????????p????????????
                        //?????????????????????????????????????????????????????????
                        c->color = red;
                        bro->color = red;
                        p->color = black;
                    } else {
                        //??????????????????????????????????????????????????????
                        //????????????????????????
                        if (p->left == bro) {
                            if (bro->left && bro->left->color == red) {
                                p->color = black;
                                bro->color = red;
                                c->color = red;
                                bro->left->color = black;
                                ll(p);
                                //??????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                                //???????????????????????????????????????????????????????????????????????????????????????????????????
                                //????????????????????????
                            } else {
                                p->color = black;
                                c->color = red;
                                lr(p);
                            }
                        } else {
                            if (bro->right && bro->right->color == red) {
                                p->color = black;
                                bro->color = red;
                                c->color = red;
                                bro->right->color = black;
                                rr(p);
                            } else {
                                p->color = black;
                                c->color = red;
                                rl(p);
                            }
                        }
                    }
                } else {
                    //????????????c???????????????????????????????????????????????????
                    if (!cmp.operator()(k, c->data->first) && !cmp.operator()(c->data->first, k)) {
                        //????????????????????????????????????????????????
                        if (c->left && c->right) {
                            //?????????????????????????????????????????????????????????????????????????????????????????????????????????
                            if (c->right->color == black) {
                                c->color = red;
                                c->left->color = black;
                                //??????????????????????????????????????????????????????????????????
                                ll(c);
                                //?????????????????????????????????????????????c??????????????????
                                p = c->parent;//??????????????????p?????????????????????????????????????????????p??????
                                return;
                            }
                            //????????????????????????????????????????????????
                            //????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                            return;
                        } else if (c->left) {
                            //????????????????????????????????????????????????????????????????????????
                            c->color = red;
                            c->left->color = black;
                            ll(c);
                            //????????????c?????????????????????
                            p = c->parent;
                            return;
                        } else {
                            //??????????????????????????????
                            c->color = red;
                            c->right->color = black;
                            rr(c);
                            p = c->parent;
                            return;
                        }
                    } else {
                        //???????????????????????????????????????????????????????????????
                        p = c;
                        c = (cmp.operator()(k, c->data->first) ? c->left : c->right);
                        bro = (p->left == c ? p->right : p->left);
                        if (c->color == black) {
                            bro->color = black;
                            p->color = red;
                            if (p->right == bro)rr(p);
                            else ll(p);
                            bro = (p->left == c ? p->right : p->left);
                            remove_adjust(p, c, bro, k);
                            return;
                        } else return;
                    }
                }
            }

            node *find(const Key &k, node *t) const {
                if (t == nullptr)return t;
                if (!cmp.operator()(t->data->first, k) && !cmp.operator()(k, t->data->first))return t;
                if (cmp.operator()(k, t->data->first))return find(k, t->left);
                else return find(k, t->right);
            }
        };


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

            //???????????????????????????????????????????????????????????????
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
        rbTree tree;
        size_t size_;
    public:
        map() {
            size_ = 0;
        }

        map(const map &other) {
            size_ = other.size_;
            tree.create(tree.root, other.tree.root, nullptr);
        }

        map &operator=(const map &other) {
            if (this == &other)return *this;
            size_ = other.size_;
            tree.create(tree.root, other.tree.root, nullptr);
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
            pair<node *, bool> tmp_pair = tree.insert(key, tmp_t);
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
            tree.~rbTree();
            tree.root = nullptr;
            size_ = 0;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        pair<iterator, bool> insert(const value_type &value) {
            pair<node *, bool> tmp_pair = tree.insert(value.first,value.second);
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
    };

}

#endif

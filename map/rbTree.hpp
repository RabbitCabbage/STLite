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

            //这里要用指针的引用传递，因为等会儿改到me上的所有操作，是要直接呈现在左右儿子上的
            void create(node *&me, const node *other, node *myparent) {
                if (other == nullptr)return;
                else {
                    //这里不能直接用复制构造，因为我们要产生的是两颗互不干扰的树，不能直接指针相等
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
                node *current = root;//现在检查到的节点
                node *p = nullptr;//现在检查到的节点的父节点
                node *gp = nullptr;//现在检查到的节点的祖父节点
                direction dir = left_child;
                while (1) {
                    //while循环的目的是消灭掉有两个红儿子的节点，想办法让这样的点消失
                    if (current) {
                        if (!cmp.operator()(k, current->data->first) && !cmp.operator()(current->data->first, k))
                            return {current, false};
                        if (current->left && current->right && current->left->color == red &&
                            current->right->color == red) {
                            //现在不管它父亲的颜色是什么，反正现在出现了两个红色的儿子，直接修改儿子颜色，然后留给adjust去调整
                            current->color = red;
                            current->left->color = black;
                            current->right->color = black;
                            insert_adjust(gp, p, current);
                        }
                        //然后就是没有遇见有两个红色儿子的，就可以往下走一层
                        //因为在使用旋转函数的时候只改变了gp，那么现在就可以直接往下走
                        gp = p;
                        p = current;
                        current = ((cmp.operator()(k, current->data->first)) ? current->left : current->right);
                        dir = ((cmp.operator()(k, p->data->first)) ? left_child : right_child);
                    } else break;
                }
                //现在到了要插入的空节点
                current = new node(p, k, t, red);//插入红节点
                if (dir == left_child)p->left = current;
                else p->right = current;
                insert_adjust(gp, p, current);
                root->color = black;//防止把根节点的颜色设置成了红色，修改一下
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
                    return;//只要root节点有一个儿子，我们就不能用这种办法，要想办法把root变成红节点后再删除
                }
                node *current = root;
                node *bro = nullptr, *p = nullptr;
                while (1) {
                    if (current == nullptr) {
//                        std::cout << "删除的是一个不存在的元素，直接return了" << std::endl;
                        delete k;
                        return;
                    }
                    remove_adjust(p, current, bro, *k);
                    //现在c已经是红节点了。如果可以删除的话就可以删除了
                    if ((!cmp.operator()(*k, current->data->first) && !cmp.operator()(current->data->first, *k)) &&
                        current->left != nullptr && current->right != nullptr) {
                        //现在此节点有两个儿子，只能向下找替代品
                        node *tmp = current->right;
                        while (tmp->left != nullptr) tmp = tmp->left;
                        //现在tmp是我们找好的替代品，我们应该在不改变指针本身指向的情况下把他们替换掉
                        //先替换指向的值，在替换指针的位置，最后要删除掉，包含有tmp值的current节点
                        //k = tmp->data->first;//我们下面所寻找的目标变成了k
                        if (current == root)root = tmp;
                        delete k;
                        k = new Key(tmp->data->first);
                        current->data->first.~Key();
                        current->data->second.~T();//将current的值变成等会要删除的那个
                        new(current->data) value_type({tmp->data->first, tmp->data->second});
                        colortype tmp_color = current->color;
                        current->color = tmp->color;
                        tmp->color = tmp_color;
                        if (current->right == tmp) {
                            //相当于把两个节点互换一下位置
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
                            //互换结束，下面就不用走了，因为此时current就是要删除的节点，更新p bro
                            //相当于自动往下走了一层
                            p = current->parent;
                            bro = p->left;
                            continue;//但虽然说c就是言删除的节点，它的颜色总得处理一下
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
                            //tmp的左孩子是空的，可以直接换
                            tmp->left = current->left;
                            if (current->left)current->left->parent = tmp;
                            current->left = nullptr;
                            node *tmp_r = tmp->right;
                            tmp->right = current->right;
                            if (current->right)current->right->parent = tmp;
                            current->right = tmp_r;
                            if (tmp_r)tmp_r->parent = current;
                            //然后还得往下走一层
                            current = tmp->right;
                            bro = tmp->left;
                            p = tmp;
                            continue;
                        }
                    } else if (!cmp.operator()(*k, current->data->first) && !cmp.operator()(current->data->first, *k))
                        break;//找到了要删除并且可以删除的（叶节点，或者只有一个孩子的节点）
                    else {
                        //否则就往下走一层了
                        p = current;
                        current = ((cmp.operator()(*k, current->data->first)) ? current->left : current->right);
                        bro = (current == p->left ? p->right : p->left);
                    }
                }
                //如果只有一个孩子，不可能是红节点的，因为这样就无法满足所有空路径上的黑节点个数相同
                //所以不用考虑，只有可能是没有儿子 的红节点直接删掉
                //这里要求p必须被更新掉！！！
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
                }//先更新parent，再把它赋给别的
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
                //任务就是调整c,使得不出现连续的红节点
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
                //任务就是调整c，让他在不影响红黑树性质的情况下，变成红节点；
                if (c->color == red)return;
                if (c == root) {
                    //现在c本来就是root，那么如果说它的两个儿子都是红色,或者都是黑色的的，就先可以把root改了，后来在remove里面又改回来了
                    if (c->left && c->right && c->left->color == c->right->color) {
                        c->color = red;
                        c->left->color = c->right->color = black;
                        return;
                    }
                    //如果是在根节点，本身黑色，一个儿子红颜色一个儿子黑颜色，就可以按照下面的讨论，
                    // 就是说，我接着往下走一个，看看额能不能走到红的那个，不用单列一种情况了
                }
                if ((c->left == nullptr || c->left->color == black) &&
                    (c->right == nullptr || c->right->color == black)) {
                    //现在c是黑色的，p是红色的，因为我们每一次都保证上一个过到c的时候本身的颜色是红色
                    //那么现在应该对它的兄弟节点进行讨论
                    if ((bro->left == nullptr || bro->left->color == black) &&
                        (bro->right == nullptr || bro->right->color == black)) {
                        //我们想把c变成红色，就干脆把他和他的兄弟都变成红色，p变成黑色
                        //反正现在兄弟节点都是黑儿子，不会有冲突
                        c->color = red;
                        bro->color = red;
                        p->color = black;
                    } else {
                        //也就是说兄弟节点有至少一个红色的儿子
                        //现在开始各种讨论
                        if (p->left == bro) {
                            if (bro->left && bro->left->color == red) {
                                p->color = black;
                                bro->color = red;
                                c->color = red;
                                bro->left->color = black;
                                ll(p);
                                //因为我们写旋转函数的时候并没有用引用，所有的指针旋转完之后还指向原来的节点，
                                //现在就要考虑，等会是要往下在走一层的，我们要保证往下走一层的正确性
                                //好像是的，就这吧
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
                    //否则就是c有红色的儿子。就要试着往下走一步了
                    if (!cmp.operator()(k, c->data->first) && !cmp.operator()(c->data->first, k)) {
                        //也就是说这个点就是我们想删除的点
                        if (c->left && c->right) {
                            //这个点有至少一个红色的儿子，是我们要删除的点，并且有两个儿子，要找替身
                            if (c->right->color == black) {
                                c->color = red;
                                c->left->color = black;
                                //因为后面压根没有用到这些节点，直接重新更新了
                                ll(c);
                                //虽然没找到合适的替身，但是现在c变成了红色的
                                p = c->parent;//下面可能要把p的对应节点变成空姐点，必须更新p节点
                                return;
                            }
                            //否则就说明这个点的右儿子是红色的
                            //那么我就偷懒不调整了，反正等会还要找替身，找到替身之后要往右走，往右走正好到了这个红节点
                            return;
                        } else if (c->left) {
                            //只有一个左孩子了，那么这个左孩子一定是红色的孩子
                            c->color = red;
                            c->left->color = black;
                            ll(c);
                            //现在这个c节点是红色的了
                            p = c->parent;
                            return;
                        } else {
                            //只有一个红色的右孩子
                            c->color = red;
                            c->right->color = black;
                            rr(c);
                            p = c->parent;
                            return;
                        }
                    } else {
                        //这个节点不是应当删除的节点，直接往下走一个
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

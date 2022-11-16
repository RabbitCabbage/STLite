#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"
//#include <iostream>
//#include <stack>

namespace sjtu {

/**
 * a container like std::priority_queue which is a heap internal.
 */
    template<typename T, class Compare = std::less<T>>
    class priority_queue {
    public:
        struct node {
            int npl;
            node *left;
            node *right;
            T *value;

            node() {
                npl = -1;
                left = nullptr;
                right = nullptr;
                value = nullptr;
            }

            node(const node &other) {
                npl = other.npl;
                left = nullptr;
                right = nullptr;
                value = (T *) malloc(sizeof(T));
                new(value)T(*(other.value));
            }

            explicit node(T v) {
                npl = 0;
                left = nullptr;
                right = nullptr;
                value = (T *) malloc(sizeof(T));
                new(value)T(v);
            }

            ~node() {
                if (value) {
//                    std::cout << "delete value " << (long) value << std::endl;
                    value->~T();
                    free(value);
                }
            }
        };

    public:
        node *root;
        int size_ = 0;
    private:
        Compare cmp;

        void create(node *&me, const node *other) {
            if (other == nullptr) {
                me = nullptr;
                return;
            } else {
                me = (node *) malloc(sizeof(node));
                new(me)node(*other);
//                std::cout << "create node " << (long) me << std::endl;
                create(me->left, other->left);
                create(me->right, other->right);
            }
        }

        void clear(node *tree) {
//            std::cout << "clear " << std::endl;
            if (tree == nullptr)return;
            else {
                clear(tree->left);
                clear(tree->right);
//                std::cout << "clear free " << (long) tree << std::endl;
                tree->~node();
                free(tree);
            }
        }

        node *merge(node *me, node *other) {
//            std::cout << "inside merge " << std::endl;
            if (other == nullptr)return me;
            else if (me == nullptr) {
                me = other;
                other = nullptr;
                return me;
            }
            node *smaller = (cmp.operator()(*me->value, *other->value) ? me : other);
            node *bigger = (!cmp.operator()(*me->value, *other->value) ? me : other);
            //下面想实现头元素最大，那么就要把较小的放到较大的右子堆里面，所以要检查较大的右子堆是否是空的
            if (bigger->right == nullptr) {
                //put smaller into the bigger's right heap;
                bigger->right = smaller;
                if ((bigger->left == nullptr && bigger->right != nullptr) || (
                        bigger->right != nullptr && bigger->left->npl < bigger->right->npl)) {
                    node *tmp = bigger->left;
                    bigger->left = bigger->right;
                    bigger->right = tmp;
                }
                if (bigger->right != nullptr)bigger->npl = bigger->right->npl + 1;
                else bigger->npl = 0;
                return bigger;
            } else {
                //现在把bigger的右子树和比较小的合并
                //会返回merge之后的根节点
                bigger->right = merge(bigger->right, smaller);
                //合并完了之后，要调整bigger的左右子树的形态
                if ((bigger->left == nullptr && bigger->right != nullptr) || (
                        bigger->right != nullptr && bigger->left->npl < bigger->right->npl)) {
                    node *tmp = bigger->left;
                    bigger->left = bigger->right;
                    bigger->right = tmp;
                }
                if (bigger->right != nullptr)bigger->npl = bigger->right->npl + 1;
                else bigger->npl = 0;
                return bigger;
            }

        }

    public:

        void merge(priority_queue &other) {
            root = merge(root, other.root);
            other.root = nullptr;
            other.size_ = 0;
        }


        priority_queue() {
            size_ = 0;
            root = nullptr;
        }

//        explicit priority_queue(node *root_node) {
//            size_ = 1;
//            root = (node *) malloc(sizeof(node));
////            std::cout << "constructor malloc " << (long) root << std::endl;
//            new(root)node(*root_node);
//        }

        priority_queue(const priority_queue &other) {
            size_ = other.size();
            root = nullptr;
            create(root, other.root);
        }

        ~priority_queue() {
//            std::cout << "deconstruct " << std::endl;
            clear(root);
        }

        priority_queue &operator=(const priority_queue &other) {
            if (this == &other)return *this;
            else {
                size_ = other.size();
                clear(root);
                create(root, other.root);
                return *this;
            }
        }

        /**
         * get the top of the queue.
         * @return a reference of the top element.
         * throw container_is_empty if empty() returns true;
         */
        const T &top() const {
            if (root == nullptr) {
                container_is_empty e;
                throw e;
            } else return *(root->value);
        }

        void push(const T &e) {
//            std::cout << "push " << e << std::endl;
//            std::cout.flush();
            if (root == nullptr) {
                size_ = 1;
                root = (node *) malloc(sizeof(node));
                new(root)node(e);
//                std::cout<<"push root malloc"<<root<<std::endl;
                root->npl = 0;
                return;
            }
            ++size_;
            node *root_node = (node *) malloc(sizeof(node));
            new(root_node)node(e);
//            std::cout<<"push non-root malloc"<<root_node<<std::endl;
            root = merge(root, root_node);
            return;
            //after merging the other is empty, and the tree itself has been changed;
        }


        void pop() {
//            std::cout << "pop " << top() << std::endl;
            if (root == nullptr) {
                container_is_empty e;
                throw e;
            }
            --size_;
            node *left = root->left;
            node *right = root->right;
            root->~node();
            free(root);
            if (right == nullptr) {
                root = left;
            } else root = merge(left, right);
//            std::cout << "size " << size() << std::endl;
        }

        /**
         * return the number of the elements.
         */
        size_t

        size() const {
            return size_;
        }

        /**
         * check if the container has at least an element.
         * @return true if it is empty, false if it has at least an element.
         */
        bool empty() const {
            return root == nullptr;
        }

//        void traverse() {
//            std::cout << "Traverse: " << std::endl;
//            std::cout.flush();
//            std::stack<node *> node_stack;
//            node_stack.push(root);
//            while (!node_stack.empty()) {
//                node *tmp = node_stack.top();
//                node_stack.pop();
//                if (tmp != nullptr) {
//                    std::cout << *(tmp->value) << " ";
//                    node_stack.push(tmp->right);
//                    node_stack.push(tmp->left);
//                }
//            }
//            std::cout << "\nTraverse ends" << std::endl;
//        }
    };

}

#endif

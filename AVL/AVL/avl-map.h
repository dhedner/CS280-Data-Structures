#ifndef BSTMAP_H
#define BSTMAP_H

namespace CS280
{

template <typename KEY_TYPE, typename VALUE_TYPE>
class AVLmap
{
public:
    class Node
    {
    public:
        Node(KEY_TYPE k, VALUE_TYPE val, Node* p, int h, int b, Node* l, Node* r)
            : key(k), value(val), parent(p), left(l), right(r), height(h), balance(b)
        {
        }

        Node(const Node&) = delete;
        Node* operator=(const Node&) = delete;

        KEY_TYPE const& Key() const
        {
            return key;
        }

        VALUE_TYPE& Value()
        {
            return value;
        }

        VALUE_TYPE const& Value() const
        {
            return value;
        }

        Node* first()
        {
            Node* current = this;

            while (current->left)
            {
                current = current->left;
            }

            return current;
        }

        Node* last()
        {
            Node* current = this;

            while (current->right)
            {
                current = current->right;
            }

            return current;
        }

        Node* increment()
        {
            Node* current = this;

            if (current->right)
            {
                current = current->right;
                return current->first();
            }

            while (current->parent && current->parent->right == current)
            {
                current = current->parent;
            }

            return current->parent;
        }

        Node* decrement()
        {
            Node* current = this;

            if (current->left)
            {
                current = current->left;
                return current->last();
            }

            while (current->parent && current->parent->left == current)
            {
                current = current->parent;
            }

            return current->parent;
        }

        void print(std::ostream& os) const
        {
            os << key << " -> " << value << "\n";
        }

    private:
        KEY_TYPE key;
        VALUE_TYPE value;
        Node* parent;
        Node* left;
        Node* right;
        int height, balance;

        friend class AVLmap;
    };

    static int getdepth(Node* node)
    {
        int depth = 0;
        while (node)
        {
            node = node->parent;
            depth++;
        }
        return depth - 1;
    }

private:
    struct AVLmap_iterator
    {
    private:
        Node* p_node;

    public:
        AVLmap_iterator(Node* p = nullptr) : p_node(p)
        {
        }

        AVLmap_iterator(const AVLmap_iterator& rhs) : p_node(rhs.p_node)
        {
        }

        AVLmap_iterator& operator=(const AVLmap_iterator& rhs)
        {
            p_node = rhs.p_node;

            return *this;
        }

        AVLmap_iterator& operator++()
        {
            p_node = p_node->increment();
            return *this;
        }

        AVLmap_iterator operator++(int)
        {
            AVLmap_iterator temp = *this;
            p_node = p_node->increment();
            return temp;
        }

        Node& operator*()
        {
            return *p_node;
        }

        Node* operator->()
        {
            return p_node;
        }

        bool operator!=(const AVLmap_iterator& rhs)
        {
            return p_node != rhs.p_node;
        }

        bool operator==(const AVLmap_iterator& rhs)
        {
            return p_node == rhs.p_node;
        }
        friend class AVLmap;
    };
    struct AVLmap_iterator_const
    {
    private:
        Node* p_node;

    public:
        AVLmap_iterator_const(Node* p = nullptr) : p_node(p)
        {
        }

        AVLmap_iterator_const(const AVLmap_iterator_const& rhs) : p_node(rhs.p_node)
        {
        }

        AVLmap_iterator_const& operator=(const AVLmap_iterator_const& rhs)
        {
            p_node = rhs.p_node;
        }

        AVLmap_iterator_const& operator++()
        {
            p_node = p_node->increment();
            return *this;
        }

        AVLmap_iterator_const operator++(int)
        {
            AVLmap_iterator_const temp = *this;
            p_node = p_node->increment();
            return temp;
        }

        Node const& operator*()
        {
            return *p_node;
        }

        Node const* operator->()
        {
            return p_node;
        }

        bool operator!=(const AVLmap_iterator_const& rhs)
        {
            return p_node != rhs.p_node;
        }

        bool operator==(const AVLmap_iterator_const& rhs)
        {
            return p_node == rhs.p_node;
        }

        friend class AVLmap;
    };
    // AVLmap implementation
    Node* pRoot = nullptr;
    unsigned int size_ = 0;
    // end iterators are same for all AVLmaps, thus static
    // make AVLmap_iterator a friend
    // to allow AVLmap_iterator to access end iterators
    static AVLmap_iterator end_it;
    static AVLmap_iterator_const const_end_it;

public:
    // BIG FOUR
    AVLmap() : pRoot(nullptr), size_(0)
    {
    }

    AVLmap(const AVLmap& rhs) : pRoot(nullptr), size_(0)
    {
        for (const_iterator it = rhs.begin(); it != rhs.end(); it++)
        {
            (*this)[it->Key()] = it->Value();
        }
    }

    AVLmap(AVLmap&& rhs) : pRoot(rhs.pRoot), size_(rhs.size_)
    {
        rhs.pRoot = nullptr;
        rhs.size_ = 0;
    }

    AVLmap& operator=(const AVLmap& rhs)
    {
        clear();

        if (rhs.pRoot)
        {
            do_copy(rhs.pRoot);
        }

        size_ = rhs.size_;
        return *this;
    }

    AVLmap& operator=(AVLmap&& rhs)
    {
        clear();
        pRoot = rhs.pRoot;
        size_ = rhs.size_;
        rhs.pRoot = nullptr;
        rhs.size_ = 0;

        return *this;
    }

    virtual ~AVLmap()
    {
        clear();
    }

    unsigned int size()
    {
        return size_;
    }

    void clear()
    {
        if (pRoot)
        {
            do_clear(pRoot);
        }
        pRoot = nullptr;
        size_ = 0;
    }

    // value setter and getter
    VALUE_TYPE& operator[](KEY_TYPE const& key)
    {
        Node* new_node = nullptr;
        do_insert(pRoot, nullptr, &new_node, key);
        return new_node->value;
    }

    // standard names for iterator types
    typedef AVLmap_iterator iterator;
    typedef AVLmap_iterator_const const_iterator;

    // AVLmap methods dealing with non-const iterator
    AVLmap_iterator begin()
    {
        Node* current = pRoot;

        if (!current)
        {
            return end();
        }

        return AVLmap_iterator(current->first());
    }

    AVLmap_iterator end()
    {
        return end_it;
    }

    AVLmap_iterator find(KEY_TYPE const& key)
    {
        Node* current = pRoot;

        while (current)
        {
            if (key == current->key)
            {
                return AVLmap_iterator(current);
            }
            if (key < current->key)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }

        return end();
    }

    void erase(AVLmap_iterator it)
    {
        Node* current = it.p_node;
        if (!current)
        {
            return;
        }

        // No children
        if (!current->left && !current->right)
        {
            if (current->parent)
            {
                if (current->parent->left == current)
                {
                    current->parent->left = nullptr;
                }
                else
                {
                    current->parent->right = nullptr;
                }
            }
            else
            {
                pRoot = nullptr;
            }
        }
        // Case for two children
        else if (current->left && current->right)
        {
            // Traverse iteratively to find the smallest node in the right subtree
            Node* next = current->increment();

            // Swap the key and value
            std::swap(current->key, next->key);
            std::swap(current->value, next->value);
            current = next;

            if (current->parent->left == current)
            {
                current->parent->left = current->right;
                if (current->right)
                {
                    current->right->parent = current->parent;
                }
            }
            else
            {
                current->parent->right = current->right;
                if (current->right)
                {
                    current->right->parent = current->parent;
                }
            }
        }
        // Case for one child
        // If the current node has a child, then we can just replace the current node with its child
        else if (current->parent)
        {
            Node* child = current->left ? current->left : current->right;
            if (current->parent->left == current)
            {
                current->parent->left = child;
            }
            else
            {
                current->parent->right = child;
            }

            child->parent = current->parent;
        }
        // If the current node is the root, then we can just replace the root with its child
        else
        {
            Node* child = current->left ? current->left : current->right;
            pRoot = child;
            if (child)
            {
                child->parent = nullptr;
            }
        }

        // Rebalance the tree
        Node* current_parent = current->parent;
        while (current_parent)
        {
            current_parent->height = 1 + std::max(get_height(current_parent->left), get_height(current_parent->right));
            int balance = get_balance(current_parent);

            // Left-Left Case
            if (balance > 1 && get_balance(current_parent->left) >= 0)
            {
                current_parent = rotate_right(current_parent);
            }

            // Right-Right Case
            if (balance < -1 && get_balance(current_parent->right) <= 0)
            {
                current_parent = rotate_left(current_parent);
            }

            // Left-Right Case
            if (balance > 1 && get_balance(current_parent->left) < 0)
            {
                current_parent->left = rotate_left(current_parent->left);
                current_parent = rotate_right(current_parent);
            }

            // Right-Left Case
            if (balance < -1 && get_balance(current_parent->right) > 0)
            {
                current_parent->right = rotate_right(current_parent->right);
                current_parent = rotate_left(current_parent);
            }

            current_parent = current_parent->parent;
        }

        delete current;
        --size_;
    }

    // AVLmap methods dealing with const iterator
    AVLmap_iterator_const begin() const
    {
        Node* current = pRoot;

        if (!current)
        {
            return end();
        }

        return AVLmap_iterator_const(current->first());
    }

    AVLmap_iterator_const end() const
    {
        return const_end_it;
    }

    AVLmap_iterator_const find(KEY_TYPE const& key) const
    {
        Node* current = pRoot;

        while (current)
        {
            if (key == current->key)
            {
                return AVLmap_iterator_const(current);
            }
            if (key < current->key)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }

        return end();
    }
    // do not need this one (why)
    // AVLmap_iterator_const erase(AVLmap_iterator& it) const;

    char getedgesymbol(const Node* node) const;

    void print(std::ostream& os, bool print_value = false) const;
    bool sanityCheck();

    // inner class (AVLmap_iterator) doesn't have any special priveleges
    // in accessing private data/methods of the outer class (AVLmap)
    // so need friendship to allow AVLmap_iterator to access private "AVLmap::end_it"
    // BTW - same is true for outer class accessing inner class private data
    friend struct AVLmap_iterator;
    friend struct AVLmap_iterator_const;

private:
    int get_height(Node* node)
    {
        return node ? node->height : 0;
    }

    int get_balance(Node* node)
    {
        return node ? get_height(node->left) - get_height(node->right) : 0;
    }

    Node* rotate_right(Node* node)
    {
        Node* left = node->left;
        Node* leftright = left->right;

        if (node->parent)
        {
            if (node->parent->left == node)
            {
                node->parent->left = left;
            }
            else
            {
                node->parent->right = left;
            }
        }
        else
        {
            pRoot = left;
        }

        left->right = node;
        left->parent = node->parent;
        node->left = leftright;
        if (node->left)
        {
            node->left->parent = node;
        }
        node->parent = left;

        node->height = 1 + std::max(get_height(node->left), get_height(node->right));
        left->height = 1 + std::max(get_height(left->left), get_height(left->right));

        return left;
    }

    Node* rotate_left(Node* node)
    {
        Node* right = node->right;
        Node* rightleft = right->left;

        if (node->parent)
        {
            if (node->parent->left == node)
            {
                node->parent->left = right;
            }
            else
            {
                node->parent->right = right;
            }
        }
        else
        {
            pRoot = right;
        }

        right->left = node;
        right->parent = node->parent;
        node->right = rightleft;
        if (node->right)
        {
            node->right->parent = node;
        }
        node->parent = right;

        node->height = 1 + std::max(get_height(node->left), get_height(node->right));
        right->height = 1 + std::max(get_height(right->left), get_height(right->right));

        return right;
    }

    Node* do_insert(Node* current, Node* parent, Node** new_node, KEY_TYPE const& key)
    {
        if (!current)
        {
            current = new Node(key, VALUE_TYPE(), parent, 1, 0, nullptr, nullptr);
            size_++;
            *new_node = current;

            if (!pRoot)
            {
                pRoot = current;
            }
        }

        if (key < current->key)
        {
            current->left = do_insert(current->left, current, new_node, key);
        }
        else if (key > current->key)
        {
            current->right = do_insert(current->right, current, new_node, key);
        }
        else
        {
            return current;
        }

        // Update height and find balance
        current->height = 1 + std::max(get_height(current->left), get_height(current->right));

        int balance = get_balance(current);

        // Left-Left Case
        if (balance > 1 && key < current->left->key)
        {
            return rotate_right(current);
        }

        // Right-Right Case
        if (balance < -1 && key > current->right->key)
        {
            return rotate_left(current);
        }

        // Left-Right Case
        if (balance > 1 && key > current->left->key)
        {
            current->left = rotate_left(current->left);
            return rotate_right(current);
        }

        // Right-Left Case
        if (balance < -1 && key < current->right->key)
        {
            current->right = rotate_right(current->right);
            return rotate_left(current);
        }

        return current;
    }

    void do_copy(Node* node)
    {
        (*this)[node->key] = node->value;

        if (node->left)
        {
            do_copy(node->left);
        }
        if (node->right)
        {
            do_copy(node->right);
        }
    }

    void do_clear(Node* node)
    {
        if (node->left)
        {
            do_clear(node->left);
        }

        if (node->right)
        {
            do_clear(node->right);
        }

        delete node;
    }
};

// notice that it doesn't need to be friend
template <typename KEY_TYPE, typename VALUE_TYPE>
std::ostream& operator<<(std::ostream& os, AVLmap<KEY_TYPE, VALUE_TYPE> const& map);
} // namespace CS280

#include "avl-map.cpp"
#endif

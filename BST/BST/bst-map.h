#ifndef BSTMAP_H
#define BSTMAP_H

namespace CS280
{

template <typename KEY_TYPE, typename VALUE_TYPE>
class BSTmap
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
        int height, balance; // optional

        friend class BSTmap;
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
    struct BSTmap_iterator
    {
    private:
        Node* p_node;

    public:
        BSTmap_iterator(Node* p = nullptr) : p_node(p)
        {
        }

        BSTmap_iterator(const BSTmap_iterator& rhs) : p_node(rhs.p_node)
        {
        }

        BSTmap_iterator& operator=(const BSTmap_iterator& rhs)
        {
            p_node = rhs.p_node;

            return *this;
        }

        BSTmap_iterator& operator++()
        {
            p_node = p_node->increment();
            return *this;
        }

        BSTmap_iterator operator++(int)
        {
            BSTmap_iterator temp = *this;
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

        bool operator!=(const BSTmap_iterator& rhs)
        {
            return p_node != rhs.p_node;
        }

        bool operator==(const BSTmap_iterator& rhs)
        {
            return p_node == rhs.p_node;
        }
        friend class BSTmap;
    };
    struct BSTmap_iterator_const
    {
    private:
        Node* p_node;

    public:
        BSTmap_iterator_const(Node* p = nullptr) : p_node(p)
        {
        }

        BSTmap_iterator_const(const BSTmap_iterator_const& rhs) : p_node(rhs.p_node)
        {
        }

        BSTmap_iterator_const& operator=(const BSTmap_iterator_const& rhs)
        {
            p_node = rhs.p_node;
        }

        BSTmap_iterator_const& operator++()
        {
            p_node = p_node->increment();
            return *this;
        }

        BSTmap_iterator_const operator++(int)
        {
            BSTmap_iterator_const temp = *this;
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

        bool operator!=(const BSTmap_iterator_const& rhs)
        {
            return p_node != rhs.p_node;
        }

        bool operator==(const BSTmap_iterator_const& rhs)
        {
            return p_node == rhs.p_node;
        }

        friend class BSTmap;
    };
    // BSTmap implementation
    Node* pRoot = nullptr;
    unsigned int size_ = 0;
    // end iterators are same for all BSTmaps, thus static
    // make BSTmap_iterator a friend
    // to allow BSTmap_iterator to access end iterators
    static BSTmap_iterator end_it;
    static BSTmap_iterator_const const_end_it;

public:
    // BIG FOUR
    BSTmap() : pRoot(nullptr), size_(0)
    {
    }

    BSTmap(const BSTmap& rhs) : pRoot(nullptr), size_(0)
    {
        for (const_iterator it = rhs.begin(); it != rhs.end(); it++)
        {
            (*this)[it->Key()] = it->Value();
        }
    }

    BSTmap(BSTmap&& rhs) : pRoot(rhs.pRoot), size_(rhs.size_)
    {
        rhs.pRoot = nullptr;
        rhs.size_ = 0;
    }

    BSTmap& operator=(const BSTmap& rhs)
    {
        clear();

        if (rhs.pRoot)
        {
            do_copy(rhs.pRoot);
        }

        size_ = rhs.size_;
        return *this;
    }

    BSTmap& operator=(BSTmap&& rhs)
    {
        clear();
        pRoot = rhs.pRoot;
        size_ = rhs.size_;
        rhs.pRoot = nullptr;
        rhs.size_ = 0;

        return *this;
    }

    virtual ~BSTmap()
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
        Node* current = pRoot;
        Node* parent = nullptr;

        while (current)
        {
            if (key == current->key)
            {
                return current->value;
            }
            parent = current;
            if (key < current->key)
            {
                current = current->left;
            }
            else
            {
                current = current->right;
            }
        }

        Node* newNode = new Node(key, VALUE_TYPE(), parent, 0, 0, nullptr, nullptr);
        if (!parent)
        {
            pRoot = newNode;
        }
        else if (key < parent->key)
        {
            parent->left = newNode;
        }
        else
        {
            parent->right = newNode;
        }
        ++size_;
        return newNode->value;
    }

    // next method doesn't make sense
    // because operator[] inserts a non-existing element
    // which is not allowed on const maps
    // VALUE_TYPE operator[](int key) const;

    // standard names for iterator types
    typedef BSTmap_iterator iterator;
    typedef BSTmap_iterator_const const_iterator;

    // BSTmap methods dealing with non-const iterator
    BSTmap_iterator begin()
    {
        Node* current = pRoot;

        if (!current)
        {
            return end();
        }

        return BSTmap_iterator(current->first());
    }

    BSTmap_iterator end()
    {
        return end_it;
    }

    BSTmap_iterator find(KEY_TYPE const& key)
    {
        Node* current = pRoot;

        while (current)
        {
            if (key == current->key)
            {
                return BSTmap_iterator(current);
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

    void erase(BSTmap_iterator it)
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

        delete current;
        --size_;
    }

    // BSTmap methods dealing with const iterator
    BSTmap_iterator_const begin() const
    {
        Node* current = pRoot;

        if (!current)
        {
            return end();
        }

        return BSTmap_iterator_const(current->first());
    }

    BSTmap_iterator_const end() const
    {
        return const_end_it;
    }

    BSTmap_iterator_const find(KEY_TYPE const& key) const
    {
        Node* current = pRoot;

        while (current)
        {
            if (key == current->key)
            {
                return BSTmap_iterator_const(current);
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
    // BSTmap_iterator_const erase(BSTmap_iterator& it) const;

    char getedgesymbol(const Node* node) const;

    void print(std::ostream& os, bool print_value = false) const;
    bool sanityCheck();

    // inner class (BSTmap_iterator) doesn't have any special priveleges
    // in accessing private data/methods of the outer class (BSTmap)
    // so need friendship to allow BSTmap_iterator to access private "BSTmap::end_it"
    // BTW - same is true for outer class accessing inner class private data
    friend struct BSTmap_iterator;
    friend struct BSTmap_iterator_const;

private:
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
std::ostream& operator<<(std::ostream& os, BSTmap<KEY_TYPE, VALUE_TYPE> const& map);
} // namespace CS280

#include "bst-map.cpp"
#endif

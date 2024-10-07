/*****************************************************************
 * @file   lariat.h
 * @brief  Interface to a Lariat class template
 * @author david.hedner@digipen.edu
 * @date   February 2024
 *
 * @copyright © 2024 DigiPen (USA) Corporation.
 *****************************************************************/
///////////////////////////////////////////////////////////////////////////////////
#ifndef LARIAT_H
#define LARIAT_H
////////////////////////////////////////////////////////////////////////////////

#include <cstring> // memcpy
#include <string>  // error strings
#include <utility> // error strings

class LariatException : public std::exception
{
private:
    int m_ErrCode;
    std::string m_Description;

public:
    LariatException(int ErrCode, const std::string& Description)
        : m_ErrCode(ErrCode), m_Description(Description)
    {
    }

    virtual int code(void) const
    {
        return m_ErrCode;
    }

    virtual const char* what(void) const throw()
    {
        return m_Description.c_str();
    }

    virtual ~LariatException() throw()
    {
    }

    enum LARIAT_EXCEPTION
    {
        E_NO_MEMORY,
        E_BAD_INDEX,
        E_DATA_ERROR
    };
};

// forward declaration for 1-1 operator<<
template <typename T, int Size>
class Lariat;

template <typename T, int Size>
std::ostream& operator<<(std::ostream& os, Lariat<T, Size> const& rhs);

template <typename T, int Size>
class Lariat
{
    template <typename OtherT, int OtherSize>
    friend class Lariat;

public:
    Lariat() : head_(nullptr), tail_(nullptr), size_(0), nodecount_(0), asize_(Size)
    {
    }

    Lariat(Lariat<T, Size> const& copy) : Lariat()
    {
        copy_from(copy);
    }

    template <typename OtherT, int OtherSize>
    Lariat(Lariat<OtherT, OtherSize> const& copy) : Lariat()
    {
        copy_from(copy);
    }

    ~Lariat()
    {
        clear();
    }

    Lariat& operator=(Lariat<T, Size> const& rhs)
    {
        clear();
        copy_from(rhs);
        return *this;
    }

    template <typename OtherT, int OtherSize>
    Lariat& operator=(Lariat<OtherT, OtherSize> const& rhs)
    {
        clear();
        copy_from(rhs);
        return *this;
    }

    // inserts
    void insert(int index, const T& value)
    {
        // Throw exception for bad index
        if (index < 0 || index > size_)
        {
            throw LariatException(LariatException::E_BAD_INDEX, "Subscript is out of range");
        }

        // If the list is empty, create a new node and set the head and tail to it
        if (!head_)
        {
            head_ = new LNode;
            tail_ = head_;
            nodecount_++;
        }

        // Find the node to insert into and the local index within that node
        std::pair<LNode*, int> position = find_element(index);
        LNode* current = position.first;
        int local_index = position.second;

        // Shift all the elements of the node to the right starting at the index
        shift_up(current, local_index);

        // If the node is full, take the element at the position of the index and move it to a new
        // node
        if (current->count == asize_)
        {
            int split_index = static_cast<int>(asize_ / 2 + 1);
            split_node(current, split_index);

            // The insertion index is in the right half, put it in the next node
            T* overflow = local_index == asize_ ? nullptr : &current->values[local_index];
            T* overflow_destination = nullptr;
            if (local_index >= split_index)
            {
                local_index -= current->count;
                current = current->next;
                overflow_destination = &current->values[current->count];
            }
            else
            {
                overflow_destination = &current->next->values[current->next->count];
                current->count--;
                current->next->count++;
            }

            if (overflow != nullptr)
            {
                std::swap(*overflow, *overflow_destination);
            }
        }

        // There was room in the node for the inserted value
        current->values[local_index] = value;
        current->count++;
        size_++;
    }

    void push_back(const T& value)
    {
        insert(size_, value);
    }

    void push_front(const T& value)
    {
        insert(0, value);
    }

    // deletes
    void erase(int index)
    {
        // Throw exception for bad index
        if (index < 0 || index >= size_)
        {
            throw LariatException(LariatException::E_BAD_INDEX, "Subscript is out of range");
        }

        // If the erase is at the end or beginning, use pop_back or pop_front
        if (index == size_ - 1)
        {
            pop_back();
            return;
        }
        else if (index == 0)
        {
            pop_front();
            return;
        }

        // Find the node to erase from and the local index within that node
        std::pair<LNode*, int> position = find_element(index);
        LNode* current = position.first;
        int local_index = position.second;

        // Shift all the elements of the node to the left starting at the index
        shift_down(current, local_index, 1);

        current->count--;
        size_--;

        // If the node is empty after the erase, delete the node and update the pointers
        if (current->count == 0)
        {
            delete_node(current);
        }
    }
    void pop_back()
    {
        // If the list is empty, do nothing
        if (!head_)
        {
            return;
        }

        tail_->count--;
        size_--;

        // If the node is now empty, delete the node and update the pointers
        if (tail_->count == 0)
        {
            delete_node(tail_);
        }
    }
    void pop_front()
    {
        // If the list is empty, do nothing
        if (!head_)
        {
            return;
        }

        // Shift all the elements of the node to the left
        shift_down(head_, 0, 1);

        head_->count--;
        size_--;

        // If the node is now empty, delete the node and update the pointers
        if (head_->count == 0)
        {
            delete_node(head_);
        }
    }

    // access
    T& operator[](int index) // for l-values
    {
        std::pair<LNode*, int> element = find_element(index);
        return element.first->values[element.second];
    }
    const T& operator[](int index) const // for r-values
    {
        std::pair<LNode*, int> element = find_element(index);
        return element.first->values[element.second];
    }

    T& first()
    {
        return head_->values[0];
    }
    T const& first() const
    {
        return head_->values[0];
    }
    T& last()
    {
        return tail_->values[tail_->count - 1];
    }
    T const& last() const
    {
        return tail_->values[tail_->count - 1];
    }

    // returns index, size (one past last) if not found
    unsigned find(const T& value) const
    {
        LNode* current = head_;
        int index = 0;
        while (current)
        {
            for (int i = 0; i < current->count; i++)
            {
                if (current->values[i] == value)
                {
                    return index;
                }
                index++;
            }
            current = current->next;
        }
        return static_cast<unsigned>(size());
    }

    friend std::ostream& operator<< <T, Size>(std::ostream& os, Lariat<T, Size> const& list);

    // and some more
    size_t size(void) const // total number of items (not nodes)
    {
        return static_cast<size_t>(size_);
    }

    void clear(void) // make it empty
    {
        while (head_)
        {
            delete_node(head_);
        }
        size_ = 0;
        nodecount_ = 0;
    }

    void compact() // push data in front reusing empty positions and delete remaining nodes
    {

        // Walk the list with two pointers, one for the current node and one for the next node
        LNode* left = head_;
        LNode* right = (head_ != nullptr) ? head_->next : nullptr;

        // Walk the list with both pointers until left isn't already full and next isn't null
        while (left && right)
        {
            if (left->count < asize_)
            {
                break;
            }
            left = right;
            right = right->next;
        }

        // Walk through the list while the next node isn't null
        while (right)
        {
            // Store the count of the next node, then set the count to 0
            int count = right->count;
            int moved = 0;
            right->count = 0;

            // Move the values from the next node to the left node
            for (int i = 0; count > 0; i++)
            {
                if (left->count == asize_)
                {
                    break;
                }

                left->values[left->count] = std::move(right->values[i]);
                left->count++;
                moved++;
                count--;
            }

            right->count = count;
            shift_down(right, 0, moved);

            if (left->count == asize_)
            {
                left = left->next;
            }

            if (count == 0 || left == right)
			{
                right = right->next;
			}
        }

        // If there are empty nodes at the end of the list, delete them
        while (tail_ && tail_->count == 0)
        {
            delete_node(tail_);
        }
    }

private:
    struct LNode
    { // DO NOT modify provided code
        LNode* next = nullptr;
        LNode* prev = nullptr;
        int count = 0; // number of items currently in the node
        T values[Size];
    };
    // DO NOT modify provided code
    LNode* head_;           // points to the first node
    LNode* tail_;           // points to the last node
    int size_;              // the number of items (not nodes) in the list
    mutable int nodecount_; // the number of nodes in the list
    int asize_;             // the size of the array within the nodes

    template <typename OtherT, int OtherSize>
    void copy_from(Lariat<OtherT, OtherSize> const& copy)
    {
        // typename Lariat<OtherT, OtherSize>::LNode* current = copy.head_;
        auto* current = copy.head_;
        while (current)
        {
            for (int i = 0; i < current->count; i++)
            {
                push_back(static_cast<T>(current->values[i]));
            }
            current = current->next;
        }
    }

    /**
     * @brief Find the node and the local index within that node for a given index
     *
     * @param index
     * @return std::pair<LNode*, int>
     */
    std::pair<LNode*, int> find_element(int index) const
    {
        LNode* current = head_;

        // if current is null, return the tail and the index
        if (!current)
        {
            return std::make_pair(tail_, index);
        }

        if (index == size_)
        {
            return std::make_pair(tail_, tail_->count);
        }

        while (current != nullptr)
        {
            if (index < current->count)
            {
                return std::make_pair(current, index);
            }

            index -= current->count;
            current = current->next;
        }

        throw LariatException(LariatException::E_BAD_INDEX, "Subscript is out of range");
    }

    /**
     * @brief Shift the elements of the node to the right starting at the index
     *
     * @param currentNode
     * @param index
     */
    void shift_up(LNode* currentNode, int index)
    {
        int n = currentNode->count;
        if (currentNode->count == asize_)
        {
            --n;
        }

        for (int i = index; i < n; i++)
        {
            std::swap(currentNode->values[index], currentNode->values[i + 1]);
        }
    }

    /**
     * @brief Shift the elements of the node to the left starting at the index and given an offset,
     * which is the number of elements to shift
     *
     * @param currentNode
     * @param index
     * @param offset
     */
    void shift_down(LNode* currentNode, int index, int offset)
    {
        // There's nothing to shift
        if (offset == 0)
        {
            return;
        }

        // Shift the elements of the node to the left starting at the index, up to the count minus
        // the offset
        for (int i = index; i < asize_ - offset; i++)
        {
            std::swap(currentNode->values[i], currentNode->values[i + offset]);
        }
    }

    /**
     * @brief Split the node at the index
     *
     * @param currentNode
     * @param index
     */
    void split_node(LNode* currentNode, int index)
    {
        // Make a new node
        LNode* new_node = new LNode;
        new_node->next = currentNode->next;
        new_node->prev = currentNode;
        currentNode->next = new_node;

        // If the new node isn't the last node, set the next node's previous to the new node
        if (new_node->next)
        {
            new_node->next->prev = new_node;
        }

        int count = 0;

        for (int i = index; i < currentNode->count; i++)
        {
            new_node->values[count] = std::move(currentNode->values[i]);
            count++;
        }
        new_node->count = count;
        currentNode->count = index;
        nodecount_++;

        // If the last node is split, set the tail to the new node
        if (currentNode == tail_)
        {
            tail_ = new_node;
        }
    }

    /**
     * @brief Delete a node
     *
     * @param node
     */
    void delete_node(LNode* node)
    {
        if (node->prev)
        {
            node->prev->next = node->next;
        }

        if (node->next)
        {
            node->next->prev = node->prev;
        }

        if (node == head_)
        {
            head_ = node->next;
        }

        if (node == tail_)
        {
            tail_ = node->prev;
        }

        delete node;
        nodecount_--;
    }
};

#include "lariat.cpp"

#endif // LARIAT_H

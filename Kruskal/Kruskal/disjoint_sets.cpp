#include "disjoint_sets.h"

// class Node implementation
Node::Node(size_t const& value) : _value(value), _next(NULL)
{
}

Node* Node::Next() const
{
    return _next;
}

void Node::SetNext(Node* new_next)
{
    _next = new_next;
}

size_t Node::Value() const
{
    return _value;
}

std::ostream& operator<<(std::ostream& os, Node const& node)
{
    os << "(" << node._value << " ) -> ";
    return os;
}

// class Head implementation
Head::Head() : _counter(), _first(), _last()
{
}

Head::~Head()
{
    while (_first)
    {
        Node* p_node = _first;
        _first = _first->Next();
        delete p_node;
    }
}

size_t Head::Size() const
{
    return _counter;
}

void Head::Reset()
{
    _counter = 0;
    _first = nullptr;
    _last = nullptr;
}

Node* Head::GetFirst() const
{
    return _first;
}

Node* Head::GetLast() const
{
    return _last;
}

void Head::Init(size_t value)
{
    _counter = 1;
    _first = new Node(value);
    _last = _first;
}

void Head::Join(Head* pHead2)
{
    if (pHead2 == this)
    {
        return;
    }

    _last->SetNext(pHead2->_first);
    _last = pHead2->_last;
    _counter += pHead2->_counter;
    pHead2->Reset();
}

std::ostream& operator<<(std::ostream& os, Head const& head)
{
    os << "[" << head._counter << " ] -> ";
    return os;
}

// class DisjointSets implementation
DisjointSets::DisjointSets(size_t const& capacity)
    : _size(0), _capacity(capacity), _representatives(new size_t[capacity]), _heads(new Head[capacity])
{
}

DisjointSets::~DisjointSets()
{
    delete[] _representatives;
    delete[] _heads;
}

void DisjointSets::Make()
{
    if (_size == _capacity)
    {
        return;
    }

    _representatives[_size] = _size;
    _heads[_size].Init(_size);
    ++_size;
}

void DisjointSets::Join(size_t const& id1, size_t const& id2)
{
    size_t i_rep = GetRepresentative(id1);
    size_t j_rep = GetRepresentative(id2);

    if (i_rep == j_rep)
    {
        return;
    }

    Head& i_head = _heads[i_rep];
    Head& j_head = _heads[j_rep];

    if (i_head.Size() > j_head.Size())
    {
        // Move j's set under i's representative
        _representatives[j_rep] = i_rep;
        i_head.Join(&j_head);
    }
    else
    {
        // Move i's set under j's representative
        _representatives[i_rep] = j_rep;
        j_head.Join(&i_head);
    }
}

size_t DisjointSets::GetRepresentative(size_t const& id) const
{
    if (_representatives[id] == id)
    {
        return id;
    }

    size_t rep = GetRepresentative(_representatives[id]);
    _representatives[id] = rep;
    return rep;
}

size_t DisjointSets::operator[](size_t const& id) const
{
    return GetRepresentative(id);
}

std::ostream& operator<<(std::ostream& os, DisjointSets const& ds)
{
    for (size_t i = 0; i < ds._size; ++i)
    {
        os << i << ":  ";
        Head* p_head = &ds._heads[i];
        os << *p_head;
        Node* p_node = p_head->GetFirst();
        while (p_node)
        {
            os << *p_node;
            p_node = p_node->Next();
        }
        os << "NULL (representative " << ds._representatives[i] << ")\n";
    }
    return os;
}

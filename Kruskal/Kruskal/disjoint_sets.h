/*!
  \brief  Disoint sets using quick find implementation (linked lists and
  a look-up table)

Implements:
    ctor (size)
    Make( id )      initialize
    Join( id,id )   join 2 sets
    GetRepresentative( id )

Rationale:
    elements of the set are assumed to be contiguous 0,1,2,3,....
*/

#ifndef DISJOINT_SETS_H
#define DISJOINT_SETS_H
#include <cstdlib>
#include <fstream> //ostream, istream
#include <vector>

// Node class
class Node
{
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

public:
    Node(size_t const& value);
    Node* Next() const;
    void SetNext(Node* next);
    size_t Value() const;

    friend std::ostream& operator<<(std::ostream& os, Node const& node);

private:
    size_t _value;
    Node* _next;
};

// Head class
class Head
{
    Head(const Head&) = delete;
    Head& operator=(const Head&) = delete;

public:
    Head();
    ~Head();
    size_t Size() const;
    void Reset();
    Node* GetFirst() const;
    Node* GetLast() const;
    void Init(size_t value);
    void Join(Head* pHead2);

    friend std::ostream& operator<<(std::ostream& os, Head const& head);

private:
    size_t _counter;
    Node* _first;
    Node* _last;
};

////////////////////////////////////////////////////////////
class DisjointSets
{ // you MAY modify this
public:
    DisjointSets(size_t const& capacity);
    DisjointSets(DisjointSets const&) = delete;
    DisjointSets& operator=(DisjointSets const&) = delete;
    DisjointSets(DisjointSets&&) = delete;
    DisjointSets& operator=(DisjointSets&&) = delete;
    ~DisjointSets();                    
    void Make();
    void Join(size_t const& id1, size_t const& id2);
    size_t GetRepresentative(size_t const& id) const;
    size_t operator[](size_t const& id) const;
    friend std::ostream& operator<<(std::ostream& os, DisjointSets const& ds);

private:
    size_t _size;             // current size
    size_t _capacity;         // capacity - NOT growing, provided as ctor arg
    size_t* _representatives; // look-up table ID -> representative's ID
    Head* _heads;             // lists' heads
};

#endif

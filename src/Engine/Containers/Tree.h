#pragma once
#include "Array.h"
#include "List.h"
#include "Pair.h"

namespace Seele {
template <typename KeyType, typename NodeData, typename KeyFun, typename Compare, typename Allocator> struct Tree {
  protected:
    struct Node {
        Node(Node* left, Node* right, const NodeData& nodeData) : leftChild(left), rightChild(right), data(nodeData) {}
        Node(Node* left, Node* right, NodeData&& nodeData) : leftChild(left), rightChild(right), data(std::move(nodeData)) {}
        Node* leftChild;
        Node* rightChild;
        NodeData data;
    };
    using NodeAlloc = std::allocator_traits<Allocator>::template rebind_alloc<Node>;

  public:
    template <typename IterType> class IteratorBase {
      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = IterType;
        using difference_type = std::ptrdiff_t;
        using reference = IterType&;
        using pointer = IterType*;

        constexpr IteratorBase(Node* x = nullptr, Array<Node*>&& beginIt = Array<Node*>()) : node(x), traversal(std::move(beginIt)) {}
        constexpr IteratorBase(const IteratorBase& i) : node(i.node), traversal(i.traversal) {}
        constexpr IteratorBase(IteratorBase&& i) noexcept : node(std::move(i.node)), traversal(std::move(i.traversal)) {}
        constexpr IteratorBase& operator=(const IteratorBase& other) {
            if (this != &other) {
                node = other.node;
                traversal = other.traversal;
            }
            return *this;
        }
        constexpr IteratorBase& operator=(IteratorBase&& other) noexcept {
            if (this != &other) {
                node = std::move(other.node);
                traversal = std::move(other.traversal);
            }
            return *this;
        }
        constexpr reference operator*() const { return node->data; }
        constexpr pointer operator->() const { return &(node->data); }
        constexpr bool operator!=(const IteratorBase& other) { return node != other.node; }
        constexpr bool operator==(const IteratorBase& other) { return node == other.node; }
        constexpr std::strong_ordering operator<=>(const IteratorBase& other) { return node <=> other.node; }
        constexpr IteratorBase& operator++() {
            node = node->rightChild;
            while (node != nullptr && node->leftChild != nullptr) {
                traversal.add(node);
                node = node->leftChild;
            }
            if (node == nullptr && traversal.size() > 0) {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        constexpr IteratorBase& operator--() {
            node = node->leftChild;
            while (node != nullptr && node->rightChild != nullptr) {
                traversal.add(node);
                node = node->rightchild;
            }
            if (node == nullptr && traversal.size() > 0) {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        constexpr IteratorBase operator--(int) {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
        constexpr IteratorBase operator++(int) {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }
        Node* getNode() { return node; }

      private:
        Node* node;
        Array<Node*> traversal;
    };
    using Iterator = IteratorBase<NodeData>;
    using ConstIterator = IteratorBase<const NodeData>;

    using key_type = KeyType;
    using value_type = NodeData;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr Tree() noexcept
        : alloc(NodeAlloc()), root(nullptr), beginIt(nullptr), endIt(nullptr), iteratorsDirty(true), _size(0), comp(Compare()) {}
    constexpr explicit Tree(const Compare& comp, const Allocator& alloc = Allocator()) noexcept(noexcept(Allocator()))
        : alloc(alloc), root(nullptr), beginIt(nullptr), endIt(nullptr), iteratorsDirty(true), _size(0), comp(comp) {}
    constexpr explicit Tree(const Allocator& alloc) noexcept(noexcept(Compare()))
        : alloc(alloc), root(nullptr), beginIt(nullptr), endIt(nullptr), iteratorsDirty(true), _size(0), comp(Compare()) {}
    constexpr Tree(std::initializer_list<NodeData> init)
        : alloc(NodeAlloc()), root(nullptr), beginIt(nullptr), endIt(nullptr), iteratorsDirty(true), _size(0), comp(Compare()) {
        for (const auto& it : init) {
            insert(it);
        }
    }
    constexpr Tree(const Tree& other) : alloc(other.alloc), root(nullptr), iteratorsDirty(true), _size(), comp(other.comp) {
        for (const auto& elem : other) {
            insert(elem);
        }
    }
    constexpr Tree(Tree&& other) noexcept
        : alloc(std::move(other.alloc)), root(std::move(other.root)), iteratorsDirty(true), _size(std::move(other._size)),
          comp(std::move(other.comp)) {
        other._size = 0;
    }
    constexpr ~Tree() noexcept { clear(); }
    constexpr Tree& operator=(const Tree& other) {
        if (this != &other) {
            clear();
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
                alloc = other.alloc;
            }
            for (const auto& elem : other) {
                insert(elem);
            }
            markIteratorsDirty();
        }
        return *this;
    }
    constexpr Tree& operator=(Tree&& other) noexcept {
        if (this != &other) {
            clear();
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
                alloc = std::move(other.alloc);
            }
            root = std::move(other.root);
            _size = std::move(other._size);
            comp = std::move(other.comp);
            other._size = 0;
            markIteratorsDirty();
        }
        return *this;
    }
    constexpr void clear() {
        while (_size > 0) {
            root = _remove(root, keyFun(root->data));
        }
        root = nullptr;
        markIteratorsDirty();
    }
    constexpr iterator begin() {
        if (iteratorsDirty) {
            refreshIterators();
        }
        return beginIt;
    }
    constexpr iterator end() {
        if (iteratorsDirty) {
            refreshIterators();
        }
        return endIt;
    }
    constexpr iterator begin() const {
        if (iteratorsDirty) {
            return calcBeginIterator();
        }
        return beginIt;
    }
    constexpr iterator end() const {
        if (iteratorsDirty) {
            return calcEndIterator();
        }
        return endIt;
    }
    constexpr bool empty() const { return _size == 0; }
    constexpr size_type size() const { return _size; }

  protected:
    constexpr iterator find(const key_type& key) {
        root = _splay(root, key);
        if (root == nullptr || !equal(root->data, key)) {
            return end();
        }
        return iterator(root);
    }
    constexpr iterator find(const key_type& key) const {
        Node* it = root;
        while (it != nullptr && !equal(it->data, key)) {
            if (comp(key, keyFun(it->data))) {
                it = it->leftChild;
            } else {
                it = it->rightChild;
            }
        }
        if (it == nullptr) {
            return end();
        }
        return iterator(it);
    }
    constexpr Pair<iterator, bool> insert(const NodeData& data) {
        auto [r, inserted] = _insert(root, data);
        root = r;
        return Pair<iterator, bool>(iterator(root), inserted);
    }
    constexpr Pair<iterator, bool> insert(NodeData&& data) {
        auto [r, inserted] = _insert(root, std::move(data));
        root = r;
        return Pair<iterator, bool>(iterator(root), inserted);
    }
    constexpr iterator remove(const key_type& key) {
        root = _remove(root, key);
        return iterator(root);
    }

  private:
    /*void verifyTree() {
        size_t numElems = 0;
        for (const auto& it : *this) {
            numElems++;
        }
        assert(numElems == _size);
    }*/
    void markIteratorsDirty() { iteratorsDirty = true; }
    void refreshIterators() {
        beginIt = calcBeginIterator();
        endIt = calcEndIterator();
        iteratorsDirty = false;
    }
    constexpr Iterator calcBeginIterator() const {
        Node* begin = root;
        Array<Node*> beginTraversal;
        while (begin != nullptr) {
            beginTraversal.add(begin);
            begin = begin->leftChild;
        }
        if (!beginTraversal.empty()) {
            begin = beginTraversal.back();
            beginTraversal.pop();
        }
        return Iterator(begin, std::move(beginTraversal));
    }
    constexpr Iterator calcEndIterator() const {
        Node* endIndex = root;
        Array<Node*> endTraversal;
        while (endIndex != nullptr) {
            endTraversal.add(endIndex);
            endIndex = endIndex->rightChild;
        }
        return Iterator(endIndex, std::move(endTraversal));
    }
    NodeAlloc alloc;
    Node* root;
    Iterator beginIt;
    Iterator endIt;
    bool iteratorsDirty;
    size_type _size;
    Compare comp;
    KeyFun keyFun = KeyFun();
    Node* rotateLeft(Node* x) {
        Node* y = x->rightChild;
        x->rightChild = y->leftChild;
        y->leftChild = x;
        return y;
    }
    Node* rotateRight(Node* x) {
        Node* y = x->leftChild;
        x->leftChild = y->rightChild;
        y->rightChild = x;
        return y;
    }
    template <class NodeDataType> Pair<Node*, bool> _insert(Node* r, NodeDataType&& data) {
        markIteratorsDirty();
        if (r == nullptr) {
            root = alloc.allocate(1);
            std::allocator_traits<NodeAlloc>::construct(alloc, root, nullptr, nullptr, std::forward<NodeDataType>(data));
            _size++;
            return Pair<Node*, bool>(root, true);
        }
        r = _splay(r, keyFun(data));

        if (equal(r->data, keyFun(data)))
            return Pair<Node*, bool>(r, false);

        Node* newNode = alloc.allocate(1);
        std::allocator_traits<NodeAlloc>::construct(alloc, newNode, nullptr, nullptr, std::forward<NodeDataType>(data));

        if (comp(keyFun(newNode->data), keyFun(r->data))) {
            newNode->rightChild = r;
            newNode->leftChild = r->leftChild;
            r->leftChild = nullptr;
        } else {
            newNode->leftChild = r;
            newNode->rightChild = r->rightChild;
            r->rightChild = nullptr;
        }
        _size++;
        return Pair<Node*, bool>(newNode, true);
    }
    template <class K> Node* _remove(Node* r, K&& key) {
        markIteratorsDirty();
        Node* temp;
        if (r == nullptr)
            return nullptr;

        r = _splay(r, key);

        if (!equal(r->data, key))
            return r;

        if (r->leftChild == nullptr) {
            temp = r;
            r = r->rightChild;
        } else {
            temp = r;

            r = _splay(r->leftChild, key);
            r->rightChild = temp->rightChild;
        }
        std::allocator_traits<NodeAlloc>::destroy(alloc, temp);
        alloc.deallocate(temp, 1);
        _size--;
        return r;
    }
    template <class K> Node* _splay(Node* r, K&& key) {
        markIteratorsDirty();
        if (r == nullptr || equal(r->data, key)) {
            return r;
        }
        if (comp(key, keyFun(r->data))) {
            if (r->leftChild == nullptr)
                return r;

            if (comp(key, keyFun(r->leftChild->data))) {
                r->leftChild->leftChild = _splay(r->leftChild->leftChild, key);

                r = rotateRight(r);
            } else if (comp(keyFun(r->leftChild->data), key)) {
                r->leftChild->rightChild = _splay(r->leftChild->rightChild, key);

                if (r->leftChild->rightChild != nullptr) {
                    r->leftChild = rotateLeft(r->leftChild);
                }
            }
            return (r->leftChild == nullptr) ? r : rotateRight(r);
        } else {
            if (r->rightChild == nullptr)
                return r;

            if (comp(key, keyFun(r->rightChild->data))) {
                r->rightChild->leftChild = _splay(r->rightChild->leftChild, key);

                if (r->rightChild->leftChild != nullptr) {
                    r->rightChild = rotateRight(r->rightChild);
                }
            } else if (comp(keyFun(r->rightChild->data), key)) {
                r->rightChild->rightChild = _splay(r->rightChild->rightChild, key);

                r = rotateLeft(r);
            }
            return (r->rightChild == nullptr) ? r : rotateLeft(r);
        }
    }
    template <typename K> bool equal(const NodeData& a, K&& b) const { return !comp(keyFun(a), b) && !comp(b, keyFun(a)); }
};
} // namespace Seele
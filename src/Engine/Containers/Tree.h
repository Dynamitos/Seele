#pragma once
#include "Array.h"
#include "Pair.h"
#include <map>
#include <memory_resource>
#include <memory>

namespace Seele {
template <typename KeyType, typename NodeData, typename KeyFun, typename Compare, typename Allocator> struct Tree {
  protected:
    struct Node {
        Node(const NodeData& data)
            : data(data)
            {}
        Node(NodeData&& data)
            : data(std::move(data)) {}

        Node* parent = nullptr;
        Node* leftChild;
        Node* rightChild;
        NodeData data;
    };
    using NodeAlloc = std::allocator_traits<Allocator>::template rebind_alloc<Node>;

  public:
    using key_type = KeyType;
    using value_type = NodeData;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    class ConstIterator {
      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = value_type;
        using difference_type = difference_type;
        using pointer = const_pointer;
        using reference = const value_type&;

        constexpr ConstIterator(Node* x = nullptr) : node(x) {}
        constexpr ConstIterator(const ConstIterator& i) = default;
        constexpr ConstIterator(ConstIterator&& i) noexcept = default;
        constexpr ConstIterator& operator=(const ConstIterator& other) = default;
        constexpr ConstIterator& operator=(ConstIterator&& other) noexcept = default;
        constexpr reference operator*() const noexcept { return node->data; }
        constexpr pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }
        constexpr bool operator==(const ConstIterator& other) { return node == other.node; }
        constexpr bool operator!=(const ConstIterator& other) { return node != other.node; }
        constexpr std::strong_ordering operator<=>(const ConstIterator& other) { return node <=> other.node; }
        constexpr ConstIterator& operator++() noexcept {
            node = node->rightChild;
            while (node != nullptr && node->leftChild != nullptr) {
                node = node->leftChild;
            }
            if (node == nullptr && node->parent != nullptr) {
                node = node->parent;
            }
            return *this;
        }
        constexpr ConstIterator& operator--() {
            node = node->leftChild;
            while (node != nullptr && node->rightChild != nullptr) {
                node = node->rightchild;
            }
            if (node == nullptr && node->parent != nullptr) {
                node = node->parent;
            }
            return *this;
        }
        constexpr ConstIterator operator--(int) {
            ConstIterator tmp(*this);
            --*this;
            return tmp;
        }
        constexpr ConstIterator operator++(int) {
            ConstIterator tmp(*this);
            ++*this;
            return tmp;
        }

        Node* getNode() { return node; }

      protected:
        Node* node;
    };

    class Iterator : public ConstIterator {
      public:
        using value_type = value_type;
        using difference_type = difference_type;
        using pointer = pointer;
        using reference = value_type&;

        constexpr Iterator(Node* x = nullptr) : ConstIterator(x) {}
        constexpr Iterator(const Iterator& i) = default;
        constexpr Iterator(Iterator&& i) noexcept = default;
        constexpr Iterator& operator=(const Iterator& other) = default;
        constexpr Iterator& operator=(Iterator&& other) noexcept = default;
        constexpr reference operator*() const noexcept { return const_cast<reference>(ConstIterator::operator*()); }
        constexpr pointer operator->() const noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }
        constexpr Iterator& operator++() noexcept {
            ConstIterator::operator++();
            return *this;
        }
        constexpr Iterator& operator--() noexcept {
            ConstIterator::operator--();
            return *this;
        }
        constexpr Iterator& operator++(int) noexcept {
            Iterator tmp = *this;
            ConstIterator::operator++();
            return tmp;
        }
        constexpr Iterator& operator--(int) noexcept {
            Iterator tmp = *this;
            ConstIterator::operator--();
            return tmp;
        }
    };

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
        const auto& [r, inserted] = _insert(root, data);
        root = r;
        return Pair<iterator, bool>(iterator(root), inserted);
    }
    constexpr Pair<iterator, bool> insert(NodeData&& data) {
        const auto& [r, inserted] = _insert(root, std::move(data));
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
        while (begin != nullptr) {
            begin = begin->leftChild;
        }
        if (begin->parent != nullptr) {
            begin = begin->parent;
        }
        return Iterator(begin);
    }
    constexpr Iterator calcEndIterator() const {
        Node* endNode = root;
        while (endNode != nullptr) {
            endNode = endNode->rightChild;
        }
        return Iterator(endNode);
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
        x->rightChild->parent = x;
        y->leftChild = x;
        y->leftChild->parent = y;
        return y;
    }
    Node* rotateRight(Node* x) {
        Node* y = x->leftChild;
        x->leftChild = y->rightChild;
        x->leftChild->parent = x;
        y->rightChild = x;
        y->rightChild->parent = y;
        return y;
    }
    template <class NodeDataType> Pair<Node*, bool> _insert(Node* r, NodeDataType&& data) {
        markIteratorsDirty();
        if (r == nullptr) {
            root = alloc.allocate(1);
            std::allocator_traits<NodeAlloc>::construct(alloc, root, std::forward<NodeDataType>(data));
            _size++;
            return Pair<Node*, bool>(root, true);
        }
        r = _splay(r, keyFun(data));

        if (equal(r->data, keyFun(data)))
            return Pair<Node*, bool>(r, false);

        Node* newNode = alloc.allocate(1);
        std::allocator_traits<NodeAlloc>::construct(alloc, newNode, std::forward<NodeDataType>(data));

        if (comp(keyFun(newNode->data), keyFun(r->data))) {
            newNode->rightChild = r;
            newNode->rightChild->parent = newNode;
            newNode->leftChild = r->leftChild;
            newNode->leftChild->parent = newNode;
            r->leftChild = nullptr;
        } else {
            newNode->leftChild = r;
            newNode->leftChild->parent = newNode;
            newNode->rightChild = r->rightChild;
            newNode->rightChild->parent = newNode;
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
            r->rightChild->parent = r;
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
                r->leftChild->leftChild->parent = r->leftChild;

                r = rotateRight(r);
            } else if (comp(keyFun(r->leftChild->data), key)) {
                r->leftChild->rightChild = _splay(r->leftChild->rightChild, key);
                r->leftChild->rightChild->parent = r->leftChild;

                if (r->leftChild->rightChild != nullptr) {
                    r->leftChild = rotateLeft(r->leftChild);
                    r->leftChild->parent = r;
                }
            }
            return (r->leftChild == nullptr) ? r : rotateRight(r);
        } else {
            if (r->rightChild == nullptr)
                return r;

            if (comp(key, keyFun(r->rightChild->data))) {
                r->rightChild->leftChild = _splay(r->rightChild->leftChild, key);
                r->rightChild->leftChild->parent = r->rightChild;

                if (r->rightChild->leftChild != nullptr) {
                    r->rightChild = rotateRight(r->rightChild);
                    r->rightChild->parent = r;
                }
            } else if (comp(keyFun(r->rightChild->data), key)) {
                r->rightChild->rightChild = _splay(r->rightChild->rightChild, key);
                r->rightChild->rightChild->parent = r->rightChild;

                r = rotateLeft(r);
            }
            return (r->rightChild == nullptr) ? r : rotateLeft(r);
        }
    }
    template <typename K> bool equal(const NodeData& a, K&& b) const { return !comp(keyFun(a), b) && !comp(b, keyFun(a)); }
};
} // namespace Seele
#pragma once
#include "Array.h"
#include "List.h"
#include "Pair.h"

namespace Seele {
template <typename KeyType, typename NodeData, typename KeyFun, typename Compare, typename Allocator> struct Tree {
  protected:
    struct Node {
        Node() {}
        Node(Node* left, Node* right, const NodeData& nodeData) : leftChild(left), rightChild(right), data(nodeData) {}
        Node(Node* left, Node* right, NodeData&& nodeData) : leftChild(left), rightChild(right), data(std::move(nodeData)) {}
        Node* leftChild = nullptr;
        Node* rightChild = nullptr;
        Node* parent = nullptr;
        NodeData data;
        void setLeftChild(Node* child) {
            leftChild = child;
            if (child != nullptr) {
                child->parent = this;
            }
        }
        void setRightChild(Node* child) {
            rightChild = child;
            if (child != nullptr) {
                child->parent = this;
            }
        }
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

        constexpr IteratorBase(Node* x = nullptr) : node(x) {}
        constexpr IteratorBase(const IteratorBase& i) : node(i.node) {}
        constexpr IteratorBase(IteratorBase&& i) noexcept : node(std::move(i.node)) {}
        constexpr IteratorBase& operator=(const IteratorBase& other) {
            if (this != &other) {
                node = other.node;
            }
            return *this;
        }
        constexpr IteratorBase& operator=(IteratorBase&& other) noexcept {
            if (this != &other) {
                node = std::move(other.node);
            }
            return *this;
        }
        constexpr reference operator*() const { return node->data; }
        constexpr pointer operator->() const { return std::pointer_traits<pointer>::pointer_to(**this); }
        constexpr bool operator!=(const IteratorBase& other) { return node != other.node; }
        constexpr bool operator==(const IteratorBase& other) { return node == other.node; }
        constexpr std::strong_ordering operator<=>(const IteratorBase& other) { return node <=> other.node; }
        constexpr IteratorBase& operator++() {
            // if current node has no right subtree
            if (node->rightChild == nullptr) {
                if (node->parent != nullptr) {
                    Node* temp = node;
                    node = node->parent;
                    // walk up hierarchy until we were the left subtree of any parent
                    // this means that that parent is the correct next element
                    while (node->rightChild == temp) {
                        temp = node;
                        node = node->parent;
                    }
                }
            } else {
                // if there is a right subtree, descend there
                node = node->rightChild;
                // and find the leftmost node in that tree
                while (node->leftChild != nullptr) {
                    node = node->leftChild;
                }
            }
            return *this;
        }
        constexpr IteratorBase& operator--() {
            // if current node has no left subtree
            if (node->leftChild == nullptr) {
                Node* temp = node;
                node = node->parent;
                // walk up hierarchy until we were the right subtree of any parent
                // this means that that parent is the correct next element
                while (node->leftChild == temp) {
                    temp = node;
                    node = node->parent;
                }
            } else {
                // if there is a left subtree, descend there
                node = node->leftChild;
                // and find the rightmost node in that tree
                while (node->rightChild != nullptr) {
                    node = node->rightChild;
                }
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
        Node* getNode() const { return node; }

      private:
        Node* node;
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
        : alloc(std::move(other.alloc)), root(std::move(other.root)), pseudoRoot(std::move(other.pseudoRoot)), iteratorsDirty(true),
          _size(std::move(other._size)), comp(std::move(other.comp)) {
        pseudoRoot.setLeftChild(root);
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
            pseudoRoot.setLeftChild(root);
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
            pseudoRoot = std::move(other.pseudoRoot);
            _size = std::move(other._size);
            comp = std::move(other.comp);
            other._size = 0;
            pseudoRoot.setLeftChild(root);
            markIteratorsDirty();
        }
        return *this;
    }
    constexpr void clear() {
        while (_size > 0) {
            root = _remove(root, keyFun(root->data));
        }
        root = nullptr;
        pseudoRoot.setLeftChild(nullptr);
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
        pseudoRoot.setLeftChild(root);
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
        pseudoRoot.setLeftChild(root);
        return Pair<iterator, bool>(iterator(root), inserted);
    }
    constexpr Pair<iterator, bool> insert(NodeData&& data) {
        auto [r, inserted] = _insert(root, std::move(data));
        root = r;
        pseudoRoot.setLeftChild(root);
        return Pair<iterator, bool>(iterator(root), inserted);
    }
    constexpr iterator remove(const key_type& key) {
        root = _remove(root, key);
        pseudoRoot.setLeftChild(root);
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
        // start at pseudoroot so that if no regular nodes exist begin == end
        Node* begin = const_cast<Node*>(&pseudoRoot);
        while (begin != nullptr && begin->leftChild != nullptr) {
            begin = begin->leftChild;
        }
        return Iterator(begin);
    }
    constexpr Iterator calcEndIterator() const { return Iterator(const_cast<Node*>(&pseudoRoot)); }
    NodeAlloc alloc;
    Node* root;
    // where the end iterator points to
    Node pseudoRoot;
    Iterator beginIt;
    Iterator endIt;
    bool iteratorsDirty;
    size_type _size;
    Compare comp;
    KeyFun keyFun = KeyFun();
    Node* rotateLeft(Node* x) {
        Node* y = x->rightChild;
        x->setRightChild(y->leftChild);
        y->setLeftChild(x);
        return y;
    }
    Node* rotateRight(Node* x) {
        Node* y = x->leftChild;
        x->setLeftChild(y->rightChild);
        y->setRightChild(x);
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
            newNode->setRightChild(r);
            newNode->setLeftChild(r->leftChild);
            r->setLeftChild(nullptr);
        } else {
            newNode->setLeftChild(r);
            newNode->setRightChild(r->rightChild);
            r->setRightChild(nullptr);
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
            r->setRightChild(temp->rightChild);
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
                r->leftChild->setLeftChild(_splay(r->leftChild->leftChild, key));

                r = rotateRight(r);
            } else if (comp(keyFun(r->leftChild->data), key)) {
                r->leftChild->setRightChild(_splay(r->leftChild->rightChild, key));

                if (r->leftChild->rightChild != nullptr) {
                    r->leftChild = rotateLeft(r->leftChild);
                }
            }
            return (r->leftChild == nullptr) ? r : rotateRight(r);
        } else {
            if (r->rightChild == nullptr)
                return r;

            if (comp(key, keyFun(r->rightChild->data))) {
                r->rightChild->setLeftChild(_splay(r->rightChild->leftChild, key));

                if (r->rightChild->leftChild != nullptr) {
                    r->setRightChild(rotateRight(r->rightChild));
                }
            } else if (comp(keyFun(r->rightChild->data), key)) {
                r->rightChild->setRightChild(_splay(r->rightChild->rightChild, key));

                r = rotateLeft(r);
            }
            return (r->rightChild == nullptr) ? r : rotateLeft(r);
        }
    }
    template <typename K> bool equal(const NodeData& a, K&& b) const { return !comp(keyFun(a), b) && !comp(b, keyFun(a)); }
};
} // namespace Seele
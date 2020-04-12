#pragma once
#include "Array.h"

namespace Seele
{
template <typename K, typename V>
struct Pair
{
public:
	Pair()
		: key(K()), value(V())
	{
	}
	Pair(K key, V value)
		: key(key), value(value)
	{
	}
	K key;
	V value;
};
template <typename K, typename V>
struct Map
{
private:
	struct Node
	{
		Pair<K, V> pair;
		Node *leftChild;
		Node *rightChild;
		Node(const K &key)
			: leftChild(nullptr), rightChild(nullptr), pair(key, V())
		{
		}
		Node()
			: leftChild(nullptr), rightChild(nullptr), pair(K(), V())
		{
		}
		~Node()
		{
			if (leftChild != nullptr)
			{
				delete leftChild;
			}
			if (rightChild != nullptr)
			{
				delete rightChild;
			}
		}
	};

public:
	Map()
		: root(nullptr), _size(0)
	{
	}
	~Map()
	{
		delete root;
	}
	class Iterator
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef Pair<K, V> value_type;
		typedef std::ptrdiff_t difference_type;
		typedef Pair<K, V> &reference;
		typedef Pair<K, V> *pointer;

		Iterator(Node *x = nullptr)
			: node(x)
		{
		}
		Iterator(Node *x, Array<Node *> &&beginIt)
			: node(x), traversal(std::move(beginIt))
		{
		}
		Iterator(const Iterator &i)
			: node(i.node), traversal(i.traversal)
		{
		}
		reference operator*() const
		{
			return node->pair;
		}
		pointer operator->() const
		{
			return &node->pair;
		}
		inline bool operator!=(const Iterator &other)
		{
			return node != other.node;
		}
		inline bool operator==(const Iterator &other)
		{
			return node == other.node;
		}
		Iterator &operator++()
		{
			node = node->rightChild;
			while (node != nullptr && node->leftChild != nullptr)
			{
				traversal.add(node);
				node = node->leftChild;
			}
			if (node == nullptr && traversal.size() > 0)
			{
				node = traversal.back();
				traversal.pop();
			}
			return *this;
		}
		Iterator &operator--()
		{
			node = node->leftChild;
			while (node != nullptr && node->rightchild != nullptr)
			{
				traversal.add(node);
				node = node->rightChild;
			}
			if (node == nullptr && traversal.size() > 0)
			{
				node = traversal.back();
				traversal.pop();
			}
			return *this;
		}
		Iterator operator--(int)
		{
			Iterator tmp(*this);
			++*this;
			return tmp;
		}
		Iterator operator++(int)
		{
			Iterator tmp(*this);
			++*this;
			return tmp;
		}

	private:
		Node *node;
		Array<Node *> traversal;
	};
	V &operator[](const K &key)
	{
		root = splay(root, key);
		if (root == nullptr || root->pair.key < key || key < root->pair.key)
		{
			root = insert(root, key);
			_size++;
		}
		refreshIterators();
		return root->pair.value;
	}
	Iterator find(const K &key)
	{
		root = splay(root, key);
		if (root == nullptr || root->pair.key != key)
		{
			return endIt;
		}
		return Iterator(root);
	}
	Iterator erase(const K &key)
	{
		root = remove(root, key);
		refreshIterators();
		return Iterator(root);
	}
	void clear()
	{
		delete root;
		root = nullptr;
		_size = 0;
		refreshIterators();
	}
	bool exists(const K &key)
	{
		return find(key) != endIt;
	}
	Iterator begin() const
	{
		return beginIt;
	}
	Iterator end() const
	{
		return endIt;
	}
	bool empty() const
	{
		return root == nullptr;
	}
	uint32 size() const
	{
		return _size;
	}

private:
	void refreshIterators()
	{
		Node *beginNode = root;
		if (root == nullptr)
		{
			beginIt = Iterator(nullptr);
		}
		else
		{
			Array<Node *> beginTraversal;
			while (beginNode != nullptr)
			{
				beginTraversal.add(beginNode);
				beginNode = beginNode->leftChild;
			}
			beginNode = beginTraversal.back();
			beginTraversal.pop();
			beginIt = Iterator(beginNode, std::move(beginTraversal));
		}
		Node *endNode = root;
		if (root == nullptr)
		{
			endIt = Iterator(nullptr);
		}
		else
		{
			Array<Node *> endTraversal;
			while (endNode != nullptr)
			{
				endTraversal.add(endNode);
				endNode = endNode->rightChild;
			}
			endIt = Iterator(endNode, std::move(endTraversal));
		}
	}
	Node *root;
	Iterator beginIt;
	Iterator endIt;
	uint32 _size;
	Node *rotateRight(Node *node)
	{
		Node *y = node->leftChild;
		node->leftChild = y->rightChild;
		y->rightChild = node;
		return y;
	}
	Node *rotateLeft(Node *node)
	{
		Node *y = node->rightChild;
		node->rightChild = y->leftChild;
		y->leftChild = node;
		return y;
	}
	Node *makeNode(const K &key)
	{
		return new Node(key);
	}
	Node *insert(Node *r, const K &key)
	{
		if (r == nullptr)
			return makeNode(key);

		r = splay(r, key);

		if (!(r->pair.key < key || key < r->pair.key))
			return r;

		Node *newNode = makeNode(key);

		if (key < r->pair.key)
		{
			newNode->rightChild = r;
			newNode->leftChild = r->leftChild;
			r->leftChild = nullptr;
		}
		else
		{
			newNode->leftChild = r;
			newNode->rightChild = r->rightChild;
			r->rightChild = nullptr;
		}
		return newNode;
	}
	Node *remove(Node *r, const K &key)
	{
		Node *temp;
		if (!r)
			return nullptr;

		r = splay(r, key);

		if (r->pair.key < key || key < r->pair.key)
			return r;

		if (!r->leftChild)
		{
			temp = r;
			r = r->rightChild;
		}
		else
		{
			temp = r;

			r = splay(r->leftChild, key);
			r->rightChild = temp->rightChild;
		}
		temp->leftChild = nullptr;
		temp->rightChild = nullptr;
		_size--;
		delete temp;
		return r;
	}
	Node *splay(Node *r, const K &key)
	{
		if (r == nullptr || !(r->pair.key < key || key < r->pair.key))
		{
			return r;
		}

		if (key < r->pair.key)
		{
			if (r->leftChild == nullptr)
				return r;

			if (key < r->leftChild->pair.key)
			{
				r->leftChild->leftChild = splay(r->leftChild->leftChild, key);

				r = rotateRight(r);
			}
			else if (r->leftChild->pair.key < key)
			{
				r->leftChild->rightChild = splay(r->leftChild->rightChild, key);

				if (r->leftChild->rightChild != nullptr)
				{
					r->leftChild = rotateLeft(r->leftChild);
				}
			}
			return (r->leftChild == nullptr) ? r : rotateRight(r);
		}
		else
		{
			if (r->rightChild == nullptr)
				return r;

			if (key < r->rightChild->pair.key)
			{
				r->rightChild->leftChild = splay(r->rightChild->leftChild, key);

				if (r->rightChild->leftChild != nullptr)
				{
					r->rightChild = rotateRight(r->rightChild);
				}
			}
			else if (r->rightChild->pair.key < key)
			{
				r->rightChild->rightChild = splay(r->rightChild->rightChild, key);
				r = rotateLeft(r);
			}
			return (r->rightChild == nullptr) ? r : rotateLeft(r);
		}
	}
};
} // namespace Seele
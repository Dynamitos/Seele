#pragma once
#include "Array.h"

namespace Seele
{
	template<typename K, typename V>
	struct Pair
	{
	public:
		Pair()
			: key(K())
			, value(V())
		{}
		Pair(K key, V value)
			: key(key)
			, value(value)
		{}
		K key;
		V value;
	};
	template<typename K, typename V>
	struct Map
	{
	private:
		struct Node
		{
			Pair<K, V> pair;
			Node* leftChild;
			Node* rightChild;
			Node(const K& key)
				: leftChild(nullptr)
				, rightChild(nullptr)
				, pair(key, V())
			{}
			Node()
				: leftChild(nullptr)
				, rightChild(nullptr)
				, pair(K(), V())
			{}
			~Node()
			{
				if(leftChild != nullptr)
				{
					delete leftChild;
				}
				if(rightChild != nullptr)
				{
					delete rightChild;
				}
			}
		};

	public:
		Map()
			: root(nullptr)
		{}
		~Map()
		{
			delete root;
		}
		class Iterator {
		public:
			typedef std::bidirectional_iterator_tag iterator_category;
			typedef Pair<K, V> value_type;
			typedef std::ptrdiff_t difference_type;
			typedef Pair<K, V>& reference;
			typedef Pair<K, V>* pointer;

			Iterator(Node* x = nullptr)
				: node(x)
			{}
			Iterator(Node* x, Array<Node*>&& beginIt)
				: node(x)
				, traversal(std::move(beginIt))
			{}
			Iterator(const Iterator& i)
				: node(i.node)
				, traversal(i.traversal)
			{}
			reference operator*() const
			{
				return node->pair;
			}
			pointer operator->() const
			{
				return &node->pair;
			}
			inline bool operator!=(const Iterator& other)
			{
				return node != other.node;
			}
			inline bool operator==(const Iterator& other)
			{
				return node == other.node;
			}
			Iterator& operator++() {
				node = node->rightChild;
				while(node != nullptr && node->leftChild != nullptr)
				{
					traversal.add(node);
					node = node->leftChild;
				}
				if(node == nullptr && traversal.size() > 0)
				{
					node = traversal.back();
					traversal.pop();
				}
				return *this;
			}
			Iterator& operator--() {
				node = node->leftChild;
				while(node != nullptr && node->rightchild != nullptr)
				{
					traversal.add(node);
					node = node->rightChild;
				}
				if(node == nullptr && traversal.size() > 0)
				{
					node = traversal.back();
					traversal.pop();
				}
				return *this;
			}
			Iterator operator--(int) {
				Iterator tmp(*this);
				++* this;
				return tmp;
			}
			Iterator operator++(int) {
				Iterator tmp(*this);
				++* this;
				return tmp;
			}
		private:
			Node* node;
			Array<Node*> traversal;
		};
		V& operator[](const K& key)
		{
			root = splay(root, key);
			if (root == nullptr || root->pair.key < key || key < root->pair.key)
			{
				root = insert(root, key);
			}
			refreshIterators();
			return root->pair.value;
		}
		Iterator find(const K& key)
		{
			root = splay(root, key);
			if (root == nullptr || root->pair.key != key)
			{
				return endIt;
			}
			return Iterator(root);
		}
		Iterator erase(const K& key)
		{
			root = remove(root, key);
			refreshIterators();
			return Iterator(root);
		}
		bool exists(const K& key)
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
	private:
		void refreshIterators()
		{
			Node* beginNode = root;
			if(root == nullptr)
			{
				beginIt = Iterator(nullptr);
			}
			else
			{
				Array<Node*> beginTraversal;
				while(beginNode != nullptr)
				{	
					beginTraversal.add(beginNode);
					beginNode = beginNode->leftChild;
				}
				beginNode = beginTraversal.back();
				beginTraversal.pop();
				beginIt = Iterator(beginNode, std::move(beginTraversal));
			}
			Node* endNode = root;
			if(root == nullptr)
			{
				endIt = Iterator(nullptr);
			}
			else
			{
				Array<Node*> endTraversal;
				while(endNode != nullptr)
				{
					endTraversal.add(endNode);
					endNode = endNode->rightChild;
				}
				endIt = Iterator(endNode, std::move(endTraversal));
			}
		}
		Node* root;
		Iterator beginIt;
		Iterator endIt;
		Node* rotateRight(Node* node)
		{
			Node* y = node->leftChild;
			node->leftChild = y->rightChild;
			y->rightChild = node;
			return y;
		}
		Node* rotateLeft(Node* node)
		{
			Node* y = node->rightChild;
			node->rightChild = y->leftChild;
			y->leftChild = node;
			return y;
		}
		Node* makeNode(const K& key)
		{
			return new Node(key);
		}
		Node* insert(Node* root, const K& key)
		{
			if (root == nullptr) return makeNode(key);

			root = splay(root, key);

			if (!(root->pair.key < key || key < root->pair.key)) return root;

			Node* newNode = makeNode(key);

			if (key < root->pair.key)
			{
				newNode->rightChild = root;
				newNode->leftChild = root->leftChild;
				root->leftChild = nullptr;
			}
			else
			{
				newNode->leftChild = root;
				newNode->rightChild = root->rightChild;
				root->rightChild = nullptr;
			}
			return newNode;
		}
		Node* remove(Node* root, const K& key)
		{
			Node* temp;
			if (!root)
				return nullptr;

			root = splay(root, key);

			if (root->pair.key < key || key < root->pair.key)
				return root;

			if (!root->leftChild)
			{
				temp = root;
				root = root->rightChild;
			}
			else
			{
				temp = root;

				root = splay(root->leftChild, key);
				root->rightChild = temp->rightChild;
			}
			temp->leftChild = nullptr;
			temp->rightChild = nullptr;
			delete temp;
			return root;
		}
		Node* splay(Node* root, const K& key)
		{
			if (root == nullptr || !(root->pair.key < key || key < root->pair.key))
			{
				return root;
			}

			if (key < root->pair.key)
			{
				if (root->leftChild == nullptr)
					return root;

				if (key < root->leftChild->pair.key)
				{
					root->leftChild->leftChild = splay(root->leftChild->leftChild, key);

					root = rotateRight(root);
				}
				else if (root->leftChild->pair.key < key)
				{
					root->leftChild->rightChild = splay(root->leftChild->rightChild, key);

					if (root->leftChild->rightChild != nullptr)
					{
						root->leftChild = rotateLeft(root->leftChild);
					}
				}
				return (root->leftChild == nullptr) ? root : rotateRight(root);
			}
			else
			{
				if (root->rightChild == nullptr)
					return root;

				if (key < root->rightChild->pair.key)
				{
					root->rightChild->leftChild = splay(root->rightChild->leftChild, key);

					if (root->rightChild->leftChild != nullptr)
					{
						root->rightChild = rotateRight(root->rightChild);
					}
				}
				else if (root->rightChild->pair.key < key)
				{
					root->rightChild->rightChild = splay(root->rightChild->rightChild, key);
					root = rotateLeft(root);
				}
				return (root->rightChild == nullptr) ? root : rotateLeft(root);
			}
		}
	};
}
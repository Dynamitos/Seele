#pragma once
#include "MinimalEngine.h"
namespace Seele
{
template <typename T>
class List
{
private:
	struct Node
	{
		Node *prev;
		Node *next;
		T data;
	};

public:
	List()
	{
		root = nullptr;
		tail = nullptr;
		beginIt = Iterator(root);
		endIt = Iterator(tail);
		_size = 0;
	}
	List(const List& other)
	{
		//TODO: improve
		for(const auto& it : other)
		{
			add(it);
		}
	}
	List(List&& other)
		: root(std::move(other.root))
		, tail(std::move(other.tail))
		, beginIt(std::move(other.beginIt))
		, endIt(std::move(other.endIt))
		, _size(std::move(other._size))
	{
	}
	~List()
	{
		clear();
	}
	List& operator=(const List& other)
	{
		if(this != &other)
		{
			if(root != nullptr)
			{
				delete root;
			}
			if(tail != nullptr)
			{
				delete tail;
			}
			_size = 0;
			for(const auto& it : other)
			{
				add(it);
			}
		}
		return *this;
	}
	List& operator=(List&& other)
	{
		if(this != &other)
		{
			if(root != nullptr)
			{
				clear();
			}
			root = other.root;
			tail = other.tail;
			beginIt = other.beginIt;
			endIt = other.endIt;
			_size = other._size;
		}
		return *this;
	}
	template <typename X>
	class IteratorBase
	{
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef X value_type;
		typedef std::ptrdiff_t difference_type;
		typedef X &reference;
		typedef X *pointer;

		IteratorBase(Node *x = nullptr)
			: node(x)
		{
		}
		IteratorBase(const IteratorBase &i)
			: node(i.node)
		{
		}
		IteratorBase(IteratorBase&& i)
			: node(std::move(i.node))
		{
		}
		~IteratorBase()
		{
		}
		IteratorBase& operator=(const IteratorBase& other)
		{
			if(this != &other)
			{
				node = other.node;
			}
			return *this;
		}
		IteratorBase& operator=(IteratorBase&& other)
		{
			if(this != &other)
			{
				node = std::move(other.node);
			}
			return *this;
		}
		reference operator*() const
		{
			return node->data;
		}
		pointer operator->() const
		{
			return &node->data;
		}
		inline bool operator!=(const IteratorBase &other)
		{
			return node != other.node;
		}
		inline bool operator==(const IteratorBase &other)
		{
			return node == other.node;
		}
		IteratorBase &operator--()
		{
			node = node->prev;
			return *this;
		}
		IteratorBase operator--(int)
		{
			IteratorBase tmp(*this);
			--*this;
			return tmp;
		}
		IteratorBase &operator++()
		{
			node = node->next;
			return *this;
		}
		IteratorBase operator++(int)
		{
			IteratorBase tmp(*this);
			++*this;
			return tmp;
		}

	private:
		Node *node;
		friend class List<T>;
	};
	typedef IteratorBase<T> Iterator;
	typedef IteratorBase<const T> ConstIterator;

	T &front()
	{
		return root->data;
	}
	T &back()
	{
		return tail->prev->data;
	}
	void clear()
	{
		if (empty())
		{
			return;
		}
		for (Node *tmp = root; tmp != tail;)
		{
			tmp = tmp->next;
			delete tmp->prev;
		}
		delete tail;
		tail = nullptr;
		root = nullptr;
	}
	//Insert at the end
	Iterator add(const T &value)
	{
		if (root == nullptr)
		{
			root = new Node();
			tail = root;
		}
		tail->data = value;
		Node *newTail = new Node();
		newTail->prev = tail;
		newTail->next = nullptr;
		tail->next = newTail;
		Iterator insertedElement(tail);
		tail = newTail;
		refreshIterators();
		_size++;
		return insertedElement;
	}
	Iterator add(T&& value)
	{
		if (root == nullptr)
		{
			root = new Node();
			tail = root;
		}
		tail->data = std::move(value);
		Node *newTail = new Node();
		newTail->prev = tail;
		newTail->next = nullptr;
		tail->next = newTail;
		Iterator insertedElement(tail);
		tail = newTail;
		refreshIterators();
		_size++;
		return insertedElement;
	}
	Iterator remove(Iterator pos)
	{
		_size--;
		Node *prev = pos.node->prev;
		Node *next = pos.node->next;
		if (prev == nullptr)
		{
			root = next;
		}
		else
		{
			prev->next = next;
		}
		if(next == nullptr)
		{
			root = prev;
		}
		else
		{
			next->prev = prev;
		}
		delete pos.node;
		refreshIterators();
		return Iterator(next);
	}
	Iterator insert(Iterator pos, const T &value)
	{
		_size++;
		if (root == nullptr)
		{
			root = new Node();
			root->data = value;
			tail = new Node();
			root->next = tail;
			root->prev = nullptr;
			tail->prev = root;
			tail->next = nullptr;
			refreshIterators();
			return beginIt;
		}
		Node *tmp = pos.node->prev;
		Node *newNode = new Node();
		newNode->data = value;
		tmp->next = newNode;
		newNode->prev = tmp;
		newNode->next = pos.node;
		pos.node->prev = newNode;
		return Iterator(newNode);
	}
	Iterator find(const T &value)
	{
		for (Node *i = root; i != tail; i = i->next)
		{
			if (!(i->data < value) && !(value < i->data))
			{
				return Iterator(i);
			}
		}
		return endIt;
	}
	bool empty()
	{
		return _size == 0;
	}
	uint32 size()
	{
		return _size;
	}
	Iterator begin()
	{
		return beginIt;
	}
	const Iterator begin() const
	{
		return beginIt;
	}
	Iterator end()
	{
		return endIt;
	}
	const Iterator end() const
	{
		return endIt;
	}

private:
	void refreshIterators()
	{
		beginIt = Iterator(root);
		endIt = Iterator(tail);
	}
	Node *root;
	Node *tail;
	Iterator beginIt;
	Iterator endIt;
	uint32 _size;
};
} // namespace Seele
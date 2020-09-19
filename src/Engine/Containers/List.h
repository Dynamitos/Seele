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
		size = 0;
	}
	~List()
	{
		clear();
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
		size++;
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
		size++;
		return insertedElement;
	}
	Iterator remove(Iterator pos)
	{
		size--;
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
		size++;
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
		return size == 0;
	}
	uint32 length()
	{
		return size;
	}
	Iterator begin()
	{
		return beginIt;
	}
	Iterator end()
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
	uint32 size;
};
} // namespace Seele
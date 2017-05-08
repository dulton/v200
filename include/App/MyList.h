#ifndef _MY_LIST_H_
#define _MY_LIST_H_
typedef void* POS;

template <class T>
class CMyList  
{
typedef struct _node
{
	T  node;
	_node *pNext;
	_node *pPrev;
}
NODE;
public:
	POS			AddTail(const T &node);
	POS			AddHead(const T &node);
	T			RemoveTail();
	T			RemoveHead();
	T			GetTail() const;
	T&			GetTail();
	T			GetHead() const;
	T&			GetHead();
	T			GetNext(POS &pos) const;
	T&			GetNext(POS &pos);
	T			GetPrev(POS &pos) const;
	T&			GetPrev(POS &pos);
	POS			GetHeadPosition() const;
	POS			GetTailPosition() const;
	T			GetAt(POS pos) const;
	T&			GetAt(POS pos);
	POS			InsertBefore(POS pos, const T &node);
	POS			InsertAfter(POS pos,const T &node);
	void			SetAt(POS pos,T newnode);
	POS			Find(T node,POS startAfter);
	void			RemoveAll();
	int			GetCount() const;
	bool			IsEmpty() const;
	void			RemoveAt(POS pos);
	CMyList();
	virtual ~CMyList();
private:
	NODE *m_pHead;
	NODE *m_pTail;
	int	m_Count;
};

#endif//_MY_LIST_H_

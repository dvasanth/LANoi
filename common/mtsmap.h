#pragma once
#include <map>
#include <list>
#include <string>
#include <iostream>
#include "ThreadLock.h"
using namespace std;
/**************************************
Thread safe maps
**************************************/
template <class Arg_Key,class Arg_Value>
class CMTSMap:private multimap <Arg_Key,Arg_Value>{
private:
	ThreadLock               m_ThreadLock;
	typedef multimap <Arg_Key,Arg_Value> mtsmapbase; 
public:
	using mtsmapbase::begin; 
	using mtsmapbase::end; 

	void Insert(Arg_Key key,Arg_Value Value)
	{
		pair <Arg_Key,Arg_Value> MapPair(key,Value);
		m_ThreadLock.Lock();
		insert(MapPair);
		m_ThreadLock.UnLock(); 

	}
	void Erase(Arg_Key key)
	{
		m_ThreadLock.Lock();
		erase(key);
		m_ThreadLock.UnLock(); 
	}
	void Clear()
	{
		m_ThreadLock.Lock();
		clear();
		m_ThreadLock.UnLock(); 
	}
	void EraseByValue(Arg_Value Value)
	{
		multimap<Arg_Key,Arg_Value> ::iterator	iteratorItems;

		//find the context associated with the socket
		m_ThreadLock.Lock();
		for (iteratorItems = begin();iteratorItems != end();iteratorItems++ )
		{
			if((*iteratorItems).second==Value)
			{
				erase((*iteratorItems).first);
				break;	
			}
		}
		m_ThreadLock.UnLock(); 
	}
	BOOL Get(Arg_Key key,Arg_Value &Value)
	{
		multimap<Arg_Key,Arg_Value> ::iterator	iteratorItem;
		BOOL									bExists=FALSE;

		m_ThreadLock.Lock();
		iteratorItem =  find(key);
		if(iteratorItem!=end())
		{
			Value=(*iteratorItem).second;
			bExists=TRUE;
		}
		m_ThreadLock.UnLock(); 
		return bExists;
	
	}

	Arg_Value operator[](Arg_Key key)
	{

		multimap<Arg_Key,Arg_Value> ::iterator	iteratorItem;
		m_ThreadLock.Lock();
		iteratorItem =  find(key);
		m_ThreadLock.UnLock(); 
		return (*iteratorItem).second;
	}
	BOOL IsKeyExists(Arg_Key key)
	{
		multimap<Arg_Key,Arg_Value> ::iterator	iteratorItem;
		m_ThreadLock.Lock();
		iteratorItem =  find(key);
		m_ThreadLock.UnLock(); 
		return iteratorItem==end()?FALSE:TRUE;
	}
	BOOL IsEmpty()
	{
		m_ThreadLock.Lock();
		BOOL bRet=empty();
		m_ThreadLock.UnLock(); 
		return bRet;
	}
	
	int GetSize()
	{
		int iSize;
		m_ThreadLock.Lock();
		iSize=size();
		m_ThreadLock.UnLock(); 
		return (int)iSize;
	}
	void ThreadSafeLock()
	{
		m_ThreadLock.Lock();
	}

	void ThreadSafeUnLock()
	{
		m_ThreadLock.UnLock();
	}

	Arg_Value* GetItems(DWORD &dwItemCount)
	{
		multimap<Arg_Key,Arg_Value> ::iterator	iteratorItems;
		Arg_Value*							pValues=NULL;	

		if(IsEmpty())
		{
			dwItemCount=0;
			return NULL;
		}

		//find the context associated with the socket
		m_ThreadLock.Lock();
		pValues= new Arg_Value[(int)size()];
		dwItemCount=0;
		for (iteratorItems = begin();iteratorItems != end();iteratorItems++ )
		{
			pValues[dwItemCount]=(*iteratorItems).second;
			dwItemCount++;
		}
		m_ThreadLock.UnLock(); 
		return pValues;

	}
	Arg_Value* GetItemsByKey(Arg_Key key,DWORD &dwItemCount)
	{
		multimap<Arg_Key,Arg_Value>::iterator iterator;
		pair<multimap<Arg_Key,Arg_Value>::iterator,multimap<Arg_Key,Arg_Value>::iterator> ret;
		Arg_Value*							pValues=NULL;	


		if(IsEmpty())
		{
			dwItemCount=0;
			return NULL;
		}
		pValues= new Arg_Value[GetSize() ];
		dwItemCount=0;

		m_ThreadLock.Lock();
		ret=equal_range(key);
		for (iterator=ret.first; iterator!=ret.second; ++iterator)
		{
			pValues[dwItemCount]=(*iterator).second;
			dwItemCount++;
		}
		m_ThreadLock.UnLock(); 
		return pValues;
     
	}
	Arg_Key* GetKeys(DWORD &dwItemCount)
	{
		multimap<Arg_Key,Arg_Value> ::iterator	iteratorItems;
		Arg_Key*							pKeys=NULL;	

		if(IsEmpty())
		{
			dwItemCount=0;
			return NULL;
		}
		pKeys= new Arg_Key[GetSize() ];
		dwItemCount=0;

		m_ThreadLock.Lock();
		for (iteratorItems = begin();iteratorItems != end();iteratorItems++ )
		{
			pKeys[dwItemCount]=(*iteratorItems).first;
			dwItemCount++;
		}
		m_ThreadLock.UnLock(); 
		return pKeys;
     
	}
	void FreeItems(VOID* pItems)
	{
		delete pItems;
	}
};
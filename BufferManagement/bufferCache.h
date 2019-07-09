#ifndef BUFFERCACHE_H
#define BUFFERCACHE_H

#include<string.h>
#include<iostream>
using namespace std;

struct buffer
{
    int blockNumber;
    bool free, delayedWrite;
    int data;
    struct buffer* nextHashQueue;
    struct buffer* prevHashQueue;
    struct buffer* nextFreeList;
    struct buffer* prevFreeList;

public:

    buffer(int blockNumber = 0)
    {
        this->blockNumber = blockNumber;
        free = true;
        delayedWrite = false;
        data = 100;
        nextHashQueue = NULL;
        prevHashQueue = NULL;
        nextFreeList = NULL;
        prevFreeList = NULL;
    }
};

class bufferCache
{
    buffer* head;
    buffer* tail;

public:
//Check if HashQueue or FreeList is Empty or Not
    bool isEmpty()
    {
        return (head == NULL);
    }

    buffer* getHead()
    {
        return head;
    }

    buffer* getTail()
    {
        return tail;
    }
//Find a Buffer in HashQueue in the bufferCache
    buffer* findBufferHashQueue(int blockNumber)
    {
        buffer* foundBuffer = NULL;
        if(!isEmpty())
        {
            buffer* temp = head;
            do{
                if(temp->blockNumber == blockNumber)
                {
                    foundBuffer = temp;
                    break;
                }
                temp = temp->nextHashQueue;
            }while(temp != head);
        }
        return foundBuffer;
    }

//Find a Buffer in FreeList in the bufferCache
    buffer* findBufferFreeList(int blockNumber)
    {
        buffer* foundBuffer = NULL;
        if(!isEmpty())
        {
            buffer* temp = head;
            do{
                if(temp->blockNumber == blockNumber)
                {
                    foundBuffer = temp;
                    break;
                }
                temp = temp->nextFreeList;
            }while(temp != head);
        }
        return foundBuffer;
    }

//Add Buffer at Start of the Free List in the bufferCache
    void insertBufferAtHeadFreeList(buffer* insertBuffer)
    {
        if(head == NULL)
        {
            insertBuffer->nextFreeList = insertBuffer->prevFreeList = insertBuffer;
            head = tail = insertBuffer;
        }
        else
        {
            insertBuffer->nextFreeList = head;
            insertBuffer->prevFreeList = tail;
            tail->nextFreeList = head->prevFreeList = insertBuffer;
            head = insertBuffer;
        }
    }

//Add Buffer at End of the HashQueue in bufferCache
    void insertBufferAtHashQueueTail(buffer* insertBuffer)
    {
        if(head == NULL)
        {
            insertBuffer->nextHashQueue = insertBuffer->prevHashQueue = insertBuffer;
            head = tail = insertBuffer;
        }
        else
        {
            insertBuffer->prevHashQueue = tail;
            insertBuffer->nextHashQueue = head;
            head->prevHashQueue = tail->nextHashQueue = insertBuffer;
            tail = tail->nextHashQueue;
        }
    }

//Add Buffer at End of the FreeList in bufferCache
    void insertBufferAtFreeListTail(buffer* insertBuffer)
    {
        if(head == NULL)
        {
            insertBuffer->nextFreeList = insertBuffer->prevFreeList = insertBuffer;
            head = tail = insertBuffer;
        }
        else
        {
            insertBuffer->prevFreeList = tail;
            insertBuffer->nextFreeList = head;
            head->prevFreeList = tail->nextFreeList = insertBuffer;
            tail = tail->nextFreeList;
        }
    }

//Remove Buffer from Head of the FreeList
    buffer* removeBufferFromHeadFreeList()
    {
        buffer* removed = NULL;
        if(!isEmpty())
        {
            if(head == head->nextFreeList)  //Only Buffer in the FreeList
            {
                removed = head;
                head = tail = NULL;
            }
            else
            {
                tail->nextFreeList = head->nextFreeList;
                head->nextFreeList->prevFreeList = head->prevFreeList;
                removed = head;
                head = head->nextFreeList;
            }
        }
        return removed;
    }

//Remove a particular Buffer from HashQueue in the bufferCache
    bool removeBufferHashQueue(int blockNumber)
    {
        bool removed = false;
        buffer* temp = findBufferHashQueue(blockNumber);
        if(temp != NULL)
        {
            removed = true;
            if(temp->nextHashQueue == temp){       //Only Buffer in the HashQueue in bufferCache
                head = tail = NULL;
            }
            else
            {
                if(temp == head){               //If Head is to be removed from HashQueue in bufferCache
                    head = head->nextHashQueue;
                }else if(temp == tail){         //If Tail is to be removed from HashQueue in bufferCache
                    tail = tail->prevHashQueue;
                }
                temp->prevHashQueue->nextHashQueue = temp->nextHashQueue;
                temp->nextHashQueue->prevHashQueue = temp->prevHashQueue;
            }
        }
        return removed;
    }

//Remove a particular Buffer from FreeList in the bufferCache
    bool removeBufferFreeList(int blockNumber)
    {
        bool removed = false;
        buffer* temp = findBufferFreeList(blockNumber);
        if(temp != NULL)
        {
            removed = true;
            if(temp->nextFreeList == temp){     //Only Buffer in FreeList
                head = tail = NULL;
            }
            else
            {
                if(temp == head){               //If Head is to be removed from FreeList
                    head = head->nextFreeList;
                }else if(temp == tail){         //If Tail is to be removed from FreeList
                    tail = tail->prevFreeList;
                }
                temp->prevFreeList->nextFreeList = temp->nextFreeList;
                temp->nextFreeList->prevFreeList = temp->prevFreeList;
            }
        }
        return removed;
    }

    void displayFreeList()
    {
        buffer* temp = head;
        if(!isEmpty())
        {
            do{
                cout<<"{"<<temp->prevFreeList->blockNumber<<","<<temp->blockNumber<<","<<temp->nextFreeList->blockNumber<<",";
                cout<<(temp->free ? "F" : "L")<<" & "<<(temp->delayedWrite ? "DW," : "NDW,")<<temp->data<<"}\t";
                temp = temp->nextFreeList;
            }while(temp != head);
        }
        else
            cout<<"Empty List";
    }

    void displayHashQueue()
    {
        buffer* temp = head;
        if(!isEmpty())
        {
            do{
                cout<<"{"<<temp->prevHashQueue->blockNumber<<","<<temp->blockNumber<<","<<temp->nextHashQueue->blockNumber<<",";
                cout<<(temp->free ? "F" : "L")<<" & "<<(temp->delayedWrite ? "DW," : "NDW,")<<temp->data<<"}\t";
                temp = temp->nextHashQueue;
            }while(temp != head);
        }
        else
            cout<<"Empty List";
    }
};
#endif // BUFFERCACHE_H

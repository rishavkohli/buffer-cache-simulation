#ifndef GETBLK_H
#define GETBLK_H

#include "bufferCache.h"
#include<thread>
#include<chrono>
#include<condition_variable>
#include<mutex>
#include<stdlib.h>
#include<time.h>
using namespace std;

#define NUM_OF_QUEUE 4
#define NUM_OF_BUFFERS 3
#define MEM_BLOCKS 20

condition_variable anyFreeBuffer, BufferCV;
mutex mtx, myLock, bufferLock, anyBufferLock;
//bool *blockBusy;
bool specificBuffer = false, anyBuffer = false;
int *MEMORY;
bufferCache **hashQueueLists;
bufferCache *freeList;

//Creating & Initializing HashQueueLists;
void init()
{
    MEMORY = new int[MEM_BLOCKS+1];
    //blockBusy = new bool[MEM_BLOCKS+1];
    for(int iterator=0; iterator <= MEM_BLOCKS; iterator++)
    {
        MEMORY[iterator] = 100;
        //blockBusy[iterator] = false;
    }

    hashQueueLists = new bufferCache*[NUM_OF_QUEUE];
    for(int iterator = 0;iterator < NUM_OF_QUEUE;iterator++)
    {
        hashQueueLists[iterator] = new bufferCache();
    }

    //Creating & Initializing Free List
    freeList = new bufferCache();
    for(int iterator = 0;iterator < NUM_OF_BUFFERS;iterator++)
    {
        //int blockNumber = iterator + 1;
        freeList->insertBufferAtFreeListTail(new buffer());
        //buffer* node = freeList->getTail();
       // node->blockNumber = blockNumber;
        //node->data = MEMORY[blockNumber];
        //hashQueueLists[blockNumber % NUM_OF_QUEUE]->insertBufferAtHashQueueTail(node);
    }
}

int hashValue(int blockNumber)
{
    int hashValue = 0;
    hashValue = blockNumber % NUM_OF_QUEUE;
    return hashValue;
}

//Display
void display()
{
    cout<<"\nThe Format of Details is : {PreviousBlockNumber,BlockNumber,NextBlockNumber,Status,Data}\n\n";
    cout<<"HashQueue Details ";
    for(int iterator = 0;iterator < NUM_OF_QUEUE;iterator++)
    {
        cout<<"\nHashQueue "<<iterator<<" : ";
        hashQueueLists[iterator]->displayHashQueue();
    }
    cout<<"\n-----------------------------------------------------------------------------------\n";
    cout<<"\nFreeList Details \n";
    freeList->displayFreeList();
    cout<<"\n----------------------------------------------------------------------------------- \n";
    cout<<"\nMEMORY BLOCK CONTENTS \n";
    for(int iterator=1; iterator <= MEM_BLOCKS; iterator++)
    {
        cout<<MEMORY[iterator]<<" ";
    }
    cout<<"\n----------------------------------------------------------------------------------- \n";
}

void packUp()
{
    delete [] MEMORY;
    //delete [] blockBusy;
    for(int iterator=0;iterator<NUM_OF_QUEUE;iterator++)
        delete [] hashQueueLists[iterator];
    delete [] freeList;
}

void updateMemory()
{
    if(!freeList->isEmpty())
    {
        buffer* temp = freeList->getTail();
        do{
            if(temp->delayedWrite)
            {
                MEMORY[temp->blockNumber] = temp->data;
                temp->delayedWrite = false;
            }
            temp = temp->nextFreeList;
        }while(temp != freeList->getTail());
        //delete temp;
    }
}

//DelayedWrite Procedure called Asynchronously
void delayedWrite(buffer* delayedBuffer)
{
    myLock.lock();
    cout<<"\n\t\t~~~~~~~~~~~~~~~~~~~~";
    cout<<"Asynchronous Write(DelayedWrite) of Buffer Data Initiated for BlockNumber <"<<delayedBuffer->blockNumber<<">";
    cout<<"~~~~~~~~~~~~~~~~~~~~\n\n";
    myLock.unlock();
    this_thread::sleep_for(chrono::milliseconds(4000));
    myLock.lock();
    delayedBuffer->delayedWrite = false;
    delayedBuffer->free = true;
    MEMORY[delayedBuffer->blockNumber] = delayedBuffer->data;
    freeList->insertBufferAtHeadFreeList(delayedBuffer); //Adding Buffer to Start of FreeList After DelayedWrite
    cout<<"\n\t\t~~~~~~~~~~~";
    cout<<"Buffer of BlockNumber <"<<delayedBuffer->blockNumber<<"> marked for DelayedWrite added to Head of FreeList Asynchronously...";
    cout<<"~~~~~~~~~~~~~\n\n";
    myLock.unlock();
    anyBuffer = true;
    BufferCV.notify_all();
    anyFreeBuffer.notify_all();
}

// Buffer Release
bool brelse(int blockNumber)
{
    bool blockFreed = false;
    int hashQueueNumber = hashValue(blockNumber);
    buffer* block = hashQueueLists[hashQueueNumber]->findBufferHashQueue(blockNumber);

    if(block != NULL)
    {
        freeList->insertBufferAtFreeListTail(block);
        if(block->delayedWrite){
            cout<<"\t\tBuffer of BlockNumber <"<<blockNumber<<"> added to Tail of FreeList marked DelayedWrite ...\n\n";
        }
        else{
            cout<<"\t\tBuffer of BlockNumber <"<<blockNumber<<"> added to Tail of FreeList ...\n\n";
        }
        blockFreed = true;
        block->free = true;
        anyBuffer = true;
        BufferCV.notify_all();
        anyFreeBuffer.notify_all();
    }
    return blockFreed;
}

//Get Block Algorithm
buffer* getblk(int blockNumber)
{
    buffer* allocatedBuffer = NULL;
    int hashQueueNumber = hashValue(blockNumber);
    buffer* blockBuffer;
    buffer* freeBuffer;
    while(allocatedBuffer == NULL)
    {
        myLock.lock();
        //int hashQueueNumber = hashValue(blockNumber);
        blockBuffer = hashQueueLists[hashQueueNumber]->findBufferHashQueue(blockNumber);
        myLock.unlock();
        if(blockBuffer != NULL)     //Block found on HashQueue
        {
            myLock.lock();
            if(blockBuffer->free == false)   //Scenario 5: Block on HashQueue & Busy
            {
                cout<<"\n\tBuffer for BlockNumber <"<<blockNumber<<"> is Busy. Requesting Process Sleeps...\n";
                myLock.unlock();

                unique_lock<std::mutex> BufferLock(bufferLock);
                while(!blockBuffer->free)
                {
                    BufferCV.wait(BufferLock);
                }
                continue;
            }
            //Scenario 1: Block on HashQueue and is Free
            cout<<"\n\tBlock for BlockNumber <"<<blockNumber<<"> found on HashQueue and is Free as well...\n";
            blockBuffer->free = false;
            //blockBusy[blockNumber] = true;
            cout<<"\tBuffer for BlockNumber <"<<blockNumber<<"> marked Busy(Locked)...\n";
            blockBuffer->blockNumber = blockNumber;
            freeList->removeBufferFreeList(blockNumber);    //Remove Buffer from FreeList
            allocatedBuffer = blockBuffer;
            myLock.unlock();
        }
        else    //Block not on HashQueue
        {
            myLock.lock();
            if(freeList->isEmpty())     //Scenario 4: No Buffers on FreeList
            {
                cout<<"\n\tNo Free Buffer Available (FreeList is Empty). Requesting Process Sleeps...\n";
                anyBuffer = false;
                myLock.unlock();

                unique_lock<std::mutex> AnyBufferLock(anyBufferLock);
                while(!anyBuffer)
                {
                    anyFreeBuffer.wait(AnyBufferLock);
                }
                continue;
            }
            freeBuffer = freeList->removeBufferFromHeadFreeList();
            if(freeBuffer->delayedWrite == true)     //Scenario 3: Buffer on FreeList and Marked for DelayedWrite
            {
                freeBuffer->free = false;
                myLock.unlock();
                //blockBusy[freeBuffer->blockNumber] = true;
                std::thread temp(delayedWrite,freeBuffer);
                temp.detach();
                continue;
            }
            //Scenario 2: Buffer allocated from FreeList
            if(freeBuffer->blockNumber != 0)  //FreeList not in Initial State i.e. all BlockNumbers are non-zero
            {
                int tempHashQueueNumber = hashValue(freeBuffer->blockNumber);
                buffer* temp = hashQueueLists[tempHashQueueNumber]->findBufferHashQueue(freeBuffer->blockNumber);
                if(temp == NULL){
                    cout<<"\nBuffer Not Found !!!\n";
                }
                else
                {
                    if(!hashQueueLists[tempHashQueueNumber]->removeBufferHashQueue(temp->blockNumber)){
                        cout<<"\nError occurred while removing the Buffer from old HashQueue !!!\n";
                    }
                }
                //delete temp;
            }
            freeBuffer->blockNumber = blockNumber;
            freeBuffer->free = false;
            freeBuffer->delayedWrite = false;
            freeBuffer->data = MEMORY[blockNumber];
            cout<<"\n\tBuffer for BlockNumber <"<<blockNumber<<"> marked Busy(Locked)...\n";
            hashQueueLists[hashQueueNumber]->insertBufferAtHashQueueTail(freeBuffer);
            allocatedBuffer = freeBuffer;
            myLock.unlock();
        }
    }
    return allocatedBuffer;
}

#endif // GETBLK_H

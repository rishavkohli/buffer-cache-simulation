#include "bufferCache.h"
#include "getblk.h"
using namespace std;

#define REQUESTS 2
#define NO_OF_THREADS 3

void process(int processNumber, int increment)
{
    int requests = REQUESTS;
    int blockNumber;
    buffer *blockBuffer;
    string requestType;
    while(requests>0)
    {
        myLock.lock();
        cout<<"\n\nProcess "<<processNumber<<" executing ...";
        cout<<"\n\tEnter BlockNumber to be requested (from 1 to "<<MEM_BLOCKS<<" ) : ";
        cin>>blockNumber;
        cout<<"\tEnter Block Request Type Read (R or r) or Write (W or w) : ";
        cin>>requestType;
        if(requestType == "R" || requestType == "r")
        {
            cout<<"\n\tProcess "<<processNumber<<" puts a Read Request for BlockNumber <"<<blockNumber<<"> \n";
            myLock.unlock();
            blockBuffer = getblk(blockNumber);
            myLock.lock();
            cout<<"\n\tWorking on the requested BlockNumber <"<<blockNumber<<"> ...\n";
            cout<<"\tReading Contents of the BlockNumber <"<<blockNumber<<"> and it is found to be : "<<blockBuffer->data<<"\n";
            cout<<"\nCONTENTS BEFORE RELEASING BLOCK\n";
            display();
            myLock.unlock();
            this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        else
        {
            cout<<"\n\tProcess "<<processNumber<<" puts a Write Request for BlockNumber <"<<blockNumber<<"> \n";
            myLock.unlock();
            blockBuffer = getblk(blockNumber);
            myLock.lock();
            cout<<"\n\tWorking on the requested BlockNumber <"<<blockNumber<<"> ...\n";
            cout<<"\tCurrently Contents of the BlockNumber <"<<blockNumber<<"> is : "<<blockBuffer->data<<"\n";
            cout<<"\tIncrementing the Block Data by "<<increment<<" and Updating the result. \n";
            blockBuffer->data += increment;
            blockBuffer->delayedWrite = true;
            cout<<"\nCONTENTS BEFORE RELEASING BLOCK\n";
            display();
            myLock.unlock();
            this_thread::sleep_for(chrono::milliseconds(8000));
        }
        myLock.lock();
        cout<<"\n\t\tReleasing the requested Block <"<<blockNumber<<"> ...\n";
        if(!brelse(blockNumber)){
            cout<<"Error occurred while releasing the requested Block !!!\n";
        }
        cout<<"\n\nCONTENTS AFTER RELEASING BLOCK \n";
        display();
        myLock.unlock();
        requests--;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}


int main()
{
    init();
    cout<<"This program is a simulation of getblk algorithm in UNIX operating system.\n";
    cout<<"The BufferCache consists of "<<NUM_OF_BUFFERS<<" Buffers being accessed in synchronization by processes.\n";
    cout<<"Execution of the program is based on "<<NO_OF_THREADS<<" processes (lightly loaded system) simulated via threads.\n";
    cout<<"Each process requests for "<<REQUESTS<<" blocks randomly to work upon and then release them in synchronization (supposedly) with other fellow processes.\n";
    cout<<"\nNOTE :\tThe program works on threads and makes use of random numbers therefore the program may not work as expected, may fall into infinite loop,\n";
    cout<<"\tmay terminate abruptly or may result in unexpected output and error.\n";
    cout<<"\tMeasures have been taken to avoid any kind of error being occurring however the same can't be guaranteed.";
    cout<<"\n\nInitially the BufferCache is as follows \n";
    display();
    thread threads[NO_OF_THREADS];
    for(int iterator = 0; iterator < NO_OF_THREADS; iterator++)
    {
        threads[iterator] = thread(process, iterator+1, (iterator+1)*50);
        this_thread::sleep_for(chrono::milliseconds(3000));
    }
    for(int iterator = 0; iterator < NO_OF_THREADS; iterator++)
        threads[iterator].join();
    cout<<"\n\n\nFinally the BufferCache is as follows \n";
    updateMemory();
    display();
    //packUp();
    return 0;
}

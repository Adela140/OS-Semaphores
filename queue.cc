#include <iostream>
using namespace std;
#include "queue.h"

Queue::Queue(){
    // initialise both start and end to -1
    start=-1;
    end =-1;
    buffer = NULL;  
    size=0;
}

void Queue::createQueue(int _size){
    // create an array of size '_size'
    buffer = new int[_size];    
    size=_size;
}

Queue::~Queue(){
    delete [] buffer;
}

void Queue::addElement(int num){
    cout<<"In add element; size="<<size<<" start ="<<start<<" end="<<end<<endl;
    // check if the queue is full 
    // this can happen if start is at index 0 and end is at i ndex size-1 
    // OR if the queue has looped around and end is one index before start
    if((start==0 && end ==size-1)||((end==(start-1)%(size-1)))){
        printf("\nCircular queue is full");
        cout<<"QUEUE FULL: size="<<size<<" start ="<<start<<" end="<<end<<endl;
        return;
    }

    // if empty and inserting first element, change start to 0
    else{
    if(start == -1){
        start = 0;
    }
    
    // increment end and add element
     end = (end+1)%size;
     buffer[end] = num;
    }

}

int Queue::deleteElement(){
    
    // check if queue is empty
    if(start==-1){
        printf("\n Circular queue is empty");
        return -1;
    }

    int element = buffer[start];
    buffer[start]=-1;
    // if the queue has one element only, we reset start and end to -1 after
    // deleting the element 
    if(start==end){
        end = -1;
        start = -1;   
    }
    // increment the start
    else{
        start = (start+1)%size;
    }
    return element;
}

int Queue::get_start() const{
    return start;
}

int Queue::get_end() const{
    return end;
}

int Queue::get_element(int index) const{
    return buffer[index];
}
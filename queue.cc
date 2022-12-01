#include <iostream>
using namespace std;
#include "queue.h"

Queue::Queue(int _size){
    size=_size;
    element_count=0;
    // initialise both start and end to -1
    start=0;
    end =0;
    // create an array of integers
    buffer = new int[_size];
    // initialise all positions to -1 to indicate empty position
    for(int i=0; i<size; i++){
        buffer[i]=-1;
    }
    
}

Queue::~Queue(){
    delete [] buffer;
}

void Queue::addElement(int num){
    // check if the queue is full 
    if(element_count==size){
        printf("\nCircular queue is full\n");
        return;
    }
    // add element and increment 'end'
    buffer[end] = num;
    end = (end+1)%size;
    element_count++;
}

int Queue::deleteElement(){
    
    // check if queue is empty
    if(element_count==0){
        printf("\n Circular queue is empty\n");
        return -1;
    }
    // retrieve element from start
    int element = buffer[start];
    // reset position to -1 (to indicate an empty position)
    buffer[start]=-1;
    // increment the start
    start = (start+1)%size;
    element_count--;
    
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
#ifndef QUEUE_H
#define QUEUE_H

/* Circular queue for positive integers
 * A position contains -1 to indicate empty position */
class Queue{
    private:
        int start, end;
        int size;
        int *buffer;
        int element_count;
    public:
        // constructs a Queue containing an array of size '_size'
        // start and end are initialised to -1 by default
        Queue(int _size);
        ~Queue();

        // adds element to circular queue
        void addElement(int num);

        // deletes element from circular queue 
        int deleteElement();

        // getter for the start index
        int get_start() const;

        // getter for the end index
        int get_end() const;

        // getter for element at index '_index'
        int get_element(int _index) const;

};

#endif
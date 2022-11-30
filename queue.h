#ifndef QUEUE_H
#define QUEUE_H

class Queue{
    private:
        int start, end;
        int size;
        int *buffer;
    public:
        Queue();
        ~Queue();

        void createQueue(int _size);

        // adds element to circular queue
        void addElement(int num);

        // deletes element from circular queue 
        int deleteElement();

        int get_start() const;

        int get_end() const;

        int get_element(int index) const;

};

#endif
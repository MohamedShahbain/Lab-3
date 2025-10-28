#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Node {
private:
    Node* next;                      // keep raw link private (protect the chain)
public:
    string name;
    int    servicetime;
    int    state;                    // 0=waiting, 1=missed

    // Build an unlinked record; structures will link it.
    Node(string name, int state, int servicetime) {
        this->name = name;
        this->state = state;
        this->servicetime = servicetime;
        this->next = nullptr;        // start unlinked
    }

    Node* getNext() { return next; }                   // read one hop
    void  setNext(Node* nextNode) { next = nextNode; } // write link
};

/* STACK (LIFO) for MISSED
   push: new node points to old top, then top moves up.
   pop : take top, move down one, copy-out-before-delete. */
class Stack {
private:
    Node* top;                          // most recent node
public:
    Stack() { top = nullptr; }          // start empty

    ~Stack() {                          // free all nodes we allocated
        while (top != nullptr) {
            Node* nodeToDelete = top;   // current top
            top = top->getNext();       // drop one level
            delete nodeToDelete;        // free memory
        }
    }

    bool isEmpty() { return top == nullptr; }

    void push(const string& customerName, int customerState, int serviceTime) {
        Node* newNode = new Node(customerName, customerState, serviceTime);
        newNode->setNext(top);          // new -> old top
        top = newNode;                  // move top to new
    }

    // pop data out; return true if we popped one
    bool pop(string& outName, int& outState, int& outServiceTime) {
        if (isEmpty()) return false;
        Node* removedNode = top;        // node leaving
        top = top->getNext();           // drop top

        // copy BEFORE delete (needed for print/sum)
        outName = removedNode->name;
        outState = removedNode->state;
        outServiceTime = removedNode->servicetime;

        delete removedNode;             // free node
        return true;
    }
};

/* QUEUE (FIFO) for WAITING
   enqueue: attach at tail and move tail; empty-case becomes head & tail.
   dequeue: take from head, advance one hop; if head clears, clear tail too. */
class Queue {
private:
    Node* head;                         // front of line
    Node* tail;                         // back of line
public:
    Queue() { head = nullptr; tail = nullptr; }

    ~Queue() {                          // free all nodes we allocated
        while (head != nullptr) {
            Node* nodeToDelete = head;  // current front
            head = head->getNext();     // move front
            delete nodeToDelete;        // free node
        }
        tail = nullptr;                 // consistent empty state
    }

    bool isEmpty() { return head == nullptr; }

    void enqueue(const string& customerName, int customerState, int serviceTime) {
        Node* newNode = new Node(customerName, customerState, serviceTime);
        if (tail == nullptr) {          // empty queue
            head = tail = newNode;      // first node is both ends
        }
        else {
            tail->setNext(newNode);     // old tail -> new
            tail = newNode;             // move tail
        }
    }

    // dequeue data out; return true if we removed one
    bool dequeue(string& outName, int& outState, int& outServiceTime) {
        if (isEmpty()) return false;
        Node* removedNode = head;       // current front
        head = head->getNext();         // move front
        if (head == nullptr) tail = nullptr; // now empty

        // copy BEFORE delete (needed for print/sum)
        outName = removedNode->name;
        outState = removedNode->state;
        outServiceTime = removedNode->servicetime;

        delete removedNode;             // free node
        return true;
    }
};

/* loadFile
   read one record at a time; route immediately:
   "waiting" ? enqueue, "missed" ? push. */
bool loadFile(const string& path, Queue& waitingQueue, Stack& missedStack) {
    ifstream in(path.c_str());
    if (!in) return false;

    string fileName;
    string stateWord;
    int    fileTime;
    bool   loadedAny = false;     // track whether we loaded at least one valid record

    while (in >> fileName >> stateWord >> fileTime) {
        if (fileTime < 0) continue;                // skip bad time

        if (stateWord == "waiting") {
            waitingQueue.enqueue(fileName, 0, fileTime); // FIFO
            loadedAny = true;
        }
        else if (stateWord == "missed") {
            missedStack.push(fileName, 1, fileTime);     // LIFO
            loadedAny = true;
        }
        else {
            // unknown word -> skip
        }
    }
    return loadedAny;            // false if file opened but had rows that make no sense
}


/* serveAll
   loop while there’s work: up to 3 from waiting, then 1 from missed.
   print each removal and add minutes to the running total. */
void serveAll(Queue& waitingQueue, Stack& missedStack) {
    int totalTime = 0;                 // running sum
    int lineNumber = 0;                // print order

    cout << "serve order\n";

    while (!waitingQueue.isEmpty() || !missedStack.isEmpty()) {
        // up to 3 waiting
        int servedFromWaiting = 0;
        while (servedFromWaiting < 3 && !waitingQueue.isEmpty()) {
            string customerName; int customerState; int serviceTime;
            waitingQueue.dequeue(customerName, customerState, serviceTime);
            totalTime += serviceTime;
            ++lineNumber;
            cout << lineNumber << ". " << customerName
                << " waiting time " << serviceTime << "\n";
            ++servedFromWaiting;
        }

        // 1 missed
        if (!missedStack.isEmpty()) {
            string customerName; int customerState; int serviceTime;
            missedStack.pop(customerName, customerState, serviceTime);
            totalTime += serviceTime;
            ++lineNumber;
            cout << lineNumber << ". " << customerName
                << " missed time " << serviceTime << "\n";
        }
    }

    cout << "\n" << "total time " << totalTime << "\n";
}

int main() {
    string inputFile;
    cout << "enter input file name\n";
    if (!(cin >> inputFile)) return 0;

    Queue waitingQueue;    // FIFO
    Stack missedStack;     // LIFO

    if (!loadFile(inputFile, waitingQueue, missedStack)) {
        cout << "file open failed\n";
        return 0;
    }

    serveAll(waitingQueue, missedStack);   // 3 then 1, repeat
    return 0;                              // destructors run here
}

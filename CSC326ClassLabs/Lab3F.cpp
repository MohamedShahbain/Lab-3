#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Node {                                       // single list node
public:
    string customerName;
    int    customerState;                          // 0 waiting, 1 missed
    int    serviceMinutes;
    Node* next;

    Node(const string& name, int state, int minutes)
        : customerName(name), customerState(state),
        serviceMinutes(minutes), next(nullptr) {
    }
};

/* STACK
   LIFO. Keep the top at head so push and pop are O(1). */
class Stack {
private:
    Node* head;                                    // top of stack
public:
    Stack() : head(nullptr) {}

    ~Stack() {                                     // release all nodes
        while (head) { Node* nodeToDelete = head; head = head->next; delete nodeToDelete; }
    }

    bool isEmpty() const { return head == nullptr; }

    void push(const string& name, int state, int minutes) {
        Node* newNode = new Node(name, state, minutes);  // create new record
        newNode->next = head;                            // link to old top
        head = newNode;                                  // new top
    }

    bool pop(string& outName, int& outState, int& outMinutes) {
        if (!head) return false;                         // nothing to remove
        Node* nodeToRemove = head;                       // take top
        head = head->next;                               // drop one
        outName = nodeToRemove->customerName;          // copy data before delete
        outState = nodeToRemove->customerState;
        outMinutes = nodeToRemove->serviceMinutes;
        delete nodeToRemove;
        return true;
    }
};

/* QUEUE
   FIFO. Head is front, tail is back. Two pointers make enqueue and dequeue O(1). */
class Queue {
private:
    Node* head;                                    // front
    Node* tail;                                    // back
public:
    Queue() : head(nullptr), tail(nullptr) {}

    ~Queue() {                                     // release all nodes
        while (head) { Node* nodeToDelete = head; head = head->next; delete nodeToDelete; }
        tail = nullptr;
    }

    bool isEmpty() const { return head == nullptr; }

    void enqueue(const string& name, int state, int minutes) {
        Node* newNode = new Node(name, state, minutes);  // new back node
        if (!tail) head = tail = newNode;                // first element
        else { tail->next = newNode; tail = newNode; }   // attach at tail
    }

    bool dequeue(string& outName, int& outState, int& outMinutes) {
        if (!head) return false;                         // nothing to take
        Node* nodeToRemove = head;                       // take front
        head = head->next;                               // advance front
        if (!head) tail = nullptr;                       // queue became empty
        outName = nodeToRemove->customerName;
        outState = nodeToRemove->customerState;
        outMinutes = nodeToRemove->serviceMinutes;
        delete nodeToRemove;
        return true;
    }
};

/* File loader
   Reads triplets: name state minutes. Routes waiting to queue, missed to stack. */
bool loadFile(const string& path, Queue& waitingQueue, Stack& missedStack) {
    ifstream in(path);
    if (!in) return false;

    string name, state;
    int minutes;
    bool any = false;

    while (in >> name >> state >> minutes) {
        if (state == "waiting") { waitingQueue.enqueue(name, 0, minutes); any = true; }
        else if (state == "missed") { missedStack.push(name, 1, minutes); any = true; }
    }
    return any;                                    // true if at least one row
}

/* Service policy
   Serve up to three from the queue, then one from the stack, repeat. */
void serveAll(Queue& waitingQueue, Stack& missedStack) {
    cout << "serve order\n";

    int totalMinutes = 0;
    int orderNumber = 0;

    while (!waitingQueue.isEmpty() || !missedStack.isEmpty()) {
        int fromQueue = 0;                         // limit three per round

        while (fromQueue < 3 && !waitingQueue.isEmpty()) {
            string name; int state; int minutes;
            waitingQueue.dequeue(name, state, minutes);   // O(1)
            totalMinutes += minutes;
            cout << ++orderNumber << ". " << name << " waiting time " << minutes << "\n";
            ++fromQueue;
        }

        if (!missedStack.isEmpty()) {
            string name; int state; int minutes;
            missedStack.pop(name, state, minutes);        // O(1)
            totalMinutes += minutes;
            cout << ++orderNumber << ". " << name << " missed time " << minutes << "\n";
        }
    }

    cout << "\n" << "total time " << totalMinutes << "\n";
}

int main() {
    cout << "enter input file name\n";
    string inputPath;
    if (!(cin >> inputPath)) return 0;

    Queue waitingQueue;                            // FIFO structure
    Stack missedStack;                             // LIFO structure

    if (!loadFile(inputPath, waitingQueue, missedStack)) {
        cout << "file open failed\n";
        return 0;
    }

    serveAll(waitingQueue, missedStack);
    return 0;
}

#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include <bits/stdc++.h>
using namespace std;

#define BPQINT_MAX 0x7fffffff
#define BPQINT_MIN 0x80000000

#define BPQLONG_MAX 0x7fffffffffffffff
#define BPQLONG_MIN 0x8000000000000000

#define BMAXPRIORITY 0x7fffffffffffffff  // long long
#define BPQSize long long
// take getPriority function as input
// max heap by default
// synchronized pq
// size, pop, push, empty, top, reserve, shrinkToFit should be synchronized
// peer class and piece class must support synchronoous getPriority function 

namespace BPQSpace {
    template<typename T>
    bool _lessThan(T &a, T &b) {
        return a->getPriority() < b->getPriority();
    }

    template<typename T>
    bool __greaterThan(T &a, T &b) {
        return a->getPriority() > b->getPriority();
    }

    template <typename T>
    void _swap(T &a, T &b) {
        T t = b;
        b = a;
        a = t;
    }
};

template <typename T, BPQSize MAXPRIORITY = BMAXPRIORITY, bool(*lessThan)(T&, T&) = (BPQSpace::_lessThan<T>)>   // T will be a pointer
class PriorityQueue {
    private:
        enum PQFunc {
            PQsize,
            PQpush,
            PQpop,
            PQempty,
            PQtop,
            PQreserve, 
            PQshrinkToFit
        };

        std::unordered_map<T, long long> mp;  // maps value to index
        std::mutex mtx;

        void siftDown(long long index);
        void siftUp(long long index);
        void _pop();  // pop without locks
        void syncHelper(PQFunc func, void* data);

    public:
        std::vector<T> heap;  // can be accessed

        PriorityQueue();
        void push(T x);  // update if available
        void pop();
        long long size();
        bool empty();
        T top();
        void reserve(long long size);
        void shrinkToFit();
        void clear();
};


////////////////////////////////////
      /*  implementation  */
////////////////////////////////////

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
PriorityQueue<T, MAXPRIORITY, lessThan>::PriorityQueue() {
    // do nothing
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::siftDown(long long index) {
    long long cx, cy;
    while(1) {
        cx = 2*index+1;
        cy = cx+1;

        if(cx >= heap.size() && cy >= heap.size())
            return;
        else if(cx < heap.size() && cy < heap.size()) {
            cx = (lessThan(heap[cx], heap[cy]))? cy: cx;
        }   
        else if(cy < heap.size()) {
            cx = cy;
        }

        if(lessThan(heap[index], heap[cx])) {
            BPQSpace::_swap(mp[heap[cx]], mp[heap[index]]);
            BPQSpace::_swap(heap[cx], heap[index]);
            index = cx;
        }
        else return;
    }
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::siftUp(long long index) {
    long long parent = (index-1)/2;
    while(lessThan(heap[parent], heap[index])) {
        BPQSpace::_swap(mp[heap[parent]], mp[heap[index]]);
        BPQSpace::_swap(heap[parent], heap[index]);
        index = parent;
        parent = (parent-1)/2;
    }
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::_pop() {
    if(heap.size() == 1) {
        mp.erase(heap.front());
        heap.pop_back();
        return;
    }
    BPQSpace::_swap(heap.front(), heap.back());
    mp.erase(heap.back());
    heap.pop_back();

    siftDown(0);
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::syncHelper(PQFunc func, void* data) {
    mtx.lock();
    switch(func) {
        case PQsize:
            *((long long*)data) = heap.size();
            break;
        case PQpush:
            if(mp.find((*((T*)data))) != mp.end()) {
                T temp = (*((T*)data));
                long long ltemp = temp->getPriority();
                temp->setPriority(MAXPRIORITY);
                siftUp(mp[temp]);
                temp->setPriority(ltemp);
                siftDown(0);
            }       
            else {
                T temp = (*((T*)data)); 
                mp[temp] = heap.size();
                heap.push_back(temp);
                siftUp(heap.size()-1);
            }
            break;
        case PQpop:
            _pop();
            break;
        case PQempty:
            *((bool*)data) = heap.empty();
            break;
        case PQtop:
            *((T*) data) = heap.front();
            break;
        case PQreserve:
            heap.reserve(*((long long*) data));
            break;
        case PQshrinkToFit:
            heap.shrink_to_fit();
            break;
        default:
            throw "error";
    }
    mtx.unlock();
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::push(T x) {
    syncHelper(PQpush, &x);
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::pop() {
    syncHelper(PQpop, NULL);
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
long long PriorityQueue<T, MAXPRIORITY, lessThan>::size() {
    long long res;
    syncHelper(PQsize, &res);
    return res;
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
bool PriorityQueue<T, MAXPRIORITY, lessThan>::empty() {
    bool res;
    syncHelper(PQempty, &res);
    return res;
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
T PriorityQueue<T, MAXPRIORITY, lessThan>::top() {
    T res;
    syncHelper(PQtop, &res);
    return res;
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::reserve(long long size) {
    syncHelper(PQreserve, &size);
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::shrinkToFit() {
    syncHelper(PQshrinkToFit, NULL);
}

template <typename T, BPQSize MAXPRIORITY, bool(*lessThan)(T&, T&)>
void PriorityQueue<T, MAXPRIORITY, lessThan>::clear() {
    heap.clear();
    mp.clear();
}

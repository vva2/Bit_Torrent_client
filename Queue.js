'use strict';

class Node {
    constructor(val) {
        this.val = val;
        this.next = null;
    }
};


class Queue {
    constructor() {
        this.length = 0;
        this.head = null;
        this.tail = null;
    }

    push(val) {
        if(this.head === null) {
            this.head = new Node(val);
            this.tail = this.head;
        }
        else {
            this.tail.next = new Node(val);
            this.tail = this.tail.next;
        }
        this.length++;
    }

    pop() {
        if(this.tail === this.head) {
            let temp 
            this.head = null;
            this.tail = null;
        }
        else {
            let temp = this.head;
            this.head = temp.next;
            temp.next = null;
        }

        this.length--;
    }

    front() {
        return this.head.val;
    }
};


module.exports = {
    Queue: Queue
};

const {Queue} = require('./Queue');

let q = new Queue();

console.log(q.length);
q.push(1);
q.push(2);
q.pop();
q.pop();

console.log(q);
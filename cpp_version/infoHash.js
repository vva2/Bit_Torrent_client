const fs = require('fs');
const bencode = require('bencode');
const crypto = require('crypto');

console.log('computing info hash for: ', process.argv[2])
console.log(bencode.decode(fs.readFileSync(process.argv[2]), 'utf8'))
let info = bencode.encode(bencode.decode(fs.readFileSync(process.argv[2])).info);
console.log(crypto.createHash('sha1').update(info).digest());

let bencode = require("bencode");
let fs = require('fs');
let path;

console.log(bencode.decode(fs.readFileSync(process.argv[2]), 'utf8'));

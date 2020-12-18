const fs = require('fs');

if(!fs.existsSync("/a/b/c")) {
    fs.mkdirSync("a/b/c");
}
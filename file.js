const fs = require('fs');
const path = require('path');

class File {
    constructor(start, end, filePath) {
        this.start = start;
        this.end = end;
        this.path = filePath;  // relative path with respect to destination directory


        if(!fs.existsSync(path.dirname(filePath)))
            fs.mkdirSync(path.dirname(filePath), {recursive: true});
    }

    write(offset, data) {
        fs.write(this.path, data, 0, data.length, offset, (err) => {
            console.log(err);
        });
    }
}

module.exports = {
    File: File
};
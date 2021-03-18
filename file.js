const fs = require('fs');
const Path = require('path');

class File {
    constructor(start, end, filePath) {
        this.start = start;
        this.end = end;
        this.path = filePath;  // relative path with respect to destination directory

        if(!fs.existsSync(Path.dirname(filePath)))
            fs.mkdirSync(Path.dirname(filePath), {recursive: true});

        this.file = fs.openSync(filePath, 'w');
    }

    write(offset, data) {
        console.log("writing data...")
        console.log("path: ", this.path);

        fs.write(this.file, data, 0, data.length, offset, (err) => {
            console.log(err);
        });
    }
}

module.exports = {
    File: File
};
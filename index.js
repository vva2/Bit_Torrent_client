'use strict'

const yargs = require('yargs');
const Torrent = require('./torrent');
const 

// TODO learn about paths
// TODO add error handling code here
const torrentFilePath = yargs.argv.t;
// TODO add default destination path based on the torrent name
const destinationPath = yargs.argv.d ? yargs.argv.d: "";
const torrent = Torrent(torrentFilePath, destinationPath);

torrent.download();



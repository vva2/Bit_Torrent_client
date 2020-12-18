'use strict'

const yargs = require('yargs');
const {Torrent} = require('./torrent');
const {open} = require('./util'); 

// TODO learn about paths
// TODO add error handling code here
const torrentFilePath = yargs.argv.t;
// TODO add default destination path based on the torrent name
const destinationPath = yargs.argv.d ? yargs.argv.d: "";
// TODO add code for parsing the magnet link
const torrent = new Torrent(open(torrentFilePath), destinationPath);

torrent.start();

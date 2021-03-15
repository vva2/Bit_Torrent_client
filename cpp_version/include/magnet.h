

// https://stackoverflow.com/questions/3844502/how-do-bittorrent-magnet-links-work
// extract infohash from the torrent file or magnet link and do a dht search for the peers
// once connected download the torrent info and close dht node
// use the info file to get the metadata and use the peers downloaded earlier from dht to start the download

const DHT = require('bittorrent-dht')
const magnet = require('magnet-uri')
 
const uri = 'magnet:?xt=urn:btih:2BWOLT2D6MK2S6RKM47EO7OTDPTTFDGG&dn=%5BSubsPlease%5D%20Mahouka%20Koukou%20no%20Rettousei%20S2%20%2801-13%29%20%28360p%29%20%5BBatch%5D&xl=3388489652&tr=http%3A%2F%2Fnyaa.tracker.wf%3A7777%2Fannounce&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337%2Fannounce&tr=udp%3A%2F%2F9.rarbg.to%3A2710%2Fannounce&tr=udp%3A%2F%2F9.rarbg.me%3A2710%2Fannounce&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.internetwarriors.net%3A1337%2Fannounce&tr=udp%3A%2F%2Ftracker.cyberia.is%3A6969%2Fannounce&tr=udp%3A%2F%2Fexodus.desync.com%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker3.itzmx.com%3A6961%2Fannounce&tr=udp%3A%2F%2Ftracker.torrent.eu.org%3A451%2Fannounce&tr=udp%3A%2F%2Ftracker.tiny-vps.com%3A6969%2Fannounce&tr=udp%3A%2F%2Fretracker.lanta-net.ru%3A2710%2Fannounce&tr=http%3A%2F%2Fopen.acgnxtracker.com%3A80%2Fannounce&tr=wss%3A%2F%2Ftracker.openwebtorrent.com'
const parsed = magnet(uri)
 
console.log(parsed.infoHash) // 'e3811b9539cacff680e418124272177c47477157'
 
const dht = new DHT()
 
dht.listen(20000, function () {
  console.log('now listening')
})
 
dht.on('peer', function (peer, infoHash, from) {
  console.log('found potential peer ' + peer.host + ':' + peer.port + ' through ' + from.address + ':' + from.port)
})
 
// find peers for the given torrent info hash
dht.lookup(parsed.infoHash)
// dht.destroy();

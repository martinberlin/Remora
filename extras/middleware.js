/**
 * Middleware will just receive a POST with :
 * data - Json data (Method: POST)
 * ip - Destination ip (GET)
 * port - Optional defaults to 1234 (GET)
 * 
 * And will send that data depending on the compressed flag as an UDP package to destination ip
 * 
 * Turn DEBUG false to disable console output
 */
var DEBUG = true;
var PORT = 1234;       // Default port
var HOST = '127.0.0.1';

// Requires
var dgram = require('dgram');
const http = require('http');
var url = require('url');
var client = dgram.createSocket('udp4');

http.createServer((request, response) => {
  let body = [];
  request.on('data', (chunk) => {
    body.push(chunk);

  }).on('end', () => {
    
    var url_parts = url.parse(request.url, true);
    var query = url_parts.query;
    var outBuff = Buffer.concat(body);
    if (DEBUG)   console.log(body);
    sendBuffer(outBuff, query.ip, query.port);

    response.end();
  })
  .on('error', (err) => {
    console.error(err);
  });
}).listen(1234);

function sendBuffer(inBuffer, outHost, port) {
    udpPort = (port) ? port : PORT;
    client.send(inBuffer, 0, inBuffer.length, udpPort, outHost, function(err, bytes) {
        if (err) throw err;

        if (DEBUG) console.log('Sent to ' + outHost +':'+ udpPort);
    });
}
    

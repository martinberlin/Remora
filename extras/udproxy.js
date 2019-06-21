/**
 * UDP Redirector proxy is my first nodejs project. If you don't like my code style is because there is none
 * To use configure your Id and IP combinations in ip-config.json ( Ex {"1" : "192.168.0.2"} )
 * And run:   nodejs udproxy.js
 * 
 * From Orca: Prepend your animations using this one character to lookup the IP 
 * Ex.  ;152  Will lookup IP with id 1 and redirect the rest of the message (52) to 192.168.0.2
 * This will enable to run multiple ID stripes using a single IP address as a central point
 * Turn DEBUG false to disable console output
 */
var DEBUG = true;
var PORT = 49161;       // Default Orca UDP output port
var HOST = '127.0.0.1';

var dgram = require('dgram');
var server = dgram.createSocket('udp4');
var client = dgram.createSocket('udp4');

// Configuration. Read our IPs to redirect the incoming message
var fs = require('fs');
var configFile = 'ip-config.json';
var ipConfig;
fs.readFile(configFile, 'utf8', function (err, data) {
    if (err) throw err; // we'll not consider error handling for now
    ipConfig = JSON.parse(data);
    console.log(ipConfig);
});

server.on('listening', function() {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ':' + address.port);
});
  
server.on('message', function(message, remote) {
    var ipId = message.toString().charAt(0);

    if (DEBUG) console.log(remote.address + ':' + remote.port +' - ' + message + ' ipId: '+ipId);
    // Lookup id to extract destination IP {"1":"IpAddress"}
    if (ipConfig[ipId]) {
        sendBuffer(new Buffer(message.toString().substring(1)), ipConfig[ipId]);
    } else {
        console.log('ERROR: Did not find id: '+ipId+' defined in '+configFile);
    }   
});
  
server.bind(PORT, HOST);

function sendBuffer(inBuffer, outHost) {
    client.send(inBuffer, 0, inBuffer, PORT, outHost, function(err, bytes) {
        if (err) throw err;
        if (DEBUG) console.log('Sent: ' + inBuffer.toString() + ' to ' + outHost +':'+ PORT);
    });
}
    
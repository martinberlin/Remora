#!/usr/bin/env node
/**
 * MIDI-to-UDP middleware. Written by Martin Fasani https://github.com/martinberlin
 */
const yargs = require("yargs");
const midi = require('midi');

// Set up a new input.
const input = new midi.Input();

// Count the available input ports.
input.getPortCount();
// Print port names
let ports = "ID: Port name\n";
for (let n=0; n<input.getPortCount(); n++){
  ports += n.toString() +': '+input.getPortName(n)+"\n";
}

let options = yargs
 .usage("Usage: -p <MIDI_PORT_ID>")
 .option("p", { alias: "port_id", describe: ports, type: "number", demandOption: true })
 .argv;

console.log("Listening to: "+input.getPortName(options.port_id));

// Configure a callback.
input.on('message', (deltaTime, message) => {
  // The message is an array of numbers corresponding to the MIDI bytes: https://www.cs.cf.ac.uk/Dave/Multimedia/node158.html
  if (message!='248') {
  	console.log(message);
  }
});

// Open the first available input port.
input.openPort(options.port_id);

// Sysex, timing, and active sensing messages are ignored
// by default. To enable these message types, pass false for
// the appropriate type in the function below.
// Order: (Sysex, Timing, Active Sensing)
// For example if you want to receive only MIDI Clock beats
// you should use
// input.ignoreTypes(true, false, true)
input.ignoreTypes(false, false, false);

// ... receive MIDI messages ...

// Close the port when done.
//setTimeout(function() {
//  input.closePort();
//}, 200000);

const midi = require('midi');

// Set up a new input.
const input = new midi.Input();

// Count the available input ports.
input.getPortCount();
console.log(input.getPortCount());

// Get the name of a specified input port.
input.getPortName(1);
console.log(input.getPortName(0));
console.log(input.getPortName(1));
console.log(input.getPortName(2));

// Configure a callback.
input.on('message', (deltaTime, message) => {
  // The message is an array of numbers corresponding to the MIDI bytes:
  //   [status, data1, data2]
  // https://www.cs.cf.ac.uk/Dave/Multimedia/node158.html has some helpful
  // information interpreting the messages.
  console.log(`m: ${message} d: ${deltaTime}`);
});

// Open the first available input port.
input.openPort(1);

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
setTimeout(function() {
  input.closePort();
}, 100000);

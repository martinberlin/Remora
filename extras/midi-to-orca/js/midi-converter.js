let midi = {};

// Midi note to ORCΛ note mapping
let notemap = {'A':'A', 'B':'B', 'C':'C', 'D':'D', 'E':'E', 'F':'F', 'G':'G',
         'A#':'a', 'B#':'b', 'C#':'c', 'D#':'d', 'E#':'e', 'F#':'f', 'G#':'g',  };

// Set this to false to remove console.logs
let debugMode = false;

// Pointers to DOM
let wrap = $("input#wrap"), silenced = $("input#silence-detect")
let bpm = $("input#bpm"), cols = $("input#cols"), track = $("input#midi-track")

$(document).ready(function() {
    // Load a midi file. Demo: old-town-road.mid samba-pa-ti.mid tiersen_amelie.mid
    // Demo 2 multi-track: d-mode-rumours.mid
    const midiPromise = Midi.fromUrl("midis/d-mode-rumours.mid"); // Returns promise
    midiPromise.then(function (midiIn) {
        midi = midiIn;
        midiOut();
    });
    calculateMillisPerBeat();
});

// Event handlers
wrap.change(function(){
    midiOut(parseInt(track.val()));
});
silenced.change(function(){
    midiOut(parseInt(track.val()));
});
bpm.change(function(){
    midiOut(parseInt(track.val()));
});
cols.change(function(){
    midiOut(parseInt(track.val()));
});
track.change(function(){
    midiOut(parseInt(track.val()));
});

// Simple milliseconds per beat calculation. Ex. 120 bpm will return 500 millis
function calculateMillisPerBeat() {
    let bpm = $("input#bpm").val();
    let millisPerBeat = Math.round(60000/bpm);
    $("div#millis-beat").html(millisPerBeat +" millis per beat")
}

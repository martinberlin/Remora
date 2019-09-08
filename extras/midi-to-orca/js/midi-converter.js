let midi = {};

// Midi note to ORCΛ note mapping
let notemap = {'A':'A', 'B':'B', 'C':'C', 'D':'D', 'E':'E', 'F':'F', 'G':'G',
         'A#':'a', 'B#':'b', 'C#':'c', 'D#':'d', 'E#':'e', 'F#':'f', 'G#':'g',  };
// Increments table from ORCΛ
let orcamap = {'1':'1','2':'2','3':'3','4':'4','5':'5','6':'6','7':'7','8':'8','9':'9', '10':'A', 
    '11':'B', '12':'C','13':'D','14':'E','15':'F','16':'G','17':'H','18':'I','19':'J','20':'K',
    '21':'L', '22':'M','23':'N','24':'O','25':'P','26':'Q','27':'R','28':'S','29':'T','30':'U',
    '31':'V', '32':'W','33':'X','34':'Y','35':'Z'};

// Set this to false to remove console.logs
let debugMode = true;

// Pointers to DOM
let wrap = $("input#wrap"), silenced = $("input#silence-detect")
let bpm = $("input#bpm"), cols = $("input#cols"), track = $("input#midi-track")

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

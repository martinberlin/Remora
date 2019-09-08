let midi = {};

let notemap = {'A':'A', 'B':'B', 'C':'C', 'D':'D', 'E':'E', 'F':'F', 'G':'G',
         'A#':'a', 'B#':'b', 'C#':'c', 'D#':'d', 'E#':'e', 'F#':'f', 'G#':'g',  };

let debugMode = true;
$(document).ready(function() {
    // Load a midi file in the browser. Demo: old-town-road.mid samba-pa-ti.mid tiersen_amelie.mid
    // Demo2: d-mode-rumours.mid
    const midiPromise = Midi.fromUrl("midis/d-mode-rumours.mid"); // Returns promise
    midiPromise.then(function (midiIn) {
        midi = midiIn;
        midiOut();
    });
    calculateMillisPerBeat();
});

function calculateMillisPerBeat() {
    let bpm = $("input#bpm").val();
    let millisPerBeat = Math.round(60000/bpm);
    $("div#millis-beat").html(millisPerBeat +" millis per beat")
}

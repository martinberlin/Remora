let midi = {};
/**
 * Outputs the midi into Orca format
 * @param {*} midi 
 */
function midiOut(trackId = 1) {
    let tracks = $("div#midi-tracks"),
    settings = $("div#midi-settings"),
    octaves = $("div#midi-octave"),
    chords = $("div#midi-chords"),
    bpmEl  = $("input#bpm"),
    colsEl = $("input#cols"),
    rowsEl = $("input#rows"),
    trackEl = $("input:radio")
    ;
    // If promise is done
    bpm  = bpmEl.val();
    cols = colsEl.val();
    rows = rowsEl.val();
    midiHeaderTempos = midi.header.tempos;
    midiBpm = Math.round(midiHeaderTempos[0].bpm);
    console.log("ORCΛ settings BPM:"+bpm+" COLS:"+cols+" ROWS:"+rows+ " Midi settings BPM: "+midiBpm);
    
    //the file name decoded from the first track
    const name = midi.name;
    let trackNr = 1;
    tracks.html('<b>Midi select track</b><br>');

    //get the tracks
    midi.tracks.forEach(track => {
        //tracks have notes and controlChanges
        if (trackId === trackNr) {

        //notes are an array
        const notes = track.notes;
        //console.log("Note    Time     Dur.    Name");
        let debug = 1; 
        notes.forEach(note => {
            noteTime = Math.round(1000*note.time);
            if (debug<=3) {
                console.log(note);
                debug ++; 
            } 
            if (debug<=4) {
                console.log(noteTime, note.name);
                debug ++; 
            }
            //console.log(note.midi, note.time, note.duration, note.name);
        });
        
    }
        // the track also has a channel and instrument
        trackChecked = (trackNr === trackId) ? "checked" : "";
        trackRadio = $('<input type="radio" name="track" value="'+trackNr+'" onchange="midiOut('+trackNr+')" id="t'+trackNr+'" '+trackChecked+' /> <label for="t'+trackNr+'">'+ track.instrument.name +'</label><br>');
            tracks.append(trackRadio);
            trackNr++;
        });
    }

$(document).ready(function() {

// Load a midi file in the browser. Demo: old-town-road.mid samba-pa-ti.mid
    const midiPromise = Midi.fromUrl("midis/tiersen_amelie.mid"); // Returns promise
    midiPromise.then(function (midiIn) {
        midi = midiIn;
        midiOut();
    });
});


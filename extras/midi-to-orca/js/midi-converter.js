$(document).ready(function() {
// Load a midi file in the browser. Demo: old-town-road.mid samba-pa-ti.mid
    const midiPromise = Midi.fromUrl("midis/tiersen_amelie.mid"); // Returns promise

    let tracks = $("div#midi-tracks"),
        settings = $("div#midi-settings"),
        octaves = $("div#midi-octave"),
        chords = $("div#midi-chords"),
        bpmEl  = $("input#bpm"),
        colsEl = $("input#cols"),
        rowsEl = $("input#rows")
        ;

    midiPromise.then(function (midi) {

        // If promise is done
        bpm  = bpmEl.val();
        cols = colsEl.val();
        rows = rowsEl.val();
        console.log("ORCΛ settings BPM:"+bpm+" COLS:"+cols+" ROWS:"+rows);
        console.log(midi);

        //the file name decoded from the first track
        const name = midi.name;
        let trackNr = 1;

        //get the tracks
        midi.tracks.forEach(track => {
            //tracks have notes and controlChanges

            //notes are an array
            const notes = track.notes;
            console.log("Note    Time     Dur.    Name");
            notes.forEach(note => {
                console.log(note.midi, note.time, note.duration, note.name);
            });

            //the track also has a channel and instrument
            if (trackNr===1) tracks.append("<b>Midi tracks</b><br>");
            console.log(track.instrument.name);
            tracks.append(trackNr + ".- " + track.instrument.name +"<br>");
            trackNr++;
        })
    });

});

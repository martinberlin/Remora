/**
 * Outputs the midi into Orca format
 * This midi out exports also duration (velocity)
 * @param {*} midi
 */
function midiOut(trackId = 1) {
    let notetimes = [];
    let notenotes = [];
    let noteocta  = [];
    let notedur   = [];
    // Pointers to DOM objects
    let tracks = $("div#midi-tracks"), settings = $("div#midi-settings"), 
        octavesOut = $("#orca-octaves"), notesOut = $("#orca-notes"), durOut = $("#orca-durations"),
        bpmEl = $("input#bpm"), colsEl = $("input#cols"), wrap = $("input#wrap"),
        silenced = $("input#silence-detect"), trackEl = $("input:radio");
    let silenceQuantity   = parseInt($("input#silence-m-quantity").val());
    let silenceEveryNotes = parseInt($("input#silence-m-notes").val());

    // Clean out output areas
    octavesOut.val('');
    notesOut.val('');
    durOut.val('');
    bpm = bpmEl.val();
    cols = colsEl.val();
    //console.log(midi.header); // Note: Not all midis have a tempo header
    midiHeaderTempos = midi.header.tempos;
    midiBpm = (midiHeaderTempos.length) ? Math.round(midiHeaderTempos[0].bpm) : 0;

    console.log("ORCΛ settings BPM:" + bpm + " COLS:" + cols + " / Midi BPM: " + midiBpm);
    // Build left midi input panel
    tracks.html('');
    tracks.append('<b>Midi select track</b>');
    if (midiBpm) {
        tracks.append('<div id="midi-info">Midi bpm: '+midiBpm+'</div><br>');
    } else {
        tracks.append('<br>');
    }

    let trackNr = 1;
    // Millis per beat from BMP calculation
    let mb = Math.round(60000 / bpmEl.val());
    let trackMillis = 0;
    let orcaOctaves = "";
    let orcaNotes = "";
    let orcaDurations = "";
    
    outPrefix = (wrap.is(":checked")) ? '#' : '';
    outPrepend = (wrap.is(":checked")) ? '#' : '';
    // Read the tracks
    midi.tracks.forEach(track => {
        //Tracks have notes and controlChanges for the moment we ignore the second ones
        if (trackId === trackNr) {
            if (debugMode) {
                console.log("Λlgo:" + algo);
                console.log("Processing: " + track.instrument.name);
                console.log("Note    Time     Dur.    Name");
            }
            const notes = track.notes;
            let debugCnt = 1;
            let notesCnt = 1;
            let algo = $("input[name='algo']:checked").val();
            // Iterate notes array
            notes.forEach(note => {
                noteTime = Math.round(1000 * note.time);
                // We simply make 0.31234 into 3 for duration
                noteDurCalc = Math.round(note.duration*10);
                noteDuration = (noteDurCalc in orcamap) ? orcamap[noteDurCalc] : 9;

                if (debugMode && debugCnt <= 10) {
                    console.log(note.midi, note.time, note.duration, note.name);
                    console.log(noteTime, note.name);
                    debugCnt++;
                }
                // Extract Note // console.log('Note: '+note.name);
                // Check if # Sharp note 
                if (note.name.substr(1, 1) === '#') {
                    // Extract sharp
                    notesel = note.name.substr(0, 2);
                    octasel = note.name.substr(2, 1);
                }
                else {
                    notesel = note.name.substr(0, 1);
                    octasel = note.name.substr(1, 1);
                }

                
                // console.log(' orca: '+notemap[notesel]+' '+octasel);
                // Detect if the note lands into the output grid or there is a silence
                // The most important part of the music is the silence
                switch(algo) {
                    case "1":
                         // We generate the array here but output notes later
                    n = (notesel in notemap) ? notemap[notesel] : '';
                    notetimes.push(noteTime);
                    notenotes.push(n);
                    noteocta.push(octasel);
                    notedur.push(noteDuration);
                        break;

                    case "2":
                    // All notes together no silence is taken in account
                    orcaNotes += (notesel in notemap) ? notemap[notesel] : '';
                    orcaOctaves += octasel;
                    orcaDurations += noteDuration;

                    if (notesCnt % cols == 0) {
                        notesOut.val(notesOut.val() + outPrefix + orcaNotes + outPrepend + "\n");
                        orcaNotes = '';
                        octavesOut.val(octavesOut.val() + outPrefix + orcaOctaves + outPrepend + "\n");
                        orcaOctaves = '';
                        durOut.val(durOut.val() + outPrefix + orcaDurations + outPrepend + "\n");
                        orcaDurations = '';
                    }
                    break;

                    case "3":
                    // Manual INT silence every INT Notes
                    orcaNotes += (notesel in notemap) ? notemap[notesel] : '';
                    orcaOctaves += octasel;
                    orcaDurations += noteDuration;
                    if (notesCnt % silenceEveryNotes == 0) {
                        for (var i = 1; i <= silenceQuantity; i ++) {
                            orcaNotes += '.';
                            orcaOctaves += '.';
                            orcaDurations += '.';
                        }
                    }
                    if (notesCnt % cols == 0) {
                        notesOut.val(notesOut.val() + outPrefix + orcaNotes + outPrepend + "\n");
                        orcaNotes = '';
                        octavesOut.val(octavesOut.val() + outPrefix + orcaOctaves + outPrepend + "\n");
                        orcaOctaves = '';
                        durOut.val(durOut.val() + outPrefix + orcaDurations + outPrepend + "\n");
                        orcaDurations = '';
                    }
                    break;
                }
              
                notesCnt++;
            }); // Note loop

            // Only when detect silences is checked
            if (algo === "1") {
                lastnotetime = (notetimes[notetimes.length - 1]);
                /* We iterate a stepper jumps millis per beat at a time from 0 to lastnotetime
                   that goes from initial to last note and cherry picks the note or silence
                */
                let lastIndex = notetimes[0];
                // We have two counters in place. NoteCnt is the notes index
                // gridIndex is the position in the grid
                notesCnt = 0;
                gridIndex = 1;
                orcaNotes = '';
                orcaOctaves = '';
                orcaDurations = '';
                if (debugMode) {
                    console.log('Note end time: ' + lastnotetime);
                    console.log(notetimes);
                    console.log(noteocta);
                    console.log(notenotes);
                    console.log(notedur);
                }
                debugCnt = 1;
                for (var i = notetimes[0]; i <= lastnotetime; i += mb) {
                    
                    if (notetimes[notesCnt] >= lastIndex && notetimes[notesCnt] <= i) {
                        // Note + octave + duration lands in grid
                        orcaNotes += notenotes[notesCnt];
                        orcaOctaves += noteocta[notesCnt];
                        orcaDurations += notedur[notesCnt];
                    } else {
                    // We detected a silence
                        orcaNotes += ".";
                        orcaOctaves += ".";
                        orcaDurations += ".";
                        // This will skip notes to maintein time (Bad option, but there is any other?)
                        while (i > notetimes[notesCnt]) {
                            notesCnt++;
                        }
                    }
                    // Jump to next row if Cols match
                    if (gridIndex % cols == 0) {
                        //console.log('%'+cols+' matched. orcaNotes:'+orcaNotes);
                        notesOut.val(notesOut.val() + outPrefix + orcaNotes + outPrepend + "\n");
                        octavesOut.val(octavesOut.val() + outPrefix + orcaOctaves + outPrepend + "\n");
                        durOut.val(durOut.val() + outPrefix + orcaDurations + outPrepend + "\n");
                        orcaNotes = '';
                        orcaOctaves = '';
                        orcaDurations = '';
                    }
                    if (debugMode && debugCnt<=20) {
                        console.log("noteTime:" + notetimes[notesCnt] + ">=" + lastIndex + " && <= " + i);
                        if ((notetimes[notesCnt] >= lastIndex && notetimes[notesCnt] <= i)) {
                            console.log("note: "+notenotes[notesCnt]+" found for gridIndex: "+ gridIndex);
                        } else {
                            console.log("silence . added in gridIndex: "+ gridIndex);
                        }
                        debugCnt++;
                    }
                    lastIndex = i;
                    gridIndex++;
                }
            } 
            
        } // End of selected track conditional

        // The track also has a channel and instrument
        // Build left midi input panel: Track select 
        trackChecked = (trackNr === trackId) ? "checked" : "";
        trackRadio = $('<input type="radio" name="track" value="' + trackNr + '" onchange="midiTrack(' + trackNr + ')" id="t' + trackNr + '" ' + trackChecked + ' /> <label for="t' + trackNr + '">' + track.instrument.name + '</label><br>');
        tracks.append(trackRadio);

        
        trackNr++;
    }); // Track loop

}

/**
 * Update the track 
 * @param {*} trackId 
 */
function midiTrack(trackId) {
    let miditrack = $("#midi-track");
    miditrack.val(trackId);
    midiOut(trackId);
}
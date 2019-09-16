<?php
 // This is just a simple boilerplate demo of file upload
 // Without any framework, just plain spaghetti PHP-HTML 
 $midiFile = (isset($_GET['mid'])) ? trim($_GET['mid']) : null;
 $title = "Midi chord extractor for ORCΛ ";
?>
<!DOCTYPE html>
<html lang="en" >
<head>
  <meta charset="UTF-8">
  <title>Midi extractor for ORCΛ - PHP upload</title>
  <script src='./node_modules/jquery/dist/jquery.min.js'></script>
  <script src='./node_modules/@tonejs/midi/build/Midi.js'></script>
  <script src="./js/midi-out.js"></script>
  <link rel="stylesheet" href="./style.css">
</head>

<body>

<?php  
  // Do you like spaghetti?
  if (isset($midiFile)) { ?>
  <h4><?=$title ?>    &nbsp;&nbsp;&nbsp;
  <span class="midi-title"><?=$midiFile ?> <a href="index.php">Upload midi</a></span>
  </h4>


  <?php } else { ?>

    <h4><?=$title ?>
    <form action="index-upload.php" method="post" enctype="multipart/form-data">
    Select midi file:
    <input type="file" name="fileToUpload" id="fileToUpload">
    <input type="submit" value="Upload" name="submit">
    </form>
    </h4>
  <?php } ?>

<div id="midi-tracks"></div>
<div id="midi-settings">
  <b>ORCΛ settings</b><br>
  
  <div id="settings-bpm">
      <label for="bpm">BPM</label> <input id="bpm" onchange="calculateMillisPerBeat()" type="number" max="400" maxlength="3" value="120" style="width: 4em"><br>
      <label for="cols">Cols</label>
      <input id="cols" type="number" max="35" maxlength="2" value="16" style="width: 4em;margin-left:2px"><br>
      Track<input id="midi-track" type="number" max="99" style="margin-left:2px" class="input-2"><br>
      <br>
      <div style="margin-left:-6px">
        <input type="checkbox" value="1" id="wrap" checked>
        <label for="wrap" id="wrap-label">Wrap with #</label>
      </div>
  </div>

<div id="settings-algo">
  <div id="millis-beat"> - </div>Options:<br>
  <input type="radio" value="1" id="silence-detect" name="algo">
  <label for="silence-detect">Λlgo 1 Note chopper</label><br>
  <input type="radio" value="2" id="silence-no" name="algo" checked>
  <label for="silence-no">No silences</label><br>
  <input type="radio" value="3" id="silence-manual" name="algo">
  
  <input id="silence-m-quantity" class="input-2" type="number" max="35" maxlength="2"> <label for="silence-manual">silence each</label> 
  <input id="silence-m-notes" class="input-2" type="number" max="35" maxlength="2"> <label for="silence-m-notes">notes</label><br>
  <div style="width:100%;height:6px"></div>
  <input id="octave-transpose" class="input-2" value="0" type="number" max="5" maxlength="1"> +/- Transpose octave 
  
</div>
</div>

<div id="midi-octaves">
  Octaves<br>
  <textarea cols="36" rows="35" id="orca-octaves"></textarea>
</div>
<div id="midi-notes">
  Notes<br>
  <textarea cols="36" rows="35" id="orca-notes"></textarea>
</div>
<div id="midi-durations">
  Velocity<br>
  <textarea cols="36" rows="35" id="orca-durations"></textarea>
</div>

<!-- End javascript: After most DOM is loaded -->
<script src="./js/midi-converter.js"></script>
<script>
  <?php  
  if (isset($midiFile)) { ?>
  $(document).ready(function() {
    // Load a midi file. Demo: old-town-road.mid samba-pa-ti.mid tiersen_amelie.mid
    // Demo 2 multi-track: d-mode-rumours.mid
    const midiPromise = Midi.fromUrl("midis/<?=$midiFile ?>"); // Returns promise
    midiPromise.then(function (midiIn) {
        midi = midiIn;
        midiOut();
    });
    calculateMillisPerBeat();
   });
  <?php } ?>
</script>
</body>

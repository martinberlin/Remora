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
  <script src="https://unpkg.com/@tonejs/midi"></script>
  <script src="./js/midi-out.js"></script>

  <link rel="stylesheet" href="./style.css">
</head>

<body>

<?php  
  // Do you like spaghetti?
  if (isset($midiFile)) { ?>
  <h4><?=$title ?>    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  Importing: <?=$midiFile ?> <a href="index.php">Upload midi</a>
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

<div id="midi-tracks">
  <b>Midi select track</b> 
  <br>
</div>
<div id="midi-settings">
  <b>ORCΛ settings</b><br><br>
  
  <div style="width: 30%;float:left">
      <label for="bpm">BPM</label> <input id="bpm" onchange="calculateMillisPerBeat()" type="number" max="400" maxlength="3" value="120" style="width: 4em">
      <br>
      <label for="cols">Cols</label>
      <input id="cols" type="number" max="100" maxlength="3" value="16" style="width: 4em;margin-left:2px">
  </div>
<div id="millis-beat"> - </div>
<div id="settings">
  <input type="checkbox" value="1" id="wrap" checked>
  <label for="wrap">Wrap grid with #</label><br>
  <input type="checkbox" value="1" id="silence-detect" checked>
  <label for="silence-detect">Detect grid silences</label>

  <br><br>Export track: <input id="midi-track" type="number" max="99" style="width: 2em;margin-left:2px">
</div>
</div>

<div id="midi-octaves">
  ORCΛ Octaves<br>
  <textarea cols="35" rows="35" id="orca-octaves"></textarea>
</div>
<div id="midi-notes">
    ORCΛ Notes<br>
  <textarea cols="35" rows="35" id="orca-notes"></textarea>
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

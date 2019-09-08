<?php
$target_dir = "midis/";
$targetFilename = basename($_FILES["fileToUpload"]["name"]);
$target_file = $target_dir . $targetFilename;
$uploadOk = 1;
$midiFileType = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));

// Check if it's midi
if(isset($_POST["submit"])) {
    $mimeType = mime_content_type($_FILES["fileToUpload"]["tmp_name"]);
    $checkMimeIsMidi = ($mimeType==="audio/midi");

    if($checkMimeIsMidi && $midiFileType==='mid') {
        $uploadOk = 1;
    } else {
        echo "File is not a valid midi file<br>";
        echo "MimeType: "+$mimeType;
        $uploadOk = 0;
    }
    // Check file size
    if ($_FILES["fileToUpload"]["size"] > 200000) {
        echo "Sorry, your file is too large. Max. is 200Kb";
        $uploadOk = 0;
    } 
    if ($uploadOk) {
        $filename = $_FILES["fileToUpload"]["tmp_name"];
        if (move_uploaded_file($filename, $target_file)) {
            header("Location: index.php?mid=".$targetFilename);
        } else {
            echo "Sorry, there was an error uploading your file.<br>";
            echo "error #".$_FILES["fileToUpload"]["error"];
        }
    }
}
?>

<?php
error_reporting(0);
include_once("dbconnect.php");
$beat = $_GET['beat'];

$sqlinsert = "INSERT INTO `tbl_beats`(`beat_val`) VALUES ('$beat')";

if ($conn->query($sqlinsert) === TRUE){
    echo "success";
}else {
    echo "failed";
}
?>
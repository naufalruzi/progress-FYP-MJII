<?php
$servername = "localhost";
$username 	= "slumber6_iotadmin";
$password 	= "Gi=M@Em)BZd$";
$dbname 	= "slumber6_iotdb";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}
?>

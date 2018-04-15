<?php
$id = $_GET['id'];
$state = $_GET['state'];
echo file_get_contents("http://192.168.0.102:80/relay?id={$id}&state={$state}");
?>

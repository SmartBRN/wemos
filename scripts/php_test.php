#!/usr/bin/php
<?php
	$old_dir = getcwd();
	chdir("/home/dalinar/Git/wemos/scripts");
	$relay_json = shell_exec("./get_relay_json 192.168.0.100 1");
	json_decode($relay_json);
	if(strcmp($relay_json['state'],"on")==0) {
		shell_exec("./relay_control 192.168.0.100 1 off");
	} else if(strcmp($relay_json['state'],"off")==0) {
		shell_exec("./relay_control 192.168.0.100 1 on");
	}
	chdir($old_dir);
?>

<?php
require('reprofunctions.php');
dbgSquirt("============= Update Fullname ===============");

// check that the user has authenticated
$result = checkCookies($forceLogin,$error,FALSE);
if (!($result) || $forceLogin) {
  // we got an error back that occurred while checkCookies was being run, 
  // or authentication failed.  Either way, bounce them back to the login screen
  dbgSquirt("Authentication failed");
  header("Location: http://" . $_SERVER['HTTP_HOST'] . 
	 dirname($_SERVER['PHP_SELF']) . 
	 "/index.php?error=$error");
  exit;
 }
$username = $_COOKIE['user'];

// check if we got to this page due to a submit or a cancel
dbgSquirt("Checking for cancel");
if ("Cancel" == $_POST['submit']) {
  dbgSquirt("...cancel");
  header("Location: http://" . $_SERVER['HTTP_HOST'] . 
	 dirname($_SERVER['PHP_SELF']) . 
	 "/userhome.php");
  exit;
 }


// verify that a new fullname was provided via POST
dbgSquirt("Checking post");
if (!isset($_POST['newfullname'])) {
  // error .. no post variable provided ... possibly because they've jumped
  // directly to this page?
  dbgSquirt("...not set");
  header("Location: http://" . $_SERVER['HTTP_HOST'] . 
	 dirname($_SERVER['PHP_SELF']) . 
	 "/changefullname.php?error=No new fullname was provided.  Please enter one and click Save.  If this error reoccurs, contact an administrator.");
  exit;
 }

// verify that the new fullname is non-blank
$newFullname = $_POST['newfullname'];
dbgSquirt("Checking blank -- $newFullname");
if (empty($newFullname)) {
  // error ... requested name is blank... bounce them back to change name page
  dbgSquirt("...Empty");
  header("Location: http://" . $_SERVER['HTTP_HOST'] . 
	 dirname($_SERVER['PHP_SELF']) . 
	 "/changefullname.php?error=The new fullname must not be blank.");
  exit;
 }

// update the fullname for this user with the provided value
if (updateFullname($username,$newFullname)) {
  // update successful
  $title = "Fullname changed";
  $heading = "Fullname changed";
  $msg = "Fullname changed to <em>$newFullname</em>.";
 } else {
  // update failed
  $title = "Error while changing fullname";
  $heading = "Error while changing fullname";
  $msg = "An error occurred while attempting to change your fullname.  Please contact an administrator.";
 }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<!--
System:  Repro
File:    updatefilename.php
Purpose: Check permissions, verify requested change, and update fullname
Author:  S. Chanin
-->
<html>
<head>
<link rel="stylesheet" type="text/css" href="repro_style.css" />
  <title><?php echo $title; ?></title>
</head>

<body>
<h1 class="title">Repro</h1>
<h1><?php echo $heading; ?></h1>
<hr />
<p><?php echo $msg; ?></p>
<br /><hr />
<a href="userhome.php">Return to User Home</a><br />
<a href="logout.php">Logout</a><br />

</body>
</html>

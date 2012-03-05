<?php

if(isset($_REQUEST['to'])) $to = $_REQUEST['to']; else unset ($to);
if(isset($_REQUEST['text'])) $text = $_REQUEST['text']; else unset ($text);
if(isset($_REQUEST['msgid'])) $msgid = $_REQUEST['msgid']; else unset ($msgid);
if(isset($_REQUEST['sent'])) $sent = $_REQUEST['sent']; else unset ($sent);
if(isset($_REQUEST['confirmation'])) $confirm = $_REQUEST['confirmation']; else unset ($confirm);
if(isset($_REQUEST['fromform'])) $fromform = $_REQUEST['fromform']; else unset ($fromform);


if(isset($sent)) {

  if(isset($fromform)) {

    $errdesc = 'Error Condition met: ';
    $errcond = false;
    if ($to == '') { $errcond = true; $errdesc .= 'To missing... ';}
    if ($text == '') { $errcond = true; $errdesc .= 'Text missing... ';}

    if(!$errcond) {
      $tmpfname = tempnam('/var/spool/sms/outgoing', 'send_');
    
      $fp = fopen($tmpfname, "w");
      fwrite($fp, "To: $to\n" );
      fwrite($fp, "From: " . $_SERVER['REMOTE_ADDR'] . "\n\n");
      fwrite($fp, str_replace ('%%',"\n",$text ));
      fclose($fp);
      chmod($tmpfname,0666);
      $msgid = basename ($tmpfname);
      if($confirm != '' ) {
        if($sent == 'web') {
          die('<html><head><meta http-equiv="REFRESH" content="5;url='
              . $_SERVER['PHP_SELF']
              . '?sent=web&confirmation=on&msgid='
              . $msgid
              . '"><title>SMSSend</title></head><body>SMSSTATUS: WAITING</body></html>'
              );
        } else {
          die('SMSSTATUS: WAITING#'.$msgid);
        }
      } else {
        die('SMSSTATUS: SENT (NO CHECK)!');
      }
      }else {
        form($errdesc);
      }                                   
  } 


  if(isset($msgid)) {
    $msgid = str_replace('.','',$msgid);

    if (is_file('/var/spool/sms/failed/' .$msgid)) {
      die('SMSSTATUS: FAILED');
    }

    if (!is_file('/var/spool/sms/sent/' .$msgid)) {

      if($sent == 'web') {
            die ( '<html><head><meta http-equiv="REFRESH" content="5;url=' 
            . $_SERVER['PHP_SELF'] 
            . '?sent=web&confirmation=on&msgid=' 
            . $msgid 
            . '"><title>SMSSend</title></head><body>SMSSTATUS: WAITING</body></html>');
        }else {
            die('SMSSTATUS: WAITING');
        }
    } else {
      die ( 'SMSSTATUS: SENT' );
    }
  }
} else {
  form($error=null);
}





function form ($error) {
?>
<html>
<head>
<title>SMSSend</title>
</head>
<body>
<form method="GET" action="<?php echo $_SERVER['PHP_SELF']; ?>">
<input type="hidden" name="sent" value="web">
To:<input type="text" name="to"><br />
Text: <input type="text" name="text"><br />
Confirmation: <input type="checkbox" name="confirmation"><br />
<input type="submit" name="fromform" value="Send sms">
</form>
<?php echo '<strong style="font-color:red">'. $error . '</strong>';?>
</body></html>

<?php
die();
}

?>

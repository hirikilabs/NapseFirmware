#ifndef NAPSE_WEBS_H
#define NAPSE_WEBS_H

// root page HTML
String serverRootHTML = "<!DOCTYPE html>\
<html>\
<head>\
 <title>Napse Configuration</title>\
 <style>\
    body {\
        color: #ff9bfd;\
        background-color: #494c88;\
        font-family: sans-serif;\
        font-size: 2vh;\
    }\
    h1 {\
        font-size: 5vh;\
    }\
    h2 {\
        font-size: 3vh;\
    }\
    input {\
        font-size: 2vh;\
    }\
 </style>\
</head>\
<body>\
<h1>Napse Configuration</h1>\
 <form action='/config'>\
  <h2>WiFi</h2>\
  <label for='fssid'>SSID:</label><br>\
  <input type='text' id='fssid' name='fssid' value='%%SSID%%'><br>\
  <label for='fpsk'>Password:</label><br>\
  <input type='password' id='fpsk' name='fpsk' value='%%PSK%%'>\
  <h2>Client</h2>\
  <label for='fclient'>Client IP:</label><br>\
  <input type='text' id='fclient' name='fclient' value='%%CLIENT%%'>\
  <br />\
  <input type='submit' value='Submit'>\
 </form>\
</body>\
</html>";

#endif

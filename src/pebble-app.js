function getWatchVersion() {
  // 1 = Pebble OG
  // 2 = Pebble Steel
  // 3 = Pebble Time
  // 3 = Pebble Basalt Emulator (currently Pebble Time)
  // 4 = Pebble Time Steel

  var watch_version = 1;

  if(Pebble.getActiveWatchInfo) {
    // Available for use!
    try {
      var watch = Pebble.getActiveWatchInfo();
      var watch_name = watch.model;

      if (watch_name.indexOf("time_steel") >= 0) {
        watch_version = 4;
      } else if (watch_name.indexOf("time") >= 0) {
        watch_version = 3;
      } else if (watch_name.indexOf("qemu_basalt") >= 0) {
        watch_version = 3;
      } else if (watch_name.indexOf("steel") >= 0) {
        watch_version = 2;
      }
    } catch(err) {
      console.log("getActiveWatchInfo() FAILED!");
      watch_version = 3;
    }
  } else {
    console.log("getActiveWatchInfo is not available");
  }

  return watch_version;
}

Pebble.addEventListener('ready', function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener("showConfiguration", function(e) {
    //Load the remote config page
    var url = "https://dl.dropboxusercontent.com/u/3223915/spocks_config/index.html" +
        '?watch_version=' + getWatchVersion();
    Pebble.openURL(url);
  }
); 

Pebble.addEventListener("webviewclosed", function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
    
    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      configuration,
      function(e) {
        console.log("Sending settings data succesfully!");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);
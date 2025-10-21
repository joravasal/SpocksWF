var options = {
    KEY_DATE_FORMAT: 0,
    KEY_HOUR_LEAD_ZERO: 0,
    KEY_VIBRATE_HOUR: 0,
    KEY_VIBRATE_BT_DISC: 1,
    KEY_VIBRATE_BT_CONN: 0,
    
    KEY_ALW_SHOW_BT: 0,
    KEY_ALW_SHOW_BATT: 0,
    KEY_ALW_SHOW_SEC: 0,
    
    KEY_SHAKE_SHOW_DATE: 1,
    KEY_SHAKE_SHOW_BT: 1,
    KEY_SHAKE_SHOW_BATT: 1,
    KEY_SHAKE_SHOW_SEC: 1,
    
    KEY_LENTGH_SHOW_DATE: 10,
    KEY_LENGTH_SHOW_BBS: 30,
    
    KEY_ANIMATION_RATE: 4,
    
    KEY_HAND_LENGTH: 7,
    KEY_COLOR_PATTERN: 2,
    
    KEY_COLOR_BG_H10: "0x000000",
    KEY_COLOR_SPOCK_H10: "0xFFFFFF",
    KEY_COLOR_BG_H1: "0x000000",
    KEY_COLOR_SPOCK_H1: "0xFFFFFF",
    KEY_COLOR_BG_M10: "0x000000",
    KEY_COLOR_SPOCK_M10: "0xFFFFFF",
    KEY_COLOR_BG_M1: "0x000000",
    KEY_COLOR_SPOCK_M1: "0xFFFFFF",
    KEY_COLOR_BG_BT: "0x000000",
    KEY_COLOR_SPOCK_BT: "0xFFFFFF",
    KEY_COLOR_BG_SEC: "0x000000",
    KEY_COLOR_SPOCK_SEC: "0xFFFFFF"
  };

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
    loadLocalVariables();
    console.log("Loaded variables: " + JSON.stringify(options));
  }
);

Pebble.addEventListener("showConfiguration", function(e) {
    //Load the remote config page
    var url = "https://www.dropbox.com/scl/fi/mzhg11ka9ovzta1cpa2mm/spocksv1.2.html" +
        '?watch_version=' + getWatchVersion();
    url += "&config_settings=" + encodeURIComponent(JSON.stringify(options));
    console.log("Opening URL: " + url);
    Pebble.openURL(url);
  }
);

function getItem(reference) {
	var item = localStorage.getItem(reference);
	return item;
}

function setItem(reference, item) {
	localStorage.setItem(reference ,item);
}

function loadLocalVariables() {
  var val = parseInt(getItem("KEY_DATE_FORMAT"));
  if (val) {
    options.KEY_DATE_FORMAT = val;
  }
  
  val = parseInt(getItem("KEY_HOUR_LEAD_ZERO"));
  if (val) {
    options.KEY_HOUR_LEAD_ZERO = val;
  }
  
  val = parseInt(getItem("KEY_VIBRATE_HOUR"));
  if (val) {
    options.KEY_VIBRATE_HOUR = val;
  }
  
  val = parseInt(getItem("KEY_VIBRATE_BT_DISC"));
  if (val) {
    options.KEY_VIBRATE_BT_DISC = val;
  }
  
  val = parseInt(getItem("KEY_VIBRATE_BT_CONN"));
  if (val) {
    options.KEY_VIBRATE_BT_CONN = val;
  }
  
  val = parseInt(getItem("KEY_ALW_SHOW_BT"));
  if (val) {
    options.KEY_ALW_SHOW_BT = val;
  }
  
  val = parseInt(getItem("KEY_ALW_SHOW_BATT"));
  if (val) {
    options.KEY_ALW_SHOW_BATT = val;
  }
  
  val = parseInt(getItem("KEY_ALW_SHOW_SEC"));
  if (val) {
    options.KEY_ALW_SHOW_SEC = val;
  }
  
  val = parseInt(getItem("KEY_SHAKE_SHOW_DATE"));
  if (val) {
    options.KEY_SHAKE_SHOW_DATE = val;
  }
  
  val = parseInt(getItem("KEY_SHAKE_SHOW_BT"));
  if (val) {
    options.KEY_SHAKE_SHOW_BT = val;
  }
  
  val = parseInt(getItem("KEY_SHAKE_SHOW_BATT"));
  if (val) {
    options.KEY_SHAKE_SHOW_BATT = val;
  }
  
  val = parseInt(getItem("KEY_SHAKE_SHOW_SEC"));
  if (val) {
    options.KEY_SHAKE_SHOW_SEC = val;
  }
  
  val = parseInt(getItem("KEY_LENTGH_SHOW_DATE"));
  if (val) {
    options.KEY_LENTGH_SHOW_DATE = val;
  }
  
  val = parseInt(getItem("KEY_LENGTH_SHOW_BBS"));
  if (val) {
    options.KEY_LENGTH_SHOW_BBS = val;
  }
  
  val = parseInt(getItem("KEY_ANIMATION_RATE"));
  if (val) {
    options.KEY_ANIMATION_RATE = val;
  }
  
  val = parseInt(getItem("KEY_HAND_LENGTH"));
  if (val) {
    options.KEY_HAND_LENGTH = val;
  }
  
  val = parseInt(getItem("KEY_COLOR_PATTERN"));
  if (val) {
    options.KEY_COLOR_PATTERN = val;
  }
  
  val = getItem("KEY_COLOR_BG_H10");
  if (val) {
    options.KEY_COLOR_BG_H10 = val;
  }
  
  val = getItem("KEY_COLOR_SPOCK_H10");
  if (val) {
    options.KEY_COLOR_SPOCK_H10 = val;
  }
  
  val = getItem("KEY_COLOR_BG_H1");
  if (val) {
    options.KEY_COLOR_BG_H1 = val;
  }
  
  val = getItem("KEY_COLOR_SPOCK_H1");
  if (val) {
    options.KEY_COLOR_SPOCK_H1 = val;
  }
  
  val = getItem("KEY_COLOR_BG_M10");
  if (val) {
    options.KEY_COLOR_BG_M10 = val;
  }
  
  val = getItem("KEY_COLOR_SPOCK_M10");
  if (val) {
    options.KEY_COLOR_SPOCK_M10 = val;
  }
  
  val = getItem("KEY_COLOR_BG_M1");
  if (val) {
    options.KEY_COLOR_BG_M1 = val;
  }
  
  val = getItem("KEY_COLOR_SPOCK_M1");
  if (val) {
    options.KEY_COLOR_SPOCK_M1 = val;
  }
  
  val = getItem("KEY_COLOR_BG_BT");
  if (val) {
    options.KEY_COLOR_BG_BT = val;
  }
  
  val = getItem("KEY_COLOR_SPOCK_BT");
  if (val) {
    options.KEY_COLOR_SPOCK_BT = val;
  }
  
  val = getItem("KEY_COLOR_BG_SEC");
  if (val) {
    options.KEY_COLOR_BG_SEC = val;
  }
  
  val = getItem("KEY_COLOR_SPOCK_SEC");
  if (val) {
    options.KEY_COLOR_SPOCK_SEC = val;
  }
}

Pebble.addEventListener("webviewclosed", function(e) {
    //Get JSON dictionary
    var new_config = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration returned: " + JSON.stringify(new_config));
    
    var val = new_config.KEY_DATE_FORMAT;
    if (!isNaN(val) && val != options.KEY_DATE_FORMAT) {
      options.KEY_DATE_FORMAT = val;
      setItem("KEY_DATE_FORMAT", val);
    }
    
    var val = new_config.KEY_HOUR_LEAD_ZERO;
    if (!isNaN(val) && val != options.KEY_HOUR_LEAD_ZERO) {
      options.KEY_HOUR_LEAD_ZERO = val;
      setItem("KEY_HOUR_LEAD_ZERO", val);
    }
    
    var val = new_config.KEY_VIBRATE_HOUR;
    if (!isNaN(val) && val != options.KEY_VIBRATE_HOUR) {
      options.KEY_VIBRATE_HOUR = val;
      setItem("KEY_VIBRATE_HOUR", val);
    }
    
    var val = new_config.KEY_VIBRATE_BT_DISC;
    if (!isNaN(val) && val != options.KEY_VIBRATE_BT_DISC) {
      options.KEY_VIBRATE_BT_DISC = val;
      setItem("KEY_VIBRATE_BT_DISC", val);
    }
    
    var val = new_config.KEY_VIBRATE_BT_CONN;
    if (!isNaN(val) && val != options.KEY_VIBRATE_BT_CONN) {
      options.KEY_VIBRATE_BT_CONN = val;
      setItem("KEY_VIBRATE_BT_CONN", val);
    }
    
    var val = new_config.KEY_ALW_SHOW_BT;
    if (!isNaN(val) && val != options.KEY_ALW_SHOW_BT) {
      options.KEY_ALW_SHOW_BT = val;
      setItem("KEY_ALW_SHOW_BT", val);
    }
    
    var val = new_config.KEY_ALW_SHOW_BATT;
    if (!isNaN(val) && val != options.KEY_ALW_SHOW_BATT) {
      options.KEY_ALW_SHOW_BATT = val;
      setItem("KEY_ALW_SHOW_BATT", val);
    }
    
    var val = new_config.KEY_ALW_SHOW_SEC;
    if (!isNaN(val) && val != options.KEY_ALW_SHOW_SEC) {
      options.KEY_ALW_SHOW_SEC = val;
      setItem("KEY_ALW_SHOW_SEC", val);
    }
    
    var val = new_config.KEY_SHAKE_SHOW_DATE;
    if (!isNaN(val) && val != options.KEY_SHAKE_SHOW_DATE) {
      options.KEY_SHAKE_SHOW_DATE = val;
      setItem("KEY_SHAKE_SHOW_DATE", val);
    }
    
    var val = new_config.KEY_SHAKE_SHOW_BT;
    if (!isNaN(val) && val != options.KEY_SHAKE_SHOW_BT) {
      options.KEY_SHAKE_SHOW_BT = val;
      setItem("KEY_SHAKE_SHOW_BT", val);
    }
    
    var val = new_config.KEY_SHAKE_SHOW_BATT;
    if (!isNaN(val) && val != options.KEY_SHAKE_SHOW_BATT) {
      options.KEY_SHAKE_SHOW_BATT = val;
      setItem("KEY_SHAKE_SHOW_BATT", val);
    }
    
    var val = new_config.KEY_SHAKE_SHOW_SEC;
    if (!isNaN(val) && val != options.KEY_SHAKE_SHOW_SEC) {
      options.KEY_SHAKE_SHOW_SEC = val;
      setItem("KEY_SHAKE_SHOW_SEC", val);
    }
    
    var val = new_config.KEY_LENTGH_SHOW_DATE;
    if (!isNaN(val) && val != options.KEY_LENTGH_SHOW_DATE) {
      options.KEY_LENTGH_SHOW_DATE = val;
      setItem("KEY_LENTGH_SHOW_DATE", val);
    }
    
    var val = new_config.KEY_LENGTH_SHOW_BBS;
    if (!isNaN(val) && val != options.KEY_LENGTH_SHOW_BBS) {
      options.KEY_LENGTH_SHOW_BBS = val;
      setItem("KEY_LENGTH_SHOW_BBS", val);
    }
    
    var val = new_config.KEY_ANIMATION_RATE;
    if (!isNaN(val) && val != options.KEY_ANIMATION_RATE) {
      options.KEY_ANIMATION_RATE = val;
      setItem("KEY_ANIMATION_RATE", val);
    }
    
    var val = new_config.KEY_HAND_LENGTH;
    if (!isNaN(val) && val != options.KEY_HAND_LENGTH) {
      options.KEY_HAND_LENGTH = val;
      setItem("KEY_HAND_LENGTH", val);
    }
    
    var val = new_config.KEY_COLOR_PATTERN;
    if (!isNaN(val) && val != options.KEY_COLOR_PATTERN) {
      options.KEY_COLOR_PATTERN = val;
      setItem("KEY_COLOR_PATTERN", val);
    }
    
    var val = new_config.KEY_COLOR_BG_H10;
    if (val != options.KEY_COLOR_BG_H10) {
      options.KEY_COLOR_BG_H10 = val;
      setItem("KEY_COLOR_BG_H10", val);
    }
    new_config.KEY_COLOR_BG_H10 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_SPOCK_H10;
    if (val != options.KEY_COLOR_SPOCK_H10) {
      options.KEY_COLOR_SPOCK_H10 = val;
      setItem("KEY_COLOR_SPOCK_H10", val);
    }
    new_config.KEY_COLOR_SPOCK_H10 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_BG_H1;
    if (val != options.KEY_COLOR_BG_H1) {
      options.KEY_COLOR_BG_H1 = val;
      setItem("KEY_COLOR_BG_H1", val);
    }
    new_config.KEY_COLOR_BG_H1 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_SPOCK_H1;
    if (val != options.KEY_COLOR_SPOCK_H1) {
      options.KEY_COLOR_SPOCK_H1 = val;
      setItem("KEY_COLOR_SPOCK_H1", val);
    }
    new_config.KEY_COLOR_SPOCK_H1 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_BG_M10;
    if (val != options.KEY_COLOR_BG_M10) {
      options.KEY_COLOR_BG_M10 = val;
      setItem("KEY_COLOR_BG_M10", val);
    }
    new_config.KEY_COLOR_BG_M10 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_SPOCK_M10;
    if (val != options.KEY_COLOR_SPOCK_M10) {
      options.KEY_COLOR_SPOCK_M10 = val;
      setItem("KEY_COLOR_SPOCK_M10", val);
    }
    new_config.KEY_COLOR_SPOCK_M10 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_BG_M1;
    if (val != options.KEY_COLOR_BG_M1) {
      options.KEY_COLOR_BG_M1 = val;
      setItem("KEY_COLOR_BG_M1", val);
    }
    new_config.KEY_COLOR_BG_M1 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_SPOCK_M1;
    if (val != options.KEY_COLOR_SPOCK_M1) {
      options.KEY_COLOR_SPOCK_M1 = val;
      setItem("KEY_COLOR_SPOCK_M1", val);
    }
    new_config.KEY_COLOR_SPOCK_M1 = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_BG_BT;
    if (val != options.KEY_COLOR_BG_BT) {
      options.KEY_COLOR_BG_BT = val;
      setItem("KEY_COLOR_BG_BT", val);
    }
    new_config.KEY_COLOR_BG_BT = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_SPOCK_BT;
    if (val != options.KEY_COLOR_SPOCK_BT) {
      options.KEY_COLOR_SPOCK_BT = val;
      setItem("KEY_COLOR_SPOCK_BT", val);
    }
    new_config.KEY_COLOR_SPOCK_BT = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_BG_SEC;
    if (val != options.KEY_COLOR_BG_SEC) {
      options.KEY_COLOR_BG_SEC = val;
      setItem("KEY_COLOR_BG_SEC", val);
    }
    new_config.KEY_COLOR_BG_SEC = parseInt(val, 16);
    
    var val = new_config.KEY_COLOR_SPOCK_SEC;
    if (val != options.KEY_COLOR_SPOCK_SEC) {
      options.KEY_COLOR_SPOCK_SEC = val;
      setItem("KEY_COLOR_SPOCK_SEC", val);
    }
    new_config.KEY_COLOR_SPOCK_SEC = parseInt(val, 16);
    
    console.log("Sending to Pebble: " + JSON.stringify(new_config));
    
    Pebble.sendAppMessage(
      new_config,
      function(e) {
        console.log("Sending settings data succesfully!");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);

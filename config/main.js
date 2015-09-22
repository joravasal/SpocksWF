var watch_version = -1;

(function() {
  loadOptions();
  uiHandlers();
})();

function uiHandlers() {
  var return_to = getQueryParam('return_to', 'pebblejs://close#');

  $('#submitButton').on('click', function() {
    console.log('Submit');

    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });
  
  $('#cancelButton').on('click', function() {
    console.log('Cancel');

    document.location = return_to;
  });
  
  var $tab_behaviour_content = $('#behavior_tab_content');
  var $tab_style_content = $('#style_tab_content');
  $('#tab_behaviour').on('click', function() {
    $tab_behaviour_content.removeClass('hidden');
    $tab_style_content.addClass('hidden');
  });
  $('#tab_style').on('click', function() {
    $tab_behaviour_content.addClass('hidden');
    $tab_style_content.removeClass('hidden');
  });
  
  $('#color_pattern').on('change', function() {
    changeColorsVisibility($('#color_pattern')[0].value);
  });
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}

function loadOptions() {
  watch_version = getQueryParam('watch_version', 1);
  console.log("watch_version: "+watch_version);
  
  if(watch_version <= 2 && watch_version > 0) {
    $('#colors_aplite').removeClass('hidden');
  } else if (watch_version <= 4 && watch_version >= 3) {
    $('#colors_basalt').removeClass('hidden');
  }
  
  if (localStorage.lead_zero) {
    //Settings for the behaviour
    $('#date_format')[0].value = localStorage.date_format;
    $('#lead_zero')[0].checked = localStorage.lead_zero == 1;
    
    $('#vibration_hour')[0].checked = localStorage.vibration_hour == 1;
    $('#vibration_bt_disc')[0].checked = localStorage.vibration_bt_disc == 1;
    $('#vibration_bt_conn')[0].checked = localStorage.vibration_bt_conn == 1;
    
    $('#alw_show_bt')[0].checked = localStorage.alw_show_bt == 1;
    $('#alw_show_batt')[0].checked = localStorage.alw_show_batt == 1;
    $('#alw_show_sec')[0].checked = localStorage.alw_show_sec == 1;
    
    $('#shake_show_date')[0].checked = localStorage.shake_show_date == 1;
    $('#shake_show_bt')[0].checked = localStorage.shake_show_bt == 1;
    $('#shake_show_batt')[0].checked = localStorage.shake_show_batt == 1;
    $('#shake_show_sec')[0].checked = localStorage.shake_show_sec == 1;
    
    $('#show_date_seconds')[0].value = localStorage.show_date_seconds;
    $('#show_date_seconds_textbox')[0].value = localStorage.show_date_seconds;
    $('#show_sbb_seconds')[0].value = localStorage.show_sbb_seconds;
    $('#show_sbb_seconds_textbox')[0].value = localStorage.show_sbb_seconds;
    
    $('#animation_rate')[0].value = localStorage.animation_rate;
    
    //Settings for the style
    $('#hand_length')[0].value = localStorage.hand_length;
    
    $('#color_pattern')[0].value = localStorage.color_pattern;
    if(watch_version <= 2 && watch_version > 0) {
      $('#colors_aplite').find('#byw_background_h10').each(function(index){
        $(this)[0].value = localStorage.byw_h10;
      });
      $('#byw_background_h1')[0].value = localStorage.byw_h1;
      
      $('#colors_aplite').find('#byw_background_m10').each(function(index){
        $(this)[0].value = localStorage.byw_m10;
      });
      $('#byw_background_m1')[0].value = localStorage.byw_m1;
      
      $('#byw_background_bt')[0].value = localStorage.byw_bt;
      $('#byw_background_sec')[0].value = localStorage.byw_sec;
      
    } else if (watch_version <= 4 && watch_version >= 3) {
      $('#colors_basalt').find('#color_background_h10').each(function(index){
        $(this).val(localStorage.color_background_h10);
      });
      $('#colors_basalt').find('#color_spocks_h10').each(function(index){
        $(this).val(localStorage.color_spocks_h10);
      });
      $('#color_background_h1').val(localStorage.color_background_h1);
      $('#color_spocks_h1').val(localStorage.color_spocks_h1);
      
      $('#colors_basalt').find('#color_background_m10').each(function(index){
        $(this).val(localStorage.color_background_m10);
      });
      $('#colors_basalt').find('#color_spocks_m10').each(function(index){
        $(this).val(localStorage.color_spocks_m10);
      });
      $('#color_background_m1').val(localStorage.color_background_m1);
      $('#color_spocks_m1').val(localStorage.color_spocks_m1);
      
      $('#color_background_bt').val(localStorage.color_background_bt);
      $('#color_spocks_bt').val(localStorage.color_spocks_bt);
      $('#color_background_sec').val(localStorage.color_background_sec);
      $('#color_spocks_sec').val(localStorage.color_spocks_sec);
    }
  }
  changeColorsVisibility($('#color_pattern')[0].value);
}

function changeColorsVisibility(pattern) {
  var color_group = $('#colors_basalt');
  if(watch_version <= 2 && watch_version > 0) {
    color_group = $('#colors_aplite');
  }
  
  color_group.find('.item').each(function(index){
    $(this).addClass('hidden');
  });
  color_group.find('.pattern-'+pattern).each(function(index){
    console.log('input %d is: %o', index, this)
    $(this).removeClass('hidden');
  });
}

function getAndStoreConfigData() {
  var options = {
    KEY_DATE_FORMAT: parseInt($('#date_format')[0].value, 10),
    KEY_HOUR_LEAD_ZERO: $('#lead_zero')[0].checked === true ? 1 : 0,
    KEY_VIBRATE_HOUR: $('#vibration_hour')[0].checked === true ? 1 : 0,
    KEY_VIBRATE_BT_DISC: $('#vibration_bt_disc')[0].checked === true ? 1 : 0,
    KEY_VIBRATE_BT_CONN: $('#vibration_bt_conn')[0].checked === true ? 1 : 0,
    
    KEY_ALW_SHOW_BT: $('#alw_show_bt')[0].checked === true ? 1 : 0,
    KEY_ALW_SHOW_BATT: $('#alw_show_batt')[0].checked === true ? 1 : 0,
    KEY_ALW_SHOW_SEC: $('#alw_show_sec')[0].checked === true ? 1 : 0,
    
    KEY_SHAKE_SHOW_DATE: $('#shake_show_date')[0].checked === true ? 1 : 0,
    KEY_SHAKE_SHOW_BT: $('#shake_show_bt')[0].checked === true ? 1 : 0,
    KEY_SHAKE_SHOW_BATT: $('#shake_show_batt')[0].checked === true ? 1 : 0,
    KEY_SHAKE_SHOW_SEC: $('#shake_show_sec')[0].checked === true ? 1 : 0,
    
    KEY_LENTGH_SHOW_DATE: parseInt($('#show_date_seconds')[0].value, 10),
    KEY_LENGTH_SHOW_BBS: parseInt($('#show_sbb_seconds')[0].value, 10),
    
    KEY_ANIMATION_RATE: parseInt($('#animation_rate')[0].value, 10),
    
    KEY_HAND_LENGTH: parseInt($('#hand_length')[0].value, 10),
    KEY_COLOR_PATTERN: parseInt($('#color_pattern')[0].value, 10),
    
    KEY_COLOR_BG_H10: -1,
    KEY_COLOR_SPOCK_H10: -1,
    KEY_COLOR_BG_H1: -1,
    KEY_COLOR_SPOCK_H1: -1,
    KEY_COLOR_BG_M10: -1,
    KEY_COLOR_SPOCK_M10: -1,
    KEY_COLOR_BG_M1: -1,
    KEY_COLOR_SPOCK_M1: -1,
    KEY_COLOR_BG_BT: -1,
    KEY_COLOR_SPOCK_BT: -1,
    KEY_COLOR_BG_SEC: -1,
    KEY_COLOR_SPOCK_SEC: -1
  };
  localStorage.date_format = options.KEY_DATE_FORMAT;
  localStorage.lead_zero = options.KEY_HOUR_LEAD_ZERO;
    
  localStorage.vibration_hour = options.KEY_VIBRATE_HOUR;
  localStorage.vibration_bt_disc = options.KEY_VIBRATE_BT_DISC;
  localStorage.vibration_bt_conn = options.KEY_VIBRATE_BT_CONN;
  
  localStorage.alw_show_bt = options.KEY_ALW_SHOW_BT;
  localStorage.alw_show_batt = options.KEY_ALW_SHOW_BATT;
  localStorage.alw_show_sec = options.KEY_ALW_SHOW_SEC;
  
  localStorage.shake_show_date = options.KEY_SHAKE_SHOW_DATE;
  localStorage.shake_show_bt = options.KEY_SHAKE_SHOW_BT;
  localStorage.shake_show_batt = options.KEY_SHAKE_SHOW_BATT;
  localStorage.shake_show_sec = options.KEY_SHAKE_SHOW_SEC;
  
  localStorage.show_date_seconds = options.KEY_LENTGH_SHOW_DATE;
  localStorage.show_sbb_seconds = options.KEY_LENGTH_SHOW_BBS;
  
  localStorage.animation_rate = options.KEY_ANIMATION_RATE;
  
  localStorage.hand_length = options.KEY_HAND_LENGTH;
  localStorage.color_pattern = options.KEY_COLOR_PATTERN;
  
  if(watch_version <= 2 && watch_version > 0) {
    var collection = $('#colors_aplite').find('.pattern-'+options.KEY_COLOR_PATTERN);
    var black = parseInt('000000', 16);
    var white = parseInt('FFFFFF', 16);
    
    localStorage.byw_h10 = collection.find('#byw_background_h10').first()[0].value;
    options.KEY_COLOR_BG_H10 = localStorage.byw_h10 == 0 ? white : black;
    options.KEY_COLOR_SPOCK_H10 = localStorage.byw_h10 == 0 ? black : white;
    
    localStorage.byw_h1 = $('#byw_background_h1')[0].value;
    options.KEY_COLOR_BG_H1 = localStorage.byw_h1 == 0 ? white : black;
    options.KEY_COLOR_SPOCK_H1 = localStorage.byw_h1 == 0 ? black : white;
    
    if(options.KEY_COLOR_PATTERN >= 2) {
      localStorage.byw_m10 = collection.find('#byw_background_m10').first()[0].value;
    } else {
      localStorage.byw_m10 = $('#byw_background_m10')[0].value;
    }
    options.KEY_COLOR_BG_M10 = localStorage.byw_m10 == 0 ? white : black;
    options.KEY_COLOR_SPOCK_M10 = localStorage.byw_m10 == 0 ? black : white;
    
    localStorage.byw_m1 = $('#byw_background_m1')[0].value;
    options.KEY_COLOR_BG_M1 = localStorage.byw_m1 == 0 ? white : black;
    options.KEY_COLOR_SPOCK_M1 = localStorage.byw_m1 == 0 ? black : white;
    
    localStorage.byw_bt = $('#byw_background_bt')[0].value;
    options.KEY_COLOR_BG_BT = localStorage.byw_bt == 0 ? white : black;
    options.KEY_COLOR_SPOCK_BT = localStorage.byw_bt == 0 ? black : white;
    
    localStorage.byw_sec = $('#byw_background_sec')[0].value;
    options.KEY_COLOR_BG_SEC = localStorage.byw_sec == 0 ? white : black;
    options.KEY_COLOR_SPOCK_SEC = localStorage.byw_sec == 0 ? black : white;
  } else if (watch_version <= 4 && watch_version >= 3) {
    var collection = $('#colors_basalt').find('.pattern-'+options.KEY_COLOR_PATTERN);
    
    localStorage.color_background_h10 = collection.find('#color_background_h10').first().val();
    localStorage.color_spocks_h10 = collection.find('#color_spocks_h10').first().val();
    options.KEY_COLOR_BG_H10 = parseInt(localStorage.color_background_h10, 16);
    options.KEY_COLOR_SPOCK_H10 = parseInt(localStorage.color_spocks_h10, 16);
    
    localStorage.color_background_h1 = $('#color_background_h1').val();
    localStorage.color_spocks_h1 = $('#color_spocks_h1').val();
    options.KEY_COLOR_BG_H1 = parseInt(localStorage.color_background_h1, 16);
    options.KEY_COLOR_SPOCK_H1 = parseInt(localStorage.color_spocks_h1, 16);
    
    if(options.KEY_COLOR_PATTERN >= 2) {
      localStorage.color_background_m10 = collection.find('#color_background_m10').first().val();
      localStorage.color_spocks_m10 = collection.find('#color_spocks_m10').first().val();
    } else {
      localStorage.color_background_m10 = $('#color_background_m10').val();
      localStorage.color_spocks_m10 = $('#color_spocks_m10').val();
    }
    options.KEY_COLOR_BG_M10 = parseInt(localStorage.color_background_m10, 16);
    options.KEY_COLOR_SPOCK_M10 = parseInt(localStorage.color_spocks_m10, 16);
    
    localStorage.color_background_m1 = $('#color_background_m1').val();
    localStorage.color_spocks_m1 = $('#color_spocks_m1').val();
    options.KEY_COLOR_BG_M1 = parseInt(localStorage.color_background_m1, 16);
    options.KEY_COLOR_SPOCK_M1 = parseInt(localStorage.color_spocks_m1, 16);
    
    localStorage.color_background_bt = $('#color_background_bt').val();
    localStorage.color_spocks_bt = $('#color_spocks_bt').val();
    options.KEY_COLOR_BG_BT = parseInt(localStorage.color_background_bt, 16);
    options.KEY_COLOR_SPOCK_BT = parseInt(localStorage.color_spocks_bt, 16);
    localStorage.color_background_sec = $('#color_background_sec').val();
    localStorage.color_spocks_sec = $('#color_spocks_sec').val();
    options.KEY_COLOR_BG_SEC = parseInt(localStorage.color_background_sec, 16);
    options.KEY_COLOR_SPOCK_SEC = parseInt(localStorage.color_spocks_sec, 16);
  }

  console.log('Got options: ' + JSON.stringify(options));
  return options;
}
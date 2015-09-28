var watch_version = -1;

(function() {
  loadOptions();
  uiHandlers();
})();

function uiHandlers() {
  var return_to = getQueryParam('return_to', 'pebblejs://close#');

  $('#submitButton').on('click', function() {
    console.log('Submit');

    document.location = return_to + encodeURIComponent(JSON.stringify(getConfigData()));
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
  
  var stored_settings = getQueryParam('config_settings', null);
  if (stored_settings) {
    stored_settings = JSON.parse(decodeURIComponent(stored_settings));
    //Settings for the behaviour
    $('#date_format')[0].value = stored_settings.KEY_DATE_FORMAT;
    $('#lead_zero')[0].checked = stored_settings.KEY_HOUR_LEAD_ZERO == 1;
    
    $('#vibration_hour')[0].checked = stored_settings.KEY_VIBRATE_HOUR == 1;
    $('#vibration_bt_disc')[0].checked = stored_settings.KEY_VIBRATE_BT_DISC == 1;
    $('#vibration_bt_conn')[0].checked = stored_settings.KEY_VIBRATE_BT_CONN == 1;
    
    $('#alw_show_bt')[0].checked = stored_settings.KEY_ALW_SHOW_BT == 1;
    $('#alw_show_batt')[0].checked = stored_settings.KEY_ALW_SHOW_BATT == 1;
    $('#alw_show_sec')[0].checked = stored_settings.KEY_ALW_SHOW_SEC == 1;
    
    $('#shake_show_date')[0].checked = stored_settings.KEY_SHAKE_SHOW_DATE == 1;
    $('#shake_show_bt')[0].checked = stored_settings.KEY_SHAKE_SHOW_BT == 1;
    $('#shake_show_batt')[0].checked = stored_settings.KEY_SHAKE_SHOW_BATT == 1;
    $('#shake_show_sec')[0].checked = stored_settings.KEY_SHAKE_SHOW_SEC == 1;
    
    $('#show_date_seconds')[0].value = stored_settings.KEY_LENTGH_SHOW_DATE;
    $('#show_date_seconds_textbox')[0].value = stored_settings.KEY_LENTGH_SHOW_DATE;
    $('#show_sbb_seconds')[0].value = stored_settings.KEY_LENGTH_SHOW_BBS;
    $('#show_sbb_seconds_textbox')[0].value = stored_settings.KEY_LENGTH_SHOW_BBS;
    
    $('#animation_rate')[0].value = stored_settings.KEY_ANIMATION_RATE;
    
    //Settings for the style
    $('#hand_length')[0].value = stored_settings.KEY_HAND_LENGTH;
    
    $('#color_pattern')[0].value = stored_settings.KEY_COLOR_PATTERN;
    if(watch_version <= 2 && watch_version > 0) {
      $('#colors_aplite').find('#byw_background_h10').each(function(index){
        $(this)[0].value = stored_settings.KEY_COLOR_BG_H10 === "000000" ? 1 : 0;
      });
      $('#byw_background_h1')[0].value = stored_settings.KEY_COLOR_BG_H1 === "000000" ? 1 : 0;
      
      $('#colors_aplite').find('#byw_background_m10').each(function(index){
        $(this)[0].value = stored_settings.KEY_COLOR_BG_M10 === "000000" ? 1 : 0;
      });
      $('#byw_background_m1')[0].value = stored_settings.KEY_COLOR_BG_M1 === "000000" ? 1 : 0;
      
      $('#byw_background_bt')[0].value = stored_settings.KEY_COLOR_BG_BT === "000000" ? 1 : 0;
      $('#byw_background_sec')[0].value = stored_settings.KEY_COLOR_BG_SEC === "000000" ? 1 : 0;
      
    } else if (watch_version <= 4 && watch_version >= 3) {
      $('#colors_basalt').find('#color_background_h10').each(function(index){
        $(this).val(stored_settings.KEY_COLOR_BG_H10);
      });
      $('#colors_basalt').find('#color_spocks_h10').each(function(index){
        $(this).val(stored_settings.KEY_COLOR_SPOCK_H10);
      });
      $('#color_background_h1').val(stored_settings.KEY_COLOR_BG_H1);
      $('#color_spocks_h1').val(stored_settings.KEY_COLOR_SPOCK_H1);
      
      $('#colors_basalt').find('#color_background_m10').each(function(index){
        $(this).val(stored_settings.KEY_COLOR_BG_M10);
      });
      $('#colors_basalt').find('#color_spocks_m10').each(function(index){
        $(this).val(stored_settings.KEY_COLOR_SPOCK_M10);
      });
      $('#color_background_m1').val(stored_settings.KEY_COLOR_BG_M1);
      $('#color_spocks_m1').val(stored_settings.KEY_COLOR_SPOCK_M1);
      
      $('#color_background_bt').val(stored_settings.KEY_COLOR_BG_BT);
      $('#color_spocks_bt').val(stored_settings.KEY_COLOR_SPOCK_BT);
      $('#color_background_sec').val(stored_settings.KEY_COLOR_BG_SEC);
      $('#color_spocks_sec').val(stored_settings.KEY_COLOR_SPOCK_SEC);
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

function getConfigData() {
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
  
  if(watch_version <= 2 && watch_version > 0) {
    var collection = $('#colors_aplite').find('.pattern-'+options.KEY_COLOR_PATTERN);
    var black = '000000';
    var white = 'FFFFFF';
    var val;
    
    val = collection.find('#byw_background_h10').first()[0].value;
    options.KEY_COLOR_BG_H10 = val == 0 ? white : black;
    options.KEY_COLOR_SPOCK_H10 = val == 0 ? black : white;
    
    val = $('#byw_background_h1')[0].value;
    options.KEY_COLOR_BG_H1 = val == 0 ? white : black;
    options.KEY_COLOR_SPOCK_H1 = val == 0 ? black : white;
    
    if(options.KEY_COLOR_PATTERN >= 2) {
      val = collection.find('#byw_background_m10').first()[0].value;
    } else {
      val = $('#byw_background_m10')[0].value;
    }
    options.KEY_COLOR_BG_M10 = val == 0 ? white : black;
    options.KEY_COLOR_SPOCK_M10 = val == 0 ? black : white;
    
    val = $('#byw_background_m1')[0].value;
    options.KEY_COLOR_BG_M1 = val == 0 ? white : black;
    options.KEY_COLOR_SPOCK_M1 = val == 0 ? black : white;
    
    val = $('#byw_background_bt')[0].value;
    options.KEY_COLOR_BG_BT = val == 0 ? white : black;
    options.KEY_COLOR_SPOCK_BT = val == 0 ? black : white;
    
    val = $('#byw_background_sec')[0].value;
    options.KEY_COLOR_BG_SEC = val == 0 ? white : black;
    options.KEY_COLOR_SPOCK_SEC = val == 0 ? black : white;
  } else if (watch_version <= 4 && watch_version >= 3) {
    var collection = $('#colors_basalt').find('.pattern-'+options.KEY_COLOR_PATTERN);
    
    options.KEY_COLOR_BG_H10 = collection.find('#color_background_h10').first().val();
    options.KEY_COLOR_SPOCK_H10 = collection.find('#color_spocks_h10').first().val();
    
    options.KEY_COLOR_BG_H1 = $('#color_background_h1').val();
    options.KEY_COLOR_SPOCK_H1 = $('#color_spocks_h1').val();
    
    if(options.KEY_COLOR_PATTERN >= 2) {
      options.KEY_COLOR_BG_M10 = collection.find('#color_background_m10').first().val();
      options.KEY_COLOR_SPOCK_M10 = collection.find('#color_spocks_m10').first().val();
    } else {
      options.KEY_COLOR_BG_M10 = $('#color_background_m10').val();
      options.KEY_COLOR_SPOCK_M10 = $('#color_spocks_m10').val();
    }
    
    options.KEY_COLOR_BG_M1 = $('#color_background_m1').val();
    options.KEY_COLOR_SPOCK_M1 = $('#color_spocks_m1').val();
    
    options.KEY_COLOR_BG_BT = $('#color_background_bt').val();
    options.KEY_COLOR_SPOCK_BT = $('#color_spocks_bt').val();
    
    options.KEY_COLOR_BG_SEC = $('#color_background_sec').val();
    options.KEY_COLOR_SPOCK_SEC = $('#color_spocks_sec').val();
  }

  console.log('Got options: ' + JSON.stringify(options));
  return options;
}
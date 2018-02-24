const lc3interface = require('bindings')('addon');
const sprintf = require('sprintf-js').sprintf;

lc3interface.InitializeSimulator();
$('#console').empty();
$('#console').html(lc3interface.GetOutput());
lc3interface.ClearOutputBuffer();

$('.reg-name').each(function() {
    var reg_name = $(this).html().toLowerCase();
    if(reg_name == 'cc') {
        var val = lc3interface.GetRegValue('psr');
        var cc = 'N';
        if((val & 0x0001) == 1) {
            cc = 'P';
        } else if((val & 0x0002) == 2) {
            cc = 'Z';
        }
        $('#cc-str').html(cc);
    } else {
        var val = lc3interface.GetRegValue(reg_name);
        var hex_val = sprintf('0x%04X', val);
        $('#' + reg_name + '-hex').html(hex_val);
        $('#' + reg_name + '-dec').html(val);
    }
});

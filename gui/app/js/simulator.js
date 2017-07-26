const lc3interface = require('bindings')('addon');

var numberOfRows = Math.ceil($('#mem-panel').height() / 22);
var memTable = $('#mem-table > tbody');
for (var i = 0; i < numberOfRows; i++) {
    memTable.append('<tr class="mem-addr-' + i + '"><td>Thing</td></tr>');
}

lc3interface.InitializeSimulator();
$('#console').empty();
$('#console').html(lc3interface.GetOutput());
lc3interface.ClearOutputBuffer();

$('.reg-name').each(function() {
    var reg_name = $(this).html().toLowerCase();
    if(reg_name == 'cc') {
        var val = lc3interface.GetRegValueHex('psr');
        var cc = 'N';
        if((val & 0x0001) == 1) {
            cc = 'P';
        } else if((val & 0x0002) == 2) {
            cc = 'Z';
        }
        $('#cc-str').html(cc);
    } else {
        var hex_val = lc3interface.GetRegValueHex(reg_name);
        var dec_val = lc3interface.GetRegValueDec(reg_name);
        $('#' + reg_name + '-val').html(hex_val + ' (' + dec_val + ')');
    }
});

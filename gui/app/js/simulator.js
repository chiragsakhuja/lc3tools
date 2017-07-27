const lc3interface = require('bindings')('addon');
const sprintf = require('sprintf-js').sprintf;

function generateMemRow(addr)
{
    var hex_addr = sprintf('0x%04x', addr);
    var val = lc3interface.GetMemValue(addr);
    var bin_val = sprintf('%016b', val);
    var hex_val = sprintf('0x%04X', val);
    var line = lc3interface.GetMemLine(addr);

    var html = '<tr class="mem-value" id="' + addr + '">'
    html += '<td class="mem-bp"></td>';
    html += '<td class="mem-addr">' + hex_addr + '</td>';
    html += '<td class="mem-bin">' + bin_val + '</td>';
    html += '<td class="mem-hex">' + hex_val + '</td>';
    html += '<td class="mem-line">' + line + '</td>';
    html += '</tr>'

    return html;
}

lc3interface.InitializeSimulator();
$('#console').empty();
$('#console').html(lc3interface.GetOutput());
lc3interface.ClearOutputBuffer();

var numberOfRows = Math.ceil($('#mem-panel').height() / $("#r0-val").height());
var memTable = $('#mem-table > tbody');
for (var i = 0; i < numberOfRows; i++) {
    memTable.append(generateMemRow(i + 0x1E));
}

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
        $('#' + reg_name + '-val').html(hex_val + ' (' + val + ')');
    }
});

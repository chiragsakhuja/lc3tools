const rowHeight = $('#r0-hex').height();
const numRowsDisplayed = Math.ceil($('#mem-panel').height() / rowHeight);

var currentMemBottom = 0;
var prevScrollTop = 0;

function generateMemRow(addr)
{
    var hex_addr = sprintf('0x%04X', addr);
    var val = lc3interface.GetMemValue(addr);
    var bin_val = sprintf('%016b', val);
    var hex_val = sprintf('0x%04X', val);
    var line = lc3interface.GetMemLine(addr);

    var html = '<tr class="mem-value" id="' + addr + '">';
    html += '<td class="mem-bp"></td>';
    html += '<td class="mem-addr">' + hex_addr + '</td>';
    html += '<td class="mem-bin">' + bin_val + '</td>';
    html += '<td class="mem-hex">' + hex_val + '</td>';
    html += '<td class="mem-line">' + line + '</td>';
    html += '</tr>'

    return html;
}

currentMemBottom = Math.ceil($('#mem-panel').height() / rowHeight) + 20;
var memTable = $('#mem-table > tbody');
for (var i = 0; i < currentMemBottom; i++) {
    memTable.append(generateMemRow(i));
}

$('#mem-panel-body').scroll(function() {
    var curScrollTop = $(this).scrollTop();
    var speed = curScrollTop - prevScrollTop;
    prevScrollTop = curScrollTop;

    if((curScrollTop + $(this).height()) / rowHeight > currentMemBottom) {
        for (var i = 0; i < numRowsDisplayed; i++) {
            memTable.append(generateMemRow(currentMemBottom + i));
        }

        currentMemBottom += numRowsDisplayed;
    }

    var scrollbarHeight = $('#mem-scrollbar').height();
    var bar = $('#mem-scrollbar-bar');
    var barHeight = bar.height();
    var barPos = Math.floor(((curScrollTop / rowHeight) / 65536) * (scrollbarHeight - barHeight));
    bar.css('top', '' + barPos + 'px');
});

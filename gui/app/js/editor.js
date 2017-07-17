const lc3interface = require('bindings')('addon');
const {dialog} = require('electron').remote;
const fs = require('fs');

var activeFileName = null;

function saveFile()
{
    if(activeFileName === null) {
        dialog.showSaveDialog({filters: [{name: 'lc3', extensions: ['asm']}]},
            function(fileName) {
                if(fileName === undefined) return;
                activeFileName = fileName;
                fs.writeFile(activeFileName, window.editor.getValue(), function(err) {
                    dialog.showErrorBox('File save error', err.message);
                });
            }
        );
    }

    if(activeFileName !== undefined) {
        fs.writeFile(activeFileName, window.editor.getValue(), function(err) {
            if(err !== null) {
                dialog.showErrorBox('File save error', err.message);
            }
        });
    }
}        

function openFile()
{
    dialog.showOpenDialog({filters: [{name: 'lc3', extensions: ['asm']}]},
        function(fileNames) {
            if(fileNames === undefined) return;
            activeFileName = fileNames[0];
            fs.readFile(activeFileName, 'utf-8', function(err, data) {
                if(err !== null) {
                    dialog.showErrorBox('File open error', err.message);
                    return;
                }
                window.editor.setValue(data);
            });
        }
    );
}

$('#toolbar-build').on('click', function(e) {
    try {
        lc3interface.Assemble(activeFileName);
    } catch(err) {
        console.log(err.message);
    }
    $('#console').empty();
    $('#console').html(lc3interface.GetOutput());
    $('#console-panel').stop().animate({
        scrollTop: $('#console-panel')[0].scrollHeight
    }, 200);
})

$('#toolbar-save').on('click', function(e) { saveFile(); });

$('#toolbar-open').on('click', function(e) { openFile(); });

$(window).resize(function() {
    window.editor.layout();
});

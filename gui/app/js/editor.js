const lc3interface = require('bindings')('addon');
const {dialog} = require('electron').remote;
const fs = require('fs');

var activeFileName = undefined;

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

$('#toolbar-save').on('click', function(e) {
    if(activeFileName === undefined) {
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
            dialog.showErrorBox('File save error', err.message);
        });
    }
});

$('#toolbar-open').on('click', function(e) {
    dialog.showOpenDialog({filters: [{name: 'lc3', extensions: ['asm']}]},
        function(fileNames) {
            if(fileNames === undefined) return;
            activeFileName = fileNames[0];
            fs.readFile(activeFileName, 'utf-8', function(err, data) {
                if(err !== undefined) {
                    dialog.showErrorBox('File open error', err.message);
                    return;
                }
                window.editor.setValue(data);
            });
        }
    );
});

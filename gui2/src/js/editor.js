const lc3interface = require('lc3interface')

$('#build-button').on('click', (e) => {
    try {
        lc3interface.Assemble('test.asm');
    } catch(err) {
        console.log(err);
    }

    var console = $('#console');
    console.html(lc3interface.GetOutput());
    lc3interface.ClearOutputBuffer();
});

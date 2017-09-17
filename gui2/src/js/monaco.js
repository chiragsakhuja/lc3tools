function getMonarchDescription()
{
    return {
        ignoreCase: true,

        instructions: [
            'add', 'and', 'br', 'brn', 'brz', 'brp', 'brnz', 'brzp', 'brnp', 'brnzp',
            'jmp', 'jsr', 'jsrr', 'ld', 'ldi', 'ldr', 'lea', 'not', 'ret', 'rti', 'st',
            'sti', 'str', 'trap', 'getc', 'out', 'puts', 'in', 'putsp', 'halt'
        ],

        pseudo: [
            '.orig', '.end', '.fill', '.stringz', '.blkw'
        ],

        // C# style strings
        escapes: /\\(?:[abfnrtv\\"']|x[0-9A-Fa-f]{1,4}|u[0-9A-Fa-f]{4}|U[0-9A-Fa-f]{8})/,

        // The main tokenizer for our languages
        tokenizer: {
            root: [
                // numbers
                [/0?[xX]-?[0-9a-f]+/, 'number'],
                [/b-?[01]+/, 'number'],
                [/#?-?\d+/, 'number'],

                // strings
                [/"([^"\\]|\\.)*$/, 'string.invalid' ],  // non-teminated string
                [/"/,  { token: 'string.quote', bracket: '@open', next: '@string' } ],

                // identifiers and keywords
                [/[a-z]\w+/, { cases: { '@instructions': 'keyword',
                                        '@default': 'identifier' } }],
                [/\.[a-z]\w+/, { cases: { '@pseudo': 'pseudo',
                                          '@default': 'identifier' } }],

                // whitespace
                { include: '@whitespace' }
            ],

            string: [
                [/[^\\"]+/ , 'string'],
                [/@escapes/, 'string.escape'],
                [/\\./     , 'string.escape.invalid'],
                [/"/       , { token: 'string.quote', bracket: '@close', next: '@pop' } ]
            ],

            whitespace: [
                [/[ \t\r\n]+/, 'white'],
                [/\/\/.*$/   , 'comment'],
                [/;.*$/      , 'comment'],
            ]
        }
    };
}

function uriFromPath(_path) {
    var pathName = path.resolve(_path).replace(/\\/g, '/');
    if (pathName.length > 0 && pathName.charAt(0) !== '/') {
        pathName = '/' + pathName;
    }
    return encodeURI('file://' + pathName);
}

// require node modules before loader.js comes in
var path = require('path');

amdRequire.config({
    baseUrl: uriFromPath(path.join(__dirname, '../node_modules/monaco-editor/min'))
});

// workaround monaco-css not understanding the environment
self.module = undefined;

// workaround monaco-typescript not understanding the environment
self.process.browser = true;

amdRequire(['vs/editor/editor.main'], function() {
    // Register a new language
    monaco.languages.register({ id: 'lc3' });

    // Register a tokens provider for the language
    monaco.languages.setMonarchTokensProvider('lc3', getMonarchDescription());

    // Define a new theme that constains only rules that match this language
    monaco.editor.defineTheme('lc3theme', {
        base: 'vs-dark',
        inherit: true,
        rules: [
            { token: 'pseudo', foreground: 'ff0000' },
            { token: 'comment', foreground: '666666' }
        ]
    });

    // Register a completion item provider for the new language
    monaco.languages.registerCompletionItemProvider('lc3', {
        provideCompletionItems: () => { return [] }
    });

    window.editor = monaco.editor.create(document.getElementById('editor'), {
        value: '.orig x3000\n\n    halt\n.end',
        language: 'lc3',
        theme: 'lc3theme'
    });
});

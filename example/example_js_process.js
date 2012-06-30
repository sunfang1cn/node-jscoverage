var jsc = require('jscoverage');
var fs = require('fs');

var processFile = jsc.processFile('./demo/hello.js');
console.log('=== processFile > demo/hello_instrumented.js ===');
console.log(processFile);
fs.writeFileSync('./demo/hello_instrumented.js',processFile);
console.log('');
console.log('');

var content = 'function test(a,b){\
  if(a >= b){\
    return a;\
  }else{\
    return b;\
  }\
}';
var processContent = jsc.process('test_demo',content);
console.log('=== processContent ===');
console.log(processContent);
console.log('');
console.log('');

jsc.processDir('./demo','./demo_instrumented','.svn');
console.log('=== processDir ===');
console.log('');
console.log('');

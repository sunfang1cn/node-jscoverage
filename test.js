expect = require('expect.js');
var jsc = require('./index');
require = jsc.mock(module);
var testCase = require('./test_case.js',true);

describe("jscoverage",function(){
  it('#_rest()',function(){
    testCase._replace('conn',function(cfg){
      return db.close(cfg);
    });
    var res = testCase._call('conn',[{}]);
    expect(res).to.be('close');
  });
  it('#_replace()',function(){
    testCase._reset('conn');
    var conn = testCase._get('conn');
    var res = conn({})
    expect(res).to.be('conn');
  });
});

process.on('exit',function(){
  jsc.coverage();
  jsc.coverageDetail();
});

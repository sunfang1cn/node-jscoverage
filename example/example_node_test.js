var jsc = require('jscoverage');
var expect = require('expect.js');
require = jsc.require(module);
var hello = require('./demo/hello',true);

describe('Module hello',function(){
  it('#hello',function(){
      expect(hello.hello()).to.be('hello'); 
  });

  it('#msg',function(){
    console.log(hello.msg());
    expect(hello.msg().length).to.not.be(0);
  });

  it('#translate',function(){
    console.log(hello.translate());
    expect(hello.translate().length).to.not.be(0);
  });
  
  it('#save',function(done){
    hello._replace('DB',{
      save:function(name,cb){
        if(name === 'true'){
          cb(null,[{date:'2012-01-02',msg:'x'}]);
        }else{
          cb(true,[{date:'2012-01-02',msg:'x'}]); 
        }
      }
    });
    var count = 0;
    hello.save('false',function(err,data){
      expect(err).to.match(/db save error/);
      end();
    });
    hello.save('true',function(err,data){
      expect(data).to.eql([{date:'2012-01-02',msg:'x',label:'今年'}]);
      end();
    });
    function end(){
      count ++ ;
      if(count >= 2){
        done();
      }
    }
  });

  it('#inner:process',function(){
    var data = hello._call('process',[[{date:'2010-10-10'},{date:'2012-06-30'}]]);
    expect(data).to.be.eql([{date:'2010-10-10'},{date:'2012-06-30',label:'今年'}]);
  });

});

process.on('exit',function(){
  jsc.coverage();
});

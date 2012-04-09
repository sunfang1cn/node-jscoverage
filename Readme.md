# node-jscoverage

   [JScoverage](http://siliconforks.com/jscoverage/) for node.

## Installation

      $ npm  install jscoverage
    
## How to Use
      /** for nodejs **/
      var jsc = require('jscoverage');
      require = jsc.require(module); // rewrite require function
      var test_module = require('module for test',true); // pay attention to the second param, means do jscoverage process for this module
      
      describe('module',function(){
         describe('#func',function(){
            it('some test',function(){
               //TODO test code here
            });
         });
      });
      // output the coverage when test process exit, you can custom your own coverage report 
      // because the jscoverage report is really easy to DIY
      process.on('exit',function(){
         jsc.coverage();
      });
      
      /** 
         for javascript in browser,you need a node server to hold your js code,
         when browser request the js file, using jscoverage to process the file content,
         then output the jscoverage injected code .
      **/
      http.create(function(req,res){
         var path = wwwroot + req.url;
         // process single file
         var content = jsc.processFile(path,encode);
         res.end(content);
      }
      
   
## API

      jsc.processFile(path|content,encode);
         path | content 待转化的文件地址，或文件内容
         encode 文件编码
      jsc.processDir(source_path, dest_path, exclude, exclude_file, options);
      jsc.require(module)
      jsc.coverage()
      
## test private functions in a module

      ======== mo.js ========
      function inner(a,b){
         return a+b;
      }
      exports.mo = function(){}
      
      ======= test.js =======
      var test_mo = require('./mo.js', true);
      test_mo._test('inner',[1,2]); // test private function inner
      var inner = test_mo._get('inner');
      inner(1,2); // call the private inner function of the mo module 
      


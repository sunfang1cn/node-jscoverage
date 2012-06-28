# node-jscoverage

   [JScoverage](http://siliconforks.com/jscoverage/) for node. Have been Modified to works both for binary executables and node.js modules.

## Installation
   for use in node.js modules:
      $ npm  install jscoverage
  
   for use by binary executables just like the source version, just pull it to local and:
      
      $ ./configure && make && make install

## How to Use
      /** for nodejs **/
      var jsc = require('jscoverage');
      require = jsc.require(module); // rewrite require function
      // pay attention to the second param, means do jscoverage process for this module
      var test_module = require('module for test',true); 
      
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

      jscoverage.processFile(path|content, encode): process single file or some js content, will return the process results
         path | content: the file to process or the content to process
         encode: the charset of the file or content

      jscoverage.processDir(source_path, dest_path, exclude, exclude_file, options): process every js file in a dir
         source_path: the dir to process
         dest_path: the output dir to put result files
         exclude: the dir to exclude
         exclude_file: the file to exclude

      jscoverage.require(module, flag): automatic process when flag is true, will mock the node.js`s require

      jscoverage.coverage(): show coverage rate on console
      1
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
      


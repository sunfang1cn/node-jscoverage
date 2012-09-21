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

      jscoverage.processFile(path, encode): process single file, will return the process results
         path: the file to process
         encode: the charset of the file or content , encode: utf-8,gbk

      jscoverage.process(filename, content, encode): process content input, will return the results
         filename: this param is needed for _$jscoverage[filename], it is used as a key name.
         content: script content,string type
         encode: input content encode

      jscoverage.processDir(source_path, dest_path, exclude, exclude_file, options): process every js file in a dir
         source_path: the dir to process, support relative path
         dest_path: the output dir to put result files
         exclude: the dir to exclude, like '.svn|.git'
         exclude_file: the file to exclude '.npmignore|.vimrc'
         options: {}

      jscoverage.require(module): automatic process when flag is true, will mock the node.js`s require
         return the mocked require function ,which call as : 
            var mod = require(path,isInstrument);
      jscoverage.coverage(): show coverage rate on console
         you can custom this function to make your own coverage report!
   
      
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

## extend API 
   
      InstrumentedMod._test(funcName,args);
         test inner functions
      InstrumentedMod._call(funcName,args);
         same as _test
      InstrumentedMod._replace(objName,value);
         replace the inner object
      InstrumentedMod._get(objName);
         get the inner object;
      InstrumentedMod._reset(objName);
         rollback the object replace by InstrumentedMod._replace();
      
      jsc.config(opt);
         you can re-defined the above extend api by call this config function
         opt has properties:
            call:
            test:
            get:
            replace:
            reset:
            
      jsc will test if your module is conflict with this inject functions , it is save!


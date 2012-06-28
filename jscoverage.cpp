/*
    jscoverage.c - main routine for `jscoverage' program
    Copyright (C) 2007, 2008 siliconforks.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
     
    Modified by kate.sf@taobao.com for node_modules
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
extern "C" {
#include "instrument.h"
#include "instrument-js.h"
#include "encoding.h"
#include "stream.h"
}
#include "resource-manager.h"
#include "util.h"

//for node v8
#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace std;
using namespace node;
using namespace v8;

#define REQ_FUN_ARG(I, VAR) \
if (args.Length() <= (I) || !args[I]->IsFunction()) \
return ThrowException(Exception::TypeError( \
String::New("Argument " #I " must be a function"))); \
Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_STR_ARG(I, VAR) \
if (args.Length() <= (I) || !args[I]->IsString()) \
return ThrowException(Exception::TypeError( \
String::New("Argument " #I " must be a String"))); \
Local<String> VAR = Local<String>::Cast(args[I]);

#define REQ_OBJ_ARG(I, VAR) \
if (args.Length() <= (I) || !args[I]->IsObject()) \
return ThrowException(Exception::TypeError( \
String::New("Argument " #I " must be an Object"))); \
Local<Object> VAR = args[I]->ToObject();
//end for node v8

const char * jscoverage_encoding = "ISO-8859-1";
bool jscoverage_highlight = true;


//for node v8

//cut the options and so on
char * proc_msg_string(char * mesg, int type) {
    char segm[3][1000];
    char * p;
    char req[20000]={0};
    int seg_count = 0, i = 0;
    p = strtok (mesg, "|"); 
    while(p!=NULL) { 
        strcpy(segm[seg_count],p);;
        seg_count++;
        p = strtok(NULL, "|"); 
    } 
    if(type==0) { //options
        for(i=0; i<seg_count; i++) {
            strcat(req, " --");
            strcat(req, segm[i]);
        }
    }
    if(type==1) { //exclude
        for(i=0; i<seg_count; i++) {
            strcat(req, " --exclude=");
            strcat(req, segm[i]);
        }
    }
    if(type==2) { //no-inst
        for(i=0; i<seg_count; i++) {
            strcat(req, " --no-instrument=");
            strcat(req, segm[i]);
        }
    }
    return req;
}

class Jscoverage : public ObjectWrap
{
private:
    int m_count;
public:

    static void Init(Handle<Object> target)
    {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        Persistent<FunctionTemplate> s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("Jscoverage"));

        NODE_SET_PROTOTYPE_METHOD(s_ct, "doDirSync", doSync);
        NODE_SET_PROTOTYPE_METHOD(s_ct, "doBufferSync", doBufferSync);

        target->Set(String::NewSymbol("Jscoverage"),
                    s_ct->GetFunction());
    }

    Jscoverage() :
        m_count(0)
    {
    }

    ~Jscoverage()
    {
    }

    static Handle<Value> New(const Arguments& args)
    {
        HandleScope scope;
        Jscoverage* hw = new Jscoverage();
        hw->Wrap(args.This());
        return args.This();
    }
    
    static Handle<Value> doBufferSync(const Arguments& args)
    {
      HandleScope scope;
      REQ_STR_ARG(2, fname);
      REQ_STR_ARG(1, text_enc);
      REQ_OBJ_ARG(0, bobj);
      String::AsciiValue _encoding(text_enc);
      String::AsciiValue _fname(fname);
      char * _buf = Buffer::Data(bobj);
      size_t buf_len = Buffer::Length(bobj);
      
      
      Stream * input_stream = Stream_new(0);
      Stream * output_stream = Stream_new(0);
      
      Stream_write(input_stream, _buf, buf_len);
      
      size_t num_characters = input_stream->length;
      uint16_t * characters = NULL;
      int result = jscoverage_bytes_to_characters(*_encoding, input_stream->data, input_stream->length, &characters, &num_characters);
      if (result == JSCOVERAGE_ERROR_ENCODING_NOT_SUPPORTED) {
          fatal("encoding %s not supported", *_encoding);
      }
      else if (result == JSCOVERAGE_ERROR_INVALID_BYTE_SEQUENCE) {
          fatal("error decoding %s in file %s", *_encoding, *_fname);
      }
      
      jscoverage_highlight = false;
      jscoverage_init();
      jscoverage_instrument_js(*_fname, characters, num_characters, output_stream);
      jscoverage_cleanup();
      free(characters);

      char * output_buffer = (char *)malloc(output_stream->length);
      size_t out_len = output_stream->length;
      memcpy(output_buffer, output_stream->data, output_stream->length);
      Stream_delete(input_stream);
      Stream_delete(output_stream);
      
      v8::Local<v8::Object> globalObj = v8::Context::GetCurrent()->Global();
      v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));
      v8::Handle<v8::Value> constructorArgs[3] = { Buffer::New(output_buffer, out_len)->handle_, v8::Integer::New(out_len), v8::Integer::New(0) };
      v8::Local<v8::Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
      
      return scope.Close(actualBuffer);
    }
    
    static Handle<Value> doSync(const Arguments& args)
    {
      HandleScope scope;
            

      REQ_STR_ARG(0, osource); String::AsciiValue _source(osource);
      REQ_STR_ARG(1, dest);   String::AsciiValue _dest(dest);
      REQ_STR_ARG(2, oexclude);String::AsciiValue _exclude(oexclude);
      REQ_STR_ARG(3, no_inst);String::AsciiValue _no_inst(no_inst);
      REQ_STR_ARG(4, ooptions);String::AsciiValue _options(ooptions);
            
      Jscoverage* hw = ObjectWrap::Unwrap<Jscoverage>(args.This());
            
      char reqstr[50000] = {0};
      char **argv = (char **)malloc( 100 * sizeof(char *) );
      for (int k=0;k<100;k++)
         argv[k] = (char *)malloc( 300 * sizeof(char) );  
            
      strcat(reqstr, *_source);
      strcat(reqstr, " ");
      strcat(reqstr, *_dest);
      strcat(reqstr, proc_msg_string(*_exclude, 1));
      strcat(reqstr, proc_msg_string(*_no_inst, 2));
      strcat(reqstr, proc_msg_string(*_options, 0));

      strcpy(argv[0], "jscoverage");
      int argc = 1;
      char * p;
      p = strtok (reqstr, " "); 
      while(p!=NULL) { 
           strcpy(argv[argc],p);
           argc++;
           p = strtok(NULL, " "); 
      } 
      int ret = 0;
            
      int verbose = 0;
      
      // program = argv[0];
      program = "jscoverage";

      char * source = NULL;
      char * destination = NULL;

      char ** no_instrument = xnew(char *, argc - 1);
      int num_no_instrument = 0;

      char ** exclude = xnew(char *, argc - 1);
      int num_exclude = 0;
      
      jscoverage_highlight = false;

      for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
          copy_resource_to_stream("jscoverage-help.txt", stdout);
          exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
          version();
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
          verbose = 1;
        }
        else if (strcmp(argv[i], "--mozilla") == 0) {
          jscoverage_mozilla = true;
          jscoverage_set_js_version("180");
        }
        else if (strcmp(argv[i], "--no-instrument") == 0) {
          i++;
          if (i == argc) {
            fatal_command_line("--no-instrument: option requires an argument");
          }
          no_instrument[num_no_instrument] = argv[i];
          num_no_instrument++;
        }
        else if (strncmp(argv[i], "--no-instrument=", 16) == 0) {
          no_instrument[num_no_instrument] = argv[i] + 16;
          num_no_instrument++;
        }
        else if (strcmp(argv[i], "--exclude") == 0) {
          i++;
          if (i == argc) {
            fatal_command_line("--exclude: option requires an argument");
          }
          exclude[num_exclude] = argv[i];
          num_exclude++;
        }
        else if (strncmp(argv[i], "--exclude=", 10) == 0) {
          exclude[num_exclude] = argv[i] + 10;
          num_exclude++;
        }
        else if (strcmp(argv[i], "--encoding") == 0) {
          i++;
          if (i == argc) {
            fatal_command_line("--encoding: option requires an argument");
          }
          jscoverage_encoding = argv[i];
        }
        else if (strncmp(argv[i], "--encoding=", 11) == 0) {
          jscoverage_encoding = argv[i] + 11;
        }
        else if (strcmp(argv[i], "--js-version") == 0) {
          i++;
          if (i == argc) {
            fatal_command_line("--js-version: option requires an argument");
          }
          jscoverage_set_js_version(argv[i]);
        }
        else if (strncmp(argv[i], "--js-version=", 13) == 0) {
          jscoverage_set_js_version(argv[i] + 13);
        }
        else if (strncmp(argv[i], "-", 1) == 0) {
          fatal_command_line("unrecognized option `%s'", argv[i]);
        }
        else if (source == NULL) {
          source = argv[i];
        }
        else if (destination == NULL) {
          destination = argv[i];
        }
        else {
          fatal_command_line("too many arguments");
        }
      }

      if (source == NULL || destination == NULL) {
        fatal_command_line("missing argument");
      }
      
      //exclude .svn .cvs .git dictionary
      exclude[num_exclude] = ".svn";
      exclude[num_exclude+1] = ".git";
      exclude[num_exclude+2] = ".cvs";
      num_exclude += 3;
      
      source = make_canonical_path(source);
      destination = make_canonical_path(destination);

      jscoverage_init();
      jscoverage_instrument(source, destination, verbose, exclude, num_exclude, no_instrument, num_no_instrument);
      jscoverage_cleanup();

      free(source);
      free(destination);
      free(exclude);
      free(no_instrument);

      exit(EXIT_SUCCESS);
        
      free(argv);
      return scope.Close(Number::New(ret));
    }

};



extern "C" void init (Handle<Object> target) {
      HandleScope scope;
      Jscoverage::Init(target);
}
//end for node v8

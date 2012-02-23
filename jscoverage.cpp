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
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "instrument.h"
#include "instrument-js.h"
#include "resource-manager.h"
#include "util.h"

//for node v8
#include <v8.h>
#include <node.h>
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
//end for node v8

const char * jscoverage_encoding = "ISO-8859-1";
bool jscoverage_highlight = true;

int main(int argc, char ** argv) {
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
}



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

class Jscoverage: ObjectWrap
{
private:
    int m_count;
public:

    static Persistent<FunctionTemplate> s_ct;
    static void Init(Handle<Object> target)
    {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("jscoverage"));

        NODE_SET_PROTOTYPE_METHOD(s_ct, "doDirSync", doSync);

        target->Set(String::NewSymbol("jscoverage"),
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

    static Handle<Value> doSync(const Arguments& args)
    {
        HandleScope scope;
        

        REQ_STR_ARG(0, source); String::AsciiValue _source(source);
        REQ_STR_ARG(1, dest);   String::AsciiValue _dest(dest);
        REQ_STR_ARG(2, exclude);String::AsciiValue _exclude(exclude);
        REQ_STR_ARG(3, no_inst);String::AsciiValue _no_inst(no_inst);
        REQ_STR_ARG(4, options);String::AsciiValue _options(options);
        
        Jscoverage* hw = ObjectWrap::Unwrap<Jscoverage>(args.This());
        
        char reqstr[50000] = {0};
        char argv[100][300] = {{0}};
        
        strcat(reqstr, *_source);
        strcat(reqstr, " ");
        strcat(reqstr, *_dest);
        strcat(reqstr, *_exclude);
        strcat(reqstr, *_no_inst);
        strcat(reqstr, *_options);
        
        strcpy(argv[0], "jscoverage");
        int argc = 1;
        char * p;
        p = strtok (reqstr, " "); 
        while(p!=NULL) { 
            strcpy(argv[argc],p);;
            argc++;
            p = strtok(NULL, " "); 
        } 
        int ret;
        try
        {
            ret = main(argc, (char **)argv);
        }
        catch(Exception * e)
        {
            ret = -1;
        }
        return scope.Close(Number::New(ret));
    }

};

Persistent<FunctionTemplate> Jscoverage::s_ct;

extern "C" {
    static void init (Handle<Object> target)
    {
        Jscoverage::Init(target);
    }

    NODE_MODULE(Jscoverage, init);
}
//end for node v8

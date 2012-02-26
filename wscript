import os
import sys

def make(ctx, rule):
	node_path = ctx.env['CPPPATH_NODE']

	bin = './build.sh'
	cmd = 'export NODE_INCLUDE="%s"&&%s' % (node_path, bin)
	os.system(cmd)
	os.system(cmd)
	cmd_clean = 'rm *.c *.cpp -f && make clean'
	os.system(cmd_clean);

def set_options(ctx):
	pass

def configure(ctx):
	ctx.check_tool('node_addon')

def build(ctx):
	make(ctx, 'all')

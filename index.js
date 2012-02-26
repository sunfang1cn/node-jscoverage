/***
 * Jscoverage for node
 * Based on jscoverage@siliconforks.com
 * Modified by kate.sf@taobao.com
 ***/

var fs = require('fs');
var _jscoverage = require('./jscoverage.node')
var _jsclass = new _jscoverage.Jscoverage();

exports.processSingle = function(filename, content, encode) {

  if(content!=null && typeof(content)==='Object' && encode!=null) 
    var _content = content;
  if(content!=null && typeof(content)==='string' && encode==null)
    var _encode = content;
  if(encode!=null && typeof(encode)==='string')
    var _encode = encode;
  if(filename==null || typeof(filename)!='string') {
    console.error('filename is required.');
    return null;
  }
  if(_encode==null)  var _encode = 'utf-8';
 
  if(_content!=null) {
    var _buf = _jsclass.doBufferSync(_content, _encode, filename);
    return _buf.toSting(_encode);
  }else{
    var _stats = fs.statSync(filename);
    if(!_stats.isFile()) {
      console.error('file is not correct.');
      return null;
    }
    var _buffer = new Buffer(_stats.size);
    var _fd = fs.openSync(filename, 'r');
    fs.readSync(_fd, _buffer, 0, _stats.size, 0);
    var _buf = _jsclass.doBufferSync(_buffer, _encode, filename);
    return _buf.toString(_encode);
  }
}

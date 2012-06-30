var HELLO = ['o','l','l','e','h'];
var DB // = require('some db');

exports.hello = function(){
  return HELLO.reverse().join('');
};

exports.msg = function(){
  return '@gmm:Tonight go party,I\'m a sexy girl.I sometimes good sometimes bad，this is me,you don\'t like me you can get out！';
};

exports.translate = function(){
  return '@热心网友翻译：子时趋党部，妾本是性奴；间或有良运，间或艺稍疏；本色方是我，任君来去如。'
};

exports.save = function(name,cb){
  DB.save(name,function(err,data){
    if(err){
      err = new Error('db save error!!!');
    }else{
      data = process(data);
    }
    cb(err,data);
  });
}

function process(data){
console.log(data);
  data.forEach(function(v,i,a){
    if(v.date >= '2012-01-01'){
      v.label = '今年'
    }
  });
  return data;
}


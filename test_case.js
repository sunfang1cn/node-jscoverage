var base = {
  host:'127.0.0.1',
  port:80
};
var db = {
  conn:function(){
    return 'conn';
  },
  close:function(){
    return 'close';
  }
};

function conn(cfg){
  return db.conn(cfg);
}


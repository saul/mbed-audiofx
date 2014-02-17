function renderTemplateRemote(path, callback) {
  $.ajax({
    url: 'app://templates/' + path,
    cache: true,
    success: function(data) {
      var template = Handlebars.compile(data);
      callback(template);
    }
  });
}


Handlebars.registerHelper('ifEq', function(v1, v2, options) {
  if(v1 === v2) {
    return options.fn(this);
  }
  return options.inverse(this);
});

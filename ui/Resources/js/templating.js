/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 * templating.js - Renders HTML templates using the Handlebars template
 * library.
 */

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


Handlebars.registerHelper('exists', function(variable, options) {
		if (typeof variable !== 'undefined') {
				return options.fn(this);
		} else {
				return options.inverse(this);
		}
});

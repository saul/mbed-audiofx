/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 * ui.js - Handles UI interaction events.
 */

$(function() {
	$('#reset-board').click(function() {
		// Send a reset packet to the board
		var packet = ResetPacket(serialStream);
		packet.send();
	});

	$('#clear-console').click(function() {
		$('#console-text').html('');
	});

	$('#command-prompt').keypress(function(event) {
		var $this = $(this);

		// We're only interested in the enter key
		if(event.which != 13)
			return;

		// Send command packet
		packet = CommandPacket(serialStream);
		packet.send($this.val());

		// Clear the prompt
		$this.val('');
	});

	$('#save-chain-name').keypress(function(event) {
		var $this = $(this);

		// We're only interested in the enter key
		if(event.which != 13)
			return;

		// Clean up text (trim spaces and slugify)
		var name = $.trim($this.val())
					.toLowerCase()
		       		.replace(/[^\w ]+/g,'')
		       		.replace(/ +/g,'-');

		// Send command packet
		packet = CommandPacket(serialStream);
		packet.send('chain_save ' + name);

		// Clear the prompt
		$this.val('');
	})

	// Simulate a board reset
	onBoardReset();
});


function onBoardReset() {
	// Clear all stages and hide container
	$('#filter-container').hide().children().remove();

	// Clear popover content
	creationPopoverContent = '(waiting...)';

	// Show initial stage
	appendStage();
}


(function($) {
	var idCounter = new Date().getTime();

	$.fn.uniqueId = function() {
		if(this.length > 1)
			throw new Error('cannot get unique ID of more than 1 element');

		var node = this.get(0);
		return node.id ? node.id : (node.id = 'uuid-' + (idCounter++));
	};
})(jQuery);


// Create a new stage row
function appendStage(completed) {
	renderTemplateRemote('filter_stage.html', function(template) {
		$('#filter-container').append(template({}));

		if(typeof completed !== 'undefined')
			completed();

		$('.stage-row:last-child .filter-create button').popover({
			html: true,
			placement: 'bottom',
			content: getCreationPopoverContent,
			container: '#' + $('.stage-row:last-child').uniqueId(),
		});
	});
}


// Append a filter to a stage
function appendFilterToStage(stageIdx, filterIdx) {
	packet = FilterCreatePacket(serialStream);
	packet.send(stageIdx, filterIdx, 0, 1.0);

	var filter = filters[filterIdx];

	renderTemplateRemote('filter.html', function(template) {
		$('.stage-row:nth-child(' + (stageIdx + 1) + ') .filter-create').before(template({
			index: filterIdx,
			filter: filter,
			enabled: false,
		}));
	});
}


function getCreationPopoverContent() {
	return creationPopoverContent;
}


// Filter title toggle
// ============================================================================
$(document).on('change', '.filter input[name="filter-enabled"]', function() {
	var $this = $(this);
	var $filter = $this.parents('.filter');
	var $stage = $this.parents('.stage-row');

	if($this.prop('checked')) {
		$filter.removeClass('panel-danger').addClass('panel-success');
	} else {
		$filter.removeClass('panel-success').addClass('panel-danger');
	}

	// Enable/disable the filter
	packet = FilterFlagPacket(serialStream);
	packet.send($stage.index(), $filter.index(), 0, $this.prop('checked'));
});


// Filter delete
// ============================================================================
$(document).on('click', '.filter .close', function() {
	if(!confirm('Delete this filter?'))
		return;

	var $this = $(this);
	var $filter = $this.parents('.filter');
	var $stage = $this.parents('.stage-row');

	// Delete the filter
	packet = FilterDeletePacket(serialStream);
	packet.send($stage.index(), $filter.index());

	// Is this the last filter in the stage?
	if($filter.siblings('.filter').length === 0) {
		var $stage = $filter.parents('.stage-row');

		// Don't delete the last stage
		if($stage.is(':last-child'))
			return;

		// Remove stage if it's empty
		$stage.remove();
		return;
	}

	// Remove the filter
	$filter.remove();
});


// Create filter button
// ============================================================================
$(document).on('change', 'select[name=filter-create]', function() {
	var $this = $(this);
	var filterIdx = $this.val();

	var $stage = $this.parents('.stage-row');

	// Close popover
	var $button = $stage.find('.filter-create button');
	$button.popover('hide');

	if(filterIdx === '-')
		return;

	var filter = filters[filterIdx];

	console.log('Creating filter "' + filter.name + '" for stage ' + $stage.index());

	// If there are no filters in this stage, create another row with a "+"
	if($stage.find('.filter').length === 0)
		appendStage();

	appendFilterToStage($stage.index(), filterIdx);
});


// Chain item clicked (Saul individual)
// ============================================================================
$(document).on('click', '#stored-chain-list a[data-name]', function(event) {
	var $this = $(event.target);

	if(!$this.hasClass('list-group-item'))
		return;

	if(!confirm('Are you sure you want to overwrite your current chain?'))
		return;

	// Send command packet
	packet = CommandPacket(serialStream);
	packet.send('chain_restore ' + $this.data('name'));

	// Go back to chain
	$('[href="#chain"]').click();
});


// Chain delete clicked (Saul individual)
// ============================================================================
$(document).on('click', '#stored-chain-list a[data-name] .close', function(event) {
	var $this = $(this);

	if(!confirm('Are you sure you want to delete this chain?'))
		return;

	// Send command packet
	packet = CommandPacket(serialStream);
	packet.send('chain_delete ' + $this.parent().data('name'));
});


// Filter parameter input change
// ============================================================================
$(document).on('change', '.form-group[data-param-name]:not([type=checkbox])', $.debounce(250, function(event) {
	updateFilterParameter($(event.target));
}));


function updateFilterParameter($this) {
	var $stage = $this.parents('.stage-row');
	var $filter = $this.parents('.filter');
	var $widget = $this.parent();
	var paramName = $widget.data('param-name');
	var filter = filters[$filter.data('filter-index')];

	var newVal = $this.val();
	var textVal = newVal;

	if(paramName === 'mix') {
		textVal = parseFloat(textVal).toFixed(3);

		// Change branch mix percentage
		packet = FilterMixPacket(serialStream);
		packet.send($stage.index(), $filter.index(), parseFloat(newVal));
	} else {
		var param = filter.params[paramName];

		if(param['f'] == 'f') {
			newVal = parseFloat(newVal);
			textVal = newVal.toFixed(3);
		}

		// Modify filter parameter data on board
		packet = FilterModPacket(serialStream);
		packet.send($stage.index(), $filter.index(), param['o'], param['f'], newVal);
	}

	// Update label text
	$this.siblings('label').children('.value').text(textVal);
}


/* Tom individual */
// Called when a user changes any of the analog control checkboxes
$(document).on('change', '.ac-checkbox', $.debounce(250, function() {
	updateAnalogControls(ac_value);
}));

var ac_value = 0;

// Update checked analoog controls with the analog value
function updateAnalogControls(new_value) {
	ac_value = new_value;

	$('.ac-checkbox:checked').each(function() {
		var $element = $(this).parents('.form-group').children('input');
		var min = parseInt($element.attr('min'));
		var max = parseInt($element.attr('max'));
		var step = parseFloat($element.attr('step'));
		var scaled_value = Math.floor((min + (max-min)*new_value) / step) * step;
		$element.val(scaled_value);
		updateFilterParameter($element);
		console.log("ac=" + new_value + ", min=" + min + ", max=" + max + ", step=" + step + " => " + scaled_value);
	});
}
/* End Tom individual */

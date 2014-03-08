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


function appendFilterToStage(stageIdx, filterIdx) {
	packet = FilterCreatePacket(serialStream);
	packet.send(stageIdx, filterIdx, 0, 1.0);

	var filter = filters[filterIdx];

	renderTemplateRemote('filter.html', function(template) {
		$('.stage-row:nth-child(' + (stageIdx + 1) + ') .filter-create').before(template({
			index: filterIdx,
			filter: filter,
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

	packet = FilterDeletePacket(serialStream);
	packet.send($stage.index(), $filter.index());

	// Is this the last filter in the stage?
	if($filter.siblings('.filter').length === 0) {
		var $stage = $filter.parents('.stage-row');

		// Don't delete the last stage
		if($stage.is(':last-child'))
			return;

		$stage.remove();
		return;
	}

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


// Filter parameter input change
// ============================================================================
$(document).on('change', '.form-group[data-param-name]', $.debounce(250, function(event) {
	updateFromEvent(event.target);
}));

function updateFromEvent(target) {
	var $this = $(target);
	if ($this.hasClass('ac-checkbox')) {
		updateAnalogControls(ac_value);
	} else {
		var $stage = $this.parents('.stage-row');
		var $filter = $this.parents('.filter');
		var $widget = $this.parent();
		var paramName = $widget.data('param-name');
		var filter = filters[$filter.data('filter-index')];

		if(paramName === 'mix') {
			packet = FilterMixPacket(serialStream);
			packet.send($stage.index(), $filter.index(), parseFloat($this.val()));
		} else if(! $this.hasClass('checkbox')) {
			var param = filter.params[paramName];

			packet = FilterModPacket(serialStream);
			packet.send($stage.index(), $filter.index(), param['o'], param['f'], $this.val());
		}

		$this.siblings('label').children('.value').text($this.val());
	}
}


/* Tom individual */
function updateAnalogControls(new_value) {
	ac_value = new_value;
	new_value = new_value / 100;
	$('.ac-checkbox:checked').each(function(entry) {
		var element = $('.ac-checkbox:checked')[entry];
		var name = element.name;
		var control_element = $('[name="'+(name.replace('-ac', ''))+'"]');
		var min = control_element.attr('min');
		var max = control_element.attr('max');
		new_value = min + (max-min)*new_value;
		control_element.val(new_value);
		var form_element = control_element.parent();
		// setTimeout(function(){updateFromEvent(form_element)}, 3);
	});
}
/* End Tom individual */

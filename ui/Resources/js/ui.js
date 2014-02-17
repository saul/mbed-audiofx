$(function() {
	$('#reset-board').click(function() {
		// Send a reset packet to the board
		var packet = ResetPacket(serialStream);
		packet.send();
	});

	$('#clear-console').click(function() {
		$('#console-text').html('');
	});

	// Show initial stage
	appendFilterStage();
});


function appendFilterStage(completed) {
	renderTemplateRemote('filter_stage.html', function(template) {
		var index = $('#filter-container').children('.filter-row').length;

		$('#filter-container').append(template({
			index: index,
		}));

		reindexStages();

		if(typeof completed !== 'undefined')
			completed(index);
	});
}


function appendFilterToStage(stageIdx, filter) {
	// TODO: send stage creation to board
	// TODO: send filter creation to board

	renderTemplateRemote('filter.html', function(template) {
		$('.filter-row[data-index=' + stageIdx + '] .filter-create').before(template({
			name: filter,
			parameters: [
				{
					name: 'Mix perc',
					fieldName: 'mix-perc',
					type: 'range',
					min: '0',
					max: '1',
					step: '0.01',
					value: '1',
				},
				{
					name: 'Delay',
					fieldName: 'delay',
					type: 'range',
					min: '0',
					max: '10000',
					step: '1',
					value: '100',
				},
				{
					name: 'Speed',
					fieldName: 'speed',
					type: 'range',
					min: '0',
					max: '1',
					step: '0.01',
					value: '1',
				},
				{
					name: 'Wave type',
					fieldName: 'wave-type',
					type: 'choice',
					choices: ['Sine', 'Sawtooth', 'Triangle'],
				},
			]
		}));
	});
}


function reindexStages() {
	$('#filter-container .filter-row').each(function(index) {
		$(this).attr('data-index', index);

		$('.filter-row[data-index=' + index + '] .filter-create button').popover('destroy').popover({
			html: true,
			placement: 'left',
			content: getCreationPopoverContent,
			container: '.filter-row[data-index=' + index + ']',
		});
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

	if($this.prop('checked')) {
		$filter.removeClass('panel-danger').addClass('panel-success');
	} else {
		$filter.removeClass('panel-success').addClass('panel-danger');
	}

	// TODO: send filter enable/disable
});


// Filter delete
// ============================================================================
$(document).on('click', '.filter .close', function() {
	var $this = $(this);
	var $filter = $this.parents('.filter');

	if(!confirm('Delete this filter?'))
		return;

	// TODO: send filter deletion

	// Is this the last filter in the stage?
	if($filter.siblings('.filter').length === 0) {
		$filter.parents('.filter-row').remove();
		reindexStages();
		return;
	}

	$filter.remove();
});


// Create filter button
// ============================================================================
$(document).on('change', 'select[name=filter-create]', function() {
	var $this = $(this);
	var filterName = $this.val();

	var $row = $this.parents('.filter-row');
	var index = $row.data('index');
	console.log('Creating filter for: ' + index);

	// Close popover
	var $button = $row.find('.filter-create button');
	$button.popover('hide');

	if(filterName === '-')
		return;

	// If there are no filters in this stage, create another row with a "+"
	if($row.find('.filter').length === 0) {
		appendFilterStage();
	}

	appendFilterToStage(index, filterName);
});

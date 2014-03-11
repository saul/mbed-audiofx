var readingThread = null;
var packetHandlers = {};


$(function() {
	// Send a probe packet to boot the board
	var packet = ProbePacket(serialStream);
	packet.send();

	// Start the packet read thread
	pollReadThread();
});


function pollReadThread() {
	// Inner poll function
	(function() {
		// If there is no reading thread to poll, create one
		if(readingThread == null) {
			readingThread = readPacket(serialStream);
			return;
		}

		// Get the result from the read thread. If we are still reading
		// packets, this returns null
		var packet = getThreadResult(readingThread);
		if(packet == null)
			return;

		// Start another read thread on next call
		readingThread = null;

		if(packet.type_ != PacketTypes.B2U_PRINT)
			console.log('Received packet: ' + packet.type_);

		// Call the packet handler if one exists
		if(packet.type_ in packetHandlers)
			packetHandlers[packet.type_](packet);
		else
			console.warn('Packet ' + packet.type_ + ' has no handler function!');
	})();

	// Poll thread again immediately (let the UI paint)
	setTimeout(pollReadThread, 0);
}


// A2A_PROBE
// ============================================================================
packetHandlers[PacketTypes.A2A_PROBE] = function(packet) {
	onBoardReset();
};


// B2U_PRINT
// ============================================================================
packetHandlers[PacketTypes.B2U_PRINT] = function(packet) {
	var msg = packet.msg;

	// Clear console
	var parts = msg.split('\x1b[2J');

	if(parts.length > 1) {
		$('#console-text').html('<span class="text-muted">(console cleared)</span>\n');
		msg = parts[parts.length-1];
	}

	// Strip cursor movement and make HTML safe
	msg = msg.replace(/\x1b\[(\d+)?;?(\d+)?H/g, '')
			 .replace(/&/g, '&amp;')
		     .replace(/"/g, '&quot;')
		     .replace(/</g, '&lt;')
		     .replace(/>/g, '&gt;');

	$('#console-text').append(ansi_up.ansi_to_html(msg));
};


// B2U_FILTER_LIST
// ============================================================================
var creationPopoverContent;
var filters = null;

packetHandlers[PacketTypes.B2U_FILTER_LIST] = function(packet) {
	filters = _.toArray(packet.filters);

	for (var i = 0; i < filters.length; i++) {
		for(var paramKey in filters[i].params) {
			var param = filters[i].params[paramKey];

			if(!('c' in param))
				continue;

			param.c = _.toArray(param.c);
		}
	};

	// Re-render "create filter" popover content
	renderTemplateRemote('add_popover.html', function(template) {
		creationPopoverContent = template({filters: filters});
	});

	// Show filter container
	$('#filter-container').show();
};

// B2U_ANALOG_CONTROL (Tom individual)
// ============================================================================
packetHandlers[PacketTypes.B2U_ANALOG_CONTROL] = function(packet) {
	updateAnalogControls(packet.value / 4096);
}


// B2U_STORED_LIST (Saul individual)
// ============================================================================
packetHandlers[PacketTypes.B2U_STORED_LIST] = function(packet) {
	// todo!!!
}


// B2U_CHAIN_BLOB (Saul individual)
// ============================================================================
function syncChain(packet) {
	for (var i = 0; i < packet.stages.length; i++) {
		var stageIdx = i;
		var stage = packet.stages[i];

		for (var j = 0; j < stage.length; j++) {
			var branchIdx = j;
			var branch = stage[j];
			var defaultFilter = filters[branch.filter];
			var filledFilter = {};
			filledFilter.index = branch.filter;
			filledFilter.name = defaultFilter.name;
			filledFilter.slug = defaultFilter.slug;
			filledFilter.params = [];

			for (var k = 0; k < branch.params.length; k++) {
				var param = branch.params[k];
				var filledParam = _.clone(defaultFilter.params[param.name]);
				filledParam.val = param.value;
				filledFilter.params.push(filledParam);
			};

			renderTemplateRemote('filter.html', function(template) {
				$('.stage-row:nth-child(' + (stageIdx+1) + ') :nth-child(' + (branchIdx+1) + ')')[0].outerHTML = template({
					index: filledFilter.index,
					filter: filledFilter,
				});
			});
		};
	};
}

packetHandlers[PacketTypes.B2U_CHAIN_BLOB] = function(packet) {
	var stages = [];

	for (var i = 0; i < packet.stages.length; i++) {
		stages.push(_.toArray(packet.stages[i]));
	};

	// Create skeleton DOM structure of chain
	renderTemplateRemote('chain_sync.html', function(template) {
		$('#filter-container').html(template({
			stages: stages,
		}));
		syncChain(packet);
	});
}

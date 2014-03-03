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

	// Strip cursor movement
	msg = msg.replace(/\x1b\[(\d+)?;?(\d+)?H/g, '');

	$('#console-text').append(ansi_up.ansi_to_html(msg));
};


// B2U_FILTER_LIST
// ============================================================================
var creationPopoverContent;
var filters = null;

packetHandlers[PacketTypes.B2U_FILTER_LIST] = function(packet) {
	filters = _.toArray(packet.filters);

	// Re-render "create filter" popover content
	renderTemplateRemote('add_popover.html', function(template) {
		creationPopoverContent = template({filters: filters});
	});

	// Show filter container
	$('#filter-container').show();
};

/* Tom individual

packetHandlers[PacketTypes.B2U_ANALOG_CONTROL] = function(packet) {
	var msg = packet.msg;
	updateAnalogControls(msg);
}
End Tom individual */
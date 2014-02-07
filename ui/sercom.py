#!/usr/bin/env python
import serial
import struct
import time


def read_ascii_string(s, offset):
	"""Reads an ASCII string from `s` at offset `offset`.
	Returns a tuple of (str, new_offset), where `new_offset` is the index of
	character after the NULL byte.
	"""
	s = s[offset:]
	end = s.index(0)
	return s[:end].decode('ascii'), offset + end + 1


# little-endian "MBED"
PACKET_IDENT = ord('D') << 24 | ord('E') << 16 | ord('B') << 8 | ord('M')

# TODO: replace with enum
A2A_PROBE = 0
U2B_RESET = 1
B2U_PRINT = 2
B2U_FILTER_LIST = 3
U2B_FILTER_CHAIN = 4
U2B_FILTER_MOD = 5


class Packet:
	def __init__(self, stream):
		self.stream = stream

	def send(self, *args, **kwargs):
		"""Send this packet down the serial stream."""
		data = self.construct(*args, **kwargs)
		assert data is None or isinstance(data, bytes)
		self.stream.send_packet(self.type_, data)

	def construct(self):
		"""Construct the packet data."""
		raise NotImplementedError

	def receive(self, data):
		"""Called when a packet has been received."""
		raise NotImplementedError


class ProbePacket(Packet):
	type_ = A2A_PROBE

	def receive(self, data):
		# When we receive a probe packet, send one back
		# This tells the board to proceed with startup
		self.send()

	def construct(self):
		return None


class ResetPacket(Packet):
	type_ = U2B_RESET

	def construct(self):
		return None


class PrintPacket(Packet):
	type_ = B2U_PRINT

	def receive(self, data):
		chars = struct.unpack('<%dc' % len(data), data)
		print(b''.join(chars).decode('ascii'), end='')


class FilterListPacket(Packet):
	type_ = B2U_FILTER_LIST

	def receive(self, data):
		HEADER_FORMAT = '<B'
		offset = 0
		num_filters = struct.unpack_from(HEADER_FORMAT, data, offset)[0]
		offset += struct.calcsize(HEADER_FORMAT)

		for i in range(num_filters):
			name, offset = read_ascii_string(data, offset)
			desc, offset = read_ascii_string(data, offset)
			param_format, offset = read_ascii_string(data, offset)

			print('Filter: %s, %s, %s' % (name, desc, param_format))


# TODO: ordering doesn't matter
PACKET_MAP = [
	ProbePacket, # B2U_PROBE
	ResetPacket, # U2B_RESET
	PrintPacket, # B2U_PRINT
	FilterListPacket, # B2U_FILTER_LIST
	#FilterChainPacket, # U2B_FILTER_CHAIN
	#FilterModPacket, # U2B_FILTER_MOD
]


class SerialStream:
	def __init__(self, baudrate=9600):
		self.serial = serial.Serial('/dev/ttyACM0', baudrate)

		# Flush input
		print('Waiting to flush serial buffer...')
		time.sleep(1)
		self.serial.flushInput()

		# Send reset packet so board will reboot
		# If board is waiting for UI so it can start, the reset packet resumes
		# startup
		ResetPacket(self).send()

	def read_packet(self):
		"""Reads a packet from the serial port, doesn't return until a packet
		has been completely read.
		"""
		ident = self.serial.read(4)

		if ident != b'MBED':
			print('read_packet: invalid packet ident (%s)' % ident)
			return

		# Read packet type
		pack_type = struct.unpack('<B', self.serial.read(1))[0]
		if pack_type >= len(PACKET_MAP):
			print('read_packet: got unknown packet type (%d)' % pack_type)
			return

		# Read packet data
		size = struct.unpack('<B', self.serial.read(1))[0]

		if size == 0:
			data = None
		else:
			data = self.serial.read(size)

			#if b'\xff' in data:
			#	print('read_packet: found \\xFF byte in packet payload, discarding')
			#	return

		packet = PACKET_MAP[pack_type](self)

		# Debug packet receipt
		if packet.__class__ is not PrintPacket:
			print('read_packet: received packet %s (size=%d,data=%s)' % (packet.__class__.__qualname__, size, data))

		try:
			packet.receive(data)
		except:
			print('read_packet: exception occurred when parsing packet data:\n%r' % data)
			raise


	def send_packet(self, type_, data=None):
		# Calculate payload size
		size = len(data) if data is not None else 0

		# Write header
		header = struct.pack('<LBB', PACKET_IDENT, type_, size)
		self.serial.write(header)

		# Write packet data to serial
		if data is not None:
			self.serial.write(data)


if __name__ == '__main__':
	serstr = SerialStream()

	# Just read packets indefinitely
	while True:
		serstr.read_packet()

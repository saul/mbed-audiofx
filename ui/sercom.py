import serial
import struct


def read_ascii_string(s, offset):
	i = offset

	while s[i]:
		i += 1

	return s[offset:i].decode('ascii')


class Packet:
	def __init__(self, size):
		self.size = size

	def send(self):
		raise NotImplementedError

	def received(self):
		raise NotImplementedError


class PrintPacket(Packet):
	def receive(self, data):
		chars = struct.unpack('<%dc' % self.size, data)
		print(b''.join(chars).decode('ascii'), end='')


class FilterListPacket(Packet):
	def receive(self, data):
		HEADER_FORMAT = '<B'
		offset = 0
		num_filters = struct.unpack_from(HEADER_FORMAT, data, offset)[0]
		offset += struct.calcsize(HEADER_FORMAT)

		for i in range(num_filters):
			name = read_ascii_string(data, offset)
			offset += len(name) + 1

			desc = read_ascii_string(data, offset)
			offset += len(desc) + 1

			param_format = read_ascii_string(data, offset)
			offset += len(param_format) + 1

			print('Filter: %s, %s, %s' % (name, desc, param_format))


# This map must match the ordering in PacketType_e
PACKET_MAP = [
	PrintPacket, # B2U_PRINT
	FilterListPacket, # B2U_FILTER_LIST
	#FilterChainPacket, # U2B_FILTER_CHAIN
	#FilterModPacket, # U2B_FILTER_MOD
]


class SerialStream:
	def __init__(self, baudrate=9600):
		self.serial = serial.Serial('/dev/ttyACM0', baudrate)
		self.serial.flushInput()

		# Send Hello byte so board will boot
		self.serial.write(b'H')

	def read_packet(self):
		"""Reads a packet from the serial port, doesn't return until a packet
		has been completely read.
		"""
		ident = self.serial.read(4)

		if ident != b'MBED':
			print('read_packet got packet ident %s' % ident)
			return

		# Read packet type
		pack_type = struct.unpack('<B', self.serial.read(1))[0]
		if pack_type >= len(PACKET_MAP):
			print('read_packet got invalid packet type %d' % pack_type)
			return

		# Read packet data
		size = struct.unpack('<B', self.serial.read(1))[0]
		data = self.serial.read(size)

		packet = PACKET_MAP[pack_type](size)

		# Debug packet receipt
		if packet.__class__ is not PrintPacket:
			print(' * Received packet %s (size=%d,data=%s)' % (packet.__class__.__qualname__, size, data))

		packet.receive(data)


if __name__ == '__main__':
	serstr = SerialStream()

	# Just read packets indefinitely
	while True:
		serstr.read_packet()

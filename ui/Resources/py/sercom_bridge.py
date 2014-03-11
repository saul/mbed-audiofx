# Setup path
import os
import sys
print sys.path
sys.path.append(os.path.join(sys.path[-1], 'Resources', 'py'))

import threading
import Queue
import sercom


def init(window):
	"""Map everything from sercom to the JavaScript global scope."""
	for attr in sercom.__all__:
		print 'Mapping %s -> window' % attr
		setattr(window, attr, getattr(sercom, attr))

	window.serialStream = sercom.SerialStream()

	window.readPacket = read_packet
	window.getThreadResult = get_thread_result


def threaded(f):
    def wrapped_f(q, *args, **kwargs):
        """Call the decorated function and put the result in a queue."""
        ret = f(*args, **kwargs)
        q.put(ret)

    def wrap(*args, **kwargs):
        """Fire off wrapped_f in a new thread and returns the thread object
        with the result queue attached."""
        q = Queue.Queue()

        t = threading.Thread(target=wrapped_f, args=(q,)+args, kwargs=kwargs)
        t.start()
        t.result_queue = q
        return t

    return wrap


@threaded
def read_packet(stream):
	packet = stream.read_packet()
	return packet


def get_thread_result(thread):
	try:
		return thread.result_queue.get_nowait()
	except Queue.Empty:
		return None

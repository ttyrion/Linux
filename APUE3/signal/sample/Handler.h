#ifndef __APUE3_SIGNAL_SAMPLE_HANDLER_H
#define __APUE3_SIGNAL_SAMPLE_HANDLER_H

class Handler {
public:
	Handler();
	~Handler();
	static void OnSignal(int id);
};

#endif

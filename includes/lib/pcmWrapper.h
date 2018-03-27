#ifndef PCMWRAPPER_H
#define PCMWRAPPER_H

class PcmWrapper
{
	static int _papi_evt_set;
	static long long _papi_before;
	static long long _papi_after;
public:
	static void init();

	static void startRecord();

	static void stopRecord();

	static long long getInstructionsRetired();

};

#endif // PCMWRAPPER_H
